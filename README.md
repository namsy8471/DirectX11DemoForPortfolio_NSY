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

## 레거시 데모 리팩터링: 문제에서 검증까지

초기 버전은 그래픽 기능을 시연할 수 있었지만, `SystemClass`와 `GraphicsClass`가 Win32 실행 흐름, 입력, 씬 구성, 게임 규칙, 렌더 패스와 자원 해제를 함께 담당했습니다. 기능 하나를 수정해도 애플리케이션 수명주기와 렌더 상태 전체를 함께 확인해야 했고, 부분 초기화 실패나 종료 순서에 따라 자원 누수·중복 해제가 발생하기 쉬운 구조였습니다.

### 문제와 근본 원인

| 관찰된 문제 | 근본 원인 | 적용한 해결 |
|---|---|---|
| 창, 메인 루프와 데모 구현이 서로 직접 의존 | 플랫폼과 게임 사이의 수명주기 계약이 없고 전역 Win32 콜백 상태를 사용 | `Application`–`Win32Window`–`IGame` 경계를 만들고 `NativeWindow`, `FrameContext`, `UpdateResult`로 플랫폼 경계를 명시 |
| COM raw pointer, `new/delete`, `Release()`가 여러 클래스에 분산 | 누가 자원을 소유하며 부분 초기화 실패 때 누가 정리하는지가 타입에 표현되지 않음 | COM은 `ComPtr`, 단독 객체는 `unique_ptr`, CPU 배열은 `vector`, 레거시 `Shutdown()` 객체는 `ScopedResource`로 소유권 통일 |
| 입력 읽기, 카메라 이동과 렌더 시점이 결합 | 장치 상태와 게임 의도를 구분하는 프레임 단위 데이터 계약이 없음 | `InputSnapshot`과 `FirstPersonCameraController`를 도입하고 카메라 갱신을 Update 단계로 이동 |
| 6단계 소프트 섀도우가 거대한 렌더 함수의 상태를 공유 | 렌더 패스별 책임, 자원 소유권과 상태 복구 경계가 없음 | 섀도 맵→흑백→다운샘플→가로 블러→세로 블러→업샘플을 `SoftShadowPipeline`으로 캡슐화 |
| 씬 경로와 변환이 `GraphicsClass` 초기화 코드에 하드코딩 | 씬 데이터, 모델 자원과 게임 오브젝트의 책임이 섞여 있음 | `SceneDefinition`, `GameObject`, `Transform`으로 설정·소유권·활성 상태·월드 변환을 분리 |
| Assimp/ImGui 및 빌드 설정 편차 때문에 clean build 재현이 어려움 | 사용 범위보다 큰 외부 의존성과 구성별로 달라진 컴파일/HLSL 설정 | 필요한 OBJ 기능만 C++17로 구현하고 미사용 통합을 제거한 뒤 네 구성 모두 `/W4 /WX`로 통일 |
| 리팩터링 후 곰·칠면조·말이 상자로 표시 | 기존 `.gitignore`의 `*.obj`가 컴파일 산출물과 Wavefront 모델을 함께 제외해 동물 OBJ가 Git에 한 번도 포함되지 않았고, 리팩터링 중 추가한 큐브 fallback이 이 누락을 오류 대신 시각 회귀로 숨김 | 기존 데모 백업에서 OBJ 4개를 복구하고 `Graphics/data/*.obj`를 ignore 대상에서 제외. 기존 Assimp의 Z축·UV Y·winding 반전을 재현하고, 필수 메시/텍스처가 없으면 상자로 대체하지 않고 초기화 실패 처리 |
| 비정사각 지형과 음수 월드 좌표, 장시간 프레임 정지에서 잘못된 상태 가능 | 배열 stride, 좌표 반올림과 delta time에 대한 암묵적 가정 | 높이 맵 stride와 `floor` 기반 좌표 계산을 수정하고 delta clamp, 최소화 복귀 reset, DirectInput 재획득 경로를 추가 |

### 정량 결과

측정 기준은 리팩터링 전 커밋 `132e7ec`과 2026-08-28 현재 작업 트리입니다. LOC는 공백·주석·전처리문을 포함한 physical line count입니다.

| 지표 | 이전 | 이후 | 변화 또는 결과 |
|---|---:|---:|---:|
| `GraphicsClass` (`.cpp + .h`) | 1,879 LOC | 1,089 LOC | **-790 LOC, -42.0%** |
| `graphicsclass.cpp` 단일 파일 | 1,694 LOC | 968 LOC | **-726 LOC, -42.9%** |
| 프로젝트 소유 C++ (`Graphics/**/*.cpp,h`, Assimp 제외) | 79파일 / 19,569 LOC | 93파일 / 16,253 LOC | 파일 +14, LOC **-3,316 (-16.9%)** |
| `SystemClass` | 2파일 / 431 LOC | 제거 | 실행·창 책임을 엔진 계층으로 이동 |
| 명시적 엔진 계층 | 없음 | 19파일 / 2,228 LOC | 13개 논리 모듈로 책임 분할 |
| 수동 ownership site* | 214곳 | 0곳 | `new` 67 + `delete` 42 + `Release()` 105 제거 |
| 소프트 섀도우 파이프라인 | `GraphicsClass` 내부 상태 공유 | 6개 패스 전용 객체 | `SoftShadowPipeline.cpp` 538 LOC로 분리 |
| 레거시/의존성 통합 파일 | 98파일 / 1,218,891 bytes | 제거 | Assimp 93 + ImGui 3 + SystemClass 2 제거 |
| 동물 모델 데이터 | clean clone에서 OBJ 4개 누락 | 4개 / 17,405,008 bytes | **230,504 triangles** 복구 및 모든 출력 구성에 복사 |
| 프로젝트/필터 명시 항목 | 불일치 가능 상태 | 119 / 119 | 누락 0, 초과 0, 실제 파일 누락 0 |
| 빌드 행렬 | 구성별 결과 불명확 | 4 / 4 성공 | Debug/Release × x86/x64, warning 0, error 0 |
| 런타임 스모크 테스트 | 기준 없음 | 1 / 1 성공 | 창·메인 루프 응답, `WM_CLOSE`, ExitCode 0 |

\* 수동 ownership 수치는 프로젝트 코드에서 주석, `operator new/delete`, 복사 금지용 `= delete;`, 변경하지 않은 `DDSTextureLoader`와 `AlignedAllocationPolicy`를 제외한 호출 지점 휴리스틱입니다. 정규식 기반 구조 지표이며 그 자체가 메모리 안전성의 수학적 증명은 아닙니다.

### 검증 범위와 해석

- 네 구성 모두 C++17, `/W4`, `/WX` 전체 Rebuild를 수행했으며 18개 HLSL 항목도 함께 컴파일했습니다.
- 네 출력 디렉터리에 복구한 OBJ 4개가 모두 복사되고, 필수 모델 누락 시 큐브 fallback 없이 초기화가 실패하는 것을 확인했습니다.
- `Debug | x64`에서 실제 창 생성, 모델 초기화, 메인 루프 응답과 정상 종료를 확인했습니다.
- 이번 정량 결과는 구조·소유권·빌드 재현성에 대한 측정입니다. 동일한 장면·카메라·GPU 조건의 전후 성능 벤치마크는 수행하지 않았으므로 FPS 향상을 주장하지 않습니다.

**English summary:** The legacy demo was split into explicit application, platform, game, input, scene, and rendering boundaries. Manual ownership sites were reduced from 214 to zero in project-owned code, `GraphicsClass` shrank by 42.0%, four build configurations pass `/W4 /WX`, and the four legacy animal meshes were restored instead of being hidden behind placeholder cubes.

---

## 리팩터링된 엔진 구조 / Refactored Engine Architecture

기존의 큰 실행 흐름을 애플리케이션, 플랫폼 창, 게임 구현으로 분리하고, 프레임·입력·씬 데이터를 명시적인 객체로 전달하도록 정리했습니다.

```text
WinMain
└─ Engine::Application
   ├─ Engine::Win32Window
   ├─ std::unique_ptr<Engine::IGame> → GraphicsClass (PortfolioGame)
   └─ TimerClass / FpsClass / CpuClass → Engine::FrameContext

InputClass → InputSnapshot → FirstPersonCameraController → CameraClass
SceneDefinition → GameObject → Transform + ModelClass
GraphicsClass → SoftShadowPipeline → 6 render passes
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
│  ├─ Win32Window.{h,cpp}       # Win32 창과 메시지 라우팅
│  ├─ NativeWindow.h            # Windows.h 없는 네이티브 창 핸들 경계
│  ├─ IGame.h / FrameContext.h  # 게임 경계와 프레임 데이터
│  ├─ Core/                     # ScopedResource, ScopeExit
│  ├─ Input/                    # InputSnapshot, FirstPersonCameraController
│  ├─ Rendering/                # SoftShadowPipeline
│  └─ Scene/                    # SceneDefinition, GameObject, Transform
├─ data/                        # HLSL, 텍스처, 높이 맵, 폰트, 오디오 자산
├─ main.cpp                     # Application과 PortfolioGame 조립
├─ graphicsclass.{h,cpp}        # IGame을 구현하는 포트폴리오 데모
├─ d3dclass.{h,cpp}             # Direct3D 11 장치와 렌더 상태
├─ inputclass.{h,cpp}           # DirectInput 및 InputSnapshot
├─ SoundClass.{h,cpp}           # DirectSound
├─ modelclass.{h,cpp}           # 내장 OBJ 로더와 GPU 메시 리소스
└─ *ShaderClass.{h,cpp}         # 셰이더 및 상수 버퍼 래퍼
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

> **검증 결과:** 네 구성 모두 HLSL 포함 전체 `Rebuild`에 성공했으며 경고는 오류로 처리됩니다. `Debug | x64` 실행 스모크 테스트도 통과했습니다. 환경별 도구 집합이나 Windows SDK 차이로 재대상 지정은 필요할 수 있습니다.

**English:** Open `Graphics.sln` with Visual Studio 2022 or newer, install the Desktop C++ workload and a Windows SDK, and build one of the four Debug/Release × x86/x64 configurations with C++17. All four configurations pass a full rebuild under `/W4 /WX`; Debug x64 also passes a window/main-loop/graceful-close smoke test.

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
