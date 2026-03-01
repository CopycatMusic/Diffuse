#!/bin/bash
# CopyCat Diffuse - PKG Installer Creator
# This script bundles VST3 and AU plugins into a single macOS .pkg installer.
#
# Usage: ./create_pkg.sh [--version X.X.X]

set -e

# ===========================================================================
# Parse Arguments
# ===========================================================================
VERSION=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --version)
            VERSION="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# ===========================================================================
# Auto-detect project root and version
# ===========================================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RESOURCE_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_DIR="$(dirname "$RESOURCE_DIR")"

# Auto-extract version from CMakeLists.txt if not provided
if [ -z "$VERSION" ]; then
    VERSION=$(grep -oE 'project\(CopycatDiffuse VERSION ([0-9.]+)' "$PROJECT_DIR/CMakeLists.txt" | grep -oE '[0-9]+\.[0-9]+\.[0-9]+')
    if [ -z "$VERSION" ]; then
        echo "❌ Error: Could not extract version from CMakeLists.txt"
        exit 1
    fi
    echo "📌 Auto-detected version: $VERSION"
fi

# ===========================================================================
# Set plugin variables
# ===========================================================================
PLUGIN_NAME="CopyCat Diffuse"
BUNDLE_ID="com.copycat.diffuse.pkg"
VST3_ID="com.copycat.vst3.diffuse"
AU_ID="com.copycat.au.diffuse"

# ===========================================================================
# Set paths
# ===========================================================================
RELEASE_DIR="$PROJECT_DIR/release/v$VERSION/macOS"
BUILD_DIR="$RELEASE_DIR/pkg_build_temp"
PKG_OUTPUT="$RELEASE_DIR/CopyCatDiffuse-${VERSION}-macOS.pkg"

AU_PATH="$RELEASE_DIR/${PLUGIN_NAME}.component"
VST3_PATH="$RELEASE_DIR/${PLUGIN_NAME}.vst3"

# ===========================================================================
# Validate input files exist
# ===========================================================================
if [ ! -d "$AU_PATH" ]; then
    echo "❌ Error: AU component not found: $AU_PATH"
    exit 1
fi

if [ ! -d "$VST3_PATH" ]; then
    echo "❌ Error: VST3 bundle not found: $VST3_PATH"
    exit 1
fi

echo "=================================================="
echo "📦 Creating PKG for: $PLUGIN_NAME v$VERSION"
echo "=================================================="

echo "🧹 Cleaning up previous build..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "📦 Building VST3 component package..."
pkgbuild --component "$VST3_PATH" \
         --install-location "/Library/Audio/Plug-Ins/VST3" \
         --identifier "$VST3_ID" \
         --version "$VERSION" \
         "$BUILD_DIR/vst3.pkg"

echo "📦 Building AU component package..."
pkgbuild --component "$AU_PATH" \
         --install-location "/Library/Audio/Plug-Ins/Components" \
         --identifier "$AU_ID" \
         --version "$VERSION" \
         "$BUILD_DIR/au.pkg"

echo "🏗️ Synthesizing distribution package..."
DIST_XML="$BUILD_DIR/distribution.xml"
productbuild --synthesize \
             --package "$BUILD_DIR/vst3.pkg" \
             --package "$BUILD_DIR/au.pkg" \
             "$DIST_XML"

# Inject the title into the distribution.xml
TITLE="$PLUGIN_NAME v$VERSION"
sed -i '' "s|<installer-gui-script[^>]*>|&\n    <title>$TITLE</title>|" "$DIST_XML"

# Build the final package
productbuild --distribution "$DIST_XML" \
             --package-path "$BUILD_DIR" \
             "$PKG_OUTPUT"

echo ""
echo "✅ PKG created: $PKG_OUTPUT"
echo "⚠️ Note: This package is NOT yet signed or notarized."
echo "📝 Next: Run ./resources/Scripts/release_macos.sh --version $VERSION"

# Clean up build temp files
rm -rf "$BUILD_DIR"
