# 다음 단계: Open Duck Mini v2 · Microduck

> ⚠️ **조사 메모 (미검증)** — 공개 자료를 팀 내부 공부용으로 요약한 것이다. 가격 · 배송 · 사양 · 이슈 상태는 조사 시점 기준이며 틀리거나 바뀌었을 수 있다. `TODO: 확인 필요`는 확인하지 못한 내용이다. 구매 · 제작 결정 전에 반드시 원본을 확인할 것. 각 프로젝트의 공식 입장이 아니다.

> 1단계(오또봇)를 끝낸 뒤 진행한다. 아래 조사 원본은 **2026-11-02 기준**이다. 이슈 · 가격 · 배송 일정은 바뀌므로 2단계 시작 전에 다시 확인한다.
>
> | 원본 문서 | 내용 |
> |---|---|
> | [open-duck-mini/README.md](open-duck-mini/README.md) | ODM v2 저장소 지도, 구조, 모터 ID, 핀, 미해결 문제, 부품표 금액, GPT 자료 대조 |
> | [microduck/README.md](microduck/README.md) | Microduck 결론 · 결정할 것 · 자료끼리 어긋나는 점 |
> | [microduck/01-지식-맵.md](microduck/01-지식-맵.md) | 필요한 지식, 공부 순서 |
> | [microduck/02-레퍼런스.md](microduck/02-레퍼런스.md) | 공식 자료 · 코드 · 논문 링크 |
> | [microduck/03-부품-구매-계획.md](microduck/03-부품-구매-계획.md) | 구매 계획, 금액 |
> | [microduck/04-개발-계획.md](microduck/04-개발-계획.md) | Microduck 단계별 개발 계획, 누코드 UDP 연동 |

## 1. 두 로봇 비교

| | Open Duck Mini v2 (2단계) | Microduck (3단계) |
|---|---|---|
| 누가 | apirrone (Hugging Face · Pollen 후원) 공개 프로젝트 | Pollen Robotics 완제품 |
| 크기 · 관절 | 약 42cm · 14관절 (STS3215 버스 서보) | 약 25cm, 800g 미만 · 15모터 (XL330 계열) |
| 두뇌 | Raspberry Pi Zero 2W, 정책 50Hz | RK3566 계열 보드 (판매품 실제 보드 `TODO: 확인 필요`) |
| 설계 공개 | CAD · 부품표 · 출력 파일 공개 (Apache-2.0, 일부 저장소 라이선스 표시 없음) | **기계 · 전자 설계 비공개.** 시뮬레이션용 STL만 공개 |
| 직접 만들기 | 가능. 부품 약 400유로 + 출력 51개 | **복제 비추천** (부품만 약 48만 원 + 설계 없음) → 완제품 구매 |
| 손에 넣는 시기 | 부품 오는 대로 (4~6주) | 지금 주문하면 2027년 4월 이후 배송 (11/2 스토어 기준) |
| 학습 | MuJoCo Playground, 엔비디아 GPU 필요 | mjlab, 클라우드 학습(`--hf-jobs`) 가능 |
| 위험 | 모터 품번 · 배터리 전압 · 보행 품질 이슈가 미해결 | 배송 지연, 클라우드 비용 |

**정리:** "만드는 것"은 Open Duck Mini v2로 하고, Microduck은 **사 두고 정책을 개발**하는 대상이다. 둘은 학습 코드와 부품이 다르므로 섞지 않는다.

## 2. 오또봇에서 가져가는 것

| 1단계 결과 | 2단계(ODM)에서 쓰는 곳 | 3단계(Microduck)에서 쓰는 곳 |
|---|---|---|
| 넘어짐 · 들림 감지 모델 | ODM 런타임에 안전 정지로 추가 (원본에 없음) | 정책 실행 중 감시 |
| 웹 블루투스 조종 · 텔레메트리 | Xbox 패드 대신/옆에 웹 조종 (파이 BLE 서버 필요) | 공식 `duckctl`(블루투스)과 비교 |
| 에어밴드 스틱 중계 | 스틱 → 파이 패드 입력 | 스틱 → PC 중계 → UDP 4210 → `netpadd` ([04-개발-계획](microduck/04-개발-계획.md) 5단계) |
| 트림 · 축 맞추기 · 기록 | `find_soft_offsets.py`, `calibrate_imu.py`, `record_data.py` 이해 | `testbench_sim2real.py` |
| TinyML 파이프라인 | 정책 ONNX 추론 구조 이해 | 정책 manifest(입력 61 · 출력 14) |

## 3. 2단계 — Open Duck Mini v2 제작

### 3-1. 레퍼런스 다시 확인 (시작 주에)

| 확인할 것 | 어디서 | 11/2 상태 |
|---|---|---|
| 정답 모터 품번 (STS3215-C001 1:345 맞나) | 이슈 [#48](https://github.com/apirrone/Open_Duck_Mini/issues/48), 디스코드 | 관리자 답 없음 |
| 18650 2직렬(8.4V) 전압이 서보 규격 안인가 | 이슈 [#51](https://github.com/apirrone/Open_Duck_Mini/issues/51), Feetech 자료 | 판매처 표기 서로 다름 |
| 잘 안 걷는다는 보고의 원인 | 이슈 [#52](https://github.com/apirrone/Open_Duck_Mini/issues/52), [#56](https://github.com/apirrone/Open_Duck_Mini/issues/56) | 원인 미정리 |
| 미리 만든 파이 이미지 위치 | Runtime 릴리스 | 첨부 파일 안 보임 |
| Tnkr 키트 (600달러 + 배송) 한국 배송 · 출력물 · 모터 | [Tnkr](https://tnkr.ai/explore/docs/open-duck-mini/open-duck-mini-v2/kit) | 미확인 |
| 국내 3D 출력 지원 사업 | 가천메이커스 등 | 11/30 마감 공지 있었음 — 지났으면 다음 공고 |

### 3-2. 공부 순서 (1단계와 병행 시작 → 2단계 2주차까지)

| # | 주제 | 자료 | 확인 방법 |
|---|---|---|---|
| 1 | 로봇 전체 구조 | ODM README · 영상, [open-duck-mini](open-duck-mini/README.md) 4장 | 하드웨어 · 소프트웨어 흐름도를 안 보고 그린다 |
| 2 | 버스 서보 (STS3215) | `configure_motors.md`, Feetech 프로토콜 | ID · 영점 설정 순서를 설명한다 |
| 3 | 강화학습 기초 | [01-지식-맵](microduck/01-지식-맵.md), PPO 개념 | 관측 · 행동 · 보상을 ODM 기준으로 적는다 |
| 4 | 모방 보상 · 기준 걸음 | [BDX 논문](https://arxiv.org/abs/2501.05204), `reference_motion_generator` | 오또 오실레이터 파라미터와 비교한 메모 |
| 5 | MuJoCo · sim2real | [MuJoCo 문서](https://mujoco.readthedocs.io/), ODM `sim2real.md`, [Rhoban/bam](https://github.com/Rhoban/bam) | 도메인 랜덤화가 왜 필요한지 설명 |
| 6 | 정책 실행 코드 | Runtime `v2_rl_walk_mujoco.py` | 50Hz 루프를 줄 단위로 설명 |

### 3-3. 개발 계획 (6~8주, 1단계 데모 뒤)

| 주 | 할 일 | 완료 기준 |
|---|---|---|
| 0 | 결정 회의: 직접 구매 / Tnkr 키트 / 보류. 3-1 확인 결과로 판단 | 예산 · 구매 목록 확정 |
| 1~2 | 부품 주문 (해외 배송 대기), 출력 시작. 대기 중 Playground를 클라우드 GPU로 돌려 보기 | 출력물 절반, 시뮬레이션 걷기 영상 |
| 2~3 | **조립 전에** 모터 14개 ID · 영점 설정, 전원 측정 | `check_motors.py` 통과, 전압 기록 |
| 3~4 | 조립 · 배선 · IMU 보정 · 발 스위치 | 받침대 위에서 관절 전부 움직임 |
| 4~5 | 공개 정책(`BEST_WALK_ONNX`)으로 걷기, 낮은 속도부터 | 푹신한 바닥 1m |
| 5~6 | 오또에서 만든 넘어짐 감지 · 웹 조종 붙이기 | 넘어지면 정지, 웹으로 조종 |
| 6~8 | (선택) 걸음 재학습, 머리 · 소리 · 눈 | 내가 학습한 정책으로 걷기 |

## 4. 3단계 — Microduck

자세한 계획은 [microduck/04-개발-계획.md](microduck/04-개발-계획.md). 요약:

| 단계 | 내용 | 시기 |
|---|---|---|
| 0 | 웹 시뮬레이터 체험, 한국 배송비 · 관세 · 환불 조건 확인 후 **사전 주문 결정** | 1단계 중 (빠를수록 배송이 앞선다) |
| 1~2 | `microduck_rl` 공식 학습 재현 → 내 동작("음악 오리") | 2단계와 병행 가능, 클라우드 비용 |
| 3 | `duck-sim`으로 로봇 프로그램을 시뮬레이션에서 실행 | WSL 동작 `TODO: 확인 필요` |
| 5 | 누코드 스틱 → PC 중계 → UDP 패드 → 오리 | 오또 스틱 중계를 파이썬으로 옮김 |
| 6 | 대회 "Got a Talent" 참가 (선택) | |
| 7 | 실물 도착 후 sim2real | 2027-04 이후 |

## 5. 지금 결정할 것

| # | 질문 | 추천 | 언제까지 |
|---|---|---|---|
| 1 | Microduck 사전 주문? | 배송 · 환불 조건 확인 후 예 | 이번 달 (늦을수록 배송도 늦어짐) |
| 2 | ODM v2 직접 제작 vs Tnkr 키트 | 1단계 4주차 회의에서 3-1 결과로 결정 | 12/8 |
| 3 | 학습용 GPU | 클라우드(HF Jobs 등). 리더 PC에는 엔비디아 GPU 없음 | 2단계 시작 전 |
