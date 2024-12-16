# DirectX11DemoForPortfolio_NSY
 다이렉트X11로 구현한 데모입니다.

기간: 2개월

특징
- Sky Box를 구현
- Collision Detecting를 구현해 마우스 좌클릭에 detecting된 3D 모델을 비활성화 처리
- Keyboard 처리를 통한 카메라 이동
- 3D 모델은 단일 텍스처 처리
- Terrain을 사용하여 효과적인 지형 생성
- 퐁 일루미네이션 모델을 사용한 라이팅으로 Diffuse, Ambient, specular 구현
- Render Texture를 통한 그림자를 구현
- 2D 모델의 빌보드와 인스턴스를 구현
- 불 효과를 위해 노이즈 텍스처를 사용
- Direct Sound를 통한 BGM과 효과음
- 렌더링 구역을 나누어 multi scene을 구현
- 폰트 렌더링을 통한 Scene info(FPS, CPU 사용률, 현재 모델 수, 화면 크기 등) 구현
![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/bf59dafa-3dc4-4421-88ff-8e048d593696)

노이즈 필터를 사용한 불, 텍스트 박스, 마우스 커서입니다.

![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/f4b43db1-0b06-4a5e-a63b-c8b82e987963)

하이트 맵을 사용한 Terrain, 인스턴싱(2000개)한 2D 모델(풀)과 Billboard, 퐁 일루미네이션 모델, 그림자, Direct Input을 이용한 이동 및 카메라 이동을 구현한 Scene입니다. 

Video URL: https://www.youtube.com/watch?v=YtkRcS8QO4E

// In English

It is demo made by DirectX11.

Duration: 2 months

Feature
 - SkyBox
 - Collision Detecting (Left Mouse Button Clicking can detect 3D model and make it inactive)
 - Camera can move in Keyboard input
 - Single texturing in 3D Model
 - Making Terrain by Height map
 - Diffuse, Ambient, specular in Phong Illumination model
 - Using Render Texture for soft shadow effect
 - Billboard and Instancing for 2D model
 - Using Noise mapping(Fire effect)
 - Sound by Direct Sound(BGM, SFX)
 - Multi Scene(Using enum type for seperating Rendering sectors)
 - Font Rendering for Scene info(FPS, CPU usage, numbers of present models in Scene, Screen size etc)
![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/bf59dafa-3dc4-4421-88ff-8e048d593696)

Fire by Noise filter, Textbox, and Mouse Cursor

![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/f4b43db1-0b06-4a5e-a63b-c8b82e987963)

Height map terrain, Billboard and Instancing(2000 counts) for 2D model(Grass), Phong Illumination model, Soft shadow, Moving and Camera moving by Direct Input 

Video URL: https://www.youtube.com/watch?v=YtkRcS8QO4E

// 日本語

DirectX11で実装したデモです。

機関：2ヶ月

特徴
- Sky Boxを実装
- Collision Detectingを実装し、マウス左クリックで検出された3Dモデルを非アクティブ化
- キーボード操作によるカメラ移動
- 3Dモデルは単一テクスチャで処理
- Terrainを使用して効果的な地形生成
- フォン・イルミネーションモデルを用いたライティングで、Diffuse、Ambient、Specularを実装
- Render Textureを用いた影の実装
- 2Dモデルのビルボードとインスタンシングを実装
- 火のエフェクトにノイズテクスチャを使用
- Direct Soundを利用してBGMと効果音を実装
- レンダリング領域を分割し、マルチシーンを実装
- フォントレンダリングを利用して、シーン情報（FPS、CPU使用率、現在のモデル数、画面サイズなど）を表示
![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/bf59dafa-3dc4-4421-88ff-8e048d593696)

ノイズフィルターを用いた火、テキストボックス、マウスカーソルの実装

![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/f4b43db1-0b06-4a5e-a63b-c8b82e987963)

ハイトマップを使用したTerrain、インスタンシング（2000個）された2Dモデル（草）とビルボード、フォン・イルミネーションモデル、影、Direct Inputを利用した移動およびカメラ移動を実装したシーンです。

Video URL: https://www.youtube.com/watch?v=YtkRcS8QO4E
