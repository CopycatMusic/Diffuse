#pragma once

#include "STFTProcessor.h"
#include <atomic>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

/**
 * Simplified Diffuse Engine - applies frequency-dependent delay jitter
 * to create a spectral "smearing" or "blur" effect.
 */
class DiffuseEngine {
public:
  DiffuseEngine();
  ~DiffuseEngine();

  void prepare(double sampleRate, int maxBlockSize);
  void reset();
  void process(juce::AudioBuffer<float> &buffer);

  //==========================================================================
  // Parameters

  /**
   * Set diffuse amount in milliseconds (0-2000ms).
   */
  void setDiffuseAmount(float amountMs) { diffuseAmountMs = amountMs; }
  float getDiffuseAmount() const { return diffuseAmountMs.load(); }

  /**
   * Set low cut frequency in Hz (frequencies below this are not affected).
   */
  void setLowCut(float freq) { lowCutFreq = freq; }
  float getLowCut() const { return lowCutFreq.load(); }

  /**
   * Enable/disable decay effect (longer delay = lower volume).
   */
  void setDecayEnabled(bool enabled) { decayEnabled = enabled; }
  bool getDecayEnabled() const { return decayEnabled.load(); }

  /**
   * Set dry/wet mix (0.0 = fully dry, 1.0 = fully wet).
   */
  void setMix(float mixAmount) { mix = juce::jlimit(0.0f, 1.0f, mixAmount); }
  float getMix() const { return mix.load(); }

  /**
   * Set output gain in decibels.
   */
  void setGain(float decibels) {
    outputGain = juce::Decibels::decibelsToGain(decibels);
  }
  float getGain() const {
    return juce::Decibels::gainToDecibels(outputGain.load());
  }

  /**
   * Set frequency-domain feedback amount (0.0-0.95).
   * Mixes previous frame's processed spectrum with current input.
   */
  void setFeedback(float amount) {
    feedbackAmount = juce::jlimit(0.0f, 0.95f, amount);
  }
  float getFeedback() const { return feedbackAmount.load(); }

  /**
   * Set FFT size (1024, 2048, 4096, 8192, 16384).
   */
  void setFFTSize(int size);
  int getFFTSize() const { return currentFFTSize.load(); }

  /**
   * Set LFO modulation rate in Hz (0.1-10Hz).
   */
  void setModRate(float rate) { modRate = rate; }
  float getModRate() const { return modRate.load(); }

  /**
   * Set LFO modulation depth as percentage (0-100%).
   */
  void setModDepth(float depth) { modDepth = depth; }
  float getModDepth() const { return modDepth.load(); }

  //==========================================================================
  // Info

  int getNumBins() const { return currentNumBins.load(); }
  int getLatencyInSamples() const;
  double getSampleRate() const { return currentSampleRate.load(); }

private:
  void applyDiffuse(std::complex<float> *spectrum, int numBins,
                    std::vector<std::complex<float>> &delayBuffer,
                    std::vector<int> &writePos, bool isLeftChannel);
  float binToFrequency(int binIndex) const;

  // STFT processors for each channel
  STFTProcessor stftProcessorL;
  STFTProcessor stftProcessorR;

  // Diffuse parameters
  std::atomic<float> diffuseAmountMs{0.0f}; // 0-2000ms
  std::atomic<float> lowCutFreq{400.0f};    // Hz
  std::atomic<bool> decayEnabled{false};    // Decay toggle
  std::atomic<float> mix{1.0f};             // 0.0 = dry, 1.0 = wet
  std::atomic<float> outputGain{1.0f};      // Multiplier (default 0dB = 1.0)
  std::atomic<float> feedbackAmount{0.0f};  // 0.0-0.95 feedback mix

private:
  void updateLFO(int numSamples);

  // Per-bin delay buffers (flattened for cache locality)
  std::vector<std::complex<float>> delayBufferL;
  std::vector<std::complex<float>> delayBufferR;
  std::vector<int> delayWritePosL;
  std::vector<int> delayWritePosR;
  int maxDelaySamples = 0;

  // LFO state
  double lfoPhaseL = 0.0;
  double lfoPhaseR = 0.0;
  std::atomic<float> modRate{1.0f};  // Hz
  std::atomic<float> modDepth{0.0f}; // Percentage 0-100

  // Previous frame spectrum for frequency-domain feedback
  std::vector<std::complex<float>> prevSpectrumL;
  std::vector<std::complex<float>> prevSpectrumR;

  // Dry signal delay buffers (for alignment with STFT latency)
  juce::AudioBuffer<float> dryDelayBufferL;
  juce::AudioBuffer<float> dryDelayBufferR;
  int dryDelayWritePos = 0;

  // Current settings
  std::atomic<double> currentSampleRate{44100.0};
  std::atomic<int> currentFFTSize{2048};
  std::atomic<int> currentOverlapFactor{4};
  std::atomic<int> currentHopSize{512};
  std::atomic<int> currentNumBins{1025};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiffuseEngine)
};
