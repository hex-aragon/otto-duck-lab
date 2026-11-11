> **변경 고지 (Apache-2.0 §4):** 이 파일은 [apirrone/Open_Duck_Mini](https://github.com/apirrone/Open_Duck_Mini) (Apache License 2.0) 원문을 한국어로 번역하고 "번역자 참고"를 덧붙인 수정본이다. 라이선스 전문: [LICENSE-Apache-2.0.txt](../../LICENSE-Apache-2.0.txt). 원저작자와 관계없는 비공식 번역이다.
>
> **번역본입니다.** 원문: [Open_Duck_Mini/docs/sim2real.md](https://github.com/apirrone/Open_Duck_Mini/blob/v2/docs/sim2real.md) · 번역일 2026-11-02 · 명령과 코드는 원문 그대로이며 주석만 옮겼습니다.
>
> **번역자 참고**
> - 원문의 "BDX 논문" 링크는 실제로 `joystick.py`를 가리킵니다. 원문대로 두었습니다. 논문 PDF는 https://la.disneyresearch.com/wp-content/uploads/BD_X_paper.pdf 입니다(Playground README에 적힌 주소).

> 아직 확정되지 않은 문서입니다.

# 실제 로봇으로 옮겨지는 정책 학습하기 (sim2real)

실제 로봇에서도 잘 동작하는 정책을 학습하려고 합니다. 이것이 sim2real 문제입니다. 어려운 문제이고, 특히 저희는 모델로 만들기 어렵고 힘도 그리 세지 않은 저렴한 서보모터를 쓰기 때문에 더 어렵습니다.

아래에 저희가 거친 단계를 대략 설명합니다. 각 과정의 결과를 제공하므로 모든 것을 직접 다시 할 필요는 없습니다.

## 로봇의 정확한 모델 만들기 (URDF/MJCF)

### 로봇 구조

[Onshape 문서](https://cad.onshape.com/documents/64074dfcfa379b37d8a47762/w/3650ab4221e215a4f65eb7fe/e/0505c262d882183a25049d05)에 부품별 재질을 지정해 두었습니다. 하지만 속을 채움 비율대로 출력하므로, 더 정확하도록 부품 질량을 슬라이서의 (꽤 정확한) 추정값으로 덮어씁니다.

URDF/MJCF 설명 파일을 내보낼 때 [onshape-to-robot](https://github.com/Rhoban/onshape-to-robot)을 씁니다. MJX용으로는 가벼운 모델을 만들어야 합니다. 저희의 [config.json](https://github.com/apirrone/Open_Duck_Playground/blob/main/playground/open_duck_mini_v2/xmls/config.json)을 보세요.

이렇게 하면 로봇 전체의 질량과 관성 모멘트를 담은 MJCF(MuJoCo 형식) [로봇 설명 파일](https://github.com/apirrone/Open_Duck_Playground/blob/main/playground/open_duck_mini_v2/xmls/open_duck_mini_v2.xml)이 나옵니다.

### 모터

정확한 모델에서 또 하나 아주 중요한 것은 모터의 움직임을 모델로 만드는 일입니다. 저희는 [BAM](https://github.com/Rhoban/bam/)을 씁니다. 측정 과정을 직접 할 필요는 없습니다. 결과를 [여기](https://github.com/Rhoban/bam/tree/main/params/feetech_sts3215_7_4V)에 제공합니다.

시뮬레이터가 모터를 정확하게 흉내 내는 것은 매우 중요합니다. 정책(신경망)이 감각 입력(모터 위치와 속도, IMU, 발 센서)을 받아 모터 위치를 내도록 학습하기 때문입니다. 모터가 시뮬레이션과 현실에서 다르게 움직이면 정책이 동작하지 않거나, 최악의 경우 엉망인 움직임을 만듭니다.

`BAM`으로 측정한 주요 값을 MuJoCo 단위로 내보낼 수 있습니다(`bam.to_mujoco` 사용). 이 값들을 mjcf 모델의 구동기와 관절 속성에 넣습니다.

- damping (감쇠)
- kp (위치 제어 세기)
- frictionloss (마찰 손실)
- armature (회전부 관성)
- forcerange (힘 범위)

## 정책 학습

[MuJoCo Playground](https://github.com/google-deepmind/mujoco_playground) 기반으로 직접 만든 프레임워크 [Open Duck Playground](https://github.com/apirrone/Open_Duck_Playground)를 씁니다.

[joystick](https://github.com/apirrone/Open_Duck_Playground/blob/main/playground/open_duck_mini_v2/joystick.py) 환경에서 여러 보상을 켜고 끄거나, 직접 작성하거나, 가중치와 잡음, 무작위 조건 등을 바꿔 볼 수 있습니다.

디즈니가 [BDX 논문](https://github.com/apirrone/Open_Duck_Playground/blob/main/playground/open_duck_mini_v2/joystick.py)에서 설명한 모방 보상을 구현해 좋은 결과를 얻었습니다.

이 보상을 쓰려면 기준 동작이 필요합니다. 매개변수 방식의 걷기 엔진으로 동작을 만들기 위해 [이 저장소](https://github.com/apirrone/Open_Duck_reference_motion_generator)를 만들었습니다. 거기 안내대로 하면 기준 동작을 담은 `polynomial_coefficients.pkl` 파일이 나옵니다. Playground 저장소의 `data/` 폴더에도 이미 이런 파일이 있습니다.

정책 학습이 끝나면 실행 코드 저장소의 [이 스크립트](https://github.com/apirrone/Open_Duck_Mini_Runtime/blob/v2/scripts/v2_rl_walk_mujoco.py)로 실제 로봇에서 돌려 볼 수 있습니다. 그 전에 [점검 목록](https://github.com/apirrone/Open_Duck_Mini_Runtime/blob/v2/checklist.md)을 모두 끝냈는지 확인하세요.
