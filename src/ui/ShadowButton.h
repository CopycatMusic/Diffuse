#pragma once

#include "UIConstants.h"
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * A custom TextButton that overrides hitTest to exclude the shadow margin.
 */
class ShadowButton : public juce::TextButton {
public:
  enum SegmentPosition { kSingle, kLeft, kMiddle, kRight };

  using TextButton::TextButton;

  void setSegmentPosition(SegmentPosition pos) { position = pos; }
  SegmentPosition getSegmentPosition() const { return position; }

  bool hitTest(int x, int y) override {
    return getLocalBounds()
        .reduced(UIConstants::kButtonShadowMargin)
        .contains(x, y);
  }

private:
  SegmentPosition position = kSingle;
};
