# Spec: Making Notebook File Paths Automatic

**What problem this solves:** this project has existed as more than one copy on disk at once (a
Desktop working copy and a Documents copy, at different points in this project's history), and
every notebook/test file used to hardcode one specific absolute path — e.g.
`PacletDirectoryLoad["C:\\Users\\Student\\Documents\\...\\v2\\paclet"]`. Whichever
copy wasn't the hardcoded one would silently load the *other* copy's stale DLL, which is exactly
what caused a long, confusing debugging session earlier in this project's history (fixes were made
in one copy, tested from the other, and nothing seemed to change). Moving the project, renaming a
folder, or a teammate cloning it to a different path all break these hardcoded strings the same
way.

**The fix, in one sentence:** every notebook computes its own paclet/log/data paths from
`NotebookDirectory[]` (the folder the currently-running notebook file is actually sitting in)
instead of a hardcoded absolute string, so the same notebook works correctly no matter which copy
of the project it's opened from.

## Where it's implemented

Both notebooks live in `notebooks/`, one level below the project's `v2/` root
(`v2/notebooks/TestMachineControl.nb`), so `ParentDirectory[NotebookDirectory[]]` always
resolves to that `v2/` folder, wherever it happens to be on disk.

| File | Cell | Before | After |
|---|---|---|---|
| `notebooks/TestMachineControl.nb` | Paclet load | `PacletDirectoryLoad["C:\\...\\v2"]` (hardcoded) | `PacletDirectoryLoad[ParentDirectory[NotebookDirectory[]]]` |
| `notebooks/Record.nb` | Paclet load | `PacletDirectoryLoad["C:\\...\\v2"]` (hardcoded) | `PacletDirectoryLoad[ParentDirectory[NotebookDirectory[]]]` |
| `notebooks/Record.nb` | Enable logging | `SetLogSettings[1, "C:\\...\\v2\\log.txt"]` (hardcoded) | `SetLogSettings[1, FileNameJoin[{ParentDirectory[NotebookDirectory[]], "log.txt"}]]` |
| `notebooks/Record.nb` | Export recorded data | `Export["C:\\...\\data\\data.csv", dataLog]` (hardcoded, and not even under `v2/`) | `Export[FileNameJoin[{ParentDirectory[NotebookDirectory[]], "data", "data.csv"}], dataLog]` |
| `notebooks/TestMachineControl.nb` | Run the `.wlt` suite | `TestReport["C:\\...\\v2\\paclet\\Tests\\Test_WolframMachineControl.wlt"]` (hardcoded) | `TestReport[FileNameJoin[{ParentDirectory[NotebookDirectory[]], "paclet", "Tests", "Test_WolframMachineControl.wlt"}]]` |

`src/lab.cpp`'s default `LOG_FILE` and `Test_WolframMachineControl.wlt`'s `LogFile`/
`PacletDirectoryLoad` line are **not** dynamic — those are set once, at C++ compile time or by a
plain Wolfram script (no notebook context to ask `NotebookDirectory[]` about), so they're just
hardcoded to whichever copy is currently the canonical one instead.

## How it works

- `NotebookDirectory[]` returns the folder containing whichever notebook is currently evaluating
  the cell that calls it — computed at evaluation time, not baked in when the file was saved.
- `ParentDirectory[...]` walks up one level, from `.../v2/notebooks` to `.../v2`.
- `FileNameJoin[{...}]` builds a path from parts in an OS-appropriate way (backslashes on Windows,
  forward slashes elsewhere) instead of concatenating a hardcoded separator.

None of this requires knowing the username, the drive letter, or which of the project's copies is
open — it just asks "where am I right now?" and builds every other path relative to that answer.

## What this does **not** solve

- **It only helps notebooks.** `.wl`/`.wlt` files evaluated outside a notebook context (no
  associated `NotebookObject`) can't call `NotebookDirectory[]` — `Test_WolframMachineControl.wlt`
  and `src/lab.cpp`'s compiled-in default log path are still hardcoded strings, updated by hand
  when the canonical copy changes.
- **It doesn't resolve which copy is canonical for you** — it just means that once you *do* decide
  (open the "real" one), every path in that notebook session automatically follows along. If two
  copies of the project both exist and both get opened, each one's notebook still only knows about
  itself, not the other.
- **`v2/data/` must already exist.** `Export[...]` does not create missing parent directories, so
  the target folder for recorded CSV output has to be created once ahead of time.
