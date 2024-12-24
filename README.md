# DirectX11DemoForPortfolio_NSY
다이렉트X11로 구현한 데모입니다.

개발 동기: 게임 엔진에 기반이 되는 DirectX11을 직접 건드려보고 다양한 그래픽 기술을 구현해보고 싶었습니다.

개발엔진: DirectX11

개발인수: 1명

동작환경: Window 11

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

내가 가장 주의를 기울인 부분

Unity와 달리, 오류가 발생했을 때 그 원인을 특정하기 어려운 환경이었기 때문에, 신중하고 보수적으로 개발을 진행하도록 했습니다. 이러한 접근을 통해 예상치 못한 문제를 최소화하려고 노력했습니다.


가장 어려웠던 점

솔직히 말씀드리면, C++를 이용한 DirectX 개발은 이번이 처음이었고, 더 나아가 C++로 무언가를 제작하는 것 자체도 처음이었습니다. 따라서 객체 지향 설계가 충분히 이루어지지 않았던 점이 가장 어려웠던 부분입니다. 이 경험을 통해 설계의 중요성을 다시 한번 배우게 되었습니다.


가장 공들인 부분

최종적으로 완벽하게 객체 지향을 준수할 수는 없었지만, 노이즈 필터, 빌보드, 소프트 섀도우 등 다양한 기술을 구현하려고 노력했습니다. 이러한 기술들이 어떻게 작동하는지 주목해 주시면 감사하겠습니다.


참고한 소스 파일

모든 기술의 소스 코드는 Rastertek 예제 파일의 코드를 참고했습니다. 하지만, 실제로 작동시키고, 적절하게 배치한 것은 저 자신의 능력입니다. 예를 들어, 예제 파일의 섀도우 셰이더는 Phong Illumination을 완벽하게 지원하지 않았기 때문에, 스페큘러 성분을 추가하도록 셰이더를 조정했습니다. 또한, 소프트 섀도우를 구현하기 위해 셰이더 코드를 수정하는 등 스스로 공부를 하며 살을 덧붙였습니다.

![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/bf59dafa-3dc4-4421-88ff-8e048d593696)

노이즈 필터를 사용한 불, 텍스트 박스, 마우스 커서입니다.

![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/f4b43db1-0b06-4a5e-a63b-c8b82e987963)

하이트 맵을 사용한 Terrain, 인스턴싱(2000개)한 2D 모델(풀)과 Billboard, 퐁 일루미네이션 모델, 그림자, Direct Input을 이용한 이동 및 카메라 이동을 구현한 Scene입니다. 

Video URL: https://www.youtube.com/watch?v=YtkRcS8QO4E

// In English

It is demo made by DirectX11.

Development Motivation:
I wanted to work directly with DirectX11, which serves as the foundation for game engines, and implement various graphics technologies.

Engine: DirectX11

Development Personnel: 1 Person

Operating Environment: Windows 11

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

What were I careful about when creating the program?

Unlike Unity, it was a difficult environment to identify the cause of an error, so I tried to proceed with development cautiously and conservatively. Through this approach, I tried to minimize unexpected troubles.


What did I find difficult when creating the program?

To be honest, this was my first experience developing DirectX using C++, and it was also my first time creating something with C++. Therefore, the most difficult part was that object-oriented design was not sufficiently implemented. Through this experience, I was able to learn the importance of design once again.


What points in the program do I want you to pay particular attention to in the parts I worked hard on?

Although the source code could not fully comply with object-oriented programming in the end, I tried to implement various techniques such as noise filters, billboards, and soft shadows. I would appreciate it if you could pay attention to how these techniques work.


If I took some source codes as reference, what part of them did you use?

To be honest, I referred to the code in the example files for the source code of all the techniques. However, it was my own skill that made them actually work and rearranged them appropriately. For example, the shadow shader in the example file did not fully support Phong Illumination, so I adjusted the shader to add a specular component. I also made my own modifications, such as modifying the shader code to achieve soft shadows.

![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/bf59dafa-3dc4-4421-88ff-8e048d593696)

Fire by Noise filter, Textbox, and Mouse Cursor

![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/f4b43db1-0b06-4a5e-a63b-c8b82e987963)

Height map terrain, Billboard and Instancing(2000 counts) for 2D model(Grass), Phong Illumination model, Soft shadow, Moving and Camera moving by Direct Input 

Video URL: https://www.youtube.com/watch?v=YtkRcS8QO4E

// 日本語

DirectX11で実装したデモです。

開発動機:
ゲームエンジンの基盤となるDirectX11を直接扱い、さまざまなグラフィック技術を実装してみたかったからです。

開発エンジン：DirectX11

開発人数: 1名

動作環境：Windows 11

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


一番注意した事

Unityとは異なり、エラーが発生した際にその原因箇所を特定するのが難しい環境だったため、慎重かつ保守的に開発を進めるようにしました。このアプローチにより、予期しないトラブルを最小限に抑えるよう努めました。


大変だった所

正直に言うと、C++を利用したDirectXの開発はこれが初めての経験であり、さらにC++で何かを作成すること自体も初めてでした。そのため、オブジェクト指向設計が十分に行われていなかった点が最も困難だった部分です。この経験を通じて、設計の重要性を改めて学ぶことができました。


力をいれて作った部分

最終的にソースコードは完全にオブジェクト指向を遵守することはできませんでしたが、ノイズフィルター、ビルボード、ソフトシャドウなど、さまざまな技術を実装しようと努力しました。これらの技術がどのように機能しているかに注目していただければ幸いです。


参考にしたソースファイル

正直に言うと、すべての技術のソースコードは例題ファイルのコードを参考にしました。しかし、それらを実際に動作させ、適切に配置し直したのは私自身のスキルによるものです。たとえば、例題ファイルのシャドウシェーダーはPhong Illuminationを完全にサポートしていなかったため、スペキュラ成分を追加するようにシェーダーを調整しました。また、ソフトシャドウを実現するためにシェーダーコードを修正するなど、自ら工夫を加えました。


![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/bf59dafa-3dc4-4421-88ff-8e048d593696)

ノイズフィルターを用いた火、テキストボックス、マウスカーソルの実装

![image](https://github.com/namsy8471/DirectX11DemoForPortfolio_NSY/assets/31647755/f4b43db1-0b06-4a5e-a63b-c8b82e987963)

ハイトマップを使用したTerrain、インスタンシング（2000個）された2Dモデル（草）とビルボード、フォン・イルミネーションモデル、影、Direct Inputを利用した移動およびカメラ移動を実装したシーンです。

Video URL: https://www.youtube.com/watch?v=YtkRcS8QO4E
