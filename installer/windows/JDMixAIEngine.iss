; Inno Setup Script for JD Mix AI Engine (Windows x64 VST3 + Standalone Application)
; Continuous Update & Clean Uninstall Architecture for FL Studio & Windows

#define MyAppName "JD Mix AI Engine"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "JD Audio Labs"
#define MyAppURL "https://github.com/JamalDrenthe/jd-mix-ai-engine"
#define MyAppExeName "JD Mix AI Engine.exe"
#define MyAppId "{{57022853-7386-4F1E-9A7B-JDM1XA1ENG1N}}"

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={autopf}\JD Audio Labs\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename=JDMixAIEngine_Setup_v{#MyAppVersion}
OutputDir=..\..\dist
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
; SetupIconFile=..\..\assets\favicon.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
UninstallDisplayName={#MyAppName}

; Continuous Update Flow Settings:
; Gracefully detects and closes running DAW / Standalone instances to prevent file-locking
CloseApplications=yes
CloseApplicationsFilter=*.exe,FL64.exe,FL.exe,{#MyAppExeName}
RestartApplications=no

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "flstudio"; Description: "Install FL Studio 64-bit VST3 Plugin ({commoncf}\VST3)"; GroupDescription: "Plugin Formats"; Flags: checkedonce

[Files]
; Standalone Windows Executable (.exe)
Source: "..\..\artifacts\Standalone\*.exe"; DestDir: "{app}"; DestName: "{#MyAppExeName}"; Flags: ignoreversion

; VST3 Bundle for FL Studio and other 64-bit DAWs
Source: "..\..\artifacts\VST3\*"; DestDir: "{commoncf}\VST3\JD Mix AI Engine.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs; Tasks: flstudio

; User Presets (only install if not present, never overwrite user customizations during update)
Source: "..\..\presets\*"; DestDir: "{userdocs}\Image-Line\FL Studio\Presets\Plugin presets\Effects\VST3\JD Mix AI Engine"; Flags: ignoreversion recursesubdirs createallsubdirs onlyifdoesntexist uninsneveruninstall

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
Root: HKLM; Subkey: "Software\JD Audio Labs\JD Mix AI Engine"; ValueType: string; ValueName: "Version"; ValueData: "{#MyAppVersion}"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\JD Audio Labs\JD Mix AI Engine"; ValueType: string; ValueName: "InstallDir"; ValueData: "{app}"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\JD Audio Labs\JD Mix AI Engine"; ValueType: string; ValueName: "VST3Path"; ValueData: "{commoncf}\VST3\JD Mix AI Engine.vst3"; Flags: uninsdeletekey

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#MyAppName}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Clean up app directory and VST3 bundle
Type: filesandordirs; Name: "{commoncf}\VST3\JD Mix AI Engine.vst3"
Type: filesandordirs; Name: "{app}"

[Messages]
WelcomeLabel2=This installer will setup or upgrade JD Mix AI Engine to version {#MyAppVersion}.%n%nRunning audio software (FL Studio) will be gracefully refreshed.
