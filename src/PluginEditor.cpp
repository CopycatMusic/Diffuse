#include "PluginEditor.h"
#include "BinaryData.h"
#include "ui/UIConstants.h"

CopycatDiffuseEditor::CopycatDiffuseEditor(CopycatDiffuseProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p) {
  setLookAndFeel(&customLookAndFeel);

  // Load custom font for title
  auto typefacePtr = juce::Typeface::createSystemTypefaceFor(
      BinaryData::MontserratBold_ttf, BinaryData::MontserratBold_ttfSize);
  titleFont = juce::Font(
      juce::FontOptions(typefacePtr).withHeight(UIConstants::kTitleFontSize));

  // Title Label
  titleLabel.setText("CopyCat Diffuse", juce::dontSendNotification);
  titleLabel.setFont(titleFont);
  titleLabel.setJustificationType(juce::Justification::centred);
  titleLabel.setColour(juce::Label::textColourId,
                       CustomLookAndFeel::getAccentPrimary());
  addAndMakeVisible(titleLabel);

  // Diffuse Knob
  diffuseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  diffuseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
  diffuseSlider.setMouseDragSensitivity(200);
  addAndMakeVisible(diffuseSlider);
  diffuseAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "diffuse", diffuseSlider);

  diffuseLabel.setText("Diffuse", juce::dontSendNotification);
  diffuseLabel.setJustificationType(juce::Justification::centred);
  diffuseLabel.setColour(juce::Label::textColourId,
                         CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(diffuseLabel);

  // Low Cut Knob
  lowcutSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lowcutSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
  lowcutSlider.setMouseDragSensitivity(200);
  addAndMakeVisible(lowcutSlider);
  lowcutAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "lowcut", lowcutSlider);

  lowcutLabel.setText("Low Cut", juce::dontSendNotification);
  lowcutLabel.setJustificationType(juce::Justification::centred);
  lowcutLabel.setColour(juce::Label::textColourId,
                        CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(lowcutLabel);

  // FFT Size Knob (3-level: Low/Medium/High = 512/2048/8192)
  fftSizeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  fftSizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
  fftSizeSlider.setMouseDragSensitivity(80);
  addAndMakeVisible(fftSizeSlider);
  fftSizeAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "fftsize", fftSizeSlider);

  fftSizeLabel.setText("Resolution", juce::dontSendNotification);
  fftSizeLabel.setJustificationType(juce::Justification::centred);
  fftSizeLabel.setColour(juce::Label::textColourId,
                         CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(fftSizeLabel);

  // Decay Toggle Button
  decayButton.setButtonText("Decay");
  decayButton.setColour(juce::ToggleButton::textColourId,
                        CustomLookAndFeel::getTextPrimary());
  decayButton.setColour(juce::ToggleButton::tickColourId,
                        CustomLookAndFeel::getAccentPrimary());
  addAndMakeVisible(decayButton);
  decayAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          processorRef.getParameters(), "decay", decayButton);

  // Mix Knob
  mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
  mixSlider.setMouseDragSensitivity(150);
  addAndMakeVisible(mixSlider);
  mixAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "mix", mixSlider);

  mixLabel.setText("Mix", juce::dontSendNotification);
  mixLabel.setJustificationType(juce::Justification::centred);
  mixLabel.setColour(juce::Label::textColourId,
                     CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(mixLabel);

  setSize(UIConstants::kDefaultWindowWidth, UIConstants::kDefaultWindowHeight);
}

CopycatDiffuseEditor::~CopycatDiffuseEditor() { setLookAndFeel(nullptr); }

void CopycatDiffuseEditor::paint(juce::Graphics &g) {
  g.fillAll(CustomLookAndFeel::getBackgroundDark());
}

void CopycatDiffuseEditor::resized() {
  auto bounds = getLocalBounds();
  const int margin = UIConstants::kMargin;
  const int spacing = UIConstants::kSpacing;
  const int labelHeight = UIConstants::kLabelHeight;
  const int knobSize = UIConstants::kLargeKnobSize;

  // Title at the top
  auto titleArea = bounds.removeFromTop(45);
  titleLabel.setBounds(titleArea);

  bounds.removeFromTop(spacing / 2);

  // Center content area - 4 knobs in a row
  auto contentBounds = bounds.reduced(margin, 0);

  // Calculate positions for 4 knobs
  int knobSpacing = spacing;
  int totalKnobsWidth = knobSize * 4 + knobSpacing * 5;
  int startX = contentBounds.getCentreX() - totalKnobsWidth / 2;

  // Diffuse knob
  int diffuseX = startX;
  diffuseLabel.setBounds(diffuseX, contentBounds.getY(),
                         knobSize + knobSpacing * 2, labelHeight);
  diffuseSlider.setBounds(diffuseX + knobSpacing,
                          contentBounds.getY() + labelHeight, knobSize,
                          knobSize + 18);

  // Low Cut knob
  int lowcutX = startX + knobSize + knobSpacing;
  lowcutLabel.setBounds(lowcutX, contentBounds.getY(),
                        knobSize + knobSpacing * 2, labelHeight);
  lowcutSlider.setBounds(lowcutX + knobSpacing,
                         contentBounds.getY() + labelHeight, knobSize,
                         knobSize + 18);

  // FFT Size knob
  int fftX = startX + (knobSize + knobSpacing) * 2;
  fftSizeLabel.setBounds(fftX, contentBounds.getY(), knobSize + knobSpacing * 2,
                         labelHeight);
  fftSizeSlider.setBounds(fftX + knobSpacing,
                          contentBounds.getY() + labelHeight, knobSize,
                          knobSize + 18);

  // Mix knob
  int mixX = startX + (knobSize + knobSpacing) * 3;
  mixLabel.setBounds(mixX, contentBounds.getY(), knobSize + knobSpacing * 2,
                     labelHeight);
  mixSlider.setBounds(mixX + knobSpacing, contentBounds.getY() + labelHeight,
                      knobSize, knobSize + 18);

  // Decay button (below the knobs, centered)
  int buttonWidth = 80;
  int buttonHeight = 24;
  int buttonY = contentBounds.getY() + labelHeight + knobSize + 28;
  decayButton.setBounds(contentBounds.getCentreX() - buttonWidth / 2, buttonY,
                        buttonWidth, buttonHeight);
}
