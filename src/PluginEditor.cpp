#include "PluginEditor.h"
#include "BinaryData.h"
#include "ui/UIConstants.h"

CopycatDiffuseEditor::CopycatDiffuseEditor(CopycatDiffuseProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p) {
  setLookAndFeel(&customLookAndFeel);

  // Load embedded Montserrat Bold font
  auto montserratTypeface = juce::Typeface::createSystemTypefaceFor(
      BinaryData::MontserratBold_ttf, BinaryData::MontserratBold_ttfSize);
  titleFont =
      juce::Font(montserratTypeface).withHeight(UIConstants::kTitleFontSize);

  // ===== Title Bar: Two-tone labels =====
  brandLabel.setText("COPYCAT", juce::dontSendNotification);
  brandLabel.setFont(titleFont);
  brandLabel.setColour(juce::Label::textColourId,
                       CustomLookAndFeel::getTextPrimary());
  brandLabel.setJustificationType(juce::Justification::centredRight);
  addAndMakeVisible(brandLabel);

  titleLabel.setText("DIFFUSE", juce::dontSendNotification);
  titleLabel.setFont(titleFont);
  titleLabel.setColour(juce::Label::textColourId,
                       CustomLookAndFeel::getTextSecondary());
  titleLabel.setJustificationType(juce::Justification::centredLeft);
  addAndMakeVisible(titleLabel);

  // ===== Section 1: Diffuse + LowCut + Decay =====

  // Diffuse Knob (Medium, Label Above, Value Below)
  diffuseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  diffuseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                UIConstants::kTextBoxWidth,
                                UIConstants::kElementHeight);
  diffuseSlider.setMouseDragSensitivity(300);
  addAndMakeVisible(diffuseSlider);
  diffuseAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "diffuse", diffuseSlider);

  diffuseLabel.setText("Diffuse", juce::dontSendNotification);
  diffuseLabel.setJustificationType(juce::Justification::centred);
  diffuseLabel.setColour(juce::Label::textColourId,
                         CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(diffuseLabel);

  // Low Cut Knob (Medium, Label Above, Value Below)
  lowcutSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lowcutSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                               UIConstants::kTextBoxWidth,
                               UIConstants::kElementHeight);
  lowcutSlider.setMouseDragSensitivity(150);
  addAndMakeVisible(lowcutSlider);
  lowcutAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "lowcut", lowcutSlider);

  lowcutLabel.setText("Low Cut", juce::dontSendNotification);
  lowcutLabel.setJustificationType(juce::Justification::centred);
  lowcutLabel.setColour(juce::Label::textColourId,
                        CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(lowcutLabel);

  // Decay Toggle Button (TextButton style like reference project)
  decayButton.setClickingTogglesState(true);
  decayButton.setColour(juce::TextButton::buttonOnColourId,
                        CustomLookAndFeel::getAccentPrimary());
  decayButton.setColour(juce::TextButton::textColourOnId,
                        CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(decayButton);
  decayAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          processorRef.getParameters(), "decay", decayButton);

  // ===== Section 2: Resolution + Mix =====

  // Resolution (FFT Size) Knob (Small, Label Above, Value Below)
  fftSizeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  fftSizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                UIConstants::kTextBoxWidth,
                                UIConstants::kElementHeight);
  fftSizeSlider.setMouseDragSensitivity(80);
  addAndMakeVisible(fftSizeSlider);
  fftSizeAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "fftsize", fftSizeSlider);

  fftSizeLabel.setText("FFT Size", juce::dontSendNotification);
  fftSizeLabel.setJustificationType(juce::Justification::centred);
  fftSizeLabel.setColour(juce::Label::textColourId,
                         CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(fftSizeLabel);

  // Mix Knob (Small, Label Above, Value Below)
  mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                            UIConstants::kTextBoxWidth,
                            UIConstants::kElementHeight);
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

  // Gain Knob (Small, Label Above, Value Below)
  gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                             UIConstants::kTextBoxWidth,
                             UIConstants::kElementHeight);
  gainSlider.setMouseDragSensitivity(150);
  addAndMakeVisible(gainSlider);
  gainAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "gain", gainSlider);

  gainLabel.setText("Gain", juce::dontSendNotification);
  gainLabel.setJustificationType(juce::Justification::centred);
  gainLabel.setColour(juce::Label::textColourId,
                      CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(gainLabel);

  setSize(UIConstants::kWindowWidth, UIConstants::kWindowHeight);
}

CopycatDiffuseEditor::~CopycatDiffuseEditor() { setLookAndFeel(nullptr); }

void CopycatDiffuseEditor::paint(juce::Graphics &g) {
  auto bounds = getLocalBounds();
  auto titleArea = bounds.removeFromTop(UIConstants::kTitleBarHeight);

  // Title Bar Background
  g.setColour(CustomLookAndFeel::getBackgroundMedium());
  g.fillRect(titleArea);

  // Main Background
  g.setColour(CustomLookAndFeel::getBackgroundDark());
  g.fillRect(bounds);

  // Draw separator below title bar
  g.setColour(CustomLookAndFeel::getTextPrimary().withAlpha(0.1f));
  g.fillRect(0, UIConstants::kTitleBarHeight - 1, getWidth(), 1);

  // Draw separator between sections
  if (separatorLineY > 0) {
    g.setColour(CustomLookAndFeel::getTextSecondary().withAlpha(0.2f));
    g.drawHorizontalLine(separatorLineY,
                         static_cast<float>(UIConstants::kMargin),
                         static_cast<float>(getWidth() - UIConstants::kMargin));
  }
}

void CopycatDiffuseEditor::resized() {
  auto bounds = getLocalBounds();
  const int margin = UIConstants::kMargin;
  const int largeKnob = UIConstants::kLargeKnob;
  const int mediumKnob = UIConstants::kMediumKnob;
  const int smallKnob = UIConstants::kSmallKnob;
  int width = bounds.getWidth();

  // ===== Title Bar =====
  auto titleArea = bounds.removeFromTop(UIConstants::kTitleBarHeight);

  // Calculate widths to center the combined title
  int brandWidth = 140;
  int diffuseWidth = 140;
  int totalTitleWidth = brandWidth + UIConstants::kTitleGap + diffuseWidth;

  int titleStartX = (titleArea.getWidth() - totalTitleWidth) / 2;

  brandLabel.setBounds(titleStartX, 0, brandWidth,
                       UIConstants::kTitleBarHeight);
  titleLabel.setBounds(titleStartX + brandWidth + UIConstants::kTitleGap, 0,
                       diffuseWidth, UIConstants::kTitleBarHeight);

  int y = UIConstants::kTitleBarHeight + margin;

  // ===== Section 1: Diffuse (Large) + Low Cut (Medium) side by side =====
  int topRowItemGap = UIConstants::kBottomItemGap;
  int totalTopRowWidth = largeKnob + topRowItemGap + mediumKnob;
  int topRowStartX = (width - totalTopRowWidth) / 2;

  // Common Y-offset for labels
  int topRowLabelY = y;

  // Diffuse - Left (Large Knob)
  int diffuseKnobY = y + UIConstants::kElementHeight + UIConstants::kSubGap;
  int diffuseKnobHeightWithText =
      largeKnob + UIConstants::kElementHeight + UIConstants::kSubGap;

  int diffuseX = topRowStartX;
  diffuseLabel.setBounds(diffuseX, topRowLabelY, largeKnob,
                         UIConstants::kElementHeight);
  diffuseSlider.setBounds(diffuseX, diffuseKnobY, largeKnob,
                          diffuseKnobHeightWithText);

  // Low Cut - Right (Medium Knob, vertically centered with Diffuse)
  int lowcutKnobHeightWithText =
      mediumKnob + UIConstants::kElementHeight + UIConstants::kSubGap;
  int lowcutVerticalOffset =
      (diffuseKnobHeightWithText - lowcutKnobHeightWithText) / 2;

  int lowcutX = diffuseX + largeKnob + topRowItemGap;
  lowcutLabel.setBounds(lowcutX, topRowLabelY, mediumKnob,
                        UIConstants::kElementHeight);
  lowcutSlider.setBounds(lowcutX, diffuseKnobY + lowcutVerticalOffset,
                         mediumKnob, lowcutKnobHeightWithText);

  y += UIConstants::kElementHeight + UIConstants::kSubGap +
       diffuseKnobHeightWithText + UIConstants::kMainGap;

  // Decay button (centered)
  int buttonWidth = UIConstants::kButtonWidth;
  int buttonHeight = UIConstants::kRowHeight;
  decayButton.setBounds((width - buttonWidth) / 2, y, buttonWidth,
                        buttonHeight);
  y += buttonHeight + UIConstants::kSectionGap;

  // ===== Separator Line =====
  separatorLineY = y;
  y += UIConstants::kSectionGap;

  // ===== Section 2: FFT Size + Mix + Gain (3 knobs side-by-side, centered)
  // =====
  int itemWidth = UIConstants::kSmallKnob;
  int itemGap = UIConstants::kBottomItemGap;
  int totalBottomWidth = (itemWidth * 3) + (itemGap * 2);
  int startX = (width - totalBottomWidth) / 2;

  // Common Y-offset for the bottom row labels
  int bottomRowLabelY = y;
  int bottomRowKnobY = y + UIConstants::kElementHeight + UIConstants::kSubGap;

  // FFT Size - Left
  int fftX = startX;
  fftSizeLabel.setBounds(fftX, bottomRowLabelY, itemWidth,
                         UIConstants::kElementHeight);
  fftSizeSlider.setBounds(fftX, bottomRowKnobY, itemWidth,
                          smallKnob + UIConstants::kElementHeight +
                              UIConstants::kSubGap);

  // Gain - Center
  int gainX = fftX + itemWidth + itemGap;
  gainLabel.setBounds(gainX, bottomRowLabelY, itemWidth,
                      UIConstants::kElementHeight);
  gainSlider.setBounds(gainX, bottomRowKnobY, itemWidth,
                       smallKnob + UIConstants::kElementHeight +
                           UIConstants::kSubGap);

  // Mix - Right
  int mixX = gainX + itemWidth + itemGap;
  mixLabel.setBounds(mixX, bottomRowLabelY, itemWidth,
                     UIConstants::kElementHeight);
  mixSlider.setBounds(mixX, bottomRowKnobY, itemWidth,
                      smallKnob + UIConstants::kElementHeight +
                          UIConstants::kSubGap);
}
