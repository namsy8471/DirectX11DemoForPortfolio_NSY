<div align="center">

<h1>DirectX 11 Portfolio Runtime</h1>

<p><strong>From a graphics demo to a small, deterministic, and testable game runtime</strong></p>

<p>
  <a href="README.md">🇰🇷 한국어</a> ·
  <strong>🇺🇸 English</strong> ·
  <a href="README.ja.md">🇯🇵 日本語</a>
</p>

<p>
  <a href="https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/actions/workflows/runtime-verification.yml"><img alt="Runtime Verification" src="https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/actions/workflows/runtime-verification.yml/badge.svg"></a>
  <img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus">
  <img alt="DirectX 11" src="https://img.shields.io/badge/DirectX-11-107C10?logo=windows">
  <img alt="Windows" src="https://img.shields.io/badge/platform-Windows-0078D4?logo=windows">
</p>

<p>
  <a href="https://www.youtube.com/watch?v=YtkRcS8QO4E"><strong>▶ Watch the Demo</strong></a>
  &nbsp;·&nbsp;
  <a href="docs/runtime-platform.md">Runtime Design Document</a>
</p>

</div>

---

This project began as a graphics demo built to understand the DirectX 11 rendering pipeline firsthand. I later preserved the finished visual demo while redesigning its frame processing, error propagation, resource lifetime, and module boundaries. The result is a **runtime platform that treats graphics as one of its consumers**.

| Project type | Original development | Engine/runtime refactoring | Environment |
|:---:|:---:|:---:|:---:|
| **Solo project** | Oct–Dec 2023 | Aug 2026 | C++17 · VS 2022 · Windows 10/11 |

## Preview

| Noise Fire Effect | Terrain · Billboard · Instancing |
|:---:|:---:|
| ![Noise fire effect](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/bf59dafa-3dc4-4421-88ff-8e048d593696) | ![Terrain scene](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/f4b43db1-0b06-4a5e-a63b-c8b82e987963) |

## Key Results

| Metric | Result |
|---|---:|
| Build and verification matrix | **4/4 passed** — Debug/Release × x86/x64 |
| Headless runtime checks | **36/36 passed** — 9 checks per configuration |
| Determinism verification | Identical hash `0x45309AA8C1381D56` after 10,000 ticks with different render partitions |
| Win32/DirectX includes in Core and Runtime | **0** |
| Manual ownership sites in project code | **214 → 0** |
| `GraphicsClass` size | **1,879 → 1,141 LOC (-39.3%)** |
| C++ files in the `Graphics/` root | **74 → 4 (-94.6%)** |
| Direct error popups in `GraphicsClass` | **15 → 0** |

These numbers do not claim an FPS improvement. The purpose of this refactoring was to improve **structure, reproducibility, failure diagnostics, and testability**, not to optimize rendering performance.

## What This Project Demonstrates

### Graphics demo

- Direct3D 11 device, swap-chain, and render-state management
- HLSL Phong lighting with specular reflection
- Six-pass render-to-texture soft shadows
- Height-map terrain and a camera that follows terrain height
- Billboard rendering and instancing for 2,000 grass objects
- Fire built from noise, distortion, and alpha textures
- SkyDome, font HUD, mouse picking, and collision detection
- DirectInput 8 camera controls and DirectSound BGM/SFX
- Separate update and render paths for Title and Hunt states

### Runtime and platform engineering

- Separation of 60 Hz fixed-step simulation and render interpolation
- Platform-neutral `EngineCore` and `EngineRuntime` static libraries
- `Error` and `Result<T>` types that preserve failure context
- Structured logs containing timestamps, thread IDs, and categories
- RAII ownership using `ComPtr`, `unique_ptr`, and `vector`
- Shared texture contracts through `ITexture` and `IRenderTexture`
- Headless replay tests that require no GPU, window, audio, or assets
- PowerShell verification script and GitHub Actions automation

## Runtime Architecture

The original structure treated the render loop itself as the engine. The refactored architecture treats rendering as one consumer of a runtime platform.

```text
PortfolioEngine.exe
├─ EngineCore.lib
│  ├─ Error / Result<T>
│  ├─ Structured Log
│  └─ Ownership Utilities
├─ EngineRuntime.lib
│  └─ FixedStepScheduler
├─ Platform.Win32
│  └─ Application / Win32Window / Input
├─ Renderer.D3D11
│  └─ Device / Geometry / Pipelines / Shaders / Textures / UI
└─ PortfolioGame
   └─ GraphicsClass / SceneDefinition / GameObject

PortfolioRuntimeTests.exe
└─ EngineRuntime.lib
   └─ EngineCore.lib
```

| Build target | Responsibility | Platform/graphics dependency |
|---|---|---|
| `EngineCore.lib` | Errors, result types, logging, and ownership utilities | None |
| `EngineRuntime.lib` | Fixed timestep and simulation/render contracts | None |
| `PortfolioEngine.exe` | Win32 host, D3D11 renderer, and portfolio game | Win32, DirectX 11 |
| `PortfolioRuntimeTests.exe` | Headless determinism, error, and scheduler verification | None |

The project-reference direction is `PortfolioEngine → EngineRuntime → EngineCore`. Core and Runtime contain no `Windows.h` or DirectX headers, so they can be built and tested without a window or graphics device.

### Fixed-step frame contract

```text
PumpMessages
→ MeasureFrameTime
→ ClampAndAccumulate(max 100 ms)
→ FixedUpdate(1/60 s, up to 8 times per frame)
→ Render(interpolation alpha)
→ Present
```

Render FPS is now independent of simulation speed. A catch-up budget prevents unbounded updates after a long stall, and any discarded backlog is measured and written to the log.

### Errors and logging

The original `bool` returns reported only that something had failed, losing the subsystem and call-site context. The Application/Game boundary now uses `Result<T>` with an error code, subsystem, message, file, function, and line. Final error presentation is restricted to the Win32 launcher, while internal layers focus on recording and propagation.

```text
2026-08-28 ... [Error] [thread] [PortfolioGame] message (file:function:line)
```

Runtime logs are written to `logs/PortfolioEngine.log` relative to the process working directory.

### Texture contracts and ownership

DDS textures and render targets share one shader-resource-view contract. Consumers no longer need to know about the DDS loader, RTV/DSV details, or the internal `ComPtr` implementation.

```cpp
using namespace Engine::Rendering;

TexturePtr albedo = MakeTexture<DdsTexture>(
    device,
    L"data/stone01.dds");

RenderTexturePtr shadowMap = MakeTexture<RenderTargetTexture>(
    device,
    RenderTextureDescriptor{2048, 2048, 1000.0f, 0.1f});
```

Both types expose `GetShaderResourceView()`, while only `IRenderTexture` provides render-target operations. A failed creation returns an empty smart pointer, and any Direct3D resources created before the failure are released through RAII.

## Problem-Solving Cases

| Area changed | Problem and cause | Approach | Outcome | Quantitative result or verification |
|---|---|---|---|---|
| Build boundaries | A single executable compiled Core, Win32, D3D11, and game code together, so dependency direction could not be enforced | Split Core and Runtime into static libraries and created separate game and test executables | Runtime behavior can be verified without a graphics device | Targets **1 → 4**; platform includes in Core/Runtime **0** |
| Frame loop | Variable-delta updates coupled render FPS to game progression | Added a 60 Hz fixed step, 100 ms clamp, eight-tick budget, and interpolation value | Render frequency and simulation progress are independent | Matching hash after 10,000 ticks under two render partitions |
| Resource lifetime | Raw COM pointers and distributed `new/delete/Release()` calls made partial initialization and shutdown order fragile | Applied `ComPtr`, `unique_ptr`, `vector`, and scope guards | Ownership and destruction points are represented by types | Manual ownership sites **214 → 0** |
| Monolithic game class | Input, scene setup, render passes, audio, and error handling were concentrated in one class | Moved responsibilities into Application, Platform, Input, Scene, Pipeline, and Texture layers | Reduced change impact and navigation scope | `GraphicsClass` **-738 LOC (-39.3%)** |
| Animal models | OBJ files were absent from a clean clone, and a cube fallback hid the failure, so bears, turkeys, and horses appeared as boxes | Restored the source models, corrected coordinate conversion, and returned typed errors for missing required assets | Missing data now reports its real cause instead of rendering a false substitute | Restored **4 OBJ files** and **230,504 triangles** |
| Title fire | The alpha texture's A channel was always 255, but the shader sampled it, producing a rectangle | Applied the grayscale mask stored in the R channel to final alpha | Restored the flame silhouette while retaining one alpha-blend state | Uses **256 RGB levels** instead of **one A value** |
| Physical layout | Seventy-four C++ files were mixed in the `Graphics/` root, so paths did not match responsibilities | Moved files under `Core`, `Runtime`, `Platform`, `Scene`, `Rendering`, `Game`, and related folders | Paths now reveal responsibility and dependency layer | Root C++ files **74 → 4**; project/filter entries **122/122** |
| Regression verification | There was no repeatable procedure for configuration-specific builds and runtime behavior | Added `/W4 /WX`, headless tests, a PowerShell matrix, and CI | Local and CI environments execute the same process | Builds **4/4**; runtime checks **36/36** |

<details>
<summary><strong>View the Full Quantitative Breakdown</strong></summary>

### Quantitative Changes

The comparison uses the pre-refactoring commit `132e7ec` and the runtime-platform commit `f1fbe64`. LOC is a physical line count including blank lines, comments, and preprocessor directives.

| Metric | Before | After | Change |
|---|---:|---:|---:|
| `GraphicsClass` (`.cpp + .h`) | 1,879 LOC | 1,141 LOC | **-738 (-39.3%)** |
| `graphicsclass.cpp` | 1,694 LOC | 1,017 LOC | **-677 (-40.0%)** |
| Project-owned C++ | 79 files / 19,569 LOC | 99 files / 17,080 LOC | LOC **-2,489 (-12.7%)** |
| `SystemClass` | 2 files / 431 LOC | Removed | Launch and window responsibilities moved |
| C++ files in `Graphics/` root | 74 files | 4 files | **-70 (-94.6%)** |
| External/legacy integration files | 98 files / 1,218,891 bytes | Removed | Assimp, ImGui, and SystemClass removed |
| Explicit project/filter entries | Potentially inconsistent | 122 / 122 | No missing or extra entries |
| Runtime error popups in `GraphicsClass` | 15 | 0 | Propagated as typed results |
| Automated build matrix | None | 4 configurations | 0 warnings, 0 errors |
| Deterministic runtime checks | None | 36 / 36 | 10,000 ticks per configuration |

The manual-ownership metric is a call-site heuristic over project code. It excludes comments, `operator new/delete`, copy-prevention declarations using `= delete`, and the unchanged `DDSTextureLoader` and `AlignedAllocationPolicy`. It is a structural indicator, not a mathematical proof of memory safety.

</details>

## Build and Verification

### Requirements

- Windows 10 or Windows 11
- Visual Studio 2022 or newer
- The `Desktop development with C++` workload
- An MSVC toolset with C++17 support and a Windows SDK

### Run from Visual Studio

1. Open `Graphics.sln` from the repository root.
2. Select a `Debug/Release × x86/x64` configuration.
3. Set `PortfolioEngine` as the startup project.
4. Build and run the solution.

If the toolset recorded by the project is unavailable, retarget it to an installed MSVC toolset while preserving the C++17 setting. Runtime assets are copied to the output directory during the build.

### Automated verification

Run these commands in PowerShell from the repository root.

```powershell
# Debug/Release × x64
.\scripts\verify.ps1

# Debug/Release × x86/x64
.\scripts\verify.ps1 -AllArchitectures

# Override replay length
.\scripts\verify.ps1 -AllArchitectures -ReplayTicks 20000
```

The script locates MSBuild from Visual Studio, builds each configuration, and runs `PortfolioRuntimeTests.exe`. The default replay executes 10,000 ticks per configuration and requires no GPU, window, audio, or assets.

### Controls

| Input | Action |
|---|---|
| `W` / `A` / `S` / `D` | Move the camera forward, left, backward, and right |
| Mouse movement | Rotate the camera view |
| Left mouse button | Start from Title / select a model during Hunt |
| `Esc` | Exit |

<details>
<summary><strong>View the Directory Layout</strong></summary>

### Directory Layout

```text
Graphics.sln
├─ Graphics/
│  ├─ Engine/
│  │  ├─ Core/                 # Error, Result<T>, Log, ownership utilities
│  │  ├─ Runtime/              # FixedStepScheduler
│  │  ├─ Platform/             # Win32 window and native handle boundary
│  │  ├─ Input/                # DirectInput, snapshot, camera controller
│  │  ├─ Scene/                # Camera, light, object, scene definition
│  │  ├─ Audio/                # DirectSound
│  │  ├─ Diagnostics/          # Timer, FPS, CPU metrics
│  │  └─ Rendering/
│  │     ├─ Device/            # D3D11 device and state
│  │     ├─ Geometry/          # Model, terrain, foliage, screen geometry
│  │     ├─ Pipelines/         # SoftShadowPipeline
│  │     ├─ Shaders/           # Shader and constant-buffer wrappers
│  │     ├─ Textures/          # ITexture, DDS, render targets
│  │     └─ UI/                # Font and HUD
│  ├─ Game/                    # PortfolioGame implementation
│  └─ data/                    # HLSL, texture, model, terrain, audio assets
├─ Tests/RuntimeTests/         # Headless deterministic replay tests
├─ scripts/verify.ps1         # Local build/test matrix
├─ .github/workflows/         # CI verification
└─ docs/runtime-platform.md   # Runtime design details
```

</details>

## Scope and Remaining Work

This milestone deliberately avoided introducing an ECS, custom allocator, DLL plugin system, or separate render thread before the runtime foundation was proven. The current boundaries and limitations are documented explicitly.

- `Platform.Win32` and `Renderer.D3D11` are logically separated but still build inside the `PortfolioEngine` project.
- The new Application/Game boundary uses `Result<T>`, but some legacy shader-compilation helpers still contain modal errors.
- Assets still use relative `data/` paths. The next milestone is a VFS, `AssetId`, generation-based handles, a manifest, and a cooker.
- No before/after performance benchmark was run under identical conditions, so this project does not claim an FPS increase.

See [Runtime Platform Architecture](docs/runtime-platform.md) for the detailed frame contract, error model, and headless verification design.

## Background and Attribution

The original demo began with Rastertek examples as learning material for DirectX 11 and game-engine rendering internals. I extended it with Phong specular lighting, soft shadows, instancing, a fire effect, picking, and game states, then redesigned the completed result from a platform and runtime perspective. The current project runs without Assimp or ImGui by using its built-in OBJ loader and font HUD.
