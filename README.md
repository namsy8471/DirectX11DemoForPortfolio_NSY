<div align="center">

<h1>DirectX 11 Portfolio Runtime</h1>

<p><strong>그래픽 데모를 결정론적이고 테스트 가능한 소형 게임 런타임으로</strong></p>

<p>
  <strong>🇰🇷 한국어</strong> ·
  <a href="README.en.md">🇺🇸 English</a> ·
  <a href="README.ja.md">🇯🇵 日本語</a>
</p>

<p>
  <a href="https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/actions/workflows/runtime-verification.yml"><img alt="Runtime Verification" src="https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/actions/workflows/runtime-verification.yml/badge.svg"></a>
  <img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus">
  <img alt="DirectX 11" src="https://img.shields.io/badge/DirectX-11-107C10?logo=windows">
  <img alt="Windows" src="https://img.shields.io/badge/platform-Windows-0078D4?logo=windows">
</p>

<p>
  <a href="https://www.youtube.com/watch?v=YtkRcS8QO4E"><strong>▶ 데모 영상 보기</strong></a>
  &nbsp;·&nbsp;
  <a href="docs/runtime-platform.md">런타임 설계 문서</a>
</p>

</div>

---

처음에는 DirectX 11의 렌더링 파이프라인을 직접 이해하기 위해 만든 그래픽스 데모였습니다. 이후 완성된 데모의 결과물은 유지하면서, 프레임 처리·오류 전달·자원 수명·모듈 경계를 다시 설계해 **그래픽 기능을 사용하는 런타임 플랫폼**으로 발전시켰습니다.

| 개발 형태 | 원본 개발 | 엔진·런타임 리팩터링 | 기술 환경 |
|:---:|:---:|:---:|:---:|
| **1인 개인 프로젝트** | 2023.10–2023.12 | 2026.08 | C++17 · VS 2022 · Windows 10/11 |

## Preview

| Noise Fire Effect | Terrain · Billboard · Instancing |
|:---:|:---:|
| ![Noise fire effect](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/bf59dafa-3dc4-4421-88ff-8e048d593696) | ![Terrain scene](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/f4b43db1-0b06-4a5e-a63b-c8b82e987963) |

## 핵심 성과

| 지표 | 결과 |
|---|---:|
| 빌드·검증 행렬 | **4/4 성공** — Debug/Release × x86/x64 |
| Headless 런타임 검사 | **36/36 통과** — 구성별 9개 |
| 결정론 검증 | 서로 다른 렌더 분할로 10,000 tick 실행 후 동일 hash `0x45309AA8C1381D56` |
| Core/Runtime의 Win32·DirectX include | **0개** |
| 프로젝트 코드의 수동 ownership site | **214곳 → 0곳** |
| `GraphicsClass` 크기 | **1,879 → 1,141 LOC (-39.3%)** |
| `Graphics/` 루트 C++ 파일 | **74 → 4개 (-94.6%)** |
| `GraphicsClass` 직접 오류 팝업 | **15 → 0곳** |

이 수치는 FPS 개선을 의미하지 않습니다. 이번 리팩터링의 목표는 성능 최적화가 아니라 **구조, 재현성, 실패 진단, 테스트 가능성**을 높이는 것이었습니다.

## 무엇을 보여주는 프로젝트인가

### 그래픽스 데모

- Direct3D 11 장치, 스왑 체인과 렌더 상태 관리
- HLSL 기반 Phong 조명과 스페큘러 반사
- 6-pass render-to-texture 소프트 섀도우
- Height map 기반 지형과 지형 높이 추종 카메라
- Billboard와 instancing을 이용한 풀 2,000개 렌더링
- Noise·distortion·alpha texture 기반 불꽃 효과
- SkyDome, 폰트 HUD, 마우스 피킹과 충돌 판정
- DirectInput 8 카메라 조작과 DirectSound BGM/SFX
- Title/Hunt 상태에 따른 업데이트·렌더 경로 전환

### 런타임·플랫폼 엔지니어링

- 60 Hz fixed-step simulation과 render interpolation 분리
- 플랫폼 독립 `EngineCore`·`EngineRuntime` 정적 라이브러리
- 오류 문맥을 보존하는 `Error`와 `Result<T>`
- timestamp·thread id·category를 포함하는 구조화 로그
- `ComPtr`, `unique_ptr`, `vector` 기반 RAII 소유권
- `ITexture`·`IRenderTexture` 기반 텍스처 소비 계약
- GPU·창·오디오·에셋 없이 실행되는 headless replay test
- PowerShell 검증 스크립트와 GitHub Actions 빌드 자동화

## 런타임 아키텍처

렌더 루프 자체를 엔진으로 취급하던 구조에서, 렌더링을 런타임의 한 소비자로 두는 구조로 변경했습니다.

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

| 빌드 타깃 | 책임 | 플랫폼·그래픽 의존성 |
|---|---|---|
| `EngineCore.lib` | 오류, 결과 타입, 로그, 소유권 유틸리티 | 없음 |
| `EngineRuntime.lib` | 고정 timestep과 simulation/render 계약 | 없음 |
| `PortfolioEngine.exe` | Win32 호스트, D3D11 렌더러, 포트폴리오 게임 | Win32, DirectX 11 |
| `PortfolioRuntimeTests.exe` | 결정론·오류·스케줄러 headless 검증 | 없음 |

프로젝트 참조 방향은 `PortfolioEngine → EngineRuntime → EngineCore`입니다. Core와 Runtime에는 `Windows.h` 또는 DirectX 헤더가 포함되지 않으므로 창과 그래픽 장치 없이 독립 빌드·테스트할 수 있습니다.

### Fixed-step frame contract

```text
PumpMessages
→ MeasureFrameTime
→ ClampAndAccumulate(max 100 ms)
→ FixedUpdate(1/60 s, frame당 최대 8회)
→ Render(interpolation alpha)
→ Present
```

렌더 FPS와 시뮬레이션 속도를 분리했습니다. 긴 정지 이후에도 한 프레임에서 무한히 update하지 않도록 catch-up budget을 제한하고, 버린 backlog는 로그에 기록합니다.

### 오류와 로그

기존 `bool` 반환은 실패 여부만 전달해 어느 하위 시스템과 호출 위치에서 문제가 생겼는지 잃어버렸습니다. 현재 Application/Game 경계는 오류 코드, subsystem, message, file, function, line을 가진 `Result<T>`를 사용합니다. 최종 오류 표시는 Win32 launcher에만 남기고 내부 계층은 기록과 전달에 집중합니다.

```text
2026-08-28 ... [Error] [thread] [PortfolioGame] message (file:function:line)
```

실행 로그는 프로세스 작업 디렉터리의 `logs/PortfolioEngine.log`에 기록됩니다.

### 텍스처 계약과 소유권

DDS 텍스처와 render target이 공통 SRV 계약을 사용하도록 구성했습니다. 소비자는 DDS 로더, RTV/DSV 또는 내부 `ComPtr` 구현을 알 필요가 없습니다.

```cpp
using namespace Engine::Rendering;

TexturePtr albedo = MakeTexture<DdsTexture>(
    device,
    L"data/stone01.dds");

RenderTexturePtr shadowMap = MakeTexture<RenderTargetTexture>(
    device,
    RenderTextureDescriptor{2048, 2048, 1000.0f, 0.1f});
```

두 타입 모두 `GetShaderResourceView()`를 제공하고, `IRenderTexture`만 render target 전용 동작을 노출합니다. 생성 실패 시 빈 스마트 포인터를 반환하며 이미 생성한 Direct3D 자원은 RAII로 정리됩니다.

## 문제 해결 사례

| 바뀐 부분 | 문제와 원인 | 변경 방식 | 해결된 점 | 정량적 결과·검증 |
|---|---|---|---|---|
| 빌드 경계 | 단일 실행 프로젝트가 Core, Win32, D3D11, 게임을 모두 컴파일해 의존성 방향을 강제할 수 없었음 | Core·Runtime 정적 라이브러리와 게임·테스트 실행 파일로 분리 | 그래픽 장치 없는 런타임 검증 가능 | 타깃 **1 → 4개**, Core/Runtime 플랫폼 include **0개** |
| 프레임 루프 | 가변 delta update로 렌더 FPS와 게임 진행 속도가 결합됨 | 60 Hz fixed step, 100 ms clamp, 최대 8 tick과 보간값 도입 | 렌더 빈도와 simulation 진행 분리 | 두 render partition에서 10,000 tick hash 일치 |
| 자원 수명 | raw COM pointer와 `new/delete/Release()`가 분산되어 부분 초기화 실패와 종료 순서에 취약 | `ComPtr`, `unique_ptr`, `vector`, scope guard 적용 | 소유자와 해제 시점이 타입으로 드러남 | 수동 ownership site **214 → 0곳** |
| 거대 게임 클래스 | 입력, 씬, 렌더 패스, 오디오와 오류 처리가 한 클래스에 집중 | Application, Platform, Input, Scene, Pipeline, Texture 계층으로 책임 이동 | 변경 영향 범위와 탐색 범위 축소 | `GraphicsClass` **-738 LOC (-39.3%)** |
| 동물 모델 | clean clone에서 OBJ가 누락되고 cube fallback이 실패를 숨겨 곰·칠면조·말이 상자로 표시됨 | 원본 모델 복구, 좌표 변환 수정, 필수 asset 누락 시 typed error 반환 | 잘못된 대체 모델 대신 원인을 즉시 확인 | OBJ **4개**, **230,504 triangles** 복구 |
| Title 불꽃 | alpha texture의 A 채널이 전부 255인데 셰이더가 A를 읽어 사각형으로 출력 | grayscale mask가 저장된 R 채널을 최종 alpha에 적용 | 단일 alpha blend state를 유지하며 불꽃 외곽 투명도 복원 | A 고유값 **1개** 대신 RGB **256단계** 사용 |
| 파일 구조 | C++ 파일 74개가 `Graphics/` 루트에 섞여 실제 책임과 경로가 불일치 | `Core`, `Runtime`, `Platform`, `Scene`, `Rendering`, `Game` 등으로 물리 이동 | 경로만으로 책임과 의존 계층 파악 가능 | 루트 C++ **74 → 4개**, VS 프로젝트/필터 **122/122 일치** |
| 회귀 검증 | 구성별 빌드와 런타임 결과를 반복 재현할 자동 절차가 없었음 | `/W4 /WX`, headless test, PowerShell matrix와 CI 구성 | 로컬과 CI가 동일한 절차 실행 | 빌드 **4/4**, 런타임 검사 **36/36** |

<details>
<summary><strong>정량 지표 전체 보기</strong></summary>

### 정량적 변화

비교 기준은 리팩터링 전 커밋 `132e7ec`과 런타임 플랫폼 커밋 `f1fbe64`입니다. LOC는 공백·주석·전처리문을 포함한 physical line count입니다.

| 지표 | 이전 | 이후 | 변화 |
|---|---:|---:|---:|
| `GraphicsClass` (`.cpp + .h`) | 1,879 LOC | 1,141 LOC | **-738 (-39.3%)** |
| `graphicsclass.cpp` | 1,694 LOC | 1,017 LOC | **-677 (-40.0%)** |
| 프로젝트 소유 C++ | 79파일 / 19,569 LOC | 99파일 / 17,080 LOC | LOC **-2,489 (-12.7%)** |
| `SystemClass` | 2파일 / 431 LOC | 제거 | 실행·창 책임 이동 |
| `Graphics/` 루트 C++ | 74파일 | 4파일 | **-70 (-94.6%)** |
| 외부·레거시 통합 파일 | 98파일 / 1,218,891 bytes | 제거 | Assimp·ImGui·SystemClass 제거 |
| 프로젝트/필터 명시 항목 | 불일치 가능 | 122 / 122 | 누락·초과 0 |
| 런타임 오류 팝업 (`GraphicsClass`) | 15곳 | 0곳 | typed result로 전달 |
| 자동 빌드 행렬 | 없음 | 4개 구성 | warning 0, error 0 |
| 결정론 런타임 검사 | 없음 | 36 / 36 | 구성별 10,000 tick |

수동 ownership 수치는 프로젝트 코드에서 주석, `operator new/delete`, 복사 금지용 `= delete`, 변경하지 않은 `DDSTextureLoader`와 `AlignedAllocationPolicy`를 제외한 호출 지점 휴리스틱입니다. 구조 개선을 설명하는 지표이며 메모리 안전성의 수학적 증명은 아닙니다.

</details>

## 빌드와 검증

### 요구 사항

- Windows 10 또는 Windows 11
- Visual Studio 2022 이상
- `Desktop development with C++` 워크로드
- C++17을 지원하는 MSVC와 Windows SDK

### Visual Studio에서 실행

1. 저장소 루트의 `Graphics.sln`을 엽니다.
2. `Debug/Release × x86/x64` 중 원하는 구성을 선택합니다.
3. 시작 프로젝트를 `PortfolioEngine`으로 설정합니다.
4. 솔루션을 빌드하고 실행합니다.

프로젝트에 지정된 toolset이 없다면 Visual Studio에서 설치된 MSVC toolset으로 재대상 지정하되 C++17 설정은 유지해야 합니다. 런타임 asset은 빌드 시 출력 디렉터리로 복사됩니다.

### 자동 검증

저장소 루트의 PowerShell에서 실행합니다.

```powershell
# Debug/Release × x64
.\scripts\verify.ps1

# Debug/Release × x86/x64
.\scripts\verify.ps1 -AllArchitectures

# replay 길이 변경
.\scripts\verify.ps1 -AllArchitectures -ReplayTicks 20000
```

검증 스크립트는 Visual Studio의 MSBuild를 찾아 각 구성을 빌드한 뒤 `PortfolioRuntimeTests.exe`를 실행합니다. 기본 replay는 구성마다 10,000 tick이며 GPU, 창, 오디오와 asset이 필요하지 않습니다.

### 조작

| 입력 | 동작 |
|---|---|
| `W` / `A` / `S` / `D` | 카메라 전후·좌우 이동 |
| 마우스 이동 | 카메라 시점 회전 |
| 마우스 왼쪽 버튼 | Title 시작 / Hunt 모델 선택 |
| `Esc` | 종료 |

<details>
<summary><strong>디렉터리 구조 보기</strong></summary>

### 디렉터리 구조

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

## 설계 범위와 남은 과제

이번 단계는 기반을 먼저 검증하기 위해 ECS, custom allocator, DLL plugin, 별도 render thread를 도입하지 않았습니다. 현재 경계와 한계도 명시적으로 남겨두었습니다.

- `Platform.Win32`와 `Renderer.D3D11`은 논리적으로 분리됐지만 아직 `PortfolioEngine` 프로젝트 안에서 함께 빌드됩니다.
- 새 Application/Game 오류 경계는 `Result<T>`를 사용하지만 일부 레거시 shader compile helper에는 modal error가 남아 있습니다.
- asset은 아직 `data/` 상대 경로를 사용합니다. 다음 단계는 VFS, `AssetId`, generation 기반 handle, manifest와 cooker입니다.
- 성능 전후 벤치마크는 동일 조건으로 수행하지 않았으므로 FPS 향상은 주장하지 않습니다.

세부 프레임 계약, 오류 모델과 headless 검증 방식은 [Runtime Platform Architecture](docs/runtime-platform.md)에서 확인할 수 있습니다.

## 배경과 출처

원본 데모는 DirectX 11과 게임 엔진 내부 렌더링 흐름을 학습하기 위해 Rastertek 예제를 기반으로 시작했습니다. 이후 Phong specular, soft shadow, instancing, fire effect, picking과 게임 상태를 확장했고, 완성된 결과물을 다시 플랫폼·런타임 관점에서 재설계했습니다. 현재는 Assimp와 ImGui 없이 내장 OBJ loader와 font HUD로 실행됩니다.
