#include "PluginEditor.h"
#include "BinaryData.h"
#include "ui/UIConstants.h"

CopycatDiffuseEditor::CopycatDiffuseEditor(CopycatDiffuseProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p) {
  setLookAndFeel(&customLookAndFeel);

  // Load embedded Montserrat fonts
  auto boldTypeface = juce::Typeface::createSystemTypefaceFor(
      BinaryData::MontserratBold_ttf, BinaryData::MontserratBold_ttfSize);
  titleFont = juce::Font(boldTypeface).withHeight(UIConstants::kTitleFontSize);

  auto mediumTypeface = juce::Typeface::createSystemTypefaceFor(
      BinaryData::MontserratMedium_ttf, BinaryData::MontserratMedium_ttfSize);
  defaultFont =
      juce::Font(mediumTypeface).withHeight(UIConstants::kDefaultFontSize);

  // Set default font for all UI components via LookAndFeel
  customLookAndFeel.setDefaultFont(defaultFont);

  // ===== Title Bar: Single white label =====
  titleLabel.setName("Title");
  titleLabel.getProperties().set("isTitle", true);
  titleLabel.setText("DIFFUSE", juce::dontSendNotification);
  titleLabel.setFont(titleFont);
  titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
  titleLabel.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(titleLabel);

  // ===== Section 1: Diffuse + LowCut + Decay =====

  // Diffuse Knob (Medium, Label Above, Value Below)
  diffuseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  diffuseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                UIConstants::kLargeTextBoxWidth,
                                UIConstants::kElementHeight);
  diffuseSlider.setTextValueSuffix("ms");
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

  // Cutoff Knob (Medium, Label Above, Value Below)
  cutoffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                               UIConstants::kSmallTextBoxWidth,
                               UIConstants::kElementHeight);
  cutoffSlider.setTextValueSuffix("Hz");
  cutoffSlider.setMouseDragSensitivity(150);
  addAndMakeVisible(cutoffSlider);
  cutoffAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "lowcut", cutoffSlider);

  cutoffLabel.setText("Cutoff", juce::dontSendNotification);
  cutoffLabel.setJustificationType(juce::Justification::centred);
  cutoffLabel.setColour(juce::Label::textColourId,
                        CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(cutoffLabel);

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

  // Feedback Knob (Medium, Label Above, Value Below)
  feedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                 UIConstants::kLargeTextBoxWidth,
                                 UIConstants::kElementHeight);
  feedbackSlider.setTextValueSuffix("%");
  feedbackSlider.setMouseDragSensitivity(150);
  addAndMakeVisible(feedbackSlider);
  feedbackAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "feedback", feedbackSlider);

  feedbackLabel.setText("Feedback", juce::dontSendNotification);
  feedbackLabel.setJustificationType(juce::Justification::centred);
  feedbackLabel.setColour(juce::Label::textColourId,
                          CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(feedbackLabel);

  // ===== Section 2: FFT Size + Mix + Gain =====

  // Resolution (FFT Size) Knob (Small, Label Above, Value Below)
  fftSizeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  fftSizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                UIConstants::kSmallTextBoxWidth,
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
                            UIConstants::kSmallTextBoxWidth,
                            UIConstants::kElementHeight);
  mixSlider.setTextValueSuffix("%");
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
                             UIConstants::kSmallTextBoxWidth,
                             UIConstants::kElementHeight);
  gainSlider.setTextValueSuffix("dB");
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

  // LFO Rate Knob
  modRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  modRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                UIConstants::kSmallTextBoxWidth,
                                UIConstants::kElementHeight);
  modRateSlider.setTextValueSuffix("Hz");
  modRateSlider.setMouseDragSensitivity(150);
  modRateSlider.getProperties().set("isMini", true);
  addAndMakeVisible(modRateSlider);
  modRateAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "modrate", modRateSlider);

  modRateLabel.setText("Rate", juce::dontSendNotification);
  modRateLabel.setJustificationType(juce::Justification::centred);
  modRateLabel.setColour(juce::Label::textColourId,
                         CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(modRateLabel);

  // LFO Depth Knob
  modDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  modDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                 UIConstants::kSmallTextBoxWidth,
                                 UIConstants::kElementHeight);
  modDepthSlider.setTextValueSuffix("%");
  modDepthSlider.setMouseDragSensitivity(150);
  modDepthSlider.getProperties().set("isMini", true);
  addAndMakeVisible(modDepthSlider);
  modDepthAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "moddepth", modDepthSlider);

  // LFO Header
  lfoHeaderLabel.setText(
      juce::CharPointer_UTF8("\xe2\x80\x94\xe2\x80\x94\xe2\x80\x94 LFO "
                             "\xe2\x80\x94\xe2\x80\x94\xe2\x80\x94"),
      juce::dontSendNotification);
  lfoHeaderLabel.setJustificationType(juce::Justification::centred);
  lfoHeaderLabel.setColour(juce::Label::textColourId,
                           CustomLookAndFeel::getTextTertiary());
  addAndMakeVisible(lfoHeaderLabel);

  modDepthLabel.setText("Depth", juce::dontSendNotification);
  modDepthLabel.setJustificationType(juce::Justification::centred);
  modDepthLabel.setColour(juce::Label::textColourId,
                          CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(modDepthLabel);

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
  g.setColour(CustomLookAndFeel::getTextSecondary().withAlpha(0.1f));
  g.fillRect(0, UIConstants::kTitleBarHeight - 1, getWidth(), 1);

  // Draw shadow gradient below title bar
  juce::ColourGradient shadowGradient(juce::Colours::black.withAlpha(0.6f), 0,
                                      UIConstants::kTitleBarHeight,
                                      juce::Colours::transparentBlack, 0,
                                      UIConstants::kTitleBarHeight + 8, false);
  g.setGradientFill(shadowGradient);
  g.fillRect(0, UIConstants::kTitleBarHeight, getWidth(), 8);

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

  titleLabel.setBounds(titleArea);

  int y = UIConstants::kTitleBarHeight + margin;

  // ===== Row 1: Diffuse (Large) + Feedback (Large) side by side =====
  int topRowItemGap = UIConstants::kKnobHGap;
  int totalTopRowWidth = largeKnob + topRowItemGap + largeKnob;
  int topRowStartX = (width - totalTopRowWidth) / 2;

  // Common Y-offset for labels
  int topRowLabelY = y;

  // Diffuse - Left (Large Knob)
  int diffuseKnobY = y + UIConstants::kElementHeight + UIConstants::kSubGap;
  int topRowKnobHeightWithText =
      largeKnob + UIConstants::kElementHeight + UIConstants::kSubGap;

  int diffuseX = topRowStartX;
  diffuseLabel.setBounds(diffuseX, topRowLabelY, largeKnob,
                         UIConstants::kElementHeight);
  diffuseSlider.setBounds(diffuseX, diffuseKnobY, largeKnob,
                          topRowKnobHeightWithText);

  // Feedback - Right (Large Knob)
  int feedbackX = diffuseX + largeKnob + topRowItemGap;
  feedbackLabel.setBounds(feedbackX, topRowLabelY, largeKnob,
                          UIConstants::kElementHeight);
  feedbackSlider.setBounds(feedbackX, diffuseKnobY, largeKnob,
                           topRowKnobHeightWithText);

  y += UIConstants::kElementHeight + UIConstants::kSubGap +
       topRowKnobHeightWithText + UIConstants::kMainGap;

  // ===== Row 2: Rate, Depth, and Cutoff (Standardized 60px width) =====
  int row2CompWidth = 60;
  int row2SmallKnob = UIConstants::kSmallKnob;
  int row2MiniKnob = UIConstants::kMiniKnob;
  int row2Gap = UIConstants::kKnobHGap;
  // No gap between Rate and Depth: Total = (Rate + Depth) + Gap + Cutoff
  int totalRow2Width = (row2CompWidth * 2) + row2Gap + row2CompWidth;
  int row2StartX = (width - totalRow2Width) / 2;

  int row2KnobHeightWithText =
      row2SmallKnob + UIConstants::kElementHeight + UIConstants::kSubGap;
  int miniKnobHeightWithText =
      row2MiniKnob + UIConstants::kElementHeight + UIConstants::kSubGap;

  // 1. Headers (Top Align)
  // LFO Header spans both Rate and Depth (no internal gap)
  int lfoHeaderWidth = row2CompWidth * 2;
  lfoHeaderLabel.setBounds(row2StartX, y, lfoHeaderWidth,
                           UIConstants::kElementHeight);

  // Cutoff Label aligns with LFO Header
  int cutoffX = row2StartX + lfoHeaderWidth + row2Gap;
  cutoffLabel.setBounds(cutoffX, y, row2CompWidth, UIConstants::kElementHeight);

  // 2. Parameter Labels (LFO only)
  int labelY = y + UIConstants::kElementHeight + UIConstants::kSubGap;
  modRateLabel.setBounds(row2StartX, labelY, row2CompWidth,
                         UIConstants::kElementHeight);
  modDepthLabel.setBounds(row2StartX + row2CompWidth, labelY, row2CompWidth,
                          UIConstants::kElementHeight);

  // 3. Sliders (Bottom Align / Height Match)
  int sliderY = labelY + UIConstants::kElementHeight + UIConstants::kSubGap;
  modRateSlider.setBounds(row2StartX, sliderY, row2CompWidth,
                          miniKnobHeightWithText);
  modDepthSlider.setBounds(row2StartX + row2CompWidth, sliderY, row2CompWidth,
                           miniKnobHeightWithText);

  int cutoffSliderY = y + UIConstants::kElementHeight + 4; // Matches total 91h
  cutoffSlider.setBounds(cutoffX, cutoffSliderY, row2CompWidth,
                         row2KnobHeightWithText);

  y += 91 + UIConstants::kMainGap;

  // ===== Row 3: Decay button (centered) =====
  int buttonWidth = UIConstants::kButtonWidth;
  int buttonHeight = UIConstants::kRowHeight;
  int decayButtonX = (width - buttonWidth) / 2;
  decayButton.setBounds(decayButtonX, y, buttonWidth, buttonHeight);

  y += buttonHeight + UIConstants::kSectionGap;

  // ===== Separator Line =====
  separatorLineY = y;
  y += UIConstants::kSectionGap;

  // ===== Section 2: FFT Size + Mix + Gain (3 knobs side-by-side, centered)
  // =====
  int itemWidth = UIConstants::kSmallKnob;
  int itemGap = UIConstants::kKnobHGap;
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
