# 🎮 DirectX11 Demo for Portfolio – NSY  
다이렉트X11 기반으로 제작한 그래픽스 포트폴리오 데모  
DirectX11で実装したグラフィックスデモ  
A graphics demo implemented with DirectX 11

---

## 🎥 Preview / プレビュー / 미리보기  

| 🔥 Noise Fire Effect | 🌿 Terrain + Billboard + Instancing |
|:----------------------:|:----------------------------------:|
| ![Noise Fire](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/bf59dafa-3dc4-4421-88ff-8e048d593696) | ![Terrain Scene](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/f4b43db1-0b06-4a5e-a63b-c8b82e987963) |

🎬 **Video (YouTube):** [https://www.youtube.com/watch?v=YtkRcS8QO4E](https://www.youtube.com/watch?v=YtkRcS8QO4E)

---

## 🧠 개발 동기 / 開発動機 / Development Motivation  
**DirectX11**을 직접 다뤄보며 게임 엔진의 렌더링 원리를 이해하고, 다양한 그래픽 기술을 스스로 구현해보고자 했습니다.  
ゲームエンジンの基盤である **DirectX11** を直接扱い、レンダリングの仕組みを理解し、様々なグラフィック技術を自分で実装することを目的としました。  
I wanted to work directly with **DirectX 11**, the core API behind game engines, to understand rendering principles and implement various graphics techniques on my own.

---

## ⚙️ 개발 개요 / 開発概要 / Development Overview

| 항목 / 項目 / Item | 내용 / 内容 / Details |
|--------------------|----------------------|
| **엔진 / エンジン / Engine** | DirectX11 |
| **언어 / 言語 / Language** | C++ |
| **기간 / 期間 / Duration** | 2023/10 ~ 2023/12 |
| **개발 인원 / 開発人数 / Team** | 1명 (개인) / 1名（個人） / 1 (solo) |
| **환경 / 環境 / Environment** | Visual Studio 2022, Windows 11 |

---

## 🌟 주요 기능 / 主な機能 / Key Features

### 한국어 (Korean)
- 🌌 **SkyBox 구현** — 배경 큐브맵을 이용해 3D 공간의 하늘 표현  
- 🧱 **Collision Detection** — 마우스 좌클릭 시 감지된 3D 모델을 비활성화 처리  
- 🎮 **Keyboard 입력 기반 카메라 이동** — WASD 및 마우스 조작으로 자유로운 시점 이동  
- ⛰️ **Height Map 기반 Terrain 생성** — 고도 맵을 이용해 지형 자동 생성  
- 💡 **Phong Illumination 적용** — Diffuse, Ambient, Specular 조명 모델 구현  
- 🌫️ **Render Texture 기반 Soft Shadow** — 그림자의 부드러운 가장자리 표현  
- 🍃 **Billboard 및 Instancing (2000개 2D 오브젝트)** — 대량의 풀(Grass) 렌더링 최적화  
- 🔥 **Noise Texture 기반 불 효과** — 노이즈 맵을 활용한 실시간 파티클 불 구현  
- 🔊 **DirectSound** — BGM 및 효과음 재생  
- 🧭 **Multi Scene Rendering** — 씬 단위로 렌더링 영역 분리  
- 🖋️ **Font Rendering HUD** — FPS, CPU 사용률, 모델 수, 화면 해상도 표시  

---

### 日本語 (Japanese)
- 🌌 **SkyBox実装** — 背景キューブマップを使用して空間の空を表現  
- 🧱 **Collision Detection** — マウス左クリックで検出された3Dモデルを非アクティブ化  
- 🎮 **キーボード操作によるカメラ移動** — WASDとマウス操作で自由に視点を移動  
- ⛰️ **ハイトマップによる地形生成** — 高度マップから地形を自動生成  
- 💡 **フォン・イルミネーションモデル** — Diffuse、Ambient、Specularの光反射を実装  
- 🌫️ **Render Textureを使用したソフトシャドウ** — 柔らかい影を表現  
- 🍃 **ビルボードとインスタンシング（2000個の2Dオブジェクト）** — 草などの大量描画を最適化  
- 🔥 **ノイズテクスチャを用いた火のエフェクト** — ノイズマップを利用したリアルタイム炎表現  
- 🔊 **DirectSoundによるBGM・効果音再生**  
- 🧭 **マルチシーンレンダリング** — シーン単位でレンダリングを分割  
- 🖋️ **フォントレンダリングHUD** — FPS、CPU使用率、モデル数、画面解像度を表示  

---

### English
- 🌌 **SkyBox Implementation** — Rendered 3D sky using a background cubemap  
- 🧱 **Collision Detection** — Detects clicked 3D models and deactivates them on left mouse click  
- 🎮 **Camera Movement via Keyboard** — Move the camera freely using WASD keys and mouse input  
- ⛰️ **Height Map Terrain Generation** — Automatically generates terrain based on height maps  
- 💡 **Phong Illumination Model** — Implemented Diffuse, Ambient, and Specular lighting  
- 🌫️ **Soft Shadow via Render Texture** — Achieved smooth shadow edges using render-to-texture  
- 🍃 **Billboard and Instancing (2,000+ 2D Objects)** — Optimized large-scale grass rendering  
- 🔥 **Noise Texture-based Fire Effect** — Real-time flame simulation using noise textures  
- 🔊 **DirectSound Integration** — Implemented background music and sound effects  
- 🧭 **Multi-Scene Rendering** — Divided rendering space into separate scenes  
- 🖋️ **Font Rendering HUD** — Displays FPS, CPU usage, object count, and resolution info 

---

## 💡 어려웠던 점 / 苦労した点 / Challenges
Unity와 달리 오류의 원인을 추적하기 어려워 신중하고 보수적으로 개발했습니다.  
C++ 및 DirectX 프로젝트는 처음이라 **객체지향 설계 부족**을 겪으며, 설계의 중요성을 깊이 배웠습니다.  

Unityとは異なり、エラー原因の特定が難しい環境だったため、慎重かつ保守的に開発を進めました。  
C++やDirectXによる開発は初めてで、**オブジェクト指向設計の不足**に苦労し、設計の重要性を実感しました。  

Unlike Unity, debugging DirectX was difficult, so I took a careful and conservative development approach.  
It was my **first C++/DirectX project**, and I learned the importance of proper **object-oriented design** through the struggle.

---

## 🧩 기술적 포인트 / 技術的ポイント / Technical Highlights  
Rastertek 예제 코드를 기반으로 **스페큘러 성분 추가**, **소프트 섀도우 셰이더 수정** 등 직접 개선 및 확장.  
Direct3D 렌더링 파이프라인, 셰이더 스테이지, 최적화 구조를 체계적으로 이해함.  

Rastertekのサンプルコードを基に、**スペキュラ成分の追加**や**ソフトシャドウ実装**などを独自に改良。  
Direct3Dのレンダリングパイプラインやシェーダー構造を体系的に理解。  

Based on Rastertek examples, enhanced shaders by **adding specular reflection** and **implementing soft shadows**.  
Gained structured understanding of the **Direct3D pipeline**, shader stages, and performance optimization.

---

## 🧾 요약 / まとめ / Summary  
DirectX11을 통해 게임 엔진의 기반 구조를 이해하고, 그래픽스 렌더링의 저수준 원리를 체득했습니다.  
이 프로젝트는 Unity와 DirectX 사이의 기술적 간극을 메우는 첫 단계였습니다.  

DirectX11を通してゲームエンジンの基盤構造を理解し、グラフィックスレンダリングの低レベル原理を体得しました。  
本プロジェクトは、UnityとDirectXの技術的ギャップを埋める第一歩です。  

Through this project, I gained a solid understanding of low-level graphics rendering principles.  
It represents my **first step in bridging the gap between Unity and DirectX development.**

---

## 🔗 Links
- 🎥 **Video:** [YouTube](https://www.youtube.com/watch?v=YtkRcS8QO4E)  
- 💻 **GitHub:** [github.com/namsy8471/DirectX11DemoForPortfolio_NSY](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY)
