#ifndef AppVersion
  #error AppVersion must be provided by the installer build script.
#endif
#ifndef VersionInfoVersion
  #error VersionInfoVersion must be provided by the installer build script.
#endif
#ifndef StageDirectory
  #error StageDirectory must be provided by the installer build script.
#endif
#ifndef OutputDirectory
  #error OutputDirectory must be provided by the installer build script.
#endif
#ifndef VCRedistPath
  #error VCRedistPath must be provided by the installer build script.
#endif
#ifndef VCRuntimeMajor
  #error VCRuntimeMajor must be provided by the installer build script.
#endif
#ifndef VCRuntimeMinor
  #error VCRuntimeMinor must be provided by the installer build script.
#endif
#ifndef VCRuntimeBuild
  #error VCRuntimeBuild must be provided by the installer build script.
#endif
#ifndef VCRuntimeRevision
  #error VCRuntimeRevision must be provided by the installer build script.
#endif

#define AppName "FinEleMethod"
#define AppPublisher "Anoop Patel"
#define AppExeName "FinEleMethodGui.exe"
#define VCRedistName "vc_redist.x64.exe"

[Setup]
AppId={{99582020-4A6C-4849-82B1-382ADEB5F932}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL=https://github.com/AnoopPatel582/FinEleMethod
AppSupportURL=https://github.com/AnoopPatel582/FinEleMethod/issues
AppUpdatesURL=https://github.com/AnoopPatel582/FinEleMethod/releases
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
OutputDir={#OutputDirectory}
OutputBaseFilename=FinEleMethod-{#AppVersion}-windows-x64-setup
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#AppName} {#AppVersion}
UninstallDisplayIcon={app}\{#AppExeName}
VersionInfoVersion={#VersionInfoVersion}
VersionInfoCompany={#AppPublisher}
VersionInfoDescription={#AppName} Windows Installer
VersionInfoProductName={#AppName}
VersionInfoProductVersion={#VersionInfoVersion}
SetupLogging=yes
CloseApplications=yes
RestartApplications=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Additional shortcuts:"; Flags: unchecked

[Files]
Source: "{#StageDirectory}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#VCRedistPath}"; DestDir: "{tmp}"; DestName: "{#VCRedistName}"; Flags: dontcopy

[Icons]
Name: "{commonprograms}\{#AppName}"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"
Name: "{commondesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "Launch {#AppName}"; WorkingDir: "{app}"; Flags: postinstall nowait skipifsilent unchecked

[Code]
const
  VCRuntimeRegistryKey = 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64';

function RuntimeVersionAtLeastRequired(Major, Minor, Build, Revision: Cardinal): Boolean;
begin
  if Major <> {#VCRuntimeMajor} then
    Result := Major > {#VCRuntimeMajor}
  else if Minor <> {#VCRuntimeMinor} then
    Result := Minor > {#VCRuntimeMinor}
  else if Build <> {#VCRuntimeBuild} then
    Result := Build > {#VCRuntimeBuild}
  else
    Result := Revision >= {#VCRuntimeRevision};
end;

function RequiredVCRuntimeIsInstalled(): Boolean;
var
  Installed, Major, Minor, Build, Revision: Cardinal;
begin
  Result :=
    RegQueryDWordValue(HKLM64, VCRuntimeRegistryKey, 'Installed', Installed) and
    (Installed = 1) and
    RegQueryDWordValue(HKLM64, VCRuntimeRegistryKey, 'Major', Major) and
    RegQueryDWordValue(HKLM64, VCRuntimeRegistryKey, 'Minor', Minor) and
    RegQueryDWordValue(HKLM64, VCRuntimeRegistryKey, 'Bld', Build) and
    RegQueryDWordValue(HKLM64, VCRuntimeRegistryKey, 'Rbld', Revision) and
    RuntimeVersionAtLeastRequired(Major, Minor, Build, Revision);
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  ResultCode: Integer;
begin
  Result := '';
  if RequiredVCRuntimeIsInstalled() then
    Exit;

  ExtractTemporaryFile('{#VCRedistName}');
  if not Exec(
    ExpandConstant('{tmp}\{#VCRedistName}'),
    '/install /passive /norestart', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    Result := 'The Microsoft Visual C++ x64 Runtime could not be started.';
    Exit;
  end;

  case ResultCode of
    0, 1638:
      Result := '';
    1641, 3010:
      begin
        NeedsRestart := True;
        Result := '';
      end;
  else
    Result := 'Microsoft Visual C++ x64 Runtime installation failed with exit code ' +
      IntToStr(ResultCode) + '.';
  end;
end;
