#pragma once

#include "PluginProcessor.h"
#include "ui/CustomLookAndFeel.h"
#include <juce_audio_processors/juce_audio_processors.h>

class CopycatDiffuseEditor : public juce::AudioProcessorEditor {
public:
  explicit CopycatDiffuseEditor(CopycatDiffuseProcessor &);
  ~CopycatDiffuseEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  CopycatDiffuseProcessor &processorRef;
  CustomLookAndFeel customLookAndFeel;

  // Title labels (two-tone: "COPYCAT" + "DIFFUSE")
  juce::Label brandLabel;
  juce::Label titleLabel;
  juce::Font titleFont;

  // Knobs
  juce::Slider diffuseSlider;
  juce::Label diffuseLabel;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      diffuseAttachment;

  juce::Slider lowcutSlider;
  juce::Label lowcutLabel;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      lowcutAttachment;

  // FFT Size Knob (3-level: Low/Medium/High)
  juce::Slider fftSizeSlider;
  juce::Label fftSizeLabel;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      fftSizeAttachment;

  // Decay Toggle Button (TextButton style like reference project)
  juce::TextButton decayButton{"Decay"};
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      decayAttachment;

  // Mix Knob
  juce::Slider mixSlider;
  juce::Label mixLabel;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      mixAttachment;

  // Gain Knob
  juce::Slider gainSlider;
  juce::Label gainLabel;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      gainAttachment;

  // Separator line position (calculated in resized)
  int separatorLineY = 0;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CopycatDiffuseEditor)
};
