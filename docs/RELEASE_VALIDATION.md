# Reproducing Windows package validation

Run these checks from the source checkout in PowerShell 7. They exercise the
packaged executable, not just the in-process C++ tests. They do not replace
the [interactive GUI checklist](GUI_ACCEPTANCE.md) or clean-machine testing.

## Automated layers

| Check | Evidence required |
| --- | --- |
| Archive safety | ZIP round-trip, matching checksum, existing package preservation, invalid destination rejection |
| VTU field validation | Exactly one field, expected component/value counts, ASCII Float64 encoding, finite numeric data |
| Direct packaged solve | Release x64 build, valid summary paths/counts, all three analytical example solutions |
| Request-mode packaged solve | Unrelated working directory, paths with spaces, four ordered progress events, completed state and correct results |
| Unsuccessful request | Pre-execution cancellation, missing input, and unsupported protocol return the expected exit/state without completion outputs |
| App-local deployment | DLL-set comparison and workbench startup without developer paths |

The finite-element implementation remains custom C++. The PowerShell analytical
comparisons are test oracles for three fixed examples, not an alternative solver.

## Commands

First build and stage Release using the [Beginner's Guide](BEGINNERS_GUIDE.md).
Then run the independent validation-helper regression checks:

```powershell
.\cmake\TestWindowsArchive.ps1
.\cmake\TestVtuVerification.ps1
.\cmake\TestPackagedProgress.ps1
```

Run the complete staged-application check:

```powershell
.\cmake\VerifyStagedApplication.ps1 `
  -StageDirectory .\out\install\windows-msvc-release `
  -ReleaseDirectory .\out\build\windows-msvc\Release
```

This calls both `VerifyStagedSolver.ps1` and `VerifyStagedRequest.ps1`. CI runs
the same check again after ZIP creation, checksum verification, and extraction.
The request tests retain `stdout.jsonl`, `stderr.txt`, and any run outputs under
a unique temporary directory printed in the log. Failed tests throw, stopping
the CI step; retain their output for diagnosis.

## Analytical package oracles

All three examples have Young's modulus `1000`, Poisson's ratio `0.25`, and
unit dimensions. Q4 combines concentrated loads and edge pressure for total
X load `10`; H8 has inward top-face pressure `10`.

| Example | Axial displacement | Relevant stress | Total reaction |
| --- | ---: | --- | --- |
| Q4 plane stress | Right-edge X: `0.01` | X: `10` | X: `-10` |
| Q4 plane strain | Right-edge X: `0.009375` | X: `10`, Z: `2.5` | X: `-10` |
| H8 compression | Top-face Z: `-0.01` | Z: `-10` | Z: `10` |

The checker compares every nodal displacement, every exported cell stress and
strain component, all three principal stresses, von Mises stress, and summed
reactions. For each value, its tolerance is `1e-8 * max(1, abs(expected))`.
These homogeneous examples are distinct from mesh-dependent beam and hole
benchmarks. The latter retain their documented acceptance criteria.

Cancellation is deliberately requested before launch for a deterministic check.
This proves the packaged cancellation protocol, not GUI button responsiveness or
mid-solve cancellation latency. An invalid request version is rejected before a
run-state file is required; valid requests persist failure/cancellation state.

## Record before handing off a build

Record the source commit, workflow URL and result, ZIP checksum, build diagnostics,
test configuration/counts, and the exact cases run. Keep **Pass**, **Fail**,
**Blocked**, and **Not run** distinct. A green CI run is not proof that every
user interaction or engineering model is correct.
