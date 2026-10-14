# Project preparation and validation — 2026-10-14

- README.md preserves the original Notion introduction, parameter descriptions,
  images, audio and contact information. Audio examples use a Dry/Wet comparison
  table, and build/installation guides precede Contact. Media is stored locally.
  The Gumroad profile link omits its analytics query.
- Detailed build instructions are also available in BUILDING.md.
- Old Spectral Delay workflow reference, tracked build log, local host notes,
  downloaded font backups and the unused Light font are archived under
  `_local_archive/preparation-2026-10-14/`.
- Local build and release directories are preserved and excluded from Git.
- Local macOS signing/packaging helpers remain available in resources/Scripts/
  but are excluded from Git. The Windows installer script stays tracked for CI.
- CMake accepts JUCE paths containing spaces, preserves architecture overrides,
  selects AU only on macOS, requires C++17 and disables automatic installation.
- Font/STFT notices are restored, and the project adopts the previously selected
  AGPLv3 source licence. DSP code, parameters, version and plugin identifiers are
  unchanged.

Local Release AU/VST3 builds with JUCE 8.0.12 passed on Apple Silicon. Both report
version 1.0.1, retain `com.copycat.diffuse`, and pass ad-hoc signature verification
in the local review directory. All seven README media references exist and match
the original downloaded file hashes; original prose/table text was checked.
Workflow YAML and Git whitespace checks pass. Windows/Intel builds and DAW runtime
validation were not performed in this pass. The existing v1.0 Release draft is
separate from these source preparation changes.
