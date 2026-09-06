# Windows installer

FinEleMethod 0.1.0 uses an Inno Setup per-machine x64 installer. It installs
the application under `Program Files`, registers an uninstaller in Windows,
creates a Start Menu shortcut, and offers an unchecked desktop shortcut. The
offline installer contains the required Microsoft Visual C++ x64 Runtime and
installs it only when an equal or newer runtime is not already registered.

The installer requires Windows 10 or later and an administrator's approval.
Visual Studio, CMake, vcpkg, and Inno Setup are not required on an end user's
computer.

## Build the installer

First build, test, and stage the Release application from Developer PowerShell
for Visual Studio:

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release
cmake --install .\out\build\windows-msvc `
  --config Release `
  --prefix .\out\install\windows-msvc-release
```

Install Inno Setup 6 on the development computer, then run:

```powershell
.\cmake\BuildWindowsInstaller.ps1
```

The helper verifies the staged solver and GUI in an isolated runtime
environment, checks that the application version is 0.1.0, verifies the
Microsoft signature and version of the bundled redistributable, compiles the
installer, and writes its SHA-256 checksum under `out\installer`.

Optional parameters allow release automation to provide explicit staged,
Release, compiler, redistributable, output, and version paths. The application
version and installer version must match.

## Install and verify

1. Copy the setup `.exe` and matching `.sha256` file to the test computer.
2. Compare `Get-FileHash -Algorithm SHA256` with the checksum file.
3. Run setup and approve the Windows administrator prompt.
4. Keep the default installation directory. Confirm the Start Menu shortcut;
   select the desktop shortcut only if wanted.
5. Launch FinEleMethod, open a supplied ABAQUS example, create a project, and
   complete one analysis.
6. In **Help -> About FinEleMethod**, confirm version 0.1.0, Release, and 64-bit.
7. In **Settings -> Apps -> Installed apps**, uninstall FinEleMethod and verify
   that its Program Files directory and shortcuts are removed.

Until the project obtains a code-signing certificate, Windows may display an
unknown-publisher warning for the installer. Never disable Windows security to
install it; distribute the checksum and source release provenance with every
installer.
