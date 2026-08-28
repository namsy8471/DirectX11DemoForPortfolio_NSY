# DirectX11 Demo for Portfolio – NSY  
다이렉트X11 기반으로 제작한 그래픽스 포트폴리오 데모  
DirectX11で実装したグラフィックスデモ  
A graphics demo implemented with DirectX 11

---

## Preview / プレビュー / 미리보기  

| Noise Fire Effect | Terrain + Billboard + Instancing |
|:----------------------:|:----------------------------------:|
| ![Noise Fire](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/bf59dafa-3dc4-4421-88ff-8e048d593696) | ![Terrain Scene](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/f4b43db1-0b06-4a5e-a63b-c8b82e987963) |

**Video (YouTube):** [https://www.youtube.com/watch?v=YtkRcS8QO4E](https://www.youtube.com/watch?v=YtkRcS8QO4E)

---

## 개발 동기 / 開発動機 / Development Motivation  
**DirectX11**을 직접 다뤄보며 게임 엔진의 렌더링 원리를 이해하고, 다양한 그래픽 기술을 스스로 구현해보고자 했습니다.  
ゲームエンジンの基盤である **DirectX11** を直接扱い、レンダリングの仕組みを理解し、様々なグラフィック技術を自分で実装することを目的としました。  
I wanted to work directly with **DirectX 11**, the core API behind game engines, to understand rendering principles and implement various graphics techniques on my own.

---

## 개발 개요 / 開発概要 / Development Overview

| 항목 / 項目 / Item | 내용 / 内容 / Details |
|--------------------|----------------------|
| **엔진 / エンジン / Engine** | DirectX11 |
| **언어 / 言語 / Language** | C++ |
| **기간 / 期間 / Duration** | 2023/10 ~ 2023/12 |
| **개발 인원 / 開発人数 / Team** | 1명 (개인) / 1名（個人） / 1 (solo) |
| **환경 / 環境 / Environment** | Visual Studio 2022, Windows 11 |

---

## 결정론적 런타임 플랫폼

그래픽 데모의 프레임 루프를 테스트 가능한 런타임으로 분리했습니다. 솔루션은 이제 실행 파일 하나가 아니라 `EngineCore`, `EngineRuntime`, `PortfolioEngine`, `RuntimeTests` 네 개의 빌드 타깃으로 구성됩니다.

| 타깃 | 책임 |
|---|---|
| `EngineCore.lib` | 플랫폼 독립 `Error`, `Result<T>`, 구조화 로그와 소유권 유틸리티 |
| `EngineRuntime.lib` | 60 Hz fixed-step 스케줄러, simulation/render frame 계약 |
| `PortfolioEngine.exe` | Win32 호스트, D3D11 렌더러와 포트폴리오 게임 |
| `PortfolioRuntimeTests.exe` | 창·GPU·오디오·에셋 없이 실행되는 headless 런타임 검증 |

실제 시간은 최대 100 ms로 제한한 뒤 누적하고, 프레임당 최대 8회의 고정 tick을 실행합니다. 남은 시간은 렌더 보간값으로 제공하며 과도한 backlog는 버리고 그 양을 로그에 기록합니다. 따라서 렌더 FPS가 달라져도 게임 시뮬레이션 속도는 바뀌지 않습니다.

게임과 애플리케이션 경계의 `bool` 실패는 오류 코드, 하위 시스템, 메시지와 발생 위치를 보존하는 `Result<T>`로 교체했습니다. 신규 Core/Runtime/Application 경로는 오류를 기록하고 상위로 전달하며, `GraphicsClass`의 15개 직접 팝업은 제거했습니다. 아직 남은 레거시 셰이더 컴파일 팝업은 다음 렌더러 오류 전환 범위입니다.

Headless 검증은 동일한 입력을 한 tick씩 실행한 경우와 다섯 tick씩 묶은 경우를 비교합니다. 두 실행 모두 10,000 tick 후 `0x45309AA8C1381D56`의 동일한 64-bit world hash를 생성하며, 총 9개 검사가 통과합니다.

```powershell
.\scripts\verify.ps1
.\scripts\verify.ps1 -AllArchitectures
```

자세한 설계와 검증 계약은 [`docs/runtime-platform.md`](docs/runtime-platform.md)에 정리했습니다.

---

## 레거시 데모 리팩터링: 문제에서 검증까지

초기 버전은 그래픽 기능을 시연할 수 있었지만, `SystemClass`와 `GraphicsClass`가 Win32 실행 흐름, 입력, 씬 구성, 게임 규칙, 렌더 패스와 자원 해제를 함께 담당했습니다. 기능 하나를 수정해도 애플리케이션 수명주기와 렌더 상태 전체를 함께 확인해야 했고, 부분 초기화 실패나 종료 순서에 따라 자원 누수·중복 해제가 발생하기 쉬운 구조였습니다.

### 변경 사항: 문제 → 변경 방식 → 해결 결과

| 바뀐 부분 | 기존 문제와 원인 | 바꾼 방식 | 해결된 점 | 정량적 성과 또는 검증 |
|---|---|---|---|---|
| **빌드 모듈 경계** | 실행 파일 하나가 Core, Win32, D3D11, 게임과 테스트 코드를 모두 컴파일해 의존성 방향을 강제할 수 없었음 | 솔루션을 `EngineCore.lib`, `EngineRuntime.lib`, `PortfolioEngine.exe`, `PortfolioRuntimeTests.exe`로 분리하고 프로젝트 참조 방향을 고정 | Core/Runtime을 그래픽 장치 없이 독립적으로 빌드·테스트할 수 있음 | 빌드 타깃 **1개 → 4개**, Core/Runtime의 Win32·DirectX include **0개** |
| **프레임 런타임** | 가변 delta로 게임 상태를 갱신해 렌더 FPS와 시뮬레이션 속도가 결합되고 장시간 정지 시 상태가 크게 도약할 수 있었음 | 60 Hz `FixedStepScheduler`, 최대 100 ms frame clamp, 프레임당 최대 8 tick, render interpolation 계약을 도입 | 렌더 빈도와 게임 진행 속도가 분리되고 과도한 backlog가 제한·계측됨 | 서로 다른 render-frame 분할에서 10,000 tick 후 동일 hash `0x45309AA8C1381D56`; 실제 스모크에서 **8,690 render frame / 126 fixed tick** |
| **오류 전달과 로그** | `bool` 실패와 `MessageBox`만으로는 어느 계층·파일·호출에서 실패했는지 보존할 수 없었음 | 오류 코드·하위 시스템·메시지·소스 위치를 가진 `Error`와 `Result<T>`를 만들고 timestamp·thread·category 로그를 추가 | Application/Game 경계에서 실패 원인이 손실 없이 Win32 런처까지 전달되고 실행 로그가 남음 | `GraphicsClass` 직접 오류 팝업 **15곳 → 0곳**, 로그 필드 **5종** 기록 |
| **애플리케이션·플랫폼 경계** | `SystemClass`와 전역 Win32 콜백이 창, 메시지 펌프, 게임 수명주기를 함께 관리 | `Application`–`Win32Window`–`IGame`과 `NativeWindow`, fixed/render frame context를 도입 | Win32 메시지 처리와 게임 규칙의 수명주기·실패 경계가 명시됨 | `SystemClass` **2파일 / 431 LOC → 제거** |
| **자원 소유권** | COM raw pointer, `new/delete`, `Release()`가 여러 클래스에 분산되어 부분 초기화 실패와 종료 순서에 취약 | COM은 `ComPtr`, 단독 객체는 `unique_ptr`, CPU 배열은 `vector`, 레거시 종료 객체는 `ScopedResource`로 통일 | 소유자가 타입으로 표현되고 중간 실패에도 소멸자가 정리 | 프로젝트 코드 수동 ownership site **214곳 → 0곳** |
| **입력과 시뮬레이션 데이터** | DirectInput 읽기, 카메라 이동과 렌더 시점이 결합되어 장치 상태와 게임 의도를 구분하기 어려웠음 | `InputSnapshot`과 `FirstPersonCameraController`를 도입하고 입력·카메라 갱신을 fixed update에 배치 | 프레임 단위 입력 계약이 생기고 카메라 동작을 렌더 코드 밖에서 변경 가능 | 최소화 복귀 timer reset, DirectInput 재획득 경로와 delta 상한 검증 |
| **소프트 섀도우 파이프라인** | 6단계 패스가 거대한 렌더 함수의 상태와 임시 자원을 공유 | 섀도 맵→흑백→다운샘플→가로 블러→세로 블러→업샘플을 `SoftShadowPipeline`으로 캡슐화 | 패스별 자원 소유권과 렌더 상태 복구 범위가 한 객체로 제한됨 | 전용 파이프라인 **6개 패스**, 구현 **544 LOC**로 분리 |
| **텍스처 자원 API** | DDS 텍스처와 렌더 텍스처의 생성·조회 API가 달라 소비자가 구체 구현과 `ComPtr`를 알아야 했음 | `ITexture`/`IRenderTexture`, `DdsTexture`/`RenderTargetTexture`, `TexturePtr`와 `MakeTexture`를 도입 | 셰이더는 공통 SRV 계약만 사용하고 생성 실패 시 미완성 자원이 노출되지 않음 | 구체 타입/직접 `ComPtr` 자원 **14개 → 인터페이스 핸들 14개** |
| **물리 디렉터리** | 74개 C++ 파일이 `Graphics/` 루트에 섞여 include와 책임 위치가 일치하지 않았음 | `Core`, `Runtime`, `Platform`, `Scene`, `Input`, `Audio`, `Diagnostics`, `Rendering/*`, `Game`으로 이동하고 VS 필터 동기화 | 파일 경로만으로 계층과 책임을 판단할 수 있고 탐색 범위가 축소됨 | 루트 C++ **74개 → 4개(-94.6%)**, 프로젝트/필터 **122 / 122 일치** |
| **씬·모델 데이터** | 씬 경로·변환이 초기화 코드에 하드코딩됐고 누락 모델을 큐브 fallback이 숨겨 곰·칠면조·말이 상자로 보였음 | `SceneDefinition`, `GameObject`, `Transform`을 분리하고 OBJ ignore 예외·좌표계 변환을 복구하며 필수 에셋 누락은 실패 처리 | 씬 데이터와 런타임 객체가 분리되고 잘못된 대체 모델 대신 원인이 드러남 | OBJ **4개 / 17,405,008 bytes**, 총 **230,504 triangles** 복구 |
| **불꽃 알파 마스크** | `alpha01.dds`의 A 채널은 전부 255인데 셰이더가 `alphaColor.a`를 사용해 Title 불꽃이 사각형으로 렌더링됨 | RGB에 저장된 grayscale mask를 사용하도록 `fireColor.a *= alphaColor.r`로 수정 | 기존 단일 alpha blend state를 유지하면서 불꽃 외곽 투명도가 복원됨 | 마스크 A 고유값 **1개(255)**에서 RGB **256단계**를 실제 투명도에 사용 |
| **빌드·회귀 자동화** | 구성별 설정 차이와 외부 통합 때문에 clean build와 실행 결과 재현이 어려웠음 | 미사용 Assimp/ImGui 통합을 제거하고 네 구성 `/W4 /WX`, PowerShell 검증 스크립트와 GitHub Actions를 추가 | 로컬과 CI가 같은 빌드·headless replay 절차를 실행 | 외부 통합 **98파일 / 1,218,891 bytes 제거**, 빌드 **4/4**, 런타임 검사 **36/36 통과** |
| **지형·좌표 안정성** | 비정사각 높이 맵 stride와 음수 좌표 반올림이 잘못된 셀을 참조 | 높이 맵 stride와 `floor` 기반 월드 좌표 계산을 적용 | 비정사각 지형과 음수 좌표에서도 올바른 높이 샘플을 사용 | 네 빌드 구성에서 `/W4 /WX` 및 런타임 스모크 검증 통과 |

### 정량 결과

측정 기준은 리팩터링 전 커밋 `132e7ec`과 2026-08-28 현재 작업 트리입니다. LOC는 공백·주석·전처리문을 포함한 physical line count입니다.

| 지표 | 이전 | 이후 | 변화 또는 결과 |
|---|---:|---:|---:|
| `GraphicsClass` (`.cpp + .h`) | 1,879 LOC | 1,141 LOC | **-738 LOC, -39.3%**; 오류 원인 보존 코드 포함 |
| `graphicsclass.cpp` 단일 파일 | 1,694 LOC | 1,017 LOC | **-677 LOC, -40.0%** |
| 프로젝트 소유 C++ (`Graphics/**/*.cpp,h`, Assimp 제외) | 79파일 / 19,569 LOC | 99파일 / 17,080 LOC | 파일 +20, LOC **-2,489 (-12.7%)** |
| `SystemClass` | 2파일 / 431 LOC | 제거 | 실행·창 책임을 엔진 계층으로 이동 |
| `Engine/` 책임 트리 | 없음 | 93파일 / 15,836 LOC | Core/Runtime을 포함한 역할별 물리 구조로 정렬 |
| `Graphics/` 루트 C++ 파일 | 74파일 | 4파일 | **-70파일, -94.6%**; 진입점·PCH만 유지 |
| 텍스처 소유 자원 | 구체 타입/직접 `ComPtr` 14개 | 인터페이스 핸들 14개 | 샘플링 텍스처 8개는 `TexturePtr`, 렌더 텍스처 6개는 `RenderTexturePtr`로 통일 |
| 텍스처 래퍼 구현 | 4파일 / 322 LOC | 3파일 / 333 LOC | 공통 인터페이스 2개와 실패 안전 생성 팩토리 추가, 레거시 클래스 2개 제거 |
| 수동 ownership site* | 214곳 | 0곳 | `new` 67 + `delete` 42 + `Release()` 105 제거 |
| 소프트 섀도우 파이프라인 | `GraphicsClass` 내부 상태 공유 | 6개 패스 전용 객체 | `SoftShadowPipeline.cpp` 544 LOC로 분리 |
| 레거시/의존성 통합 파일 | 98파일 / 1,218,891 bytes | 제거 | Assimp 93 + ImGui 3 + SystemClass 2 제거 |
| 동물 모델 데이터 | clean clone에서 OBJ 4개 누락 | 4개 / 17,405,008 bytes | **230,504 triangles** 복구 및 모든 출력 구성에 복사 |
| 프로젝트/필터 명시 항목 | 불일치 가능 상태 | 122 / 122 | 누락 0, 초과 0, 실제 파일 누락 0 |
| 솔루션 빌드 타깃 | 실행 파일 1개 | 4개 | Core/Runtime 정적 라이브러리, 게임, headless 테스트로 분리 |
| 런타임 오류 팝업 (`GraphicsClass`) | 15곳 | 0곳 | typed `Result<T>`로 Application/Launcher까지 원인 전달 |
| Headless 결정론 검증 | 없음 | 36 / 36 성공 | 4개 구성 × 9개 검사, 각 10,000 tick hash 일치 |
| 빌드 행렬 | 구성별 결과 불명확 | 4 / 4 성공 | Debug/Release × x86/x64, warning 0, error 0 |
| 런타임 스모크 테스트 | 기준 없음 | 1 / 1 성공 | 창·메인 루프 응답, `WM_CLOSE`, ExitCode 0 |

\* 수동 ownership 수치는 프로젝트 코드에서 주석, `operator new/delete`, 복사 금지용 `= delete;`, 변경하지 않은 `DDSTextureLoader`와 `AlignedAllocationPolicy`를 제외한 호출 지점 휴리스틱입니다. 정규식 기반 구조 지표이며 그 자체가 메모리 안전성의 수학적 증명은 아닙니다.

### 검증 범위와 해석

- 네 구성 모두 C++17, `/W4`, `/WX` 빌드와 headless 결정론 검증을 수행했으며 18개 HLSL 항목도 함께 컴파일했습니다.
- 네 출력 디렉터리에 복구한 OBJ 4개가 모두 복사되고, 필수 모델 누락 시 큐브 fallback 없이 초기화가 실패하는 것을 확인했습니다.
- `Debug | x64`에서 실제 창 생성, 모델 초기화, fixed-step 메인 루프 응답과 정상 종료를 확인했습니다. 측정 실행에서는 8,690 render frame 동안 126 fixed tick이 독립적으로 수행됐습니다.
- 이번 정량 결과는 구조·소유권·빌드 재현성에 대한 측정입니다. 동일한 장면·카메라·GPU 조건의 전후 성능 벤치마크는 수행하지 않았으므로 FPS 향상을 주장하지 않습니다.

**English summary:** The legacy demo was split into explicit application, platform, game, input, scene, and rendering boundaries. Core and Runtime are independent static-library targets, the game now runs on a deterministic 60 Hz fixed-step scheduler, and a headless replay produces the same 64-bit world hash across render-frame partitions. Manual ownership sites were reduced from 214 to zero in project-owned code, texture ownership uses shared contracts, and all four build configurations pass `/W4 /WX` plus runtime verification.

---

## 리팩터링된 엔진 구조 / Refactored Engine Architecture

기존의 큰 실행 흐름을 애플리케이션, 플랫폼 창, 게임 구현으로 분리하고, 프레임·입력·씬 데이터를 명시적인 객체로 전달하도록 정리했습니다.

```text
WinMain
└─ Engine::Application
   ├─ Engine::Win32Window
   ├─ Engine::Runtime::FixedStepScheduler
   ├─ std::unique_ptr<Engine::IGame> → GraphicsClass (PortfolioGame)
   └─ TimerClass / FpsClass / CpuClass

FixedFrameContext → IGame::FixedUpdate (60 Hz simulation)
RenderFrameContext → IGame::Render (presentation + interpolation)

InputClass → InputSnapshot → FirstPersonCameraController → CameraClass
SceneDefinition → GameObject → Transform + ModelClass
GraphicsClass → SoftShadowPipeline → 6 render passes
ITexture ← DdsTexture / IRenderTexture ← RenderTargetTexture
```

| 구성 요소 | 책임 |
|---|---|
| `Engine::Application` | 창, 게임 인스턴스, 타이머, FPS/CPU 계측을 소유하고 `Initialize → Run → Shutdown` 수명주기를 관리합니다. |
| `Engine::Win32Window` | `WindowConfig`로 Win32 창을 만들고, `lpParam`과 `GWLP_USERDATA`를 이용해 전역 포인터 없이 메시지를 인스턴스로 전달합니다. |
| `Engine::IGame` | 플랫폼 루프와 실제 데모를 분리하며 초기화, 업데이트, 렌더링, 종료 계약만 정의합니다. `Windows.h`와 Win32 메시지 타입에 의존하지 않습니다. |
| `Engine::FrameContext` | 프레임 번호, delta time, 누적 시간, FPS, CPU 사용률을 업데이트와 렌더링에 동일하게 전달합니다. |
| `InputSnapshot` | 한 프레임의 키보드·마우스 상태를 값 객체로 묶어 입력 장치 읽기와 게임 로직의 결합을 줄입니다. |
| `FirstPersonCameraController` | `InputSnapshot`을 카메라 이동·회전으로 변환해 입력 수집과 카메라 규칙을 분리합니다. |
| `SceneDefinition` | 모델, 지형, 불, UI, 폴리지, 오디오, 섀도 맵 설정을 데이터로 정의합니다. |
| `GameObject` / `Transform` | 모델 소유권·활성 상태와 위치·회전·스케일·월드 행렬 계산을 캡슐화합니다. |
| `SoftShadowPipeline` | 섀도 맵, 흑백 변환, 다운샘플, 가로·세로 블러, 업샘플의 여섯 패스와 관련 렌더 자원을 캡슐화합니다. |
| `ITexture` / `IRenderTexture` | 모든 텍스처의 SRV 접근을 통일하고, 렌더 타깃 전용 동작은 별도 인터페이스로 분리합니다. 소비자는 DDS 로딩이나 RTV/DSV 구현을 알 필요가 없습니다. |

### 통합 텍스처 생성 API

인터페이스는 직접 값으로 생성할 수 없고 raw `new`는 소유권을 모호하게 하므로, 요청한 간단한 선언 방식은 소유권까지 표현하는 스마트 포인터 팩토리로 제공합니다.

```cpp
using namespace Engine::Rendering;

TexturePtr albedo = MakeTexture<DdsTexture>(
    device,
    L"data/stone01.dds");

RenderTexturePtr shadowMap = MakeTexture<RenderTargetTexture>(
    device,
    RenderTextureDescriptor{ 2048, 2048, 1000.0f, 0.1f });
```

두 변수 모두 `GetShaderResourceView()`로 셰이더에 전달할 수 있습니다. `shadowMap`만 `SetRenderTarget()`, `ClearRenderTarget()`, 투영·직교 행렬 접근을 추가로 제공하므로 일반 텍스처에 불필요한 렌더 타깃 책임을 강제하지 않습니다. 생성에 실패하면 빈 포인터가 반환되고 이미 만들어진 Direct3D 자원은 RAII로 정리됩니다.

### COM 자원 수명 / COM resource lifetime

- **Direct3D 11** — 디바이스, 컨텍스트, 스왑 체인, 버퍼, 텍스처, 셰이더와 상태 객체의 소유권을 `Microsoft::WRL::ComPtr`로 관리합니다.
- **DirectInput 8** — `InputClass`가 DirectInput 인터페이스와 키보드·마우스 장치를 `ComPtr`로 소유합니다.
- **DirectSound** — `SoundClass`가 DirectSound 인터페이스와 1차·2차 사운드 버퍼를 `ComPtr`로 소유합니다.
- 전환된 소유 객체는 복사를 금지하고, 소멸자와 반복 호출 가능한 `Shutdown()`을 통해 부분 초기화 실패도 안전하게 정리하도록 구성했습니다. 게임 레벨 소유권에는 `std::unique_ptr`를 사용합니다.

**English:** The runtime is split into `Application`, `Win32Window`, and an `IGame` implementation. Per-frame state flows through `FrameContext`, input through `InputSnapshot`, and scene data through `SceneDefinition`/`GameObject`/`Transform`. Direct3D, DirectInput, and DirectSound ownership now uses RAII with `ComPtr`.

---

## 디렉터리 구조 / Directory Layout

```text
Graphics.sln
Graphics/
├─ Engine/
│  ├─ Application.{h,cpp}       # 애플리케이션 수명주기와 메인 루프
│  ├─ IGame.h / FrameContext.h  # 게임 경계와 프레임 데이터
│  ├─ Audio/                    # DirectSound 자원과 재생
│  ├─ Core/                     # 범용 RAII와 scope guard
│  ├─ Diagnostics/              # 타이머, FPS, CPU 계측
│  ├─ Input/                    # DirectInput, 스냅샷, 카메라 컨트롤러
│  ├─ Platform/                 # Win32 창, 메시지, 네이티브 핸들 경계
│  ├─ Scene/                    # 카메라, 조명, 충돌, 오브젝트와 씬 정의
│  └─ Rendering/
│     ├─ Device/                # Direct3D 장치와 공통 헬퍼
│     ├─ Geometry/              # 모델, 지형, 폴리지, 스카이돔, 화면 쿼드
│     ├─ Pipelines/             # SoftShadowPipeline
│     ├─ Shaders/               # 셰이더 및 상수 버퍼 래퍼
│     ├─ Textures/              # ITexture, DDS, 렌더 타깃 텍스처
│     └─ UI/                    # 비트맵, 폰트, 텍스트 HUD
├─ Game/
│  └─ graphicsclass.{h,cpp}     # IGame을 구현하는 포트폴리오 데모
├─ data/                        # HLSL, 텍스처, 높이 맵, 폰트, 오디오 자산
├─ main.cpp                     # Application과 PortfolioGame 조립
└─ stdafx.{h,cpp} / targetver.h # 사전 컴파일 헤더와 Windows SDK 대상
```

---

## 외부 의존성과 에셋 / Dependencies & Assets

- 기존의 외부 **Assimp** 및 **ImGui** 의존성을 제거했습니다. 현재 프로젝트는 모델 로딩이나 UI 표시를 위해 해당 헤더·라이브러리·DLL을 요구하지 않습니다.
- `ModelClass`는 필요한 OBJ 요소(`v`, `vt`, `vn`, `f`)를 직접 읽고, 다각형 면을 삼각형으로 변환하며, 법선이 없으면 생성합니다.
- 기존 Assimp가 사용하던 Z축, UV Y축과 면 방향 반전을 재현해 동일한 좌수계 변환 규칙을 유지합니다.
- 기존 데모 백업에서 곰, 수컷·암컷 칠면조, 말 OBJ를 복구해 런타임 데이터에 포함했습니다. 필수 메시나 텍스처가 누락되면 상자로 대체하지 않고 초기화 오류를 반환합니다.
- 기존 FPS/CPU 등의 화면 표시는 프로젝트의 폰트 HUD 경로를 사용하며 ImGui에 의존하지 않습니다.

**English:** Assimp and ImGui are no longer required. The built-in OBJ reader preserves the legacy left-handed conversion, and the bear, turkey, and horse meshes used by the legacy demo were recovered from backup and added as runtime data. Missing required meshes or textures now fail initialization instead of silently rendering placeholder cubes.

---

## 빌드 방법 / Build

### 요구 사항

- Windows 10 또는 Windows 11
- **Visual Studio 2022 이상** 및 **Desktop development with C++** 워크로드
- MSVC의 **C++17** 지원과 Windows 10/11 SDK
- Direct3D 11, DirectInput 8, DirectSound 헤더·라이브러리(Windows SDK에 포함)

### 절차

1. 저장소 루트의 `Graphics.sln`을 Visual Studio에서 엽니다.
2. 아래 네 구성 중 하나를 선택합니다.

   - `Debug | x64`
   - `Release | x64`
   - `Debug | x86` (`Win32` 프로젝트 구성에 매핑)
   - `Release | x86` (`Win32` 프로젝트 구성에 매핑)

3. 프로젝트에 기록된 플랫폼 도구 집합을 설치하지 않은 경우, Visual Studio 안내에 따라 설치된 MSVC 도구 집합으로 재대상 지정합니다. Visual Studio 2022에서는 일반적으로 `v143`을 사용할 수 있으며, 언어 표준은 C++17을 유지합니다.
4. **Build → Build Solution**을 실행합니다. `Graphics/data`의 런타임 자산은 출력 디렉터리로 `PreserveNewest` 복사되도록 설정되어 있습니다.
5. 빌드가 완료된 구성의 `PortfolioEngine`을 Visual Studio에서 실행합니다.
6. 저장소 루트에서 `.\scripts\verify.ps1`을 실행하면 x64 Debug/Release 빌드와 10,000-tick headless replay를 자동 검증합니다. `-AllArchitectures`를 추가하면 x86까지 포함합니다.

> **검증 결과:** 네 구성 모두 HLSL 포함 빌드와 9개 런타임 검사를 통과했으며 경고는 오류로 처리됩니다. `Debug | x64` 실제 창 실행과 정상 종료 스모크 테스트도 통과했습니다. 환경별 도구 집합이나 Windows SDK 차이로 재대상 지정은 필요할 수 있습니다.

**English:** Open `Graphics.sln` with Visual Studio 2022 or newer, install the Desktop C++ workload and a Windows SDK, and build one of the four Debug/Release × x86/x64 configurations with C++17. Run `.\scripts\verify.ps1 -AllArchitectures` to build all configurations and execute the deterministic headless replay checks.

---

## 주요 기능 / 主な機能 / Key Features

### 한국어 (Korean)
- **SkyDome 구현** — 돔 메시와 색상 보간 셰이더를 이용해 배경 하늘 표현
- **Collision Detection** — 마우스 좌클릭 시 감지된 3D 모델을 비활성화 처리  
- **Keyboard 입력 기반 카메라 이동** — WASD 및 마우스 조작으로 자유로운 시점 이동  
- **Height Map 기반 Terrain 생성** — 고도 맵을 이용해 지형 자동 생성  
- **Phong Illumination 적용** — Diffuse, Ambient, Specular 조명 모델 구현  
- **Render Texture 기반 Soft Shadow** — 그림자의 부드러운 가장자리 표현  
- **Billboard 및 Instancing** — 인스턴싱으로 풀(Grass) 오브젝트 2,000개 렌더링
- **Noise Texture 기반 불 효과** — 노이즈·왜곡 셰이더와 알파 텍스처로 불 표현
- **DirectSound** — BGM 및 효과음 재생  
- **Title/Hunt 단계별 렌더링** — 게임 단계에 따라 업데이트와 렌더 경로 전환
- **Font Rendering HUD** — FPS, CPU 사용률, 모델 수, 화면 해상도 표시  

---

### 日本語 (Japanese)
- **SkyDome実装** — ドームメッシュと色補間シェーダーで空を表現
- **Collision Detection** — マウス左クリックで検出された3Dモデルを非アクティブ化  
- **キーボード操作によるカメラ移動** — WASDとマウス操作で自由に視点を移動  
- **ハイトマップによる地形生成** — 高度マップから地形を自動生成  
- **フォン・イルミネーションモデル** — Diffuse、Ambient、Specularの光反射を実装  
- **Render Textureを使用したソフトシャドウ** — 柔らかい影を表現  
- **ビルボードとインスタンシング** — 草オブジェクト2,000個をインスタンシング描画
- **ノイズテクスチャを用いた火のエフェクト** — ノイズ・歪みシェーダーとアルファテクスチャで炎を表現
- **DirectSoundによるBGM・効果音再生**  
- **Title/Hunt段階別レンダリング** — ゲーム段階に応じて更新・描画経路を切り替え
- **フォントレンダリングHUD** — FPS、CPU使用率、モデル数、画面解像度を表示  

---

### English
- **SkyDome Implementation** — Renders the sky with a dome mesh and color-interpolation shader
- **Collision Detection** — Detects clicked 3D models and deactivates them on left mouse click  
- **Camera Movement via Keyboard** — Move the camera freely using WASD keys and mouse input  
- **Height Map Terrain Generation** — Automatically generates terrain based on height maps  
- **Phong Illumination Model** — Implemented Diffuse, Ambient, and Specular lighting  
- **Soft Shadow via Render Texture** — Achieved smooth shadow edges using render-to-texture  
- **Billboard and Instancing** — Renders 2,000 grass objects with instancing
- **Noise Texture-based Fire Effect** — Renders fire with noise, distortion, and alpha textures
- **DirectSound Integration** — Implemented background music and sound effects  
- **Title/Hunt Stage Rendering** — Switches update and rendering paths by game stage
- **Font Rendering HUD** — Displays FPS, CPU usage, object count, and resolution info 

---

## 어려웠던 점 / 苦労した点 / Challenges
Unity와 달리 오류의 원인을 추적하기 어려워 신중하고 보수적으로 개발했습니다.  
C++ 및 DirectX 프로젝트는 처음이라 **객체지향 설계 부족**을 겪으며, 설계의 중요성을 깊이 배웠습니다.  

Unityとは異なり、エラー原因の特定が難しい環境だったため、慎重かつ保守的に開発を進めました。  
C++やDirectXによる開発は初めてで、**オブジェクト指向設計の不足**に苦労し、設計の重要性を実感しました。  

Unlike Unity, debugging DirectX was difficult, so I took a careful and conservative development approach.  
It was my **first C++/DirectX project**, and I learned the importance of proper **object-oriented design** through the struggle.

---

## 기술적 포인트 / 技術的ポイント / Technical Highlights  
Rastertek 예제 코드를 기반으로 **스페큘러 성분 추가**, **소프트 섀도우 셰이더 수정** 등 직접 개선 및 확장.  
Direct3D 렌더링 파이프라인, 셰이더 스테이지, render-to-texture 패스와 자원 수명 구조를 체계적으로 이해함.

Rastertekのサンプルコードを基に、**スペキュラ成分の追加**や**ソフトシャドウ実装**などを独自に改良。  
Direct3Dのレンダリングパイプライン、シェーダーステージ、render-to-textureパス、リソース寿命を体系的に理解。

Based on Rastertek examples, enhanced shaders by **adding specular reflection** and **implementing soft shadows**.  
Gained structured understanding of the **Direct3D pipeline**, shader stages, render-to-texture passes, and resource lifetime management.

---

## 요약 / まとめ / Summary  
DirectX11을 통해 게임 엔진의 기반 구조를 이해하고, 그래픽스 렌더링의 저수준 원리를 체득했습니다.  
이 프로젝트는 Unity와 DirectX 사이의 기술적 간극을 메우는 첫 단계였습니다.  

DirectX11を通してゲームエンジンの基盤構造を理解し、グラフィックスレンダリングの低レベル原理を体得しました。  
本プロジェクトは、UnityとDirectXの技術的ギャップを埋める第一歩です。  

Through this project, I gained a solid understanding of low-level graphics rendering principles.  
It represents my **first step in bridging the gap between Unity and DirectX development.**

---

## Links
- **Video:** [YouTube](https://www.youtube.com/watch?v=YtkRcS8QO4E) 
