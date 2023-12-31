# DirectX11DemoForPortfolio_NSY
 다이렉트X11로 구현한 데모입니다.

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
