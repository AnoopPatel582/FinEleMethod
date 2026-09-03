# FinEleMethod: start here

This guide is for the Windows x64 ZIP application, not for building the source.
You do not need Visual Studio, CMake, Git, or vcpkg to run the packaged binaries.
This is a development build; clean-machine and full interactive release
qualification are still pending. Independently validate results before relying
on them for engineering decisions.

## 1. Prepare the application

1. Obtain the ZIP and matching `.sha256` file from the same successful Windows
   CI run in the FinEleMethod repository. Do not mix files from different runs.
2. Optionally check the download in PowerShell using
   `Get-FileHash -Algorithm SHA256 -LiteralPath 'C:\Downloads\FinEleMethod-windows-x64.zip'`.
   Replace the example path with your actual download. Compare the hash with the
   first value in the matching `.sha256` file; stop if they differ. A checksum
   checks file integrity, not publisher identity.
3. In File Explorer, choose **Extract All**. Keep both executables and all DLLs
   together. Do not launch from inside the ZIP or copy just one executable.
4. Install the **x64 Microsoft Visual C++ v14 Redistributable** from
   [Microsoft's official download page](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist).
   The runtime must be at least as recent as the build tools used for the app.
   The ZIP does not install this prerequisite automatically.

Do not disable Windows security or download individual DLLs from third-party
sites to make the application start. If Windows blocks a build, keep the exact
message and verify its provenance before proceeding.

## 2. Run your first analysis in the GUI

1. Double-click `FinEleMethodGui.exe` in the extracted folder.
2. Click **Browse...** and select `examples\abaqus\q4_tension.inp` from that folder.
3. Confirm that the workbench identifies Q4 plane stress, four nodes, and one element.
4. Click **Create Project...**, enter a new name such as `FirstStudy`, and choose
   a writable parent folder for your study. Use a separate location from the
   application files so future application updates do not replace your work.
5. Click **Run Analysis**. After completion, the workbench shows the result
   summary and enables **Open Result** and **Open Run Folder**.
6. Open the run folder to find the run's `results` directory. Keep the whole
   study folder, including its input and runs, when backing up or moving it.

The supplied model is a verification example using consistent units. Its
right-edge X displacement is `0.01` and uniform X stress is `10.0`.
Automatic unit conversion is not implemented.

For plane strain, repeat with `q4_plane_strain_tension.inp`; for 3D, use
`h8_compression.inp`. Create a separate project for each input.

## 3. View results

Use ParaView to open the `.vtu` file in the run's results directory, then click
**Apply**. **Open Result** uses the Windows `.vtu` file association; if none is
configured, open the file from within ParaView instead. ParaView is a separate
application and is not included in this ZIP.

Select `Displacement`, `Stress`, `Strain`, or `VonMises` for coloring. Apply
**Warp By Vector** with `Displacement` to view deformation. A larger warp scale
only exaggerates the display; it does not change computed displacements.

## Optional: use the command-line solver

Open PowerShell in the extracted application folder, create an output directory,
and run:

```powershell
New-Item -ItemType Directory -Force .\results
.\FinEleMethod.exe --input .\examples\abaqus\q4_tension.inp `
  --output .\results\q4_tension.vtu `
  --summary .\results\q4_tension-summary.json
```

Choose fresh output names for later runs if you want to retain earlier results.
CLI mode writes the specified files directly; it does not create the GUI's
numbered project-run directories automatically.

## Troubleshooting and further reading

- Missing `z.dll` or another app-local DLL: extract the complete package again
  into a new folder. Never mix DLLs from different builds.
- Missing `VCRUNTIME140` or `MSVCP140` runtime components: check the official
  Microsoft runtime prerequisite above.
- No visible result: check that the run completed, open its actual VTU file in
  ParaView, click Apply, and reset the camera.
- Need to report a failure: retain the exact error, workflow run or commit,
  input model, and relevant run files. Review their contents before sharing.

The package's `docs` folder contains the beginner's guide, architecture,
formulations, benchmark reports, and GUI acceptance checklist. Those documents
also describe developer commands that require a source checkout; they are not
prerequisites for using this ZIP. The root `README.md` is the repository overview.
