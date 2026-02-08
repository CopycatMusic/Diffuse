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

  // Diffuse Knob (Large, center with label above)
  diffuseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  diffuseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 18);
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

  // Low Cut Knob (Small, label above, TextBoxRight)
  lowcutSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lowcutSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 65, 15);
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

  // Resolution (FFT Size) Knob (Small, label above, TextBoxRight)
  fftSizeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  fftSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 65, 15);
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

  // Mix Knob (Small, label above, TextBoxRight)
  mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  mixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 65, 15);
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
  const int largeKnob = UIConstants::kLargeKnobSize;
  const int smallKnob = UIConstants::kSmallKnobSize;
  const int textBoxWidth = 65;
  const int textGap = 5;
  int width = bounds.getWidth();

  // ===== Title Bar =====
  auto titleArea = bounds.removeFromTop(UIConstants::kTitleBarHeight);

  // Calculate widths to center the combined title
  int brandWidth = titleFont.getStringWidth(brandLabel.getText());
  int diffuseWidth = titleFont.getStringWidth(titleLabel.getText());
  int titleGap = 4; // Reduced from 8px to 4px
  int totalTitleWidth = brandWidth + titleGap + diffuseWidth;

  int titleStartX = (titleArea.getWidth() - totalTitleWidth) / 2;

  brandLabel.setBounds(titleStartX, 0, brandWidth,
                       UIConstants::kTitleBarHeight);
  titleLabel.setBounds(titleStartX + brandWidth + titleGap, 0, diffuseWidth,
                       UIConstants::kTitleBarHeight);

  int y = UIConstants::kTitleBarHeight + margin;

  // ===== Section 1: Diffuse (large, centered with label above) =====
  diffuseLabel.setBounds(0, y, width, UIConstants::kLabelHeight);
  y += UIConstants::kLabelHeight + 4;

  int diffuseX = (width - largeKnob) / 2;
  diffuseSlider.setBounds(diffuseX, y, largeKnob,
                          largeKnob + UIConstants::kTextBoxHeight + 5);
  y += largeKnob + UIConstants::kTextBoxHeight + 12;

  // Low Cut (knob left, label above textbox on right)
  int sliderWidthWithText = smallKnob + textBoxWidth + textGap;
  int sliderX = (width - sliderWidthWithText) / 2;

  lowcutSlider.setBounds(sliderX, y, sliderWidthWithText, smallKnob);
  // Label positioned above the textbox (right side of knob)
  lowcutLabel.setBounds(sliderX + smallKnob + textGap, y - 2, textBoxWidth,
                        UIConstants::kLabelHeight);
  y += smallKnob + 10;

  // Decay button (centered)
  int buttonWidth = 100;
  int buttonHeight = UIConstants::kRowHeight;
  decayButton.setBounds((width - buttonWidth) / 2, y, buttonWidth,
                        buttonHeight);
  y += buttonHeight + margin;

  // ===== Separator Line =====
  separatorLineY = y;
  y += margin;

  // ===== Section 2: Resolution + Mix (side by side, knob left, label above
  // textbox) =====
  int sectionHalfWidth = (width - margin * 3) / 2;
  int sideKnobWidth = smallKnob + textBoxWidth + textGap;

  // Resolution (left)
  int resX = margin + (sectionHalfWidth - sideKnobWidth) / 2;
  fftSizeSlider.setBounds(resX, y, sideKnobWidth, smallKnob);
  // Label above textbox
  fftSizeLabel.setBounds(resX + smallKnob + textGap, y - 2, textBoxWidth,
                         UIConstants::kLabelHeight);

  // Mix (right)
  int mixX = margin + sectionHalfWidth + margin +
             (sectionHalfWidth - sideKnobWidth) / 2;
  mixSlider.setBounds(mixX, y, sideKnobWidth, smallKnob);
  // Label above textbox
  mixLabel.setBounds(mixX + smallKnob + textGap, y - 2, textBoxWidth,
                     UIConstants::kLabelHeight);
}
