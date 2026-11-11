> **변경 고지 (Apache-2.0 §4):** 이 파일은 [apirrone/Open_Duck_Mini](https://github.com/apirrone/Open_Duck_Mini) (Apache License 2.0) 원문을 한국어로 번역하고 "번역자 참고"를 덧붙인 수정본이다. 라이선스 전문: [LICENSE-Apache-2.0.txt](../../LICENSE-Apache-2.0.txt). 원저작자와 관계없는 비공식 번역이다.
>
> **번역본입니다.** 원문: [Open_Duck_Mini/docs/assembly_guide.md](https://github.com/apirrone/Open_Duck_Mini/blob/v2/docs/assembly_guide.md) · 번역일 2026-11-02 · 명령과 코드는 원문 그대로이며 주석만 옮겼습니다.
>
> **번역자 참고**
> - 원문도 미완성입니다. 나사 총수와 서보 보드 사진이 빠져 있습니다.
> - 사진은 원문 GitHub에서 불러오므로 인터넷이 필요합니다.
> - 원문의 `head_roll dof`는 머리 좌우 기울임 관절을 뜻합니다.

# 조립 안내

> 오리를 조립하기 전에 먼저 [모터를 설정](./configure_motors.md)해야 합니다.

## 준비물

필요한 것:
- 납땜 인두와 기본적인 전자 공구 및 기술
- M3 나사 X개 (TODO: 정확한 개수 추가 예정)
- 전선 약간
- Loctite 나사 고정제 파란색 243

> 공통 참고: 모터에 금속 대 금속으로 나사를 조일 때마다 나사 고정제를 조금 바르세요. 로봇이 움직일 때 생기는 진동으로 나사가 풀리는 것을 막아 줍니다. 조립 시간이 조금 늘지만 그럴 만한 가치가 있습니다 ;)
>
> 플라스틱 나사에는 Loctite를 쓰지 마세요.

> 언제든 여기에서 CAD를 참고할 수 있습니다: https://cad.onshape.com/documents/64074dfcfa379b37d8a47762/w/3650ab4221e215a4f65eb7fe/e/0505c262d882183a25049d05

## 조립 순서

### 몸통 조립

`trunk_bottom`에 사진처럼 베어링을 넣고, 이 구멍들에 M3 인서트를 넣습니다. 나중에 몸체 부품을 달 수 있도록 이 부품 아래쪽에 M3 인서트 4개를 넣어 두기에도 좋은 때입니다.

<img src="https://github.com/user-attachments/assets/9ed8591a-7c96-4410-8d7e-9b7d88c6bd1f" alt="1" width="500px" >

그런 다음 `trunk_bottom`과 `trunk_top`을 맞추고, 이 구멍들로 `M3x10` 나사 2개를 조여 고정합니다.

<img src="https://github.com/user-attachments/assets/ae36b396-a34a-4691-8e62-fd916cd1f76c" alt="1" width="500px" >

가운데 모터를 사진처럼 달고, 모터와 함께 온 플라스틱 나사로 고정합니다.

<img src="https://github.com/user-attachments/assets/d4a6ba6c-852f-440e-afb7-3b9ca66ad3dc" alt="1" width="500px" >

`roll_motor_bottom`을 이렇게 끼웁니다.

<img src="https://github.com/user-attachments/assets/31c031b7-58b8-4c4c-a3fa-1f14a310c2f2" alt="1" width="500px" >


### 발 조립

두 발은 똑같습니다.

먼저 `foot_bottom_tpu`와 `foot_bottom_pla`를 맞춥니다. 이 구멍들에 M3 인서트를 넣습니다.

<img src="https://github.com/user-attachments/assets/6749a5ba-cea9-4b0a-ac32-f32e130fd057" alt="1" width="500px" >

그리고 두 부품을 `M3x6` 나사 2개로 고정합니다.

그다음 `foot_top`의 이 구멍들에 M3 인서트를 넣습니다.

<img src="https://github.com/user-attachments/assets/1a77f2f8-56ea-43d2-91c7-78130456c45b" alt="1" width="500px" >

모두 이렇게 조립합니다. 모터의 구동축 쪽이 `foot_top` 부품 쪽을 향하는지 확인하세요.

<table>
  <tr>
    <td> <img src="https://github.com/user-attachments/assets/197cd9b7-05a6-46ed-9af5-d2def37970c8" alt="1" width="500px" > </td>
    <td> <img src="https://github.com/user-attachments/assets/f532fbcc-100b-4715-96a8-66697e4fda26" alt="1" width="500px" > </td>
   </tr> 
</table>

발 스위치도 이렇게 달 수 있습니다.

> 발이 땅에 닿을 때 스위치가 눌리도록 꽉 끼워 넣습니다.

<table>
  <tr>
    <td> <img src="https://github.com/user-attachments/assets/71b9ad91-864e-423f-97a3-911d65182ab9" alt="1" width="500px" > </td>
    <td> <img src="https://github.com/user-attachments/assets/59e15c7c-3515-4408-bfe9-80e30c4cd007" alt="1" width="500px" > </td>
   </tr> 
</table>


### 정강이 조립

`leg_spacer`의 이 구멍들에 M3 인서트를 넣습니다(양쪽 모두. 총 4개).

<img src="https://github.com/user-attachments/assets/41eb01a2-d6f9-43b1-a83e-d14785907425" alt="1" width="500px" >

먼저 발 모터에 모터 선을 꽂고, 사진처럼 `right_sheet`를 지나가게 합니다.

<img src="https://github.com/user-attachments/assets/a1432505-6b6c-4765-aea6-0a7a0b8b220b" alt="1" width="500px" >

그런 다음 아래처럼 조립합니다.

<img src="https://github.com/user-attachments/assets/54615217-fec9-46dd-8b40-e21a4f527b55" alt="1" width="500px" >

### 허벅지 조립

허벅지는 거의 같습니다. 다만 `hip_pitch` 모터는 이 방향으로 답니다(영점 때문에 중요합니다).

<img src="https://github.com/user-attachments/assets/951157a4-26dc-41bb-b97f-18dbbb0c1cd3" alt="1" width="500px" >

### 엉덩이 조립

`left_roll_to_pitch` 또는 `right_roll_to_pitch`를 답니다. 이 부품은 좌우 대칭이라 맞는 쪽을 써야 합니다.

<img src="https://github.com/user-attachments/assets/368a85ad-1c58-4db2-bdd0-812334b6c784" alt="1" width="500px" >

`roll_motor_top`을 `hip_yaw` 서보에 답니다(아래쪽에서 나사 조임). 서보를 아직 몸통에 달지는 마세요.


<img src="https://github.com/user-attachments/assets/768f1664-030c-4432-9071-c52d79d3ef6b" alt="1" width="500px" >

그런 다음 `hip_roll`을 이렇게 답니다.

<img src="https://github.com/user-attachments/assets/e1287f67-320c-42cf-a5dc-84f4c64abb17" alt="1" width="500px" >

그리고 이 묶음을 이렇게 끼웁니다.

<img src="https://github.com/user-attachments/assets/dba458ae-e9cb-4e16-a82e-cbe74d2350fa" alt="1" width="500px" >

조일 수 있는 것은 모두 조입니다(서보와 함께 온 플라스틱 나사 사용).

이제 다리를 이렇게 달 수 있습니다.

<img src="https://github.com/user-attachments/assets/a7169791-9f0a-4827-aea7-00ab1d0f6212" alt="1" width="500px" >

다른 쪽 다리도 똑같이 합니다 :)

오리가 이제 이렇게 보여야 합니다.

<img src="https://github.com/user-attachments/assets/4921b29b-b38b-423d-9da1-1c8f84689e84" alt="1" width="500px" >

### 목 조립

이제 방법은 아시죠.

<img src="https://github.com/user-attachments/assets/f96fe44f-a7bc-423e-a925-4aef3e7bf568" alt="1" width="500px" >

### 머리 기구 조립

먼저 `head_pitch_to_yaw`를 이렇게 답니다.

<img src="https://github.com/user-attachments/assets/a4dba395-eb0d-4150-8980-8e14f1173d02" alt="1" width="500px" >

그런 다음 `head_yaw_to_roll`과 `head_roll_mount`를 따로 `head_roll` 관절에 답니다.

<img src="https://github.com/user-attachments/assets/4648cd6c-391e-41e4-9617-4ecebfa9215b" alt="1" width="500px" >

(나중에 머리를 다시 분해하지 않도록 `head_bot_plate`와 `body_middle_top`도 지금 끼워 둘 수 있습니다.)

다음

<img src="https://github.com/user-attachments/assets/03f3bdae-06c3-4c37-b68f-14587edd6123" alt="1" width="500px" >

다음

<img src="https://github.com/user-attachments/assets/04a219d8-bbaf-4910-b5d3-7ca67bce466c" alt="1" width="500px" >

오리가 이제 이렇게 보여야 합니다.

<img src="https://github.com/user-attachments/assets/71545d40-f0f5-411d-a8d5-1cd676a74e75" alt="1" width="500px" >

### 서보 구동 보드 달기

TODO 사진 찍기

### IMU 달기

이렇게 답니다.

> 사실 IMU는 자연스러운 바른 방향으로 다는 것이 더 좋습니다. 아래 사진과 비교하면 X축을 기준으로 뒤집은 방향입니다.
> 아래 사진의 IMU는 위아래가 뒤집혀 있습니다.
> 뒤집어 달아도 큰 문제는 아닐 겁니다. 어떻게 달았는지는 나중에 설정할 수 있습니다.

<table>
  <tr>
    <td> <img src="https://github.com/user-attachments/assets/8772996d-5906-48fa-8cbe-6b6823982375" alt="1" width="500px" ></td>
    <td> <img src="https://github.com/user-attachments/assets/172f44c7-c7c6-47f8-894a-14024abd24f4" alt="2" width="500px" ></td>
   </tr> 
</table>

## 전자 회로

참고용 전체 회로도입니다.

<table>
  <tr>
    <td> <img src="https://raw.githubusercontent.com/apirrone/Open_Duck_Mini/v2/docs/open_duck_mini_v2_wiring_diagram.png" alt="1" width="500px" ></td>
    <td> <img src="https://raw.githubusercontent.com/apirrone/Open_Duck_Mini/v2/docs/wiring.png" alt="2" width="500px" ></td>
   </tr> 
</table>

발 배선은 이렇게 합니다.

![feet](https://github.com/user-attachments/assets/d3376494-4690-4484-8352-132e6284731a)

Pi Zero 헤더 핀 배치입니다.

|       **LED**      | **Pi Zero 헤더 핀** | **Pi Zero 기능** |
|:-------------------:|:----------------------:|:--------------------:|
|  왼쪽 눈 양극(+) |           16           |        GPIO 23       |
| 오른쪽 눈 양극(+) |           18           |        GPIO 24       |
| 프로젝터 양극(+) |           22           |        GPIO 25       |
|  공통 음극(-) |            6           |          GND         |
|                     |                        |                      |
|     **안테나**    |                        |                      |
|       왼쪽 PWM      |           32           |     GPIO 12 (PWM)    |
|      오른쪽 PWM      |           33           |     GPIO 13 (PWM)    |
|          5V         |            2           |          5V          |
|         GND         |            6           |          GND         |
|                     |                        |                      |
|  **발 스위치**  |                        |                      |
|      왼발      |           15           |        GPIO 22       |
|      오른발     |           13           |        GPIO 27       |
|         GND         |            9           |          GND         |
|                     |                        |                      |
|      **BNO055**     |                        |                      |
|         VIN         |            1           |          3V3         |
|         3VO         |           NC           |           -          |
|         GND         |            9           |          GND         |
|         SDA         |            3           |        GPIO 2        |
|         SCL         |            5           |        GPIO 3        |
|         RST         |           NC           |           -          |
|                     |                        |                      |
|    **MAX98357A**    |                        |                      |
|         LRC         |           35           |        GPIO 19       |
|         BCLK        |           12           |        GPIO 18       |
|         DIN         |           40           |        GPIO 21       |
|         GAIN        |           NC           |           -          |
|          SD         |           NC           |           -          |
|         GND         |            6           |          GND         |
|         VIN         |            2           |          5V          |

### 배터리 팩

> 안전을 위해 홀더에 넣기 전에 두 셀의 충전 전압이 같은지 확인하세요.

<table>
  <tr>
    <td> <img src="https://github.com/user-attachments/assets/371db809-7cb3-47e1-b277-7fc2bdf21025" alt="1" width="500px" ></td>
    <td> <img src="https://github.com/user-attachments/assets/3acfa4e6-ecf7-41f6-a965-35a3040f52fb" alt="2" width="500px" ></td>
   </tr> 
</table>


### 머리

먼저 이 모든 구멍에 M3 인서트를 넣습니다.

![image](https://github.com/user-attachments/assets/ef4cd513-6b8d-41fa-9cc3-149fc8333d3e)

> TODO 표현 기능(카메라, 안테나, 눈 LED, 프로젝터, 스피커) 안내 추가 예정

그런 다음 베어링을 넣고, 귀 모터와 Raspberry Pi Zero 2W를 답니다.

참고로 머리 안쪽은 이제 이렇게 보입니다.

![image](https://github.com/user-attachments/assets/91284081-a563-4c02-bb3d-194b3afbc25c)


그런 다음 목과 머리를 이렇게 조립합니다.

![image](https://github.com/user-attachments/assets/96fd5347-bff7-47e9-a7bd-fde17ab1bcd2)


## 몸체

먼저 `body_middle_bottom`을 나사로 고정합니다.

![Capture d’écran du 2025-02-09 12-10-00](https://github.com/user-attachments/assets/081d8840-8e88-4938-9d9a-4d97614e6261)

그런 다음 `body_middle_bottom`과 `body_middle_top`에서 배터리 팩과 `body_front`를 달 구멍 전부에 M3 인서트를 넣습니다.

그리고 `body_middle_top`, `body_front`, 배터리 팩을 답니다.

<table>
  <tr>
    <td> <img src="https://github.com/user-attachments/assets/679a9cd2-89ca-41e8-9d03-f93fc040068b" alt="1" width="500px" ></td>
    <td> <img src="https://github.com/user-attachments/assets/ca292c48-1c72-4649-b7d1-4d3c9eac62ac" alt="2" width="500px" ></td>
   </tr> 
</table>

완성입니다 :)

<table>
  <tr>
    <td> <img src="https://github.com/user-attachments/assets/312ec747-8eb4-4145-92eb-c1c3ddba80da" alt="1" width="500px" ></td>
    <td> <img src="https://github.com/user-attachments/assets/864d9dd7-3daf-4e63-913c-2e99157e4aaf" alt="2" width="500px" ></td>
   </tr> 
</table>


> 이제 오리 조립이 끝났으니, [여기](https://github.com/apirrone/Open_Duck_Mini_Runtime)에서 Raspberry Pi와 실행 소프트웨어를 설정하세요.
