#pragma once

/**
 * Centralized UI constants for the CopyCat Diffuse plugin.
 */
namespace UIConstants {

// ===== Window Dimensions =====
constexpr int kWindowWidth = 280;
constexpr int kWindowHeight = 440;
constexpr int kTitleBarHeight = 40;
constexpr int kTitleGap = 0;

// ===== Layout Spacing =====
constexpr int kMargin = 12; // Outer window padding
constexpr int kSectionGap =
    10;                      // Vertical gap between major functional sections
constexpr int kMainGap = 10; // General vertical gap between components
constexpr int kSubGap = 2;   // Tight vertical gap (e.g. Label to Knob)

// ===== Element Dimensions =====
constexpr int kElementHeight = 15; // Standard height for labels and textboxes
constexpr int kRowHeight = 25;     // Standard height for buttons/toggles
constexpr int kLargeTextBoxWidth = 80;
constexpr int kSmallTextBoxWidth = 60;
constexpr int kTextGap = 20; // Horizontal gap between knob and text

// ===== Knob Sizes (square) =====
constexpr int kLargeKnob = 90;
constexpr int kMediumKnob = 65;
constexpr int kSmallKnob = 55;
constexpr int kMiniKnob = 40;
constexpr int kKnobHGap = 20;

// ===== Component Specifics =====
constexpr int kButtonWidth = 120;
constexpr float kBorderWidth = 1.2f;

// ===== Font Sizes =====
constexpr float kTitleFontSize = 32.0f;
constexpr float kDefaultFontSize = 16.0f; // Buttons, knob text, general UI

} // namespace UIConstants
