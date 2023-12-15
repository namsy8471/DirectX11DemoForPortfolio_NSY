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

구현에서 어려웠던 점 (Implementation issues)

노이즈 텍스처를 사용하기 위해 튜토리얼을 제 프로젝트에 맞게 변경했을 때 원인불명의 오류로 Alpha 텍스처의 값이 제대로 적용되지 않아 불꽃 모양대로 투명이 되지 않았습니다. 아마도 제 프로젝트에서 다양한 튜토리얼을 제 프로젝트 규격에 맞추다 보니 어느 것이 틀어져서 그런 것 같지만 이번 프로젝트 내에서 고칠 수 없다고 판단하여 스테이지 이전의 타이틀 화면에 넣게 됐습니다.

 Sky Box를 구현하는 과정에서 Texture된 SkyBox를 넣어야 하여 기존 튜토리얼(Color Texture Skybox)에서 DDS Texture(6 texture) SkyBox를 구현하려 했으나 너무 복잡하고 고쳐야 할 부분이 많고 시간 부족 및 역량 부족으로 인해 DDS파일로 Texture하지 않고 Color로 Texture된 SkyBox를 넣게 되었습니다.
 
 그림자를 구현하는 과정에서 기존 튜토리얼은 그림자를 구현할 때 Phong illumination model에서 Specular를 사용하지 않아서 그것을 제 프로젝트에 넣는데 많은 시간이 걸렸습니다. 또한 부드러운 그림자를 구현하는 튜토리얼 역시 Specular를 사용하지 않아 최초에 제 프로젝트에 넣었을 당시 그림자 자체가 나타나지 않았으나 현재 프로젝트를 마감하는 시점에서는 그림자가 나타나기는 하나 조금 섬세한 부분은 잘려 있는 모습을 보입니다.
 
이러한 이유 때문에 프로젝트 요구 사항에는 multiple light source이지만 그림자를 렌더링할 경우 필드에 있는 Light의 수(light의 투영 메트릭스와 뷰 메트릭스)만큼 쉐이더에 더 넘겨줘야 했고 그 점이 제 프로젝트에서 multiple light source가 들어가기 힘들다고 생각하여 조금 더 다른 것을 구현하는 것에 시간을 할애하게 됐습니다.

 Collision Detecting을 시행했을 때 Model이 사라져야 한다면 Shutdown을 하여 Release하면 된다고 생각했으나 시행하니 프로그램이 튕겨버리는 현상이 발생했습니다. 이리 하여 Model Class 내에 isAlive 변수와 함수를 넣어 살아있다면 렌더링을 시행하고 죽어있다면 그 이후로 렌더링을 하지 않고 게임 내에 카운팅된 오브젝트 수와 버텍스 수를 빼도록 했습니다.
 
 그리고 Collision Detecting에서 Boundary bounding을 사용하지 않아 제대로 모델이 피킹되지 않는 현상도 일어납니다. 즉 모델 형태에는 피킹 되지 않으면서 모델이 있는 매트릭스 위치는 모델이 피킹되어 사라지는 현상이 일어납니다.
 
 마지막으로 Debug에서 Release로 변경할 때, 텍스트에 플리커링이 있었고 그 플리커링의 원인은 if문 혹은 Switch문 안에만 RenderSentence 함수가 존재할 때였습니다. if문 혹은 Switch문 밖에 RenderSentence 함수를 하나 빼놓으니 플리커링이 사라졌습니다. 제출하는 지금까지도 이런 플리커링이 왜 일어나는지 의문입니다.
