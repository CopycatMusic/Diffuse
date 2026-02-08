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
  constexpr int maxFFTSize = 2048;
  constexpr int minFFTSize = 512;
  constexpr int maxNumBins = maxFFTSize / 2 + 1;
  constexpr int overlapFactor = 4;
  int minHopSize =
      minFFTSize /
      overlapFactor; // 512/4 = 128 (smallest hop = most frames needed)

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
}

void DiffuseEngine::process(juce::AudioBuffer<float> &buffer) {
  int numSamples = buffer.getNumSamples();
  int numChannels = buffer.getNumChannels();

  if (numChannels == 0 || numSamples == 0)
    return;

  float mixAmount = mix.load();

  // If fully dry, skip processing entirely
  if (mixAmount <= 0.0f)
    return;

  // Save dry signal for mixing
  juce::AudioBuffer<float> dryBuffer;
  if (mixAmount < 1.0f) {
    dryBuffer.makeCopyOf(buffer);
  }

  // Process left/mono channel through STFT
  auto *leftIn = buffer.getReadPointer(0);
  auto *leftOut = buffer.getWritePointer(0);

  stftProcessorL.process(leftIn, leftOut, numSamples,
                         [this](std::complex<float> *spectrum, int numBins) {
                           applyDiffuse(spectrum, numBins, delayBufferL,
                                        delayWritePosL, true);
                         });

  // Process right channel if stereo
  if (numChannels >= 2) {
    auto *rightIn = buffer.getReadPointer(1);
    auto *rightOut = buffer.getWritePointer(1);

    stftProcessorR.process(rightIn, rightOut, numSamples,
                           [this](std::complex<float> *spectrum, int numBins) {
                             applyDiffuse(spectrum, numBins, delayBufferR,
                                          delayWritePosR, false);
                           });
  }

  // Apply dry/wet mix if not fully wet
  if (mixAmount < 1.0f) {
    float dryGain = 1.0f - mixAmount;
    float wetGain = mixAmount;

    for (int ch = 0; ch < numChannels; ++ch) {
      auto *wet = buffer.getWritePointer(ch);
      auto *dry = dryBuffer.getReadPointer(ch);
      for (int i = 0; i < numSamples; ++i) {
        wet[i] = dry[i] * dryGain + wet[i] * wetGain;
      }
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
  const float transitionWidth = 100.0f; // Hz for smooth transition

  // Validate buffer sizing
  if (delayBuffer.size() <
      static_cast<size_t>(numBins) * static_cast<size_t>(maxDelaySamples))
    return;

  for (int bin = 0; bin < numBins; ++bin) {
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

        // Static hash: use golden ratio for pseudo-random distribution
        // Different seed for left/right channel
        const float gr = 0.618033988749895f;
        int channelSeed = isLeftChannel ? 0 : 7919; // Prime offset
        float hash =
            std::fmod(static_cast<float>(bin + channelSeed) * gr, 1.0f);

        // Direct mapping: [0, D_max] - no negative values, full distribution
        delayMs = hash * diffuseMaxMs * factor;
      }
    }

    float delayFramesFloat = delayMs / hopTimeMs;
    int delayFramesInt = static_cast<int>(delayFramesFloat);

    // Clamp to buffer size
    if (delayFramesInt >= maxDelaySamples) {
      delayFramesInt = maxDelaySamples - 1;
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

    // Apply decay: longer delay = lower volume (linear attenuation)
    if (decayEnabled.load() && diffuseMaxMs > 0.0f && delayMs > 0.0f) {
      float decayGain = 1.0f - (delayMs / diffuseMaxMs);
      delayedSample *= decayGain;
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
