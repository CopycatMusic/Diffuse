; CopyCat Diffuse - Windows Installer Script
; For use with Inno Setup (https://jrsoftware.org/isdl.php)
;
; Usage: Place this file alongside the plugin folder and run with Inno Setup
; Expected structure:
;   release/vX.X.X/Windows/
;     ├── CopyCat Diffuse.vst3/
;     └── installer_win.iss (copy this file here)

#define AppName "CopyCat Diffuse"
#define AppVersion "1.0.0"
#define AppPublisher "CopyCat"
#define AppURL "https://github.com/CopycatMusic/CopycatDiffuse"

[Setup]
AppId={{E4A8B2C3-D5E9-4F0A-A234-C79D83B20122}}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}

DefaultDirName={autopf}\{#AppPublisher}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes

OutputDir=Output
OutputBaseFilename=CopyCat_Diffuse_Win_v{#AppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; The plugin bundle (.vst3 folder) - relative path from script location
Source: "{#AppName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#AppName}.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"

[Messages]
WelcomeLabel2=This will install {#AppName} v{#AppVersion} on your computer.%n%nThe plugin will be installed to:%nC:\Program Files\Common Files\VST3

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#AppName}.vst3"
