#pragma once

/**
 * Centralized UI constants for the CopyCat Diffuse plugin.
 */
namespace UIConstants {

// ===== Window Dimensions =====
constexpr int kWindowWidth = 280;
constexpr int kWindowHeight = 450;
constexpr int kTitleBarHeight = 40;
constexpr int kTitleGap = 0;

// ===== Layout Spacing =====
constexpr int kMargin = 12; // Outer window padding
constexpr int kSectionGap =
    12;                      // Vertical gap between major functional sections
constexpr int kMainGap = 10; // General vertical gap between components
constexpr int kSubGap = 2;   // Tight gap (e.g. Label to Knob)

// ===== Element Dimensions =====
constexpr int kElementHeight = 15; // Standard height for labels and textboxes
constexpr int kRowHeight = 22;     // Standard height for buttons/toggles
constexpr int kLargeTextBoxWidth = 80;
constexpr int kSmallTextBoxWidth = 60;
constexpr int kTextGap = 20; // Horizontal gap between knob and text

// ===== Knob Sizes (square) =====
constexpr int kLargeKnob = 80;
constexpr int kMediumKnob = 65;
constexpr int kSmallKnob = 55;
constexpr int kMiniKnob = 40;

// Horizontal gaps for each row
constexpr int kRow1HGap = 20; // Diffuse & Feedback
constexpr int kRow2HGap = 20; // LFO & Cutoff
constexpr int kRow3HGap = 15; // Decay & Grain
constexpr int kRow4HGap = 15; // Bottom (FFT, Gain, Mix)

// ===== Component Specifics =====
constexpr int kButtonWidth = 80;
constexpr int kSegmentedItemWidth = 38;
constexpr int kButtonShadowMargin = 15;
constexpr float kBorderWidth = 1.6f;

// ===== Shadow Settings =====
constexpr float kShadowAlpha = 0.6f;
constexpr int kShadowRadius = 16;
constexpr int kShadowXOffset = 3;
constexpr int kShadowYOffset = 6;

// ===== Font Sizes =====
constexpr float kTitleFontSize = 32.0f;
constexpr float kDefaultFontSize = 16.0f; // Buttons, knob text, general UI

} // namespace UIConstants
