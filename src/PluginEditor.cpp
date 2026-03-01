#include "PluginEditor.h"
#include "BinaryData.h"
#include "ui/UIConstants.h"

CopycatDiffuseEditor::CopycatDiffuseEditor(CopycatDiffuseProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p) {
  setLookAndFeel(&customLookAndFeel);

  // Load embedded Montserrat fonts
  auto boldTypeface = juce::Typeface::createSystemTypefaceFor(
      BinaryData::MontserratBold_ttf, BinaryData::MontserratBold_ttfSize);
  titleFont = juce::Font(
      juce::FontOptions(boldTypeface).withHeight(UIConstants::kTitleFontSize));

  auto mediumTypeface = juce::Typeface::createSystemTypefaceFor(
      BinaryData::MontserratMedium_ttf, BinaryData::MontserratMedium_ttfSize);
  defaultFont = juce::Font(juce::FontOptions(mediumTypeface)
                               .withHeight(UIConstants::kDefaultFontSize));

  // Set default font for all UI components via LookAndFeel
  customLookAndFeel.setDefaultFont(defaultFont);

  // ===== Logo Labels: COPY / CAT =====
  logoLabelCopy.setText("COPY", juce::dontSendNotification);
  logoLabelCopy.setFont(titleFont.withHeight(UIConstants::kDefaultFontSize));
  logoLabelCopy.getProperties().set("isTitle", true);
  logoLabelCopy.setColour(juce::Label::textColourId,
                          CustomLookAndFeel::getTextTertiary());
  logoLabelCopy.setJustificationType(juce::Justification::centredLeft);
  logoLabelCopy.setBorderSize(
      juce::BorderSize<int>(0, 0, 0, 0)); // Remove padding to ensure tight fit

  auto logoShadow = juce::DropShadow(
      juce::Colours::black.withAlpha(UIConstants::kShadowAlpha), 6, {0, 2});
  auto *logoEffect1 = new juce::DropShadowEffect();
  logoEffect1->setShadowProperties(logoShadow);
  logoLabelCopy.setComponentEffect(logoEffect1);

  addAndMakeVisible(logoLabelCopy);

  logoLabelCat.setText("CAT", juce::dontSendNotification);
  logoLabelCat.setFont(titleFont.withHeight(UIConstants::kDefaultFontSize));
  logoLabelCat.getProperties().set("isTitle", true);
  logoLabelCat.setColour(juce::Label::textColourId,
                         CustomLookAndFeel::getTextTertiary());
  logoLabelCat.setJustificationType(juce::Justification::centredLeft);
  logoLabelCat.setBorderSize(
      juce::BorderSize<int>(0, 0, 0, 0)); // Remove padding

  auto *logoEffect2 = new juce::DropShadowEffect();
  logoEffect2->setShadowProperties(logoShadow);
  logoLabelCat.setComponentEffect(logoEffect2);

  addAndMakeVisible(logoLabelCat);

  // ===== Title Bar: Single white label =====
  titleLabel.setName("Title");
  titleLabel.getProperties().set("isTitle", true);
  titleLabel.setText("DIFFUSE", juce::dontSendNotification);
  titleLabel.setFont(titleFont);
  titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
  titleLabel.setJustificationType(juce::Justification::centred);

  auto titleShadow = juce::DropShadow(
      juce::Colours::black.withAlpha(UIConstants::kShadowAlpha), 10, {2, 3});
  auto *titleEffect = new juce::DropShadowEffect();
  titleEffect->setShadowProperties(titleShadow);
  titleLabel.setComponentEffect(titleEffect);

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

  // Decay Buttons Setup
  auto setupDecayButton = [this](ShadowButton &b, bool onState,
                                 const juce::String &text,
                                 ShadowButton::SegmentPosition pos) {
    b.setButtonText(text);
    b.setSegmentPosition(pos);
    b.setClickingTogglesState(true);
    b.setRadioGroupId(1002);
    b.setColour(juce::TextButton::buttonOnColourId,
                CustomLookAndFeel::getAccentPrimary());
    b.setColour(juce::TextButton::textColourOnId,
                CustomLookAndFeel::getTextPrimary());
    addAndMakeVisible(b);

    b.onClick = [this, onState] {
      if (auto *param = processorRef.getParameters().getParameter("decay")) {
        param->setValueNotifyingHost(onState ? 1.0f : 0.0f);
      }
    };
  };

  setupDecayButton(decayOffButton, false, "OFF", ShadowButton::kLeft);
  setupDecayButton(decayOnButton, true, "ON", ShadowButton::kRight);

  decayGroupLabel.setText("Decay", juce::dontSendNotification);
  decayGroupLabel.setJustificationType(juce::Justification::centred);
  decayGroupLabel.setColour(juce::Label::textColourId,
                            CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(decayGroupLabel);

  // Grain Buttons Setup
  auto setupGrainButton = [this](ShadowButton &b, int index,
                                 const juce::String &text,
                                 ShadowButton::SegmentPosition pos) {
    b.setButtonText(text);
    b.setSegmentPosition(pos);
    b.setClickingTogglesState(true);
    b.setRadioGroupId(1001); // Radio group for single selection
    b.setColour(juce::TextButton::buttonOnColourId,
                CustomLookAndFeel::getAccentPrimary());
    b.setColour(juce::TextButton::textColourOnId,
                CustomLookAndFeel::getTextPrimary());
    addAndMakeVisible(b);

    b.onClick = [this, index] {
      if (auto *param = processorRef.getParameters().getParameter("grain")) {
        param->setValueNotifyingHost(
            param->getNormalisableRange().convertTo0to1(
                static_cast<float>(index)));
      }
    };
  };

  setupGrainButton(grain1xButton, 0, "1x", ShadowButton::kLeft);
  setupGrainButton(grain4xButton, 1, "4x", ShadowButton::kMiddle);
  setupGrainButton(grain16xButton, 2, "16x", ShadowButton::kRight);

  grainGroupLabel.setText("Grain", juce::dontSendNotification);
  grainGroupLabel.setJustificationType(juce::Justification::centred);
  grainGroupLabel.setColour(juce::Label::textColourId,
                            CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(grainGroupLabel);

  // Initial state sync
  auto *grainParam = processorRef.getParameters().getRawParameterValue("grain");
  if (grainParam) {
    updateGrainButtons(static_cast<int>(grainParam->load()));
  }

  auto *decayParam = processorRef.getParameters().getRawParameterValue("decay");
  if (decayParam) {
    updateDecayButtons(decayParam->load() > 0.5f);
  }

  // Register listener for parameter changes
  processorRef.getParameters().addParameterListener("grain", this);
  processorRef.getParameters().addParameterListener("decay", this);

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

  // Stereo Knob (Small, Label Above, Value Below)
  stereoSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  stereoSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                               UIConstants::kSmallTextBoxWidth,
                               UIConstants::kElementHeight);
  stereoSlider.setTextValueSuffix("%");
  stereoSlider.setMouseDragSensitivity(150);
  addAndMakeVisible(stereoSlider);
  stereoAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.getParameters(), "stereo", stereoSlider);

  stereoLabel.setText("Stereo", juce::dontSendNotification);
  stereoLabel.setJustificationType(juce::Justification::centred);
  stereoLabel.setColour(juce::Label::textColourId,
                        CustomLookAndFeel::getTextPrimary());
  addAndMakeVisible(stereoLabel);

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

CopycatDiffuseEditor::~CopycatDiffuseEditor() {
  processorRef.getParameters().removeParameterListener("grain", this);
  processorRef.getParameters().removeParameterListener("decay", this);
  setLookAndFeel(nullptr);
}

void CopycatDiffuseEditor::parameterChanged(const juce::String &parameterID,
                                            float newValue) {
  if (parameterID == "grain") {
    juce::MessageManager::callAsync(
        [this, newValue] { updateGrainButtons(static_cast<int>(newValue)); });
  } else if (parameterID == "decay") {
    juce::MessageManager::callAsync(
        [this, newValue] { updateDecayButtons(newValue > 0.5f); });
  }
}

void CopycatDiffuseEditor::updateGrainButtons(int choiceIndex) {
  grain1xButton.setToggleState(choiceIndex == 0, juce::dontSendNotification);
  grain4xButton.setToggleState(choiceIndex == 1, juce::dontSendNotification);
  grain16xButton.setToggleState(choiceIndex == 2, juce::dontSendNotification);
}

void CopycatDiffuseEditor::updateDecayButtons(bool enabled) {
  decayOffButton.setToggleState(!enabled, juce::dontSendNotification);
  decayOnButton.setToggleState(enabled, juce::dontSendNotification);
}

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

  // ===== Logo Layout (Left of Title) =====
  // Stacked "COPY" and "CAT" labels, vertically centered in title bar
  const int logoFontSize = static_cast<int>(UIConstants::kDefaultFontSize);
  const int logoX = margin;
  // Total height of 2 lines. We center this block in the 40px title bar.
  // 40 - (16 * 2) = 8. So 4px padding top.
  const int totalLogoHeight = logoFontSize * 2 - 4;
  const int logoStartY = (UIConstants::kTitleBarHeight - totalLogoHeight) / 2;
  const int logoWidth = 64;                // Increased to fit shadow
  const int logoHeight = logoFontSize + 8; // Extra height for shadow

  logoLabelCopy.setBounds(logoX, logoStartY - 4, logoWidth, logoHeight);
  logoLabelCat.setBounds(logoX, logoStartY + logoFontSize - 8, logoWidth,
                         logoHeight);

  int y = UIConstants::kTitleBarHeight + margin;

  // ===== Row 1: Diffuse (Large) + Feedback (Large) side by side =====
  int topRowItemGap = UIConstants::kRow1HGap;
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
  int row2Gap = UIConstants::kRow2HGap;
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

  // ===== Row 3: Decay & Grain Segmented Controls side-by-side =====
  const int shadowMargin = UIConstants::kButtonShadowMargin;
  int buttonHeight = UIConstants::kRowHeight + (shadowMargin * 2);
  int gap = UIConstants::kRow3HGap;

  // Segment widths
  int decayBtnW = UIConstants::kSegmentedItemWidth;
  int grainBtnW = UIConstants::kSegmentedItemWidth;
  int decayBtnWWithShadow = decayBtnW + (shadowMargin * 2);
  int grainBtnWWithShadow = grainBtnW + (shadowMargin * 2);

  int decayGroupWidth = (decayBtnW * 2) + 2;
  int grainGroupWidth = (grainBtnW * 3) + 4;

  int row3TotalWidth = decayGroupWidth + gap + grainGroupWidth;
  int startX3 = (width - row3TotalWidth) / 2;

  // Decay Group
  decayGroupLabel.setBounds(startX3, y, decayGroupWidth,
                            UIConstants::kElementHeight);
  int decayBtnY = y + UIConstants::kElementHeight + UIConstants::kSubGap + 2;
  decayOffButton.setBounds(startX3 - shadowMargin, decayBtnY - shadowMargin,
                           decayBtnWWithShadow, buttonHeight);
  decayOnButton.setBounds(startX3 + decayBtnW - shadowMargin + 2,
                          decayBtnY - shadowMargin, decayBtnWWithShadow,
                          buttonHeight);

  // Grain Group
  int grainX = startX3 + decayGroupWidth + gap;
  grainGroupLabel.setBounds(grainX, y, grainGroupWidth,
                            UIConstants::kElementHeight);

  int grainBtnY = y + UIConstants::kElementHeight + UIConstants::kSubGap + 2;
  grain1xButton.setBounds(grainX - shadowMargin, grainBtnY - shadowMargin,
                          grainBtnWWithShadow, buttonHeight);
  grain4xButton.setBounds(grainX + grainBtnW - shadowMargin + 2,
                          grainBtnY - shadowMargin, grainBtnWWithShadow,
                          buttonHeight);
  grain16xButton.setBounds(grainX + (grainBtnW * 2) - shadowMargin + 4,
                           grainBtnY - shadowMargin, grainBtnWWithShadow,
                           buttonHeight);

  // Calculate the visual bottom of Row 3 buttons
  int row3VisualBottom = grainBtnY + UIConstants::kRowHeight;

  // Position the separator line exactly one kSectionGap below the buttons
  separatorLineY = row3VisualBottom + UIConstants::kSectionGap;

  // Position the next section exactly one kSectionGap below the line
  y = separatorLineY + UIConstants::kSectionGap;

  // ===== Section 2: FFT Size + Mix + Gain (3 knobs side-by-side, centered)
  // =====
  int itemWidth = 60;
  int itemGap = UIConstants::kRow4HGap;
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

  // Stereo - Center
  int stereoX = fftX + itemWidth + itemGap;
  stereoLabel.setBounds(stereoX, bottomRowLabelY, itemWidth,
                        UIConstants::kElementHeight);
  stereoSlider.setBounds(stereoX, bottomRowKnobY, itemWidth,
                         smallKnob + UIConstants::kElementHeight +
                             UIConstants::kSubGap);

  // Mix - Right
  int mixX = stereoX + itemWidth + itemGap;
  mixLabel.setBounds(mixX, bottomRowLabelY, itemWidth,
                     UIConstants::kElementHeight);
  mixSlider.setBounds(mixX, bottomRowKnobY, itemWidth,
                      smallKnob + UIConstants::kElementHeight +
                          UIConstants::kSubGap);
}
