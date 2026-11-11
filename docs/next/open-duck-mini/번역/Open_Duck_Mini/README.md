> **변경 고지 (Apache-2.0 §4):** 이 파일은 [apirrone/Open_Duck_Mini](https://github.com/apirrone/Open_Duck_Mini) (Apache License 2.0) 원문을 한국어로 번역하고 "번역자 참고"를 덧붙인 수정본이다. 라이선스 전문: [LICENSE-Apache-2.0.txt](../LICENSE-Apache-2.0.txt). 원저작자와 관계없는 비공식 번역이다.
>
> **번역본입니다.** 원문: [Open_Duck_Mini/README.md](https://github.com/apirrone/Open_Duck_Mini/blob/v2/README.md) · 번역일 2026-11-02 · 명령과 코드는 원문 그대로이며 주석만 옮겼습니다.
>
> **번역자 참고**
> - 원문의 첫 소식 날짜는 `02/04/2024`지만, 앞뒤 내용으로 보아 2025년의 오타일 수 있습니다. 원문대로 옮겼습니다.

# Open Duck Mini v2

<table>
  <tr>
    <td> <img src="https://github.com/user-attachments/assets/2a407765-70ad-48dd-8a5d-488f82503716" alt="1" width="300px" ></td>
    <td> <img src="https://github.com/user-attachments/assets/3b8fe350-73a9-4c9f-ad29-efc781be7aee" alt="2" width="300px" ></td>
    <td> <img src="https://github.com/user-attachments/assets/fd7e5949-1492-4d31-851f-feaa9b695557" alt="3" width="300px" ></td>
   </tr> 
</table>

디즈니의 BDX 드로이드를 작게 만들고 있습니다. 다리를 폈을 때 키는 약 42cm입니다.
전체 부품비는 400달러 아래여야 합니다!

이 저장소는 프로젝트 자료를 한데 모으는 중심지 같은 곳입니다. 작업 중인 저장소라 설명 없는 스크립트가 많습니다 :) 언젠가 정리해 보겠습니다.


# 가상에서 실물로 옮긴(sim2real) 현황

https://github.com/user-attachments/assets/58721d0f-2f95-4088-8900-a5d02f41bba7

https://github.com/user-attachments/assets/4129974a-9d97-4651-9474-c078043bb182

https://github.com/user-attachments/assets/a0afcd38-15d8-40c6-8171-a619107406b8


# 소식

> 2024/04/02 소식: 저희가 학습한 정책 두 개를 시험해 볼 수 있습니다. [첫 번째](https://github.com/apirrone/Open_Duck_Mini/blob/v2/BEST_WALK_ONNX.onnx)와 [두 번째](https://github.com/apirrone/Open_Duck_Mini/blob/v2/BEST_WALK_ONNX_2.onnx)입니다.
> 다음 인수로 실행하세요.
> python v2_rl_walk_mujoco.py --onnx_model_path ~/BEST_WALK_ONNX_2.onnx

> 2025/03/15 소식: 도움을 받거나 내 오리를 보여 주고 싶다면 디스코드 서버에 들어오세요 :) https://discord.gg/UtJZsgfQGe

> 2025/02/07 소식: sim2real에 큰 진전이 있었습니다. 위 영상을 보세요 :)

> 2025/02/24 소식: sim2real 작업을 열심히 하고 있습니다!

> 2025/02/07 소식: 문서를 작업하면서 함께 쓰고 있지만, 설계와 부품표가 크게 바뀌지는 않을 겁니다. "표현" 기능은 아직 없지만 로봇을 만든 뒤에 추가할 수 있습니다!

> 2025/01/22 소식: 기구 설계는 거의 확정되었습니다(군데군데 실수를 고치는 중). 현재 버전에는 최종 로봇에 넣으려는 "표현" 기능(눈 LED, 카메라, 스피커, 마이크)이 다 들어 있지 않습니다. 이제 강화학습으로 걷게 만드는 작업을 하고 있습니다!

# 커뮤니티

![duck_collage](https://github.com/user-attachments/assets/e240c06e-769f-4c87-b65f-189a442cf1e9)

디스코드 커뮤니티에 들어오세요! https://discord.gg/UtJZsgfQGe

# CAD

https://cad.onshape.com/documents/64074dfcfa379b37d8a47762/w/3650ab4221e215a4f65eb7fe/e/0505c262d882183a25049d05

Onshape 설계에서 MuJoCo 시뮬레이션 로봇까지 가는 방법은 [이 문서](docs/prepare_robot.md)를 보세요. (주의: 오래된 문서이며 한동안 갱신되지 않았습니다)

# 강화학습 자료

MuJoCo Playground로 옮기는 중입니다. [이 저장소](https://github.com/apirrone/Open_Duck_Playground)를 보세요.

https://github.com/user-attachments/assets/037a1790-7ac1-4140-b154-2c901d20d5f5


## 모방 학습용 기준 동작 생성

https://github.com/user-attachments/assets/4cb52e17-99a5-47a8-b841-4141596b7afb

[이 저장소](https://github.com/apirrone/Open_Duck_reference_motion_generator)를 보세요.

## 모터 특성 측정

Rhoban의 [BAM](https://github.com/Rhoban/bam)을 사용했습니다.

# 부품표(BOM)

https://docs.google.com/spreadsheets/d/1gq4iWWHEJVgAA_eemkTEsshXqrYlFxXAPwO515KpCJc/edit?usp=sharing

중국어판: https://zihao-ai.feishu.cn/wiki/AfAtw69vRigXaRk5UkbcrAiLnJw?from=from_copylink

# 제작 안내

> 새 소식: 이제 Tnkr 가이드를 쓸 수 있습니다! https://tnkr.ai/explore/docs/open-duck-mini/open-duck-mini-v2#home

중국어판: https://zihao-ai.feishu.cn/wiki/space/7488517034406625281

## 출력 안내

[출력 안내](docs/print_guide.md)를 보세요.

## 조립 안내

[조립 안내(미완성)](docs/assembly_guide.md)를 보세요.

# 로봇 위 실행 코드

이 저장소에 온보드 컴퓨터(Raspberry Pi Zero 2W)에서 정책을 돌리는 코드가 있습니다. https://github.com/apirrone/Open_Duck_Mini_Runtime

# 내 정책 학습하기

직접 정책을 학습해서 오리가 더 잘 걷게 돕고 싶다면 [이 문서](docs/sim2real.md)를 보세요.

> 이 프로젝트를 후원해 준 Hugging Face와 Pollen Robotics에 깊이 감사드립니다!
