> **변경 고지 (Apache-2.0 §4):** 이 파일은 [apirrone/Open_Duck_Mini](https://github.com/apirrone/Open_Duck_Mini) (Apache License 2.0) 원문을 한국어로 번역하고 "번역자 참고"를 덧붙인 수정본이다. 라이선스 전문: [LICENSE-Apache-2.0.txt](../../../../LICENSE-Apache-2.0.txt). 원저작자와 관계없는 비공식 번역이다.
>
> **번역본입니다.** 원문: [Open_Duck_Mini/print/mods/v2_Jaimes_Mods/README.md](https://github.com/apirrone/Open_Duck_Mini/blob/v2/print/mods/v2_Jaimes_Mods/README.md) · 번역일 2026-11-02 · 명령과 코드는 원문 그대로이며 주석만 옮겼습니다.

# Jaime의 V2 BD-X 스타일 몸체 덮개 개조

![3D Model Overview](https://raw.githubusercontent.com/apirrone/Open_Duck_Mini/v2/print/mods/v2_Jaimes_Mods/3D%20Model.png)

만든 사람: Jaime Machuca (jmachuca77)

## 목적
Open Duck Mini v2의 몸체와 머리 덮개를 대체하는 부품 묶음입니다. 디즈니 BD-X의 외형과 표면 선에 더 가깝게 만드는 것이 목적입니다.

## 균형과 강화학습 정책 관련 중요 참고
이 덮개는 전체 무게 분포를 바꾸며, 현재 걷기 정책을 학습할 때 가정한 원래 무게 중심과 완전히 일치하지 않습니다. 기존 정책은 여전히 동작하지만 안정성이 약간 떨어질 수 있습니다(예: 더 흔들림, 가끔 넘어지지 않으려 발을 더 디딤). 동작을 지켜보고, 불안정이 문제가 되면 바뀐 관성 값으로 미세 조정하거나 다시 학습하는 것을 고려하세요.

## 출력 안내
`Bambu_X1C_Project_File.3mf` 파일에는 Bambu Labs X1C 프린터에 맞춘 추천 부품 방향, 판 묶음, 기본 설정이 들어 있습니다.

권장 시작값(재료에 맞게 조정):
- 재료: 내구성을 위해 PETG 또는 PLA-CF
- 층 높이: 0.20mm(세부), 0.28mm(넓고 평평한 덮개는 선택)
- 채움: 15~20% 자이로이드 또는 큐빅
- 벽: 구조용 덮개는 강도를 위해 3겹
- 서포트: 꼭 필요한 곳에만(프로젝트 파일 표시 확인)
- 바닥 접착: 키가 크고 좁은 부품은 스커트나 작은 브림

![Print Orientation Reference](https://raw.githubusercontent.com/apirrone/Open_Duck_Mini/v2/print/mods/v2_Jaimes_Mods/Print%20Orientation%20Reference.png)

## 포함 부품
대표 파일 이름(전체 목록은 폴더 참고):
- 앞, 뒤, 옆 몸체 덮개 (`Front Cover.3mf`, `Back Panel.3mf`, `trunk_*`)
- 다리 덮개와 선 정리 부품 (`Left Leg Cover.3mf`, `Right Leg Cover.3mf`, 위아래 가이드)
- 장식과 패널 (`Front Greeble.3mf`, `Front Panel V4.3mf`)
- 안전, 부속 판 (`E-Stop Panel.3mf`, `E-Stop Adapter Plate.3mf`)
- 조립 보조 (`Hip Cable Holder.3mf`, `Top Body Support.3mf`)

## 사용과 출처 표시
자유롭게 변형하거나 고쳐 쓰세요. Jaime Machuca(jmachuca77)의 이름을 남기고, 디즈니 BD-X 외형에 가깝게 만드는 개조임을 밝혀 주세요.

## 앞으로의 제안
- 바뀐 CAD 질량 값으로 강화학습 정책을 다시 학습하거나 미세 조정하기.

---
문제를 찾았거나 더 좋은 출력 방향을 알게 되면 PR을 열거나 여기에 메모를 남겨 주세요.
