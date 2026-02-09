# Using JUCE AudioPluginHost

I have compiled the `AudioPluginHost` for you. You can find it at:
`build/tools/AudioPluginHost.app`

## How to use it to test your plugin

1.  **Launch the Host**:
    Open `build/tools/AudioPluginHost.app`.

2.  **Configure Audio Settings**:
    - Go to **Options -> Audio/MIDI Settings**.
    - Select your input and output devices.

3.  **Scan for Plugins**:
    - Go to **Options -> Edit the List of Available Plugins...**.
    - Click **Options... -> Scan for new or updated VST3 plugins** (or AU).
    - Ensure your plugin's install path (usually `~/Library/Audio/Plug-Ins/VST3` or `Components`) is in the list of folders to scan.
    - If your plugin is not found, try cleaning and rebuilding your plugin.

4.  **Load Your Plugin**:
    - Right-click anywhere in the main graph area.
    - Select your plugin from the list (under your Company Name or Category).

5.  **Connect the Graph**:
    - Build a signal chain by dragging wires between nodes.
    - **Input -> Your Plugin -> Output**.
    - Double-click your plugin node to open its UI.

## Troubleshooting
- If the plugin fails to load, check if it's signed (on macOS) or if there are architecture mismatches (arm64 vs x86_64).
- Use `lldb` to attach to the `AudioPluginHost` process to debug crashes in your plugin.
