#include "DiffuseEngine.h"
#include <cmath>

DiffuseEngine::DiffuseEngine() = default;
DiffuseEngine::~DiffuseEngine() = default;

void DiffuseEngine::prepare(double sampleRate, int maxBlockSize) {
  currentSampleRate = sampleRate;

  stftProcessorL.prepare(sampleRate, maxBlockSize);
  stftProcessorR.prepare(sampleRate, maxBlockSize);

  // Pre-allocation sizing: maxFFTSize for bins, minFFTSize for hop (worst case
  // delay)
  constexpr int maxFFTSize = 4096;
  constexpr int minFFTSize = 64;
  constexpr int maxNumBins = maxFFTSize / 2 + 1;
  constexpr int overlapFactor = 4;
  int minHopSize =
      minFFTSize /
      overlapFactor; // 64/4 = 16 (smallest hop = most frames needed)

  // Store current working values
  currentNumBins = stftProcessorL.getNumBins();
  currentHopSize = stftProcessorL.getHopSize();
  currentFFTSize = stftProcessorL.getFFTSize();

  // Calculate maxDelaySamples based on minimum hop size (worst case = most
  // frames needed)
  float maxDelayMs = 2000.0f;
  float minHopSizeSec =
      static_cast<float>(minHopSize) / static_cast<float>(sampleRate);
  maxDelaySamples = static_cast<int>(maxDelayMs / 1000.0f / minHopSizeSec) + 1;

  // Pre-allocate delay buffers based on max bins and max delay (done once)
  size_t totalDelayBufferSize =
      static_cast<size_t>(maxNumBins) * static_cast<size_t>(maxDelaySamples);

  delayBufferL.assign(totalDelayBufferSize, std::complex<float>(0.0f, 0.0f));
  delayBufferR.assign(totalDelayBufferSize, std::complex<float>(0.0f, 0.0f));

  delayWritePosL.assign(static_cast<size_t>(maxNumBins), 0);
  delayWritePosR.assign(static_cast<size_t>(maxNumBins), 0);

  // Initialize dry delay buffers
  // Size must be larger than max latency (maxFFTSize) to avoid read/write
  // overlap When latency == buffer size, circular buffer fails (read pos ==
  // write pos)
  constexpr int dryDelayBufferSize = maxFFTSize * 2;
  dryDelayBufferL.setSize(1, dryDelayBufferSize);
  dryDelayBufferR.setSize(1, dryDelayBufferSize);
  dryDelayBufferL.clear();
  dryDelayBufferR.clear();
  dryDelayWritePos = 0;

  // Initialize previous spectrum buffers for feedback
  prevSpectrumL.assign(static_cast<size_t>(maxNumBins),
                       std::complex<float>(0.0f, 0.0f));
  prevSpectrumR.assign(static_cast<size_t>(maxNumBins),
                       std::complex<float>(0.0f, 0.0f));
}

void DiffuseEngine::reset() {
  stftProcessorL.reset();
  stftProcessorR.reset();

  // Clear delay buffers
  std::fill(delayBufferL.begin(), delayBufferL.end(),
            std::complex<float>(0.0f, 0.0f));
  std::fill(delayBufferR.begin(), delayBufferR.end(),
            std::complex<float>(0.0f, 0.0f));
  std::fill(delayWritePosL.begin(), delayWritePosL.end(), 0);
  std::fill(delayWritePosR.begin(), delayWritePosR.end(), 0);

  dryDelayBufferL.clear();
  dryDelayBufferR.clear();
  dryDelayWritePos = 0;

  // Clear feedback spectrum buffers
  std::fill(prevSpectrumL.begin(), prevSpectrumL.end(),
            std::complex<float>(0.0f, 0.0f));
  std::fill(prevSpectrumR.begin(), prevSpectrumR.end(),
            std::complex<float>(0.0f, 0.0f));
}

void DiffuseEngine::process(juce::AudioBuffer<float> &buffer) {
  int numSamples = buffer.getNumSamples();
  int numChannels = buffer.getNumChannels();

  if (numChannels == 0 || numSamples == 0)
    return;

  float mixAmount = mix.load();
  int latency = getLatencyInSamples();

  // Save dry signal for mixing (MUST be delayed to match wet signal latency)
  // We process this ALWAYS to keep the dry delay line and write pointers in
  // sync
  juce::AudioBuffer<float> dryBuffer(numChannels, numSamples);

  for (int ch = 0; ch < numChannels; ++ch) {
    auto *in = buffer.getReadPointer(ch);
    auto *outDelayed = dryBuffer.getWritePointer(ch);
    auto *delayData = (ch == 0) ? dryDelayBufferL.getWritePointer(0)
                                : dryDelayBufferR.getWritePointer(0);
    int delaySize = dryDelayBufferL.getNumSamples();

    for (int i = 0; i < numSamples; ++i) {
      // Store current input in delay line
      int wp = (dryDelayWritePos + i) % delaySize;
      delayData[wp] = in[i];

      // Read delayed sample (circular buffer)
      // Latency is the offset: Read from (wp - latency)
      int rp = (wp - latency + delaySize) % delaySize;
      outDelayed[i] = delayData[rp];
    }
  }

  // Update write position for next block
  dryDelayWritePos =
      (dryDelayWritePos + numSamples) % dryDelayBufferL.getNumSamples();

  // Process audio modulation
  updateLFO(numSamples);

  // Process through STFT ALWAYS to keep feedback and overlap-add state
  // continuous Left/mono channel
  auto *leftIn = buffer.getReadPointer(0);
  auto *leftOut = buffer.getWritePointer(0);

  stftProcessorL.process(leftIn, leftOut, numSamples,
                         [this](std::complex<float> *spectrum, int numBins) {
                           applyDiffuse(spectrum, numBins, delayBufferL,
                                        delayWritePosL, true);
                         });

  // Right channel
  if (numChannels >= 2) {
    auto *rightIn = buffer.getReadPointer(1);
    auto *rightOut = buffer.getWritePointer(1);

    stftProcessorR.process(rightIn, rightOut, numSamples,
                           [this](std::complex<float> *spectrum, int numBins) {
                             applyDiffuse(spectrum, numBins, delayBufferR,
                                          delayWritePosR, false);
                           });
  }

  // Apply dry/wet mix and output gain
  float dryGain = 1.0f - mixAmount;
  float wetGain = mixAmount;
  float outG = outputGain.load();

  for (int ch = 0; ch < numChannels; ++ch) {
    auto *wet = buffer.getWritePointer(ch);
    auto *dry = dryBuffer.getReadPointer(ch);
    for (int i = 0; i < numSamples; ++i) {
      wet[i] = (dry[i] * dryGain + wet[i] * wetGain) * outG;
    }
  }
}

void DiffuseEngine::applyDiffuse(std::complex<float> *spectrum, int numBins,
                                 std::vector<std::complex<float>> &delayBuffer,
                                 std::vector<int> &writePos,
                                 bool isLeftChannel) {
  float sr = static_cast<float>(currentSampleRate.load());
  float hop = static_cast<float>(currentHopSize.load());
  int fftSize = currentFFTSize.load();

  if (sr <= 0.0f || hop <= 0.0f || fftSize <= 0)
    return;

  float hopTimeMs = (hop / sr) * 1000.0f;
  float diffuseMaxMs = diffuseAmountMs.load();
  float lowCut = lowCutFreq.load();
  float feedback = feedbackAmount.load();
  const float transitionWidth = 100.0f; // Hz for smooth transition

  // Validate buffer sizing
  if (delayBuffer.size() <
      static_cast<size_t>(numBins) * static_cast<size_t>(maxDelaySamples))
    return;

  // Get reference to appropriate prevSpectrum buffer
  auto &prevSpectrum = isLeftChannel ? prevSpectrumL : prevSpectrumR;

  for (int bin = 0; bin < numBins; ++bin) {
    float freq = binToFrequency(bin);

    // Apply frequency-domain feedback: mix with previous frame's spectrum
    // Only apply to frequencies above lowcut to prevent low-frequency aliasing
    if (feedback > 0.0f && bin < static_cast<int>(prevSpectrum.size()) &&
        freq >= lowCut) {
      spectrum[bin] += prevSpectrum[bin] * feedback;
    }

    float delayMs = 0.0f;

    // Add diffuse jitter (static hash-based, per bin/channel)
    if (diffuseMaxMs > 0.0f && bin > 0) {
      float freq = binToFrequency(bin);

      // Only apply above low cut frequency
      if (freq >= lowCut) {
        // Calculate smooth transition factor
        float factor = 1.0f;
        if (freq < lowCut + transitionWidth) {
          float t = (freq - lowCut) / transitionWidth;
          factor = t * t * (3.0f - 2.0f * t); // Smoothstep
        }

        // Integer hash function (Wang Hash) for chaotic per-bin distribution
        // Adjacent bins will have completely uncorrelated delay times
        auto intHash = [](uint32_t x) -> float {
          x = ((x >> 16) ^ x) * 0x45d9f3b;
          x = ((x >> 16) ^ x) * 0x45d9f3b;
          x = (x >> 16) ^ x;
          return static_cast<float>(x) / static_cast<float>(0xFFFFFFFF);
        };

        // Grain Mode logic: Group bins (1x, 4x, 16x)
        int gm = grainMode.load();
        int multiplier = 1;
        if (gm == 1)
          multiplier = 4;
        else if (gm == 2)
          multiplier = 16;

        int effectiveBin = (bin / multiplier) * multiplier;

        // Different seed offset for left/right channel stereo separation
        uint32_t seed = static_cast<uint32_t>(effectiveBin) +
                        (isLeftChannel ? 0x243F6A88u : 0x9E3779B9u);
        // Direct mapping: [0, D_max] with smooth transition near lowcut
        float hash = intHash(seed);

        // LFO Modulation: delayMs = hash * diffuseMaxMs * (1.0 + depth *
        // sin(phase + binOffset))
        float depth = modDepth.load() / 100.0f; // 0-1.0
        double phase = isLeftChannel ? lfoPhaseL : lfoPhaseR;
        float binOffset = static_cast<float>(effectiveBin) * 0.05f;
        float lfo = std::sin(static_cast<float>(phase) + binOffset);

        delayMs = hash * diffuseMaxMs * factor * (1.0f + depth * lfo);
      }
    }

    float delayFramesFloat = delayMs / hopTimeMs;
    int delayFramesInt = static_cast<int>(delayFramesFloat);
    float fractionalFrames =
        delayFramesFloat - static_cast<float>(delayFramesInt);

    // Clamp to buffer size
    if (delayFramesInt >= maxDelaySamples) {
      delayFramesInt = maxDelaySamples - 1;
      fractionalFrames = 0.0f;
    }

    // Get write position for this bin
    size_t binIdx = static_cast<size_t>(bin);
    int wp = writePos[binIdx];
    int bufferSize = maxDelaySamples;

    // Calculate base offset for this bin in the flattened buffer
    size_t binOffset = binIdx * static_cast<size_t>(maxDelaySamples);

    // Write current value to buffer
    delayBuffer[binOffset + static_cast<size_t>(wp)] = spectrum[bin];

    // Read delayed value (circular buffer)
    int readPos = (wp - delayFramesInt + bufferSize) % bufferSize;
    std::complex<float> delayedSample =
        delayBuffer[binOffset + static_cast<size_t>(readPos)];

    // HiRes: always apply phase rotation for fractional frame compensation
    if (fractionalFrames > 0.0001f) {
      float sr = static_cast<float>(currentSampleRate.load());
      int fftSize = currentFFTSize.load();
      float hop = static_cast<float>(currentHopSize.load());

      // Calculate frequency for this bin
      float binFreq =
          static_cast<float>(bin) * sr / static_cast<float>(fftSize);

      // Calculate time offset for fractional frames (in seconds)
      float fractionalDelaySec = fractionalFrames * hop / sr;

      // Phase offset: φ = -2π × f × Δt
      float phaseOffset =
          -2.0f * juce::MathConstants<float>::pi * binFreq * fractionalDelaySec;

      // Apply phase rotation
      std::complex<float> rotator(std::cos(phaseOffset), std::sin(phaseOffset));
      delayedSample *= rotator;
    }

    // Store UN-compensated spectrum for next frame's feedback
    // This prevents energy accumulation from decay compensation
    if (bin < static_cast<int>(prevSpectrum.size())) {
      // Apply decay WITHOUT compensation for feedback storage
      std::complex<float> feedbackSample = delayedSample;
      if (decayEnabled.load() && diffuseMaxMs > 0.0f && delayMs > 0.0f) {
        float decayGain = 1.0f - (delayMs / diffuseMaxMs);
        feedbackSample *= decayGain;
      }
      prevSpectrum[bin] = feedbackSample;
    }

    // Apply decay WITH compensation for output
    // Compensation: uniform hash distribution causes ~50% average loudness loss
    if (decayEnabled.load() && diffuseMaxMs > 0.0f && delayMs > 0.0f) {
      float decayGain = 1.0f - (delayMs / diffuseMaxMs);
      constexpr float decayCompensation = 2.0f;
      delayedSample *= decayGain * decayCompensation;
    }

    spectrum[bin] = delayedSample;

    // Advance write position
    writePos[binIdx] = (wp + 1) % bufferSize;
  }
}

float DiffuseEngine::binToFrequency(int binIndex) const {
  float fftSize = static_cast<float>(currentFFTSize.load());
  if (fftSize < 1.0f)
    return 0.0f;

  return static_cast<float>(binIndex) *
         static_cast<float>(currentSampleRate.load()) / fftSize;
}

void DiffuseEngine::setFFTSize(int size) {
  if (size == currentFFTSize.load())
    return;

  currentFFTSize = size;

  stftProcessorL.setFFTSize(size);
  stftProcessorR.setFFTSize(size);

  currentNumBins = stftProcessorL.getNumBins();
  currentHopSize = stftProcessorL.getHopSize();

  // Note: Buffers are already pre-allocated in prepare() based on max FFT size.
  // No memory allocation here - just clear the working portion of delay
  // buffers.
  int numBins = currentNumBins.load();
  for (int bin = 0; bin < numBins; ++bin) {
    size_t binOffset =
        static_cast<size_t>(bin) * static_cast<size_t>(maxDelaySamples);
    for (int i = 0; i < maxDelaySamples; ++i) {
      delayBufferL[binOffset + i] = std::complex<float>(0.0f, 0.0f);
      delayBufferR[binOffset + i] = std::complex<float>(0.0f, 0.0f);
    }
    delayWritePosL[bin] = 0;
    delayWritePosR[bin] = 0;
  }
}

int DiffuseEngine::getLatencyInSamples() const {
  return stftProcessorL.getLatencyInSamples();
}

void DiffuseEngine::updateLFO(int numSamples) {
  float rate = modRate.load();
  double sr = currentSampleRate.load();

  if (sr > 0.0) {
    double phaseInc =
        (2.0 * juce::MathConstants<double>::pi * rate * numSamples) / sr;
    lfoPhaseL =
        std::fmod(lfoPhaseL + phaseInc, 2.0 * juce::MathConstants<double>::pi);

    // Subtle phase offset for right channel to enhance stereo
    lfoPhaseR = std::fmod(lfoPhaseL + juce::MathConstants<double>::pi * 0.5,
                          2.0 * juce::MathConstants<double>::pi);
  }
}
