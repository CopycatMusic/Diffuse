#pragma once

/**
 * Centralized UI constants for the CopyCat Diffuse plugin.
 */
namespace UIConstants {

// ===== Window Dimensions =====
constexpr int kWindowWidth = 300;
constexpr int kWindowHeight = 450;
constexpr int kTitleBarHeight = 40;
constexpr int kTitleGap = 0;

// ===== Layout Spacing =====
constexpr int kMargin = 10; // Outer window padding
constexpr int kSectionGap =
    16;                      // Vertical gap between major functional sections
constexpr int kMainGap = 10; // General vertical gap between components
constexpr int kSubGap = 5;   // Tight vertical gap (e.g. Label to Knob)

// ===== Element Dimensions =====
constexpr int kElementHeight = 15; // Standard height for labels and textboxes
constexpr int kRowHeight = 25;     // Standard height for buttons/toggles
constexpr int kTextBoxWidth = 65;
constexpr int kTextGap = 20; // Horizontal gap between knob and text

// ===== Knob Sizes (square) =====
constexpr int kLargeKnob = 90;
constexpr int kMediumKnob = 65;
constexpr int kSmallKnob = 55;
constexpr int kBottomItemGap = 18;

// ===== Component Specifics =====
constexpr int kButtonWidth = 90;

// ===== Font Sizes =====
constexpr float kTitleFontSize = 32.0f;
constexpr float kDefaultFontSize = 14.0f; // Buttons, knob text, general UI

} // namespace UIConstants
