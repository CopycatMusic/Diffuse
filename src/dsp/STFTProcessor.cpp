#include "STFTProcessor.h"
#include <cmath>
#include <cstring>

STFTProcessor::STFTProcessor() { initBuffers(); }

void STFTProcessor::prepare(double sr, int /*maxBlockSize*/) {
  sampleRate = sr;
  // Buffers are already initialized in constructor
  prepared = true;
}

void STFTProcessor::reset() {
  count = 0;
  pos = 0;

  // Zero out the circular buffers
  std::fill(inputFifo.begin(), inputFifo.end(), 0.0f);
  std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
  std::fill(fftData.begin(), fftData.end(), 0.0f);
  std::fill(magnitudeSpectrum.begin(), magnitudeSpectrum.end(), 0.0f);
}

void STFTProcessor::setFFTSize(int size) {
  jassert(size >= 256 && size <= maxFftSize);
  jassert((size & (size - 1)) == 0); // Must be power of 2

  if (size != fftSize) {
    fftSize = size;
    fftOrder = static_cast<int>(std::log2(fftSize));
    numBins = fftSize / 2 + 1;
    hopSize = fftSize / overlapFactor;

    // Select pre-allocated FFT object based on size
    switch (fftSize) {
    case 256:
      fft = fft256;
      window = window256;
      break;
    case 512:
      fft = fft512;
      window = window512;
      break;
    case 1024:
      fft = fft1024;
      window = window1024;
      break;
    case 2048:
      fft = fft2048;
      window = window2048;
      break;
    case 4096:
    default:
      fft = fft4096;
      window = window4096;
      break;
    }

    // Reset buffer positions (no memory allocation)
    count = 0;
    pos = 0;

    // Zero out buffers to avoid artifacts
    std::fill(inputFifo.begin(), inputFifo.begin() + fftSize, 0.0f);
    std::fill(outputFifo.begin(), outputFifo.begin() + fftSize, 0.0f);
    std::fill(fftData.begin(), fftData.begin() + fftSize * 2, 0.0f);
    std::fill(magnitudeSpectrum.begin(), magnitudeSpectrum.begin() + numBins,
              0.0f);
  }
}

void STFTProcessor::setOverlapFactor(int factor) {
  jassert(factor == 2 || factor == 4 || factor == 8);

  if (factor != overlapFactor) {
    overlapFactor = factor;
    hopSize = fftSize / overlapFactor;

    // Update window correction based on overlap
    // For Hann window: 2x overlap -> 1.0, 4x overlap -> 2/3, 8x overlap -> 1/3
    if (overlapFactor == 2) {
      windowCorrection = 1.0f;
    } else if (overlapFactor == 4) {
      windowCorrection = 2.0f / 3.0f;
    } else {
      windowCorrection = 1.0f / 3.0f;
    }

    initBuffers();
  }
}

void STFTProcessor::initBuffers() {
  // Pre-allocate FFT objects for all supported sizes (done once)
  fft256Owner = std::make_unique<juce::dsp::FFT>(8);   // 2^8 = 256
  fft512Owner = std::make_unique<juce::dsp::FFT>(9);   // 2^9 = 512
  fft1024Owner = std::make_unique<juce::dsp::FFT>(10); // 2^10 = 1024
  fft2048Owner = std::make_unique<juce::dsp::FFT>(11); // 2^11 = 2048
  fft4096Owner = std::make_unique<juce::dsp::FFT>(12); // 2^12 = 4096

  fft256 = fft256Owner.get();
  fft512 = fft512Owner.get();
  fft1024 = fft1024Owner.get();
  fft2048 = fft2048Owner.get();
  fft4096 = fft4096Owner.get();

  // Pre-allocate window functions for all supported sizes
  window256Owner = std::make_unique<juce::dsp::WindowingFunction<float>>(
      257, juce::dsp::WindowingFunction<float>::hann, false);
  window512Owner = std::make_unique<juce::dsp::WindowingFunction<float>>(
      513, juce::dsp::WindowingFunction<float>::hann, false);
  window1024Owner = std::make_unique<juce::dsp::WindowingFunction<float>>(
      1025, juce::dsp::WindowingFunction<float>::hann, false);
  window2048Owner = std::make_unique<juce::dsp::WindowingFunction<float>>(
      2049, juce::dsp::WindowingFunction<float>::hann, false);
  window4096Owner = std::make_unique<juce::dsp::WindowingFunction<float>>(
      4097, juce::dsp::WindowingFunction<float>::hann, false);

  window256 = window256Owner.get();
  window512 = window512Owner.get();
  window1024 = window1024Owner.get();
  window2048 = window2048Owner.get();
  window4096 = window4096Owner.get();

  // Set current pointers based on default fftSize
  fft = fft2048;
  window = window2048;

  // Pre-allocate buffers based on maxFftSize (no runtime allocation)
  int maxNumBins = maxFftSize / 2 + 1;
  inputFifo.assign(maxFftSize, 0.0f);
  outputFifo.assign(maxFftSize, 0.0f);
  fftData.assign(maxFftSize * 2, 0.0f);
  magnitudeSpectrum.assign(maxNumBins, 0.0f);

  count = 0;
  pos = 0;
  prepared = true;
}

void STFTProcessor::process(
    const float *input, float *output, int numSamples,
    std::function<void(std::complex<float> *, int numBins)> spectralProcess) {

  if (!prepared || !fft) {
    std::memcpy(output, input, numSamples * sizeof(float));
    return;
  }

  for (int i = 0; i < numSamples; ++i) {
    // Push the new sample value into the input FIFO
    inputFifo[pos] = input[i];

    // Read the output value from the output FIFO. Since it takes fftSize
    // timesteps before actual samples are read from this FIFO instead of
    // the initial zeros, the sound output is delayed by fftSize samples.
    float outputSample = outputFifo[pos];

    // Once we've read the sample, set this position in the FIFO back to
    // zero so we can add the IFFT results to it later.
    outputFifo[pos] = 0.0f;

    // Advance the FIFO index and wrap around if necessary
    pos = (pos + 1) % fftSize;

    // Process the FFT frame once we've collected hopSize samples
    count++;
    if (count == hopSize) {
      count = 0;
      processFrame(spectralProcess);
    }

    output[i] = outputSample;
  }
}

void STFTProcessor::processFrame(
    std::function<void(std::complex<float> *, int)> &callback) {

  float *inputPtr = inputFifo.data();
  float *fftPtr = fftData.data();

  // Copy the input FIFO into the FFT working space in two parts.
  // This handles the circular buffer wrap-around.
  std::memcpy(fftPtr, inputPtr + pos, (fftSize - pos) * sizeof(float));
  if (pos > 0) {
    std::memcpy(fftPtr + fftSize - pos, inputPtr, pos * sizeof(float));
  }

  // Apply the analysis window to avoid spectral leakage
  window->multiplyWithWindowingTable(fftPtr, fftSize);

  // Perform the forward FFT
  fft->performRealOnlyForwardTransform(fftPtr, true);

  // Get complex spectrum pointer
  auto *spectrum = reinterpret_cast<std::complex<float> *>(fftPtr);

  // Apply spectral processing callback
  if (callback) {
    callback(spectrum, numBins);
  }

  // Update magnitude spectrum for visualization AFTER processing
  // This shows the actual output spectrum
  // Using 4/N for AC and 2/N for DC/Nyquist to compensate for:
  // 1. Hann window loss (0.5)
  // 2. Real FFT energy split (0.5 for AC, no split for DC/Nyquist)
  float acNormFactor = 4.0f / static_cast<float>(fftSize);
  float dcNormFactor = 2.0f / static_cast<float>(fftSize);

  // DC bin (real part of spectrum[0])
  magnitudeSpectrum[0] = std::abs(spectrum[0].real()) * dcNormFactor;

  // Intermediate bins (Bin 1 to numBins - 2)
  for (int i = 1; i < numBins - 1; ++i) {
    magnitudeSpectrum[i] = std::abs(spectrum[i]) * acNormFactor;
  }

  // Nyquist bin (imaginary part of spectrum[0])
  magnitudeSpectrum[numBins - 1] = std::abs(spectrum[0].imag()) * dcNormFactor;

  // Perform the inverse FFT
  fft->performRealOnlyInverseTransform(fftPtr);

  // Apply the synthesis window for resynthesis
  window->multiplyWithWindowingTable(fftPtr, fftSize);

  // Scale down the output samples because of the overlapping windows
  for (int i = 0; i < fftSize; ++i) {
    fftPtr[i] *= windowCorrection;
  }

  // Add the IFFT results to the output FIFO in two parts.
  // This handles the circular buffer wrap-around correctly.
  for (int i = 0; i < pos; ++i) {
    outputFifo[i] += fftData[i + fftSize - pos];
  }
  for (int i = 0; i < fftSize - pos; ++i) {
    outputFifo[i + pos] += fftData[i];
  }
}
