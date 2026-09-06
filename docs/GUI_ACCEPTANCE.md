# Windows GUI acceptance checklist

This checklist complements CTest and CI. A successful build, unit test, or GUI
startup check is not evidence that a dialog interaction passed. Run these cases
against the exact build intended for distribution.

## Test setup and evidence

- Record the source commit, Debug or Release configuration, Windows version,
  display resolution, and scaling setting.
- Use disposable projects under `out/` in a source checkout, or a dedicated
  writable test folder when checking a distribution. Do not alter a real study.
- Use the supplied `q4_tension.inp`, `q4_plane_strain_tension.inp`, and
  `h8_compression.inp` examples. Follow the [Beginner's Guide](BEGINNERS_GUIDE.md)
  for creating, opening, recovering, and running projects.
- Record each case as **Pass**, **Fail**, **Blocked**, or **Not run**, with the
  actual result and any relevant output files or screenshot evidence.
- Recheck affected cases after a code change. Do not transfer a pass from Debug
  to Release or from a developer machine to a clean Windows machine.

## Cases

| ID | Action | Required result |
| --- | --- | --- |
| G01 | Launch the workbench without a project | Main window opens; analysis and result controls are disabled; no missing-DLL error |
| G02 | Reduce the window size and scroll; repeat at the target Windows scaling | Every control remains reachable, including Close and Cancel Analysis |
| G03 | Browse to each supported example | Correct formulation and model counts appear before solving |
| G04 | Cancel Browse or select an invalid input while a project is open | Existing project and run selection remain available; invalid input produces a readable error |
| G05 | Create a disposable project, then close and reopen its JSON | Copied input and project identity are retained; no existing project is overwritten |
| G06 | Run Q4 plane stress, Q4 plane strain, and H8 projects | GUI stays responsive; each run has its own snapshot, request, state, summary, and VTU output; successful completion enables Open Result |
| G07 | Run again, then select the earlier completed run | A new numbered directory is created; the earlier run remains unchanged and its summary is restored |
| G08 | Open Result and Open Run Folder | Correct selected-run output opens in the associated viewer and the correct folder opens in Explorer |
| G09 | Refresh history after adding a valid run through another application | New run appears without losing an existing matching selection |
| G10 | Temporarily move only the disposable project's runs directory aside, then Refresh | Refresh reports an error and retains the last loaded list; restore the directory afterward |
| G11 | While a sufficiently long analysis is active, request cancellation | GUI remains responsive; an acknowledged cancellation is distinct from failure and does not enable a completed result |
| G12 | Attempt normal close while analysis is active | Close is vetoed and the analysis remains active; controls recover after completion or cancellation |
| G13 | Create a recovery snapshot, then try to replace it and choose No | Snapshot remains unchanged |
| G14 | Recover a snapshot while the disposable main JSON is missing or invalid | Recovery validates independently; title contains `[recovered, not saved]`; main JSON remains unchanged |
| G15 | With recovered metadata unsaved, close and choose No; repeat using Enter on the default choice | Window stays open with the recovered project and marker intact |
| G16 | With recovered metadata unsaved, select another project, input, or recovery snapshot; decline the leave warning for each path | Current project remains active and marked unsaved; no snapshot is removed |
| G17 | With recovered metadata unsaved, confirm leaving without saving | Requested close or switch occurs; original main JSON and snapshot are unchanged |
| G18 | Save recovered metadata | Main JSON is written, marker clears, and obsolete snapshot is removed; existing main JSON is backed up |
| G19 | Cause a save failure on a disposable project, for example by temporarily moving its input directory aside | Error is reported; unsaved marker and snapshot remain; restore input before retrying |
| G20 | Save successfully but prevent obsolete snapshot removal in a controlled fixture | Main JSON is saved and marker clears; a cleanup warning is shown instead of a save-failure message |
| G21 | Temporarily make disposable run history inaccessible as a solver finishes | Completion handling must not throw out of the event handler or leave the workbench controls disabled |

Small examples may finish before Cancel can be clicked. Record that attempt as
**Not run**, not as a cancellation pass. A controlled longer-running fixture is
needed. Do not manufacture a pass by terminating the solver externally.

Do not disable security software or change system-wide permissions to create
error cases. File manipulations above apply only to disposable test fixtures.

## Recorded check: 2026-09-03

Build: MSVC Debug, source commit `304db8d`.

- **G01: Pass.** A fresh workbench opened with the expected controls and no
  missing-DLL dialog.
- **G14: Blocked before recovery.** The recovery file picker opened, but the
  desktop automation helper rejected an observed element as unavailable and
  then returned no focused-element information after refreshing. No snapshot
  was loaded. This is a test-tool limitation, not proof of a GUI defect or pass.
- **All other cases: Not run in this session.** The 501 passing automated tests
  from this build do not replace these interactive checks.

The isolated fixture is under `out/recovery-smoke-20260903/RecoveryCheck/` in
the development checkout. It is generated test data and is not distributed.

## Completion-history finding: code fix, interactive verification pending

Source review found that `MainFrame::analysis_finished` refreshed history before
restoring controls without catching filesystem exceptions. It now uses the
tested `restore_after_analysis` boundary: controls are restored first, and a
standard exception from history refresh is returned as a warning. Success,
failure, and cancellation handling continue independently. The existing history
list remains available; a stale-history label and the outcome dialog report the
warning, and Refresh can retry the disk read.

Automated regression tests exercise ordering and injected history failures.
G21 still requires an interactive pass on the release candidate. A GUI crash
was not reproduced during the original source review.

## Recorded check: 2026-09-05 - active cancellation

Build: staged Windows acceptance executable in
`out/install/gui-acceptance-7c54a2f/`, source commit
`7c54a2f70a5b3805da6cd44736e37961c7382d4f`.

- **G11: Not passed; cancellation finding reproduced.** Using native GUI
  control, started a disposable 160,000-element Q4 plane-stress analysis and
  clicked Cancel Analysis while the run was active. The request flag was
  written at 10:11:54 local time, but the terminal state was written at
  10:12:07 as `failed`, with "Sparse static system did not converge within
  the iteration limit." The GUI displayed Analysis failed, restored Run
  Analysis, and kept Open Result disabled. No result file was produced.
- Evidence: `out/gui-acceptance-fixtures/CancellationSymmetry/runs/run-0001/`
  contains `cancellation-requested.flag` and `analysis-state.json`. Fixtures
  are ignored development data, not distribution examples.
- A standalone run of this large fixture also reached the iteration limit;
  its nonconvergence is separate from the cancellation-handling finding.
- Source inspection shows cancellation checks before and after the solver
  call, not inside the numerical solve. An exception from that call reaches
  failure handling before the post-solve cancellation check. Cancellation
  during numerical execution needs remediation and an interactive retest;
  this attempt must not be counted as a cancellation pass.
- G12-G21 were not executed in this session. No solver code was changed.

### G11 remediation implemented; GUI retest pending

The numerical solver now accepts an optional cancellation callback and checks
it before and between Conjugate Gradient iterations. Q4 and H8 analysis entry
points propagate that callback, and the CLI maps its dedicated cancellation
exception to the `cancelled` lifecycle state and exit code 6.

Focused Q4, H8, Conjugate Gradient, and CLI regression tests pass. A direct
process test using the same 160,000-element fixture placed the cancellation
flag 1.5 seconds after launch and ended with exit code 6, terminal state
`cancelled`, and no result or summary file. G11 remains **not passed** until the
updated GUI package completes the interactive acceptance procedure.

### G11 interactive retest: pass on 2026-09-06

Build: staged Windows acceptance executable in
`out/install/gui-acceptance-c4a343b/`, source commit `c4a343b`.

- **G11: Pass.** The user opened the repaired `CancellationSymmetry` fixture,
  started an analysis, requested cancellation while it was active, and received
  both "Analysis cancelled" and confirmation that the analysis was cancelled
  cooperatively. This verifies the corrected GUI-to-CLI cancellation path.
- **G12: Pass.** The user attempted to close the workbench while an analysis
  was active. The close was vetoed, the analysis remained active, and the
  controls recovered after cooperative cancellation.
- **G13: Pass.** The user created a recovery snapshot, selected Create Recovery
  Snapshot again, and chose No at the replacement confirmation. Before-and-after
  checks showed the same 100-byte size, UTC modification timestamp, and SHA-256
  hash, confirming that the existing snapshot remained unchanged.
- **G14: Pass.** The disposable main project JSON was moved to a preserved
  evidence directory before recovery. The user loaded the standalone autosave
  and confirmed the `[recovered, not saved]` title and recovery status message.
  A filesystem check confirmed that the main JSON remained missing while the
  autosave, original JSON, model input, and runs remained intact.
- **G15: Pass.** With recovered metadata unsaved, the user attempted to close
  the workbench and chose No; the window remained open with the recovered
  project and marker intact. The user repeated the close attempt and pressed
  Enter, confirming that the safe default No choice produced the same result.
- **G16: Pass.** With `CancellationSymmetry` recovered and unsaved, the user
  separately attempted to open another project, select another ABAQUS input,
  and load another recovery snapshot. Choosing No at each leave warning kept
  the current recovered project active and marked unsaved. Filesystem checks
  confirmed that both recovery snapshots remained present and the current main
  JSON remained missing.
- **G17: Pass.** The user confirmed leaving the unsaved recovered
  `CancellationSymmetry` metadata and successfully switched to the alternate
  `CancellationCheck` project. The recovered marker cleared. Hash checks
  confirmed that the abandoned project's autosave and preserved original JSON
  remained unchanged, while its main JSON remained missing as prepared.

## Release gates beyond this checklist

The current [release-validation procedure](RELEASE_VALIDATION.md) adds finite
field/cardinality checks, analytical package comparisons, successful request
lifecycles, and pre-execution cancellation and failure cases. These are automated
CLI/process checks; they do not change the recorded interactive statuses above.

The automated package check now runs all three supported analysis formulations
from both the staged directory and extracted ZIP with developer paths removed
from the solver environment. It checks exit status, summary identity/paths/counts,
and VTU mesh counts and field names. On 2026-09-03 these checks passed locally for
a fresh Release build based on `ffb0932`; a package missing the H8 example was
rejected. This is not an interactive G06 pass or a clean-machine qualification.

- Successful Windows CI for the exact release commit, including staged and
  extracted-ZIP checks.
- A Release-build interactive pass, including recovery, failure, and
  cancellation paths, with no unresolved blocking findings.
- A clean Windows-machine check without Visual Studio/vcpkg developer paths;
  document and verify the Microsoft Visual C++ Runtime prerequisite.
- Final benchmark evidence, documentation consistency, distribution setup, and
  the required 15-minute demonstration video.

The website, embedded visualization, model editing, and automatic unit
conversion remain separate future work. They are not prerequisites for these
initial Windows release checks.
