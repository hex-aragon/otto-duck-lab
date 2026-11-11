# otto-duck-lab

**오또봇으로 먼저 배우고, 오리 로봇으로 간다.**
누코드 NU40 DK(nRF52840)로 움직이는 오또봇(Otto DIY)에 Open Duck Mini v2가 하는 일 중 작은 로봇에서 가능한 것을 옮기고, TinyML까지 붙이는 팀 프로젝트다.
여기서 익힌 것을 그대로 들고 다음 단계인 Open Duck Mini v2 제작과 Microduck으로 넘어간다.

> 상태 (2026-11-11): 펌웨어 · 웹 컨트롤러 1차 버전 작성, **NU40 DK용 컴파일 확인. 실물 시험은 아직 안 함.**
> 확인 안 된 하드웨어 사실은 코드와 문서에 `TODO: 확인 필요`로 표시했다. 추측으로 채우지 않는다.

## 왜 오또봇부터 하나

| | Open Duck Mini v2 | 오또봇 + 누코드 (이번 단계) |
|---|---|---|
| 관절 | 14개 (STS3215 버스 서보) | 4개 (SG90 같은 PWM 서보) |
| 부품비 | 약 400유로 + 3D 출력 51개 | 수만 원대 (가격 조사 필요) |
| 걷는 방법 | 강화학습 정책 50Hz | 사인파 조합 (오실레이터) |
| 두뇌 | Raspberry Pi Zero 2W | NU40 DK (nRF52840, BLE 5) |
| 실패 비용 | 큼 (모터 · 전원 문제 미해결 이슈 있음) | 작음 |

오리 로봇의 어려운 부분(모터 선정, 전압, 보행 정책)은 아직 저장소에서도 답이 정리되지 않았다. 반면 **IMU 자세 추정, 넘어짐 안전 처리, 무선 조종, 트림(영점), 데이터 기록, 온디바이스 추론**은 관절 4개짜리 로봇에서도 똑같이 연습할 수 있다. 이것들을 오또봇에서 먼저 끝내 둔다.

## 로드맵

| 단계 | 내용 | 기간 | 문서 |
|---|---|---|---|
| **1** | **오또봇 + 누코드 + TinyML** | 4주 | [docs/05-개발계획-1단계.md](docs/05-개발계획-1단계.md) |
| 2 | Open Duck Mini v2 제작 (레퍼런스 · 공부 · 개발) | 6~8주 | [docs/next/README.md](docs/next/README.md) |
| 3 | Microduck (완제품, 2027-04 이후 배송) 정책 개발 | 배송 전부터 시뮬레이션 | [docs/next/README.md](docs/next/README.md) |

## 문서

| 문서 | 내용 |
|---|---|
| [01-기능-매핑](docs/01-기능-매핑.md) | Open Duck Mini v2 기능 → 오또봇에서 어떻게 하나, 무엇은 못 하나 |
| [02-하드웨어](docs/02-하드웨어.md) | 부품, 배선, 전원, 확인할 것 |
| [03-소프트웨어-구조](docs/03-소프트웨어-구조.md) | 펌웨어 흐름, BLE 패킷, 웹 컨트롤러 |
| [04-TinyML](docs/04-TinyML.md) | 무엇을 분류하나, 데이터 수집 → 학습 → 보드 탑재 |
| [05-개발계획-1단계](docs/05-개발계획-1단계.md) | 4주 일정, 역할, 완료 기준 |
| [next/](docs/next/README.md) | 2 · 3단계: Open Duck Mini v2 · Microduck 조사 · 공부 · 개발 계획 |

## 폴더

```
otto-duck-lab/
├ firmware/otto_nu40/     NU40 DK 펌웨어 (아두이노 IDE + 누코드 BSP)
│  ├ otto_nu40.ino        BLE · 서보(HwPWM) · 안전 처리 · 텔레메트리
│  ├ gait.h               사인파 동작 (걷기 · 돌기 · 춤) — 순수 계산
│  ├ imu.h                LSM6DS / MPU-6050 자동 감지, 기울기 추정
│  └ posture.h            자세 분류: 규칙 또는 TinyML 모델(posture_model.h)
├ web/index.html          조종 · 트림 · 스틱 중계 · TinyML 학습기 (빌드 없음, 파일 하나)
└ docs/                   계획 · 조사 문서
```

## 빠른 시작

1. **펌웨어**: 아두이노 IDE → 보드 `NUBoards nRF52 → NU40DK nRF52840` → `firmware/otto_nu40/otto_nu40.ino` 업로드.
   시리얼 모니터(115200)에서 `w`(앞) `a`(왼쪽) `x`(정지) `h`(차렷)로 바로 시험할 수 있다.
   ```powershell
   $cli = "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
   & $cli compile --fqbn nucode:nrf52:nu40dk -u -p COM3 firmware\otto_nu40
   ```
2. **웹**: `web/index.html`을 localhost로 연다 (`npx serve web` 또는 `python -m http.server -d web`). Web Bluetooth는 HTTPS나 localhost에서만 된다. 크롬/엣지 사용, iOS는 안 된다.
3. **오또봇 연결** → 방향 패드 · W A S D로 조종. 처음엔 **트림**으로 차렷 자세부터 반듯하게 맞추고 저장한다.

> ⚠️ 서보 전원은 보드와 따로 공급하고 GND만 묶는다. 서보 4개가 동시에 움직이면 보드 USB 전원으로는 부족하다. [02-하드웨어](docs/02-하드웨어.md) 참고.

## 관련 저장소

- [hex-aragon/air-band](https://github.com/hex-aragon/air-band) — 같은 NU40 + IMU로 만든 에어 드럼. 스틱을 오또 조종기로 그대로 쓴다 (웹 중계, 스틱 펌웨어 수정 없음)
- [OttoDIY/OttoDIYLib](https://github.com/OttoDIY/OttoDIYLib) — 오또봇 원본 라이브러리 (GPL-3.0). 이 저장소의 동작 파라미터 초기값을 참고했다. 코드는 새로 작성
- [apirrone/Open_Duck_Mini](https://github.com/apirrone/Open_Duck_Mini) — 2단계 대상
- [pollen-robotics/microduck](https://github.com/pollen-robotics/microduck) — 3단계 대상

## 라이선스 · 출처

| 대상 | 라이선스 |
|---|---|
| 코드 (`firmware/`, `web/`) | [MIT](LICENSE) |
| 문서 · 데이터 (`docs/`, `data/`, `*.md`) | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) |

- 오또봇 동작 파라미터 초기값은 [OttoDIYLib](https://github.com/OttoDIY/OttoDIYLib)(GPL-3.0)의 공개 동작을 참고했다. **원본 코드는 복사하지 않았고**, 이 저장소의 코드는 새로 작성했다.
- 오또봇 3D 파일(CC BY-SA 4.0)이나 Otto 원본 코드(GPL-3.0)를 이 저장소에 넣을 때는 해당 파일에 원래 라이선스를 그대로 적용하고 출처를 남긴다. MIT로 바꿔 올리지 않는다.
- Open Duck Mini · Microduck 관련 문서는 각 프로젝트의 공개 자료를 요약하고 링크한 조사 메모다. 원본 문서 번역본은 원본 일부 저장소에 라이선스 표시가 없어 넣지 않았다.
- 상표(Otto DIY, Open Duck Mini, Microduck, Pollen Robotics, Hugging Face, NUCODE)는 각 소유자의 것이며, 이 저장소는 그들과 관계가 없다.
