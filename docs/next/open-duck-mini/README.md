# Open Duck Mini v2 파악 자료

> ⚠️ **조사 메모 (미검증)** — 공개 자료를 팀 내부 공부용으로 요약한 것이다. 가격 · 배송 · 사양 · 이슈 상태는 조사 시점 기준이며 틀리거나 바뀌었을 수 있다. `TODO: 확인 필요`는 확인하지 못한 내용이다. 구매 · 제작 결정 전에 반드시 원본을 확인할 것. 각 프로젝트의 공식 입장이 아니다.

> 2026-11-02 조사 원본. "내 PC"는 리더 PC(엔비디아 그래픽카드 없음) 기준이다. 전체 흐름은 [../README.md](../README.md) 참고.

> 2026-11-02에 GitHub 저장소와 공식 부품표를 직접 읽고 정리했다. 조립이나 실행은 하지 않았다.
> GPT 대화 내용은 원본과 대조해 [6장](#6-gpt-내용-대조)에 따로 적었다.
> 저장소 문서의 한글 번역본은 원본 일부 저장소에 라이선스 표시가 없어 이 공개 저장소에 넣지 않았다. 필요하면 리더에게 요청.

## 1. 한 줄 요약

디즈니 BDX 로봇을 줄여 만드는 **공개 프로젝트**다. 다리를 폈을 때 키 약 42cm, 관절 14개이고, 라즈베리파이가 학습된 신경망(정책)을 1초에 50번 돌려 걷는다.

| 먼저 알아 둘 점 | 내용 |
|---|---|
| 조립 설명서 | 미완성. 저장소도 스스로 "문서 없는 스크립트가 많다"고 적음 |
| 도움 받을 곳 | 제작자들의 대화는 디스코드에 많음 |
| 보행 보장 | 없음. 그대로 만들었는데 잘 안 걷는다는 이슈가 열려 있음 (5장) |
| 후원 · 관계 | Hugging Face와 Pollen Robotics 후원. Microduck의 앞선 프로젝트 격 |

## 2. 저장소 지도

전부 `apirrone` 계정이다. **v2는 기본 브랜치 이름도 `v2`다.** `v1`을 보면 다른 로봇 자료다.

| 저장소 | 역할 | 언제 보나 | 최근 수정 | 라이선스 |
|---|---|---|---|---|
| [Open_Duck_Mini](https://github.com/apirrone/Open_Duck_Mini) | 중심. 3D 파일, 설명서, 학습된 정책 | 처음 | 2026-01-31 | Apache-2.0 |
| [Open_Duck_Mini_Runtime](https://github.com/apirrone/Open_Duck_Mini_Runtime) | 로봇 위 실행 코드 | 조립 전 모터 설정부터 | 2026-09-07 | **표시 없음** |
| [Open_Duck_Playground](https://github.com/apirrone/Open_Duck_Playground) | 정책 학습 | 걷게 만든 뒤 | 2025-08-11 | **표시 없음** |
| [Open_Duck_reference_motion_generator](https://github.com/apirrone/Open_Duck_reference_motion_generator) | 학습에 쓸 기준 걸음 생성 | 걸음을 바꿀 때 | 2026-02-09 | **표시 없음** |
| [Rhoban/bam](https://github.com/Rhoban/bam) | 모터 특성 측정 · 모델 (STS3215 지원) | 모터를 바꿀 때 | | 미확인 |

**라이선스 주의:** Runtime, Playground, 기준 걸음 저장소에는 라이선스 파일이 없다. 개인 학습에는 문제되기 어렵지만, 코드를 재배포하거나 제품에 넣으려면 작성자에게 확인해야 한다. 공개된다고 자유롭게 써도 된다는 뜻은 아니다.

## 3. 먼저 읽을 파일 (이 순서대로)

| # | 문서 | 내용 | 비고 |
|---|---|---|---|
| 1 | [README](https://github.com/apirrone/Open_Duck_Mini/tree/v2) | 전체 입구, 영상 | 10분 |
| 2 | [부품표](https://docs.google.com/spreadsheets/d/1gq4iWWHEJVgAA_eemkTEsshXqrYlFxXAPwO515KpCJc/edit?usp=sharing) | 부품, 수량, 링크, 메모 | [중국어판](https://zihao-ai.feishu.cn/wiki/AfAtw69vRigXaRk5UkbcrAiLnJw?from=from_copylink)도 있음 |
| 3 | [CAD (Onshape)](https://cad.onshape.com/documents/64074dfcfa379b37d8a47762/w/3650ab4221e215a4f65eb7fe/e/0505c262d882183a25049d05) | 3D로 내부 구조 보기 | 무료 계정 필요할 수 있음 |
| 4 | [print_guide](https://github.com/apirrone/Open_Duck_Mini/blob/v2/docs/print_guide.md) | 출력 목록 | 36종 51개 |
| 5 | [configure_motors](https://github.com/apirrone/Open_Duck_Mini/blob/v2/docs/configure_motors.md) | 모터 ID · 영점 | **조립 전에** 함 |
| 6 | [assembly_guide](https://github.com/apirrone/Open_Duck_Mini/blob/v2/docs/assembly_guide.md) | 조립, 핀 연결 | 미완성 |
| 7 | [Runtime README](https://github.com/apirrone/Open_Duck_Mini_Runtime/tree/v2) | 파이 설정, 시험, 걷기 | |
| 8 | [INSTALL](https://github.com/apirrone/Open_Duck_Mini_Runtime/blob/v2/docs/INSTALL.md) | 미리 만든 파이 이미지 | 이미지 위치 확인 필요 |
| 9 | [sim2real](https://github.com/apirrone/Open_Duck_Mini/blob/v2/docs/sim2real.md) | 가상 → 실물 옮기기 | 미완성 표시 |
| 10 | [BDX 논문](https://arxiv.org/abs/2501.05204) | 설계 사고방식, 모방 보상 | 여유 있을 때 |

보조 자료: [Tnkr 빌드 가이드](https://tnkr.ai/explore/docs/open-duck-mini/open-duck-mini-v2#home) · [중국어 빌드 가이드](https://zihao-ai.feishu.cn/wiki/space/7488517034406625281) · [디스코드](https://discord.gg/UtJZsgfQGe) · [이슈 목록](https://github.com/apirrone/Open_Duck_Mini/issues)

**쓰면 안 되는 자료:** `docs/prepare_robot.md`(저장소가 낡았다고 표시), `mini_bdx/robots/bdx`(이전 모델), `experiments/`(실험 코드)

## 4. 로봇 구조

### 하드웨어

```
[18650 2개 직렬] ─ BMS ─ 스위치 ┬─ 모터 버스 ─ 보드(Waveshare) ─ STS3215 14개
                               └─ 5V 변환 ─ Raspberry Pi Zero 2W
                                              ├ USB → 모터 보드 (/dev/ttyACM0)
                                              ├ I2C → BNO055 IMU
                                              ├ GPIO → 발 스위치 좌 · 우
                                              └ 블루투스 → Xbox 패드
```

연결도는 부품표와 핀 표를 보고 내가 그린 요약이다. 실제 배선은 assembly_guide의 전자 회로 그림을 따를 것.

### 모터 ID (`configure_motors.md`, 실행 코드와 일치 확인)

| 부위 | 관절 → ID |
|---|---|
| 왼쪽 다리 | hip_yaw 20 · hip_roll 21 · hip_pitch 22 · knee 23 · ankle 24 |
| 오른쪽 다리 | hip_yaw 10 · hip_roll 11 · hip_pitch 12 · knee 13 · ankle 14 |
| 목 · 머리 | neck_pitch 30 · head_pitch 31 · head_yaw 32 · head_roll 33 |

### 핀 연결 (`assembly_guide.md`)

| 장치 | 파이 핀 |
|---|---|
| BNO055 | VIN 1번(3V3), GND 9번, SDA 3번, SCL 5번 |
| 왼발 · 오른발 스위치 | 15번(GPIO 22) · 13번(GPIO 27), GND 9번 |
| 안테나 서보 PWM | 32번(GPIO 12) · 33번(GPIO 13), 5V 2번, GND 6번 |
| 눈 LED · 프로젝터 | 16번 · 18번 · 22번, GND 6번 |
| 스피커 앰프 MAX98357A | LRC 35번, BCLK 12번, DIN 40번, VIN 2번, GND 6번 |

### 소프트웨어 실행 흐름

```
scripts/v2_rl_walk_mujoco.py
 └ 50Hz 반복: IMU + 관절 + 발 스위치 + 패드 명령
              → ONNX 정책 → 목표 = 기본 자세 + 0.25 × 출력 → 모터
```

기본값: 다리 P 게인 30, 머리 8, 포트 `/dev/ttyACM0`. 설정 파일은 홈 폴더의 `duck_config.json`(관절 영점, IMU 뒤집힘, 부가 기능 켜기).

### 스크립트 (`Open_Duck_Mini_Runtime/scripts/`)

| 파일 | 용도 |
|---|---|
| `configure_motor.py --id <번호>` | 모터 하나씩 ID · 영점 설정 |
| `check_motors.py` | 전체 모터 연결 확인 |
| `check_voltage.py` | 전압 확인 |
| `find_soft_offsets.py` | 관절 영점 찾기 |
| `calibrate_imu.py`, `imu_server.py`, `imu_client.py` | IMU 보정 · 방향 확인 |
| `fc_test.py` | 발 스위치 시험 (파일명 기준 추정, 내용 미확인) |
| `turn_on.py`, `turn_off.py` | 힘 켜기 · 끄기 |
| `v2_rl_walk_mujoco.py` | 걷기 실행 |
| `record_data.py`, `plot_recorded_data.py` | 기록 · 그래프 |

### 학습된 정책 · 학습 환경

- 정책: [BEST_WALK_ONNX.onnx](https://github.com/apirrone/Open_Duck_Mini/blob/v2/BEST_WALK_ONNX.onnx), [BEST_WALK_ONNX_2.onnx](https://github.com/apirrone/Open_Duck_Mini/blob/v2/BEST_WALK_ONNX_2.onnx) (Runtime 문서 권장)
- 학습 프레임워크: MuJoCo Playground 기반, `jax[cuda12]` 의존성. 사실상 **엔비디아 그래픽카드가 필요**하고, 현재 내 PC에서는 찾지 못했다
- 처음 실물을 걷게 할 때는 학습이 필요 없다. 공개 정책을 쓰면 된다

```bash
# 학습 예시 (Playground README)
uv run playground/open_duck_mini_v2/runner.py --task flat_terrain_backlash --num_timesteps 300000000
# 기준 걸음 생성 (reference_motion_generator)
uv run scripts/auto_waddle.py -j8 --duck open_duck_mini_v2 --sweep --output_dir recordings
uv run scripts/fit_poly.py --ref_motion recordings/
```

생성된 `polynomial_coefficients.pkl`은 `playground/<로봇>/data/`에 넣는다. 위 명령은 문서에서 옮긴 것이고 실행하지 않았다.

## 5. 살 것을 정하기 전에 해결할 문제

**이 세 가지는 저장소에도 답이 정리돼 있지 않다.** 14개를 주문하기 전에 디스코드에서 확인할 것을 권한다.

### 문제 1: 어느 모터가 정답인가

- 부품표에는 "7.4V, 19kg.cm 모터"라고만 적혀 있다. `C001`이라는 품번은 부품표에 없다
- 시중에서 그 조건은 STS3215-C001(기어비 1:345)로 통한다
- [이슈 #48](https://github.com/apirrone/Open_Duck_Mini/issues/48)(2026-11-02 현재 열림): C001로 만든 사람이 "걸음에 비해 모터가 느리다"고 보고했다. 다른 사람은 유튜브 제작자 다수가 1:345를 쓴다며 부품표가 맞을 거라 봤다. **관리자 답변은 아직 없다.** 2026-11-01에도 제작자가 "실제 쓴 모터가 무엇이냐"고 다시 물었다
- [이슈 #52](https://github.com/apirrone/Open_Duck_Mini/issues/52): 무릎을 12V 30kg 모터로 바꿨더니 넘어지지 않았다는 제작자 보고도 있다. 이건 원본 구성이 아니다

### 문제 2: 배터리 전압

- 18650 2개 직렬은 완충 시 8.4V이고, 서보 보드는 전압을 낮추지 않는다
- 판매처 표기는 4~7.4V와 5~8.4V로 **서로 다르다**. 제조사 자료로 아직 확인하지 못했다
- [이슈 #51](https://github.com/apirrone/Open_Duck_Mini/issues/51)에 같은 질문이 있었으나 공개 답변 없이 닫혔다
- 사람들이 그대로 만든다는 것과, 규격 안이라는 것은 별개다. 또 스프링 홀더로 큰 전류를 감당할 수 있는지도 같은 이슈의 질문이다

### 문제 3: 걷기가 잘 안 된다는 보고

- [이슈 #52](https://github.com/apirrone/Open_Duck_Mini/issues/52), [#56](https://github.com/apirrone/Open_Duck_Mini/issues/56): 앞으로 잘 안 간다, 발이 덜 들린다
- 원인이 한 가지로 정리돼 있지 않다. 공개 영상의 보행과 내 로봇의 보행이 같다는 보장이 없다

### 부품표에 없거나 빠뜨리기 쉬운 것

| 항목 | 근거 |
|---|---|
| Xbox One 패드 | Runtime 걷기 조작에 사용. 부품표에는 없음 |
| 납땜 도구, 전선 | assembly_guide |
| 나사 고정제 Loctite 243 | assembly_guide. 금속 대 금속 나사에 사용, 플라스틱 나사에는 금지 |
| M3 나사 | 수량이 문서에 `TODO`로 남아 있음 |
| 베어링 치수 | 부품표는 수량 3개와 링크만 있음. CAD에서 확인 |
| 3D 프린터 또는 출력 대행 | PLA 15%, 발바닥만 TPU 40% |
| 모터 설정용 전원 | 설명서는 배터리 팩 등을 예로 듦 |
| 시험용 받침대 | `print/mods/v2_Jaimes_Mods`에 제작자 작품(Stand) 있음 |

### 부품표 금액

| 묶음 | 금액 (2024~25년 유럽 가격 추정) |
|---|---|
| 기본 | 398.10유로 |
| 눈 · 소리 · 카메라 추가 | 432.34유로 |
| 그중 모터 14개 | 196유로 (단가 14유로) |

한국 도착 가격이 아니다. 이미 나온 대안으로 [Tnkr 키트](https://tnkr.ai/explore/docs/open-duck-mini/open-duck-mini-v2/kit)가 **600달러 + 배송비, 관세 별도**이며 "걷는 데 필요한 것 포함"이라고 안내한다. 3D 출력물 포함 여부, 모터 모델, 한국 배송은 페이지에서 확인되지 않았다.

## 6. GPT 내용 대조

| GPT 내용 | 원본 확인 | 판정 |
|---|---|---|
| 42cm, 14관절, Pi Zero 2W, BNO055 | README · 부품표 일치 | 맞음 |
| 모터 ID 표 | `configure_motors.md` 일치 | 맞음 |
| €398.10, €331.10, 모터 €196 | 부품표 일치 | 맞음 |
| PLA 15%, TPU 40% | `print_guide` 일치 | 맞음 |
| 원본은 "STS3215-C001 1:345" | 부품표에 품번 없음, 이슈 #48 미해결 | **확정 아님** |
| "7.4V형 허용 전압 4~7.4V" | 판매처마다 다름. GPT가 링크한 Waveshare 설명서는 12V 모델 문서로 보임 | **미확정** |
| "14개 정지 시 최대 35A" | 2.7A는 12V 모델 값으로 보임. C001 값 아님 | **근거 부족** |
| "사전 이미지 0.2.3 사용" | README에 적혀 있으나, 릴리스에 첨부 파일이 안 보임 | **위치 확인 필요** |
| "외부 PC 없이 걸을 수 있다" | 공개 정책은 파이에서 실행, 대신 Xbox 패드 필요 | 맞지만 패드 누락 |
| "로컬 GPU, 기존 개발 PC 재사용" | 내 PC에서 엔비디아 그래픽카드를 찾지 못함 | **이 PC에는 안 맞음** |
| "원본대로 하면 시행착오가 적다" | 국산화보다는 맞지만, 보행 문제 이슈 존재 | 절반만 맞음 |
| 로보티즈 XL430 국산화 95만 원 | 설계, 전압(11.1V), 정책까지 전부 다시 해야 함 | 지금은 보류 동의 |
| 예산 85만~130만 원, 모터 40,669원 | 계산은 맞음. 가격 페이지는 직접 확인 못함 | 추정치로만 사용 |
| 국내 부품 판매처 · 재고 | 이번에 확인 안 함 | 미확인 |

### 시제품 지원

| 기관 | 이번 확인 결과 |
|---|---|
| [가천메이커스](https://www.gmct.co.kr/) | 홈페이지에 "2026 Maker Start-Up 3D프린터 출력지원 참여자 모집(~11/30까지 상시모집)"과 "시제품 제작 및 무금형 소량생산 지원사업 상시모집" 공지 있음. **세부 조건은 직접 읽지 못함** |
| [서울용산시제품제작소](https://makeinyongsan.kr/) | 사이트 열림만 확인. 조건 미확인 |
| 서울창업허브, 인천 시제품 제작터 | 이번에 확인 안 함 |

GPT 말대로 조건이 "1회 2개, 최대 2회"라면 출력물 51개 중 일부만 가능하다. 취미 · 개인 소장 제한이 있다면 원본 복제는 대상이 아닐 수 있으니, 목적을 사실대로 설명하고 전화로 먼저 묻는 것이 빠르다. **마감이 11월 30일로 표시돼 있어 가장 급하다.**

## 7. 파악 순서 (돈 안 쓰는 단계)

- [ ] README와 영상을 보고 공개 보행 수준이 기대에 맞는지 판단
- [ ] Onshape CAD에서 다리 관절 5개 찾기
- [ ] 부품표를 내 환경 기준으로 다시 정리
- [ ] 디스코드 가입 후 세 가지 묻기: 지금 쓰는 모터 품번, 전원 구성, 성공 사례 설정
- [ ] 이슈 #48, #51, #52, #56 전체 읽기
- [ ] 가천 공고 조건 확인 (11/30 전)
- [ ] Tnkr 키트 문의: 한국 배송, 출력물 포함 여부, 모터 품번
- [ ] 그다음 직접 구매 · 키트 · Microduck 중 결정

Microduck 자료와 비교는 [../microduck/README.md](../microduck/README.md)를 참고한다.
