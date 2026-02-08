#pragma once

#include <array>
#include <complex>
#include <functional>
#include <juce_dsp/juce_dsp.h>

/**
 * STFT processor with overlap-add implementation.
 * Based on hollance/fft-juce reference implementation.
 */
class STFTProcessor {
public:
  STFTProcessor();
  ~STFTProcessor() = default;

  void prepare(double sampleRate, int maxBlockSize);
  void reset();

  void setFFTSize(int size);
  void setOverlapFactor(int factor);

  int getFFTSize() const { return fftSize; }
  int getNumBins() const { return fftSize / 2 + 1; }
  int getHopSize() const { return hopSize; }
  int getLatencyInSamples() const { return fftSize; }

  /**
   * Process audio: input -> STFT -> spectral callback -> ISTFT -> output
   */
  void process(
      const float *input, float *output, int numSamples,
      std::function<void(std::complex<float> *, int numBins)> spectralProcess);

  const float *getMagnitudeSpectrum() const { return magnitudeSpectrum.data(); }

private:
  void initBuffers();
  void processFrame(std::function<void(std::complex<float> *, int)> &callback);

  // FFT configuration
  static constexpr int defaultFftOrder = 11; // 2048 samples
  static constexpr int maxFftSize = 4096;

  int fftSize = 1 << defaultFftOrder; // 2048
  int fftOrder = defaultFftOrder;
  int numBins = fftSize / 2 + 1;
  int overlapFactor = 4; // 75% overlap
  int hopSize = fftSize / overlapFactor;

  // Gain correction for using Hann window with 75% overlap
  float windowCorrection = 2.0f / 3.0f;

  // Current FFT and window (raw pointers to pre-allocated objects)
  juce::dsp::FFT *fft = nullptr;
  juce::dsp::WindowingFunction<float> *window = nullptr;

  // Pre-allocated FFT objects for each supported size (owners)
  std::unique_ptr<juce::dsp::FFT> fft256Owner;
  std::unique_ptr<juce::dsp::FFT> fft512Owner;
  std::unique_ptr<juce::dsp::FFT> fft1024Owner;
  std::unique_ptr<juce::dsp::FFT> fft2048Owner;
  std::unique_ptr<juce::dsp::FFT> fft4096Owner;

  // Pre-allocated window functions for each supported size (owners)
  std::unique_ptr<juce::dsp::WindowingFunction<float>> window256Owner;
  std::unique_ptr<juce::dsp::WindowingFunction<float>> window512Owner;
  std::unique_ptr<juce::dsp::WindowingFunction<float>> window1024Owner;
  std::unique_ptr<juce::dsp::WindowingFunction<float>> window2048Owner;
  std::unique_ptr<juce::dsp::WindowingFunction<float>> window4096Owner;

  // Raw pointers for quick access
  juce::dsp::FFT *fft256 = nullptr;
  juce::dsp::FFT *fft512 = nullptr;
  juce::dsp::FFT *fft1024 = nullptr;
  juce::dsp::FFT *fft2048 = nullptr;
  juce::dsp::FFT *fft4096 = nullptr;
  juce::dsp::WindowingFunction<float> *window256 = nullptr;
  juce::dsp::WindowingFunction<float> *window512 = nullptr;
  juce::dsp::WindowingFunction<float> *window1024 = nullptr;
  juce::dsp::WindowingFunction<float> *window2048 = nullptr;
  juce::dsp::WindowingFunction<float> *window4096 = nullptr;

  // Counts up until the next hop
  int count = 0;

  // Read/write position in the circular buffers
  int pos = 0;

  // Circular buffers for incoming and outgoing audio data
  std::vector<float> inputFifo;
  std::vector<float> outputFifo;

  // The FFT working space. Contains interleaved complex numbers.
  std::vector<float> fftData;

  // Magnitude spectrum for visualization
  std::vector<float> magnitudeSpectrum;

  double sampleRate = 44100.0;
  bool prepared = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(STFTProcessor)
};
