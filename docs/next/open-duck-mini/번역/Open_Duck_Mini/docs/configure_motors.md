> **변경 고지 (Apache-2.0 §4):** 이 파일은 [apirrone/Open_Duck_Mini](https://github.com/apirrone/Open_Duck_Mini) (Apache License 2.0) 원문을 한국어로 번역하고 "번역자 참고"를 덧붙인 수정본이다. 라이선스 전문: [LICENSE-Apache-2.0.txt](../../LICENSE-Apache-2.0.txt). 원저작자와 관계없는 비공식 번역이다.
>
> **번역본입니다.** 원문: [Open_Duck_Mini/docs/configure_motors.md](https://github.com/apirrone/Open_Duck_Mini/blob/v2/docs/configure_motors.md) · 번역일 2026-11-02 · 명령과 코드는 원문 그대로이며 주석만 옮겼습니다.
>
> **번역자 참고**
> - `configure_motor.py`는 실행 코드 저장소의 `scripts/` 폴더에 있습니다.

# 모터 설정

> 오리를 만들기 *전에* 모터마다 하나씩 따로 해야 합니다.
>
> 설정하는 동안 모터가 영점으로 움직입니다. 그때 아래 사진처럼 혼을 최대한 맞춰 끼우면 됩니다. (완벽하지 않아도 괜찮습니다. 나중에 보정합니다)

![Capture d’écran du 2025-04-25 09-55-34](https://github.com/user-attachments/assets/e3c4aefa-5e0a-4d4e-89f4-82df9bf30e29)


실행 코드 저장소 `https://github.com/apirrone/Open_Duck_Mini_Runtime`의 `v2` 브랜치를 내려받아 설치(`pip install -e .`)합니다.

설정은 내 컴퓨터나 Raspberry Pi 중 원하는 곳에서 하면 됩니다. 서보에 전원을 줄 방법만 있으면 됩니다. 예를 들면 배터리 팩입니다.


그런 다음 모터마다 이 명령을 실행합니다.

```bash
python configure_motor.py --id <id>
```

모터 ID는 다음과 같습니다.

```python
{
    "left_hip_yaw": 20,
    "left_hip_roll": 21,
    "left_hip_pitch": 22,
    "left_knee": 23,
    "left_ankle": 24,
    "neck_pitch": 30,
    "head_pitch": 31,
    "head_yaw": 32,
    "head_roll": 33,
    "right_hip_yaw": 10,
    "right_hip_roll": 11,
    "right_hip_pitch": 12,
    "right_knee": 13,
    "right_ankle": 14,
}
```
