# otto_moves — 오또 움직임 기록 · 보상 프로그램 (Anchor 0.31)

지금 웹(otto-pet.html)은 솔라나 기본 Token-2022로 "Otto Moment" 토큰만 만든다.
이 프로그램은 그 위에 **체인이 직접 검사하는 규칙**을 더한다.

| 명령 | 누가 서명 | 하는 일 |
|---|---|---|
| `init` | 관리자 | 설정 PDA + MOVE 민트(소수점 0, 발행 권한 = 프로그램) |
| `register(name)` | **NU40** + 주인 | 오또 프로필 PDA `["otto", NU40 주소]` |
| `record_moment(sha256, 요약)` | **NU40** + 수수료 지갑 | 기록 PDA `["moment", NU40, 번호]` + MOVE 보상을 NU40 지갑으로 |

- **NU40 서명 검사:** `otto`가 Signer라서 솔라나 런타임이 ed25519 서명을 검사한다. 보드 없이 만든 기록은 거부.
- **보상 규칙** (`src/score.rs`, 최대 100 MOVE): 걸음 2개당 1점(최대 40) + 춤 3초당 1점(최대 40) + 센서 기록이 초당 2줄 이상이면 +20 − 넘어짐 1번에 10. 10초 미만은 0.
- **연타 막기:** 같은 오또는 30초에 한 번.

## 시험
```
cargo test -p otto_moves --lib     # 보상 규칙 시험 (4개 통과)
```

## devnet 배포 (솔라나 · Anchor 도구 필요)
```
anchor keys sync && anchor build && anchor deploy --provider.cluster devnet
```
배포 비용 약 2~3 devnet SOL.
