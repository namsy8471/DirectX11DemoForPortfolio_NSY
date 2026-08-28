# Runtime Platform Architecture

The portfolio demo now treats rendering as one consumer of a small runtime
platform instead of using the render loop as the engine itself.

## Dependency direction

```text
PortfolioEngine.exe
├─ EngineRuntime.lib
│  └─ EngineCore.lib
├─ Platform.Win32 (currently part of the host executable)
├─ Renderer.D3D11 (currently part of the host executable)
└─ PortfolioGame

PortfolioRuntimeTests.exe
└─ EngineRuntime.lib
   └─ EngineCore.lib
```

`EngineCore` contains platform-neutral errors, results, logging, and ownership
helpers. `EngineRuntime` contains deterministic frame scheduling and the runtime
contracts consumed by the game. Neither static library includes `Windows.h` or a
DirectX header.

## Fixed-step frame contract

```text
PumpMessages
→ Measure real frame delta
→ Clamp stalls and accumulate time
→ FixedUpdate(1/60 second, zero or more times)
→ Render(interpolation alpha)
→ Present
```

The scheduler caps the amount of catch-up work performed by a frame. Any
discarded backlog is measured and logged, preventing a long stall from causing
an unbounded spiral of update work.

`FixedFrameContext` contains deterministic simulation time. `RenderFrameContext`
contains presentation time and the interpolation alpha; render frequency no
longer changes simulation speed.

## Failure contract

The application/game boundary returns `Result<T>` with a typed `Error`. An error
retains its subsystem, message, code, file, function, and line. New Core, Runtime,
Application, and PortfolioGame paths log the error and propagate it upward. The
remaining modal errors are isolated to legacy renderer shader-compilation helpers
and are tracked as the next error-model migration.

Logs are written to `logs/PortfolioEngine.log` relative to the process working
directory and contain a timestamp, severity, thread id, category, and message.

## Headless verification

`PortfolioRuntimeTests` runs without Win32, Direct3D, audio, or assets. It checks:

- invalid scheduler configuration and typed error propagation;
- long-frame clamping and the per-frame update budget;
- measured dropped simulation time;
- a 10,000-tick deterministic replay under different render-frame partitions;
- equality of the resulting 64-bit world hash.

Run the local verification pipeline from PowerShell:

```powershell
.\scripts\verify.ps1
```

Run all four architecture/configuration combinations:

```powershell
.\scripts\verify.ps1 -AllArchitectures
```

The next platform milestone is an asset boundary consisting of a virtual file
system, typed asset handles, a manifest, and a cooker. That work can build on the
error, log, independent runtime, and headless-test boundaries established here.
