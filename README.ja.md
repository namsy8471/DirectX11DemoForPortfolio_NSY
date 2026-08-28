<div align="center">

<h1>DirectX 11 Portfolio Runtime</h1>

<p><strong>グラフィックスデモから、決定論的でテスト可能な小規模ゲームランタイムへ</strong></p>

<p>
  <a href="README.md">🇰🇷 한국어</a> ·
  <a href="README.en.md">🇺🇸 English</a> ·
  <strong>🇯🇵 日本語</strong>
</p>

<p>
  <a href="https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/actions/workflows/runtime-verification.yml"><img alt="Runtime Verification" src="https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/actions/workflows/runtime-verification.yml/badge.svg"></a>
  <img alt="C++17" src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus">
  <img alt="DirectX 11" src="https://img.shields.io/badge/DirectX-11-107C10?logo=windows">
  <img alt="Windows" src="https://img.shields.io/badge/platform-Windows-0078D4?logo=windows">
</p>

<p>
  <a href="https://www.youtube.com/watch?v=YtkRcS8QO4E"><strong>▶ デモ動画を見る</strong></a>
  &nbsp;·&nbsp;
  <a href="docs/runtime-platform.md">ランタイム設計ドキュメント</a>
</p>

</div>

---

本プロジェクトは、DirectX 11のレンダリングパイプラインを直接理解するためのグラフィックスデモとして始まりました。その後、完成済みデモの見た目を維持しながら、フレーム処理、エラー伝播、リソース寿命、モジュール境界を再設計し、**グラフィックス機能を利用するランタイムプラットフォーム**へ発展させました。

| 開発形態 | 初期開発 | エンジン・ランタイムのリファクタリング | 開発環境 |
|:---:|:---:|:---:|:---:|
| **1名（個人開発）** | 2023年10月–12月 | 2026年8月 | C++17 · VS 2022 · Windows 10/11 |

## プレビュー

| ノイズテクスチャによる炎 | 地形・ビルボード・インスタンシング |
|:---:|:---:|
| ![ノイズテクスチャによる炎](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/bf59dafa-3dc4-4421-88ff-8e048d593696) | ![地形シーン](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/f4b43db1-0b06-4a5e-a63b-c8b82e987963) |

## 主な成果

| 指標 | 結果 |
|---|---:|
| ビルド・検証マトリクス | **4/4成功** — Debug/Release × x86/x64 |
| Headlessランタイム検査 | **36/36成功** — 構成ごとに9項目 |
| 決定論の検証 | 異なるレンダー分割で10,000 tick実行後、同一hash `0x45309AA8C1381D56` |
| Core/Runtime内のWin32・DirectX include | **0件** |
| プロジェクトコード内の手動ownership site | **214か所 → 0か所** |
| `GraphicsClass`の規模 | **1,879 → 1,141 LOC（-39.3%）** |
| `Graphics/`ルート直下のC++ファイル | **74 → 4件（-94.6%）** |
| `GraphicsClass`から直接表示するエラーポップアップ | **15 → 0か所** |

これらの数値はFPS向上を示すものではありません。今回のリファクタリングの目的は性能最適化ではなく、**構造、再現性、障害診断、テスト容易性**の改善です。

## このプロジェクトで示す技術

### グラフィックスデモ

- Direct3D 11デバイス、スワップチェーン、レンダーステートの管理
- HLSLによるPhongライティングとスペキュラ反射
- 6-passのrender-to-textureソフトシャドウ
- Height mapベースの地形と地形高度に追従するカメラ
- Billboardとinstancingによる2,000個の草オブジェクト描画
- Noise・distortion・alpha textureによる炎エフェクト
- SkyDome、フォントHUD、マウスピッキング、衝突判定
- DirectInput 8によるカメラ操作とDirectSoundによるBGM/SFX
- Title/Hunt状態ごとに分離された更新・描画経路

### ランタイム・プラットフォームエンジニアリング

- 60 Hz fixed-step simulationとrender interpolationの分離
- プラットフォーム非依存の`EngineCore`・`EngineRuntime`静的ライブラリ
- 障害コンテキストを保持する`Error`と`Result<T>`
- timestamp、thread ID、categoryを含む構造化ログ
- `ComPtr`、`unique_ptr`、`vector`によるRAII ownership
- `ITexture`・`IRenderTexture`による共通テクスチャ契約
- GPU、ウィンドウ、オーディオ、アセットを必要としないheadless replay test
- PowerShell検証スクリプトとGitHub Actionsによる自動化

## ランタイムアーキテクチャ

レンダーループ自体をエンジンとして扱っていた構造から、レンダリングをランタイムの一利用者として扱う構造へ変更しました。

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

| ビルドターゲット | 責務 | プラットフォーム・グラフィックス依存 |
|---|---|---|
| `EngineCore.lib` | エラー、Result型、ログ、ownership utility | なし |
| `EngineRuntime.lib` | 固定timestepとsimulation/render契約 | なし |
| `PortfolioEngine.exe` | Win32ホスト、D3D11レンダラー、ポートフォリオゲーム | Win32、DirectX 11 |
| `PortfolioRuntimeTests.exe` | 決定論・エラー・schedulerのheadless検証 | なし |

プロジェクト参照の方向は`PortfolioEngine → EngineRuntime → EngineCore`です。CoreとRuntimeには`Windows.h`またはDirectXヘッダーが含まれないため、ウィンドウやグラフィックスデバイスなしで独立してビルド・テストできます。

### Fixed-step frame contract

```text
PumpMessages
→ MeasureFrameTime
→ ClampAndAccumulate（最大100 ms）
→ FixedUpdate（1/60 s、1フレーム最大8回）
→ Render（interpolation alpha）
→ Present
```

レンダーFPSとシミュレーション速度を分離しました。長時間の停止後に無制限のupdateが発生しないようcatch-up budgetを制限し、破棄したbacklogは計測してログへ記録します。

### エラーとログ

従来の`bool`戻り値は失敗した事実しか伝えられず、どのsubsystemと呼び出し位置で問題が起きたのかという情報を失っていました。現在のApplication/Game境界では、エラーコード、subsystem、message、file、function、lineを保持する`Result<T>`を使用します。最終的なエラー表示はWin32 launcherだけに残し、内部レイヤーは記録と伝播に集中します。

```text
2026-08-28 ... [Error] [thread] [PortfolioGame] message (file:function:line)
```

実行ログはプロセスの作業ディレクトリを基準に`logs/PortfolioEngine.log`へ記録されます。

### テクスチャ契約とownership

DDSテクスチャとrender targetが共通のshader resource view契約を利用するよう構成しました。利用側はDDS loader、RTV/DSV、内部の`ComPtr`実装を知る必要がありません。

```cpp
using namespace Engine::Rendering;

TexturePtr albedo = MakeTexture<DdsTexture>(
    device,
    L"data/stone01.dds");

RenderTexturePtr shadowMap = MakeTexture<RenderTargetTexture>(
    device,
    RenderTextureDescriptor{2048, 2048, 1000.0f, 0.1f});
```

両方の型が`GetShaderResourceView()`を提供し、render target固有の操作は`IRenderTexture`だけが公開します。生成に失敗した場合は空のsmart pointerを返し、それ以前に生成されたDirect3DリソースはRAIIによって解放されます。

## 問題解決事例

| 変更箇所 | 問題と原因 | 変更方法 | 解決結果 | 定量的な成果・検証 |
|---|---|---|---|---|
| ビルド境界 | 1つの実行プロジェクトがCore、Win32、D3D11、ゲームをすべてコンパイルし、依存方向を強制できなかった | Core・Runtimeを静的ライブラリへ分離し、ゲーム・テスト実行ファイルを追加 | グラフィックスデバイスなしでランタイムを検証可能 | ターゲット **1 → 4件**、Core/Runtimeのplatform include **0件** |
| フレームループ | 可変delta updateによりレンダーFPSとゲーム進行速度が結合していた | 60 Hz fixed step、100 ms clamp、最大8 tick、補間値を導入 | レンダー頻度とsimulation進行を分離 | 2種類のrender partitionで10,000 tick後のhashが一致 |
| リソース寿命 | raw COM pointerと分散した`new/delete/Release()`により、部分初期化の失敗と終了順序に弱かった | `ComPtr`、`unique_ptr`、`vector`、scope guardを適用 | 所有者と破棄時点を型で表現 | 手動ownership site **214 → 0か所** |
| 巨大なゲームクラス | 入力、シーン、レンダーパス、オーディオ、エラー処理が1クラスに集中 | 責務をApplication、Platform、Input、Scene、Pipeline、Textureへ移動 | 変更の影響範囲と探索範囲を縮小 | `GraphicsClass` **-738 LOC（-39.3%）** |
| 動物モデル | clean cloneにOBJがなく、cube fallbackが失敗を隠したため、熊・七面鳥・馬が箱として表示された | 元モデルを復元し、座標変換を修正。必須assetの欠落時はtyped errorを返すよう変更 | 不正な代替モデルではなく実際の原因を即座に確認可能 | OBJ **4件**、**230,504 triangles**を復元 |
| Title画面の炎 | alpha textureのA channelは常に255だったが、shaderがAを参照したため長方形で表示された | grayscale maskが保存されたR channelを最終alphaへ適用 | 1つのalpha blend stateを維持したまま炎の輪郭を復元 | Aの**1値**ではなくRGBの**256段階**を使用 |
| 物理ディレクトリ | 74個のC++ファイルが`Graphics/`ルートに混在し、パスと責務が一致していなかった | `Core`、`Runtime`、`Platform`、`Scene`、`Rendering`、`Game`などへ移動 | パスから責務と依存レイヤーを判断可能 | ルートC++ **74 → 4件**、project/filter **122/122一致** |
| 回帰検証 | 構成別ビルドとランタイム結果を再現する自動手順がなかった | `/W4 /WX`、headless test、PowerShell matrix、CIを追加 | ローカルとCIが同じ手順を実行 | ビルド **4/4**、ランタイム検査 **36/36** |

<details>
<summary><strong>定量指標の詳細を見る</strong></summary>

### 定量的な変化

比較基準はリファクタリング前のコミット`132e7ec`とランタイムプラットフォームのコミット`f1fbe64`です。LOCは空行、コメント、プリプロセッサ指示を含むphysical line countです。

| 指標 | 変更前 | 変更後 | 変化 |
|---|---:|---:|---:|
| `GraphicsClass`（`.cpp + .h`） | 1,879 LOC | 1,141 LOC | **-738（-39.3%）** |
| `graphicsclass.cpp` | 1,694 LOC | 1,017 LOC | **-677（-40.0%）** |
| プロジェクト所有のC++ | 79ファイル / 19,569 LOC | 99ファイル / 17,080 LOC | LOC **-2,489（-12.7%）** |
| `SystemClass` | 2ファイル / 431 LOC | 削除 | 起動・ウィンドウ責務を移動 |
| `Graphics/`ルート直下のC++ | 74ファイル | 4ファイル | **-70（-94.6%）** |
| 外部・レガシー統合ファイル | 98ファイル / 1,218,891 bytes | 削除 | Assimp・ImGui・SystemClassを削除 |
| project/filter明示項目 | 不一致の可能性あり | 122 / 122 | 欠落・超過0 |
| `GraphicsClass`のランタイムエラーポップアップ | 15か所 | 0か所 | typed resultとして伝播 |
| 自動ビルドマトリクス | なし | 4構成 | warning 0、error 0 |
| 決定論ランタイム検査 | なし | 36 / 36 | 構成ごとに10,000 tick |

手動ownershipの数値は、プロジェクトコード内の呼び出し箇所を対象としたheuristicです。コメント、`operator new/delete`、コピー禁止の`= delete`、未変更の`DDSTextureLoader`と`AlignedAllocationPolicy`は除外しています。これは構造改善を示す指標であり、メモリ安全性の数学的な証明ではありません。

</details>

## ビルドと検証

### 必要環境

- Windows 10またはWindows 11
- Visual Studio 2022以降
- `Desktop development with C++`ワークロード
- C++17対応MSVC toolsetとWindows SDK

### Visual Studioから実行

1. リポジトリルートの`Graphics.sln`を開きます。
2. `Debug/Release × x86/x64`から構成を選択します。
3. `PortfolioEngine`をスタートアッププロジェクトに設定します。
4. ソリューションをビルドして実行します。

プロジェクトに指定されたtoolsetがない場合は、C++17設定を維持したまま、インストール済みのMSVC toolsetへ再ターゲットしてください。ランタイムassetはビルド時に出力ディレクトリへコピーされます。

### 自動検証

リポジトリルートのPowerShellで実行します。

```powershell
# Debug/Release × x64
.\scripts\verify.ps1

# Debug/Release × x86/x64
.\scripts\verify.ps1 -AllArchitectures

# replayの長さを変更
.\scripts\verify.ps1 -AllArchitectures -ReplayTicks 20000
```

検証スクリプトはVisual StudioのMSBuildを検出し、各構成をビルドして`PortfolioRuntimeTests.exe`を実行します。デフォルトのreplayは構成ごとに10,000 tickで、GPU、ウィンドウ、オーディオ、assetを必要としません。

### 操作方法

| 入力 | 動作 |
|---|---|
| `W` / `A` / `S` / `D` | カメラを前後左右へ移動 |
| マウス移動 | カメラの視点を回転 |
| マウス左ボタン | Titleから開始 / Hunt中のモデル選択 |
| `Esc` | 終了 |

<details>
<summary><strong>ディレクトリ構成を見る</strong></summary>

### ディレクトリ構成

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

## 設計範囲と今後の課題

今回の段階では、ランタイム基盤の検証前にECS、custom allocator、DLL plugin、独立render threadを導入することを避けました。現在の境界と制約も明示しています。

- `Platform.Win32`と`Renderer.D3D11`は論理的には分離されていますが、まだ`PortfolioEngine`プロジェクト内で一緒にビルドされます。
- 新しいApplication/Gameエラー境界は`Result<T>`を使用しますが、一部のレガシーshader compile helperにはmodal errorが残っています。
- assetはまだ`data/`相対パスを使用します。次の段階はVFS、`AssetId`、generationベースのhandle、manifest、cookerです。
- 同一条件で変更前後の性能benchmarkを行っていないため、FPS向上は主張しません。

詳細なフレーム契約、エラーモデル、headless検証方式については、[Runtime Platform Architecture](docs/runtime-platform.md)を参照してください。

## 背景と出典

初期デモは、DirectX 11とゲームエンジン内部のレンダリング処理を学ぶため、Rastertekのサンプルを教材として開発を始めました。その後、Phong specular、soft shadow、instancing、fire effect、picking、ゲーム状態を拡張し、完成した成果物をプラットフォーム・ランタイムの観点から再設計しました。現在は内蔵OBJ loaderとfont HUDを使用し、AssimpとImGuiなしで動作します。
