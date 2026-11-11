> **변경 고지 (Apache-2.0 §4):** 이 파일은 [apirrone/Open_Duck_Mini](https://github.com/apirrone/Open_Duck_Mini) (Apache License 2.0) 원문을 한국어로 번역하고 "번역자 참고"를 덧붙인 수정본이다. 라이선스 전문: [LICENSE-Apache-2.0.txt](../../LICENSE-Apache-2.0.txt). 원저작자와 관계없는 비공식 번역이다.
>
> **번역본입니다.** 원문: [Open_Duck_Mini/docs/prepare_robot.md](https://github.com/apirrone/Open_Duck_Mini/blob/v2/docs/prepare_robot.md) · 번역일 2026-11-02 · 명령과 코드는 원문 그대로이며 주석만 옮겼습니다.
>
> **번역자 참고**
> - **원문 저장소가 "오래된 문서"로 표시했습니다.** 예시의 관절 이름과 `robots/bd1/` 경로는 v2 로봇과 다릅니다. v2를 그대로 만드는 데는 필요 없습니다.

# MuJoCo 시뮬레이션용 로봇 준비

## OnShape에서 로봇을 설계했다면

### onshape-to-robot 제약에 맞게 설계했는지 확인
https://onshape-to-robot.readthedocs.io/en/latest/design.html


urdf에는 `closing_<...>_1`과 `closing_<...>_2`라는 이름의 좌표계가 들어 있습니다. 이것으로 mjcf 파일에서 닫힌 고리를 만들 수 있습니다.

### onshape에서 로봇 urdf 받기

실행:

```bash
$ onshape-to-robot robots/bd1/
```

#### (선택) 닫힌 고리가 있다면 onshape-to-robot 문서의 처리 방법을 따른 뒤 다음을 합니다.

`robot.urdf`에 다음을 추가합니다.
```xml
<robot name="...">
    <mujoco>
        <compiler fusestatic="false"/>
    </mujoco>
	...
</robot>
```

# URDF를 MJCF(MuJoCo)로 변환

## MuJoCo 실행 파일 받기

MuJoCo 실행 파일을 원하는 곳에 내려받습니다. https://github.com/google-deepmind/mujoco/releases

압축을 풀고 실행합니다.

```bash
$ ./compile robot.urdf robot.xml
```

## robot.xml에 구동기 추가
예시:
```xml
	<actuator>
		<position name="left_hip_yaw"    joint="left_hip_yaw"    inheritrange="1"/>
		<position name="left_hip_roll"   joint="left_hip_roll"   inheritrange="1"/>
		<position name="left_hip_pitch"  joint="left_hip_pitch"  inheritrange="1"/>
		<position name="left_knee"       joint="left_knee"       inheritrange="1"/>
		<position name="left_ankle"      joint="left_ankle"      inheritrange="1"/>
		<position name="right_hip_roll"  joint="right_hip_roll"  inheritrange="1"/>
		<position name="right_hip_yaw"   joint="right_hip_yaw"   inheritrange="1"/>
		<position name="right_hip_pitch" joint="right_hip_pitch" inheritrange="1"/>
		<position name="right_knee"      joint="right_knee"      inheritrange="1"/>
		<position name="right_ankle"     joint="right_ankle"     inheritrange="1"/>
		<position name="head_pitch1"     joint="head_pitch1"     inheritrange="1"/>
		<position name="head_pitch2"     joint="head_pitch2"     inheritrange="1"/>
		<position name="head_yaw"        joint="head_yaw"        inheritrange="1"/>
	</actuator>
```

## freejoint 추가

몸체를 freejoint로 감쌉니다.
```xml
<worldbody>
	<body>
		<freejoint />
		...
		...
	</body>
</worldbody>
```

## (선택) 닫힌 고리 제약

mjcf 파일에 다음을 추가합니다.
```xml
<equality>
    <connect body1="closing_<...>_1" body2="closing_<...>_2" anchor="x y z" />
</equality>
```

.urdf에서 x, y, z 값을 찾을 수 있습니다.

```xml
<joint name="closing_<...>_1_frame" type="fixed">
	<origin xyz="x y z" rpy="r p y" />
	...
</joint>
```

## 충돌 그룹, 감쇠, 마찰 설정
/!\ 관절의 actuatorfrcrange를 지우세요.
다음을 <mujoco> 괄호 안에 넣습니다.
```xml
<mujoco>
  <default>
    <geom contype="1" conaffinity="1" solref=".004 1" />
    <joint damping="0.09" frictionloss="0.1"/>
    <position kp="10" forcerange="-5.0 5.0"/>
  </default>
	...
	...
</mujoco>
```

아직 추가해야 할 것:
- 좌표계(frame)를 site로 바꾸기


## 화면으로 확인

```bash
$ python3 -m mujoco.viewer --mjcf=<path>/scene.xml
```

또는

```bash
$ <path_to_mujoco_bin>/bin/simulate <path>/scene.xml
```
