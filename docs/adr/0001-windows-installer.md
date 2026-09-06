# ADR 0001: Use Inno Setup for the Windows installer

- Status: Accepted
- Date: 2026-09-06

## Context

FinEleMethod already produces a verified portable Windows x64 folder and ZIP.
Public distribution also needs a familiar installation experience, runtime
prerequisite handling, shortcuts, Installed Apps registration, and clean
uninstallation. The application targets Windows and is built with MSVC.

## Decision

Use Inno Setup to produce the FinEleMethod 0.1.0 per-machine x64 installer.
Install under `Program Files`, publish as Anoop Patel, and bundle a Microsoft
Visual C++ x64 Redistributable at least as recent as the MSVC toolset used for
the application. Create a Start Menu shortcut, offer an unchecked desktop
shortcut, and offer to launch the GUI after installation.

The build helper must validate the staged application, Microsoft signature and
version of the redistributable, compiler inputs, and generated installer. The
installer must skip runtime installation when an equal or newer x64 runtime is
registered, propagate prerequisite failures, and support unattended testing.

## Consequences

- Users receive one offline installer and do not need development tools.
- Installation requires administrator approval and targets Windows 10 or later.
- The installer is larger because it contains the runtime prerequisite.
- Inno Setup becomes a release-build dependency, not an application runtime
  dependency.
- Code signing remains a separate release-hardening task. Until a certificate
  is obtained, Windows may identify the installer as an unknown publisher.
