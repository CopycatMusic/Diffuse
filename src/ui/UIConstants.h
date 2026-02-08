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
constexpr int kMargin = 12; // Outer window padding
constexpr int kSectionGap =
    16;                      // Vertical gap between major functional sections
constexpr int kMainGap = 10; // General vertical gap between components
constexpr int kSubGap = 5;   // Tight vertical gap (e.g. Label to Knob)

// ===== Element Dimensions =====
constexpr int kElementHeight = 15; // Standard height for labels and textboxes
constexpr int kRowHeight = 25;     // Standard height for buttons/toggles
constexpr int kTextBoxWidth = 65;
constexpr int kTextGap = 40; // Horizontal gap between knob and text

// ===== Knob Sizes (square) =====
constexpr int kLargeKnob = 100;
constexpr int kMediumKnob = 80;
constexpr int kSmallKnob = 50;
constexpr int kBottomItemWidth = 80;
constexpr int kBottomItemGap = 15;

// ===== Component Specifics =====
constexpr int kButtonWidth = 80;

// ===== Font Sizes =====
constexpr float kTitleFontSize = 32.0f;
constexpr float kDefaultFontSize = 12.0f;

} // namespace UIConstants
