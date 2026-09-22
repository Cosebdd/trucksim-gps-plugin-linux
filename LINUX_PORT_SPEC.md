# Linux Port Spec — trucksim-gps-plugin

Authoritative, resumable plan for adding Linux support to the telemetry plugin.
Must survive full loss of conversation context.

## 0. Binding principles (apply to every change and review)

- SOLID, DRY, YAGNI, KISS, Clean Code, Pragmatic Programmer.
- **No comments** in code I add — self-descriptive naming only; the sole exception
  is a genuinely counter-intuitive hack. Do not scrub pre-existing vendor comments
  (out of scope, risky).
- **Atomic commits**: one logical change each, human-reviewable, never a whole phase
  in one commit. Cycle = atomic change → review → commit.
- **Preserve the Windows build.** Unlike the server, the Windows plugin is in active
  use. ADD Linux support via `#ifdef _WIN32 / #else`; never replace the Windows path.
- Token-resilient: keep commits small; this spec + committed increments are recovery.

## 1. Context

The plugin is a native SCS-SDK telemetry plugin loaded by ETS2/ATS. It reads
telemetry channels and writes a packed struct into OS shared memory that the
companion server reads. Today it is Windows-only (Visual Studio, Win32 MMF).

**Contract (must stay identical so the server decodes it unchanged):**
segment name `TSGPSTelemetry`, size `32*1024`, struct layout in
`inc/scs-telemetry-common.hpp`. Server maps `/dev/shm/TSGPSTelemetry` (32 KB).

**SCS SDK supports Linux**: `scs_sdk/include/scssdk.h` has a `#elif defined(__GNUG__)`
branch (empty `SCSAPIFUNC`). Exports use a Windows `.def` on VS; on Linux they come
from default symbol visibility. No secure-CRT (`*_s`) usage; handlers are portable.

## 2. Port surface (everything else is platform-agnostic and untouched)

| File | Change |
|------|--------|
| `scs-telemetry/inc/sharedmemory.hpp` | platform name typedef + `#ifdef` members; `<windows.h>` only under `_WIN32` |
| `scs-telemetry/src/sharedmemory.cpp` | add POSIX `shm_open`+`ftruncate`+`mmap`/`munmap`+`shm_unlink` branch |
| `scs-telemetry/inc/scs-telemetry-common.hpp` | `#ifdef` the MMF name: Windows `TEXT("Local\\TSGPSTelemetry")` vs Linux `"/TSGPSTelemetry"` |
| `scs-telemetry/src/scs_telemetry.cpp` | guard `<windows.h>`/`WINVER`/`DllMain`/`scs_mmf_name` type under `_WIN32` |
| `CMakeLists.txt` (new) | build `scs-telemetry.so` for `linux_x64` (`-shared -fPIC`, default visibility, `-lrt`) |

**Deliberately skipped (YAGNI):** `src/log.cpp` is entirely `#if LOGGING` and LOGGING
is off by default → compiles to nothing on Linux. Port only if logging is enabled later.

## 3. Development cycles (each = atomic commit, Windows build preserved)

- **P0** docs: this spec. (branch `linux-port`)
- **P1** POSIX shared memory in `sharedmemory.hpp/.cpp` behind `#ifdef _WIN32/#else`;
  introduce `shm_name_t` typedef; Windows path byte-for-byte unchanged.
- **P2** `#ifdef` the `SCS_PLUGIN_MMF_NAME` macro (Linux `"/TSGPSTelemetry"`).
- **P3** guard `scs_telemetry.cpp`: `<windows.h>`+`WINVER` include block, `DllMain`,
  and the `scs_mmf_name` declaration type, all under `_WIN32`.
- **P4** add `CMakeLists.txt`; **milestone: `cmake` + `make` build `scs-telemetry.so`
  on Linux.** Verify: builds clean; `nm -D` shows `scs_telemetry_init`/`_shutdown`
  exported; code path targets `/dev/shm/TSGPSTelemetry` at 32 KB.
- **P5** docs: README Linux build/install section (`bin/linux_x64/plugins/`).

## 4. Review checklist (every cycle)

1. Principles honoured (Section 0); Windows path unchanged (diff shows only additions
   under `#else`/`#ifndef _WIN32`).
2. Atomic, human-reviewable; message states the why.
3. Linux build green from P4; `scs_telemetry_init`/`scs_telemetry_shutdown` exported.
4. Shared-memory name/size still match the server contract (`TSGPSTelemetry`, 32 KB).

## 5. Status log

- P0 done — branch `linux-port`; spec added.
- P1 done — POSIX shared memory (`shm_open`/`ftruncate`/`mmap`) behind `#ifdef`.
- P2 done — Linux MMF name `/TSGPSTelemetry`.
- P3 done — guarded `scs_telemetry.cpp` (windows.h/DllMain/name type); portable
  `vsnprintf`; added `<cstdio>`/`<cstring>`.
- P4 done — `CMakeLists.txt`; **`.so` builds**. Verified: `nm -D` exports
  `scs_telemetry_init`/`scs_telemetry_shutdown` unmangled; `/TSGPSTelemetry` +
  `shm_open`/`mmap`/`ftruncate` linked. Windows build untouched.
- P5 done — README Linux build/install (`bin/linux_x64/plugins/`).

**LINUX PLUGIN PORT COMPLETE on branch `linux-port`** (not merged/pushed). Pairs with
the server's `linux-port` branch: plugin writes `/dev/shm/TSGPSTelemetry` (32 KB),
server reads it. See §6 for the one remaining validation gap.

## 6. Known validation gap

Full runtime validation needs the native Linux game loading the `.so` and calling
the SDK callbacks. Achievable here: compile, export-symbol check, code review, and
confirming the segment name/size match the server. True end-to-end needs the game.
