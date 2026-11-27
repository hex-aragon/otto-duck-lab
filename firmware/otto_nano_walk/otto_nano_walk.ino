/*
 * otto_nano_walk — 이엘사이언스 오또봇2 키트(Arduino Nano + 확장쉴드)로 걷기 조립 테스트
 *
 * NU40 펌웨어(otto_nu40)로 옮기기 전에, 키트 그대로 "걷는가"를 먼저 확인하는 용도.
 * 걸음 계산은 otto_nu40/gait.h 와 같은 파일을 복사해서 쓴다 (Arduino는 스케치 폴더 밖 include가 안 됨).
 * gait.h를 고치면 두 폴더 모두 맞출 것.
 *
 * 시리얼 모니터 9600bps, 줄 끝 "새 줄(NL)". 블루투스 모듈을 꽂으면 같은 명령을 폰 BT 터미널 앱으로도 보낼 수 있다
 * (BT 모듈은 D0/D1 공유 → 업로드할 때는 빼기).
 *
 *   h            차렷 (90° + 트림)
 *   i            서보 하나씩 흔들기 → 핀이 어느 관절인지 확인
 *   t <n> <deg>  n번 서보 트림 설정 (0 왼다리, 1 오른다리, 2 왼발, 3 오른발). 예: t 2 -6
 *   save         트림을 EEPROM에 저장 (전원 꺼도 유지)
 *   show         현재 설정 출력
 *   w [걸음]     앞으로 (기본 4걸음)      b [걸음]  뒤로
 *   l [걸음]     왼쪽 돌기                r [걸음]  오른쪽 돌기
 *   u  위아래    s  좌우 흔들기    m  문워크    k  까치발
 *   p <ms>       한 걸음 주기 (기본 1000, 작을수록 빠름. 700 이하는 SG90이 못 따라갈 수 있음)
 *   a <0~100>    진폭 비율 % (기본 100). 처음 걸을 때는 60~70으로 줄여서 시작 추천
 *   v <도/초>    서보 최고 속도 제한 (기본 150, 0이면 제한 없음). 건전지만 쓸 때 보드가 재시작하면 낮출 것
 *   x            멈추고 차렷           off  서보 힘 빼기
 *   d            데모 바로 시작
 *   demo on|off  전원 켤 때 자동 데모 켜기/끄기 (EEPROM 저장, 기본 끔)
 *   eco on|off   절전: 걸을 때 도트 끄고 소리 멈춤 + 차렷 2초 뒤 서보 힘 빼기 (EEPROM 저장, 기본 끔)
 *   dbg on|off   디버그 기록(#B #S #C #D 줄) 켜기/끄기 (EEPROM 저장, 기본 켬)   dbg reset  켠 횟수 0으로
 *                → 노트북 크롬에서 web/otto-debug.html로 열면 전압 그래프 · 재시작 표시 · 로그 저장
 *
 *   ── 도트 · 부저 · 초음파 (extras.h, 핀은 docs/otto-조립-지도.html) ──
 *   e <표정>     도트 표정: eyes blink smile happy sad surprise heart up down left right x all off
 *   e rot <0~3>  도트를 90°씩 돌림 (거꾸로 붙였을 때)     e flip  좌우 거울     e bright <0~15>
 *   z <소리>     부저: beep hello happy sad alarm ok song billie   z <Hz> <ms>  직접 음 내기
 *   z billie <초> 빌리진 리프를 초만큼 반복 (1~30)
 *   n <Hz> <ms>  웹 노래용 음 하나 (응답 없음)   bpm <n>  박자 맞추기   dn <동작 번호 1~16> <ms>  차렷 없이 다음 동작
 *   kp <0~3>     K-pop 무대 느낌 루틴: 0 POP STAR · 1 HYPE STEP · 2 CUTIE BOUNCE · 3 MOON SHUFFLE (창작 멜로디 + 박자 맞춘 안무)
 *   mj <초>      빌리진 틀고 그 시간만큼 문워크 (기본 4초, 최대 30초). 그냥 m도 4~10초 음악이 같이 나옴
 *   mute / unmute 소리 끄기 / 켜기
 *   test         도트·부저·초음파 자동 점검 (초음파 선이 바뀌었는지도 알려 줌)
 *   dist         초음파 거리(cm) 한 번 출력            watch on|off  0.5초마다 거리 출력
 *   mode <이름>  센서 행동: off(기본) guard(걷다가 앞에 막히면 멈추고 돌아섬)
 *                follow(손을 따라 앞뒤로) react(손을 가까이 대면 놀라고 춤)
 *
 * 데모 모드: 'demo on'이면 전원을 켜고 DEMO_DELAY_MS 동안 명령이 안 오면 혼자 걷기·돌기·춤을 반복한다.
 *   USB 없이 건전지만으로 리허설할 때 쓴다. 아무 명령이나 들어오면 데모는 멈춘다.
 *   켜고 기다리는 동안 Nano의 L LED가 빠르게 깜빡인다.
 */
#include <Servo.h>
#include <EEPROM.h>
#include "gait.h"
#include "extras.h"
#include "debug.h"

// ───── 핀 · 방향 (servo_center 또는 'i' 명령으로 확인 후 고칠 것) ─────
// Otto 표준: 왼다리 D2, 오른다리 D3, 왼발 D4, 오른발 D5. 왼쪽/오른쪽 = 로봇 자신 기준.
// 이엘사이언스 키트 실측(11/20 영상): D2=오른다리, D3=왼다리, D4=오른발, D5=왼발 → Otto 표준과 좌우가 반대
static const uint8_t PINS[SERVO_COUNT]    = { 3, 2, 5, 4 };
// 서보를 거울처럼 반대로 붙였으면 true (표준 조립이면 모두 false)
// 키트 실측(11/20): 표준 설정이면 'w'가 뒤로 걸음 → 골반 두 개 방향을 뒤집음
static const bool    REVERSE[SERVO_COUNT] = { true, true, false, false };
static const char* const NAMES[SERVO_COUNT] = { "왼다리(골반)", "오른다리(골반)", "왼발", "오른발" };

static const int     MAX_TRIM   = 30;
static const uint8_t EE_MAGIC   = 0xA7;  // EEPROM 0번지: 저장 여부 표시, 1~4번지: 트림

Servo servo[SERVO_COUNT];
int8_t trim[SERVO_COUNT] = { 0, 0, 0, 0 };
float  lastAngle[SERVO_COUNT] = { 90, 90, 90, 90 };
bool   attached = false;

uint16_t periodMs = 1000;
uint8_t  ampPct   = 100;
// 서보 최고 속도(도/초). 서보가 한꺼번에 빨리 움직이면 순간 전류가 커서 쉴드 5V가 떨어지고
// 보드·블루투스가 재시작한다 (11/22 건전지만으로 걷기 시험에서 확인). 속도를 묶어 순간 전류를 줄인다.
uint16_t maxSpeed = 150;

// 동작 상태
Action   curAct = ACT_HOME;
uint32_t actStart = 0, actDuration = 0, lastTick = 0;

// ───── 서보 출력 ─────
void writeServo(int i, float deg) {
  float a = deg + trim[i];
  if (REVERSE[i]) a = 180 - a;
  a = constrain(a, 0, 180);
  servo[i].write((int)(a + 0.5f));
  lastAngle[i] = deg;
}

// 서보를 하나씩 차례로 켠다. 4개가 동시에 힘을 쓰면 켜는 순간 전압이 크게 떨어진다.
void attachAll() {
  if (attached) return;
  for (int i = 0; i < SERVO_COUNT; i++) {
    writeServo(i, lastAngle[i]);   // attach 전에 써 두면 첫 펄스가 그 각도로 나간다
    servo[i].attach(PINS[i]);
    debugServo(i);
    delay(120);
  }
  attached = true;
}

void detachAll() {
  for (int i = 0; i < SERVO_COUNT; i++) servo[i].detach();
  attached = false;
}

// 현재 각도 → 목표 각도로 ms 동안 부드럽게 (갑자기 튀면 넘어지거나 전압이 떨어져 리셋됨)
void moveTo(const float target[SERVO_COUNT], uint16_t ms) {
  attachAll();
  float from[SERVO_COUNT];
  for (int i = 0; i < SERVO_COUNT; i++) from[i] = lastAngle[i];
  const uint16_t step = 20;
  for (uint16_t t = step; t <= ms; t += step) {
    float r = (float)t / ms;
    for (int i = 0; i < SERVO_COUNT; i++) writeServo(i, from[i] + (target[i] - from[i]) * r);
    sampleVcc();
    delay(step);
  }
  for (int i = 0; i < SERVO_COUNT; i++) writeServo(i, target[i]);
}

// ───── 절전 (eco) ─────
// 걷는 동안 도트를 끄고(MAX7219 대기 모드, 0.15mA) 소리를 멈춰 서보에 전류를 몰아 준다.
// 차렷으로 돌아오면 다시 켠다. 'eco on|off' (EEPROM 10번지, 기본 끔)
static const uint8_t EE_ECO_ADDR = 10, EE_ECO_ON = 0xEC;
bool ecoOn = false, ecoSleeping = false;
void ecoSleep() {
  if (!ecoOn || ecoSleeping) return;
  dotWrite(0x0C, 0);
  sndNotes = nullptr; noTone(BUZZER);
  ecoSleeping = true;
}
// 절전일 때 차렷으로 2초 가만히 있으면 서보 힘을 뺀다. 서 있는 자세는 기어가 버텨 준다.
// 11/23 측정: 차렷으로 버티기만 해도 5V 최저가 4.77V → 4.15V(가끔 3.6V)로 떨어짐. 힘을 빼면 4.77V 유지.
uint32_t homeSince = 0;
void tickEcoRelax() {
  if (!ecoOn || !attached || curAct != ACT_HOME || !homeSince) return;
  if (millis() - homeSince < 2000) return;
  detachAll(); homeSince = 0;
  Serial.println(F("> 쉬는 중: 서보 힘 뺌 (eco, 다음 동작 때 다시 켬)"));
}
void ecoWake() {
  if (!ecoSleeping) return;
  dotWrite(0x0C, 1);
  ecoSleeping = false;
}

void home(uint16_t ms = 500) {
  ecoWake();
  float t[SERVO_COUNT] = { 90, 90, 90, 90 };
  moveTo(t, ms);
  curAct = ACT_HOME;
  homeSince = millis();
}

// 진폭 비율 적용한 파라미터
GaitParams scaled(Action a) {
  GaitParams p = gaitFor(a);
  for (int i = 0; i < SERVO_COUNT; i++) p.A[i] *= ampPct / 100.0f;
  return p;
}

// 동작에 맞는 표정
Face faceFor(Action a) {
  switch (a) {
    case ACT_WALK_FWD:   return F_SMILE;
    case ACT_WALK_BACK:  return F_DOWN;
    case ACT_TURN_LEFT:  return F_LEFT;
    case ACT_TURN_RIGHT: return F_RIGHT;
    case ACT_HOME:       return F_EYES;
    default:             return F_HAPPY;   // 춤
  }
}

// ───── K-pop 무대 느낌 루틴 (kp 0~3) ─────
// 창작 멜로디(extras.h S_KP0~3)에 맞춰 동작을 박자 단위로 이어 붙인다. 동작 사이에 차렷으로 돌아가지 않는다.
// 한 동작 주기 = 2박. web/otto-pet.html의 ROUTINES와 같은 순서·같은 박자.
struct RStep { uint8_t act; uint8_t beats; };
static const RStep R0[] PROGMEM = { {ACT_BOUNCE,4},{ACT_SWING,4},{ACT_POSE_L,2},{ACT_POSE_R,2},{ACT_TWIST,4},{ACT_WAVE,4},{ACT_KICK,4},{ACT_TIPTOE,4},{ACT_BOUNCE,4},{ACT_POSE_L,2},{ACT_POSE_R,2} };
static const RStep R1[] PROGMEM = { {ACT_SHUFFLE,4},{ACT_TWIST,4},{ACT_KICK,4},{ACT_POSE_R,2},{ACT_POSE_L,2},{ACT_SHUFFLE,4},{ACT_WAVE,4},{ACT_UPDOWN,4},{ACT_POSE_L,2},{ACT_POSE_R,2},{ACT_KICK,4} };
static const RStep R2[] PROGMEM = { {ACT_BOUNCE,4},{ACT_TIPTOE,4},{ACT_WAVE,4},{ACT_POSE_L,2},{ACT_POSE_R,2},{ACT_SWING,4},{ACT_BOUNCE,4},{ACT_POSE_R,2},{ACT_POSE_L,2} };
static const RStep R3[] PROGMEM = { {ACT_MOONWALK_LEFT,4},{ACT_MOONWALK_RIGHT,4},{ACT_SHUFFLE,4},{ACT_WAVE,4},{ACT_TWIST,4},{ACT_POSE_L,2},{ACT_POSE_R,2},{ACT_MOONWALK_LEFT,4} };
struct Routine { const char* name; uint8_t bpm; const RStep* steps; uint8_t n; };
#define RT(nm, bpm, arr) { nm, bpm, arr, (uint8_t)(sizeof(arr) / sizeof(arr[0])) }
static const Routine ROUTINES[] = { RT("POP STAR", 120, R0), RT("HYPE STEP", 128, R1), RT("CUTIE BOUNCE", 110, R2), RT("MOON SHUFFLE", 100, R3) };
static const uint8_t ROUTINE_COUNT = sizeof(ROUTINES) / sizeof(ROUTINES[0]);
int8_t   rIdx = -1;
uint8_t  rStep = 0;
uint16_t rSavedPeriod = 1000;
bool     noRamp = false;   // 웹 노래(dn): 동작이 바뀌어도 진폭을 0부터 다시 키우지 않음

void endRoutine() {
  if (rIdx < 0) return;
  rIdx = -1; periodMs = rSavedPeriod;
  sndNotes = nullptr; noTone(BUZZER);
}
// 다음 동작으로 넘어가면 true, 루틴이 끝났으면 false
bool nextRoutineStep() {
  const Routine& r = ROUTINES[rIdx];
  if (rStep >= r.n) { endRoutine(); return false; }
  RStep st; memcpy_P(&st, &r.steps[rStep++], sizeof(st));
  Action a = (Action)st.act;
  setFace(a == ACT_POSE_L || a == ACT_POSE_R ? F_HEART : a == ACT_KICK ? F_SURPRISE : F_HAPPY);
  curAct = a;
  actStart = millis();
  actDuration = (uint32_t)st.beats * 60000UL / r.bpm;
  Serial.print(F("> ")); Serial.print(ACTION_NAMES[a]);
  Serial.print(F(" x")); Serial.print(st.beats / 2);
  Serial.print(F(", 주기 ")); Serial.print(periodMs);
  Serial.print(F("ms, 진폭 ")); Serial.print(ampPct); Serial.println('%');
  return true;
}
void startRoutine(uint8_t i) {
  if (i >= ROUTINE_COUNT) return;
  endRoutine();
  const Routine& r = ROUTINES[i];
  rSavedPeriod = periodMs;
  periodMs = 120000UL / r.bpm;          // 한 주기 = 2박
  ecoSleep();
  float t[SERVO_COUNT] = { 90, 90, 90, 90 };
  moveTo(t, 300);
  rIdx = i; rStep = 0;
  uint16_t beats = 0;
  for (uint8_t k = 0; k < r.n; k++) { RStep st; memcpy_P(&st, &r.steps[k], sizeof(st)); beats += st.beats; }
  Serial.print(F("> 루틴 ")); Serial.print(i); Serial.print(' '); Serial.print(r.name);
  Serial.print(' '); Serial.print(r.bpm); Serial.print(F("BPM ")); Serial.print(beats); Serial.println(F("박"));
  if (!ecoOn) playSoundFor(SND_KP0 + i, (uint32_t)beats * 60000UL / r.bpm);
  nextRoutineStep();
  Serial.flush();
}

void startAction(Action a, int steps) {
  endRoutine();
  if (steps < 1) steps = 1;
  setFace(faceFor(a));
  ecoSleep();
  GaitParams p = scaled(a);
  for (int i = 0; i < SERVO_COUNT; i++) p.A[i] = 0;   // tickAction이 진폭 0에서 시작하므로 시작 자세도 진폭 0
  float first[SERVO_COUNT];
  gaitAngles(p, 0, periodMs, first);
  moveTo(first, 300);              // 시작 자세로 부드럽게 들어간 뒤 사인파 시작
  curAct = a; noRamp = false;
  actStart = millis();
  actDuration = (uint32_t)steps * periodMs;
  Serial.print(F("> ")); Serial.print(ACTION_NAMES[a]);
  Serial.print(F(" x")); Serial.print(steps);
  Serial.print(F(", 주기 ")); Serial.print(periodMs);
  Serial.print(F("ms, 진폭 ")); Serial.print(ampPct); Serial.println('%');
  // 문워크엔 빌리진. 춤 길이만큼(최소 4초, 최대 10초). 절전 중엔 소리 안 냄
  if (a == ACT_MOONWALK_LEFT || a == ACT_MOONWALK_RIGHT)
    if (!ecoOn) playSoundFor(SND_BILLIE, constrain(actDuration, 4000UL, 10000UL));
  Serial.flush();   // 걷기 시작 직후 재시작해도 이 줄은 나가도록
}

// 웹 노래 춤: 지금 춤추는 중이면 차렷 없이 바로 다음 동작으로 (박자는 bpm 명령으로)
void danceNext(int ai, uint32_t ms) {
  if (ai <= ACT_HOME || ai >= ACT_COUNT) return;
  endRoutine();
  Action a = (Action)ai;
  bool moving = curAct != ACT_HOME;
  setFace(a == ACT_POSE_L || a == ACT_POSE_R ? F_HEART : a == ACT_KICK ? F_SURPRISE : F_HAPPY);
  if (!moving) {
    ecoSleep();
    GaitParams p = scaled(a);
    for (int i = 0; i < SERVO_COUNT; i++) p.A[i] = 0;
    float first[SERVO_COUNT];
    gaitAngles(p, 0, periodMs, first);
    moveTo(first, 200);
  }
  noRamp = moving;
  curAct = a;
  actStart = millis();
  actDuration = constrain(ms, 100UL, 20000UL);
  Serial.print(F("> ")); Serial.print(ACTION_NAMES[a]);
  Serial.print(F(" x1, 주기 ")); Serial.print(periodMs); Serial.println(F("ms"));
}

void tickAction() {
  if (curAct == ACT_HOME) return;
  uint32_t now = millis();
  if (now - lastTick < 20) return;  // 50Hz
  lastTick = now;
  uint32_t t = now - actStart;
  if (t >= actDuration && rIdx >= 0 && nextRoutineStep()) return;   // 루틴: 차렷 없이 다음 동작
  if (t >= actDuration) { home(); setFace(F_EYES); Serial.println(F("> 끝, 차렷")); demoAfterAction(); return; }
  GaitParams p = scaled(curAct);
  // 첫 한 걸음 동안 진폭을 0 → 100%로 키운다 (처음부터 크게 움직이면 전압이 떨어짐)
  if (rIdx < 0 && !noRamp && t < periodMs) { float r = (float)t / periodMs; for (int i = 0; i < SERVO_COUNT; i++) p.A[i] *= r; }
  float out[SERVO_COUNT];
  gaitAngles(p, t, periodMs, out);
  float maxStep = maxSpeed ? maxSpeed * 0.02f : 999;   // 20ms 한 번에 움직일 수 있는 최대 각도
  for (int i = 0; i < SERVO_COUNT; i++) {
    float d = constrain(out[i] - lastAngle[i], -maxStep, maxStep);
    writeServo(i, lastAngle[i] + d);
  }
}

// ───── 데모 모드 ─────
static const uint32_t DEMO_DELAY_MS = 5000;   // 켜고 이만큼 명령이 없으면 데모 시작
static const uint16_t DEMO_GAP_MS   = 600;    // 동작 사이 쉬는 시간
struct DemoStep { Action act; uint8_t steps; uint16_t period; };
// ACT_HOME 은 period 만큼 차렷으로 쉬기
static const DemoStep DEMO[] = {
  { ACT_WALK_FWD,       4, 1200 },
  { ACT_TURN_LEFT,      3, 1200 },
  { ACT_WALK_FWD,       4, 1200 },
  { ACT_TURN_RIGHT,     3, 1200 },
  { ACT_HOME,           0, 1000 },
  { ACT_UPDOWN,         3, 1000 },
  { ACT_SWING,          3, 1000 },
  { ACT_MOONWALK_LEFT,  3, 1000 },
  { ACT_MOONWALK_RIGHT, 3, 1000 },
  { ACT_TIPTOE,         3, 1000 },
  { ACT_HOME,           0, 2000 },
};
static const uint8_t DEMO_LEN = sizeof(DEMO) / sizeof(DEMO[0]);

// 전원 켤 때 데모 자동 시작 여부 (EEPROM 5번지에 저장). 조립·영점 작업 중엔 꺼 둔다.
static const uint8_t EE_DEMO_ADDR = 5, EE_DEMO_ON = 0xD1;
static const uint8_t EE_DOT_ADDR = 6;   // 0xD0 | 회전(0~3) | 거울<<2
bool     demoAuto = false;
bool     demoDecided = false;   // 부팅 후 데모 여부를 이미 정했는지
bool     demoOn = false;
uint8_t  demoIdx = 0;
uint32_t demoNextAt = 0;
uint16_t savedPeriod = 1000;

void startDemo() {
  demoDecided = true;
  demoOn = true;
  demoIdx = 0;
  demoNextAt = millis();
  savedPeriod = periodMs;
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println(F("> 데모 시작 (아무 명령이나 보내면 멈춤)"));
}

void stopDemo() {
  demoDecided = true;
  if (!demoOn) return;
  demoOn = false;
  periodMs = savedPeriod;
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println(F("> 데모 멈춤"));
}

void tickDemo() {
  uint32_t now = millis();
  if (!demoDecided) {
    if (!demoAuto) { demoDecided = true; return; }  // 자동 데모 꺼짐: 차렷으로 명령 대기
    digitalWrite(LED_BUILTIN, (now / 125) % 2);     // 기다리는 동안 빠르게 깜빡임
    if (now >= DEMO_DELAY_MS) startDemo();
    return;
  }
  if (!demoOn || curAct != ACT_HOME || now < demoNextAt) return;
  const DemoStep& s = DEMO[demoIdx];
  demoIdx = (demoIdx + 1) % DEMO_LEN;
  if (s.act == ACT_HOME) {
    demoNextAt = now + s.period;
  } else {
    periodMs = s.period;
    startAction(s.act, s.steps);
    demoNextAt = 0;                                 // 동작이 끝나 차렷이 되면 아래에서 쉬는 시간 설정
  }
}

// 동작이 끝나 차렷으로 돌아온 순간 쉬는 시간을 둔다
void demoAfterAction() {
  if (demoOn && demoNextAt == 0) demoNextAt = millis() + DEMO_GAP_MS;
}

// ───── 센서 행동 (초음파) ─────
enum SenseMode : uint8_t { M_OFF, M_GUARD, M_FOLLOW, M_REACT };
static const char* const MODE_NAMES[] = { "off", "guard", "follow", "react" };
SenseMode senseMode = M_OFF;
bool      watchDist = false;
uint32_t  nextSense = 0, nextWatch = 0, reactReady = 0;

// 동작 예약 (센서가 "뒤로 2걸음 → 왼쪽 돌기"처럼 이어서 시킬 때)
struct Queued { Action a; uint8_t n; };
Queued   queue[4];
uint8_t  qLen = 0;
void enqueue(Action a, uint8_t n) { if (qLen < 4) queue[qLen++] = { a, n }; }
void clearQueue() { qLen = 0; }
void tickQueue() {
  if (curAct != ACT_HOME || !qLen) return;
  Queued q = queue[0];
  for (uint8_t i = 1; i < qLen; i++) queue[i - 1] = queue[i];
  qLen--;
  startAction(q.a, q.n);
}

void tickSense() {
  uint32_t now = millis();
  if (watchDist && now >= nextWatch) {
    nextWatch = now + 500;
    Serial.print(F("거리 ")); Serial.print(readCm()); Serial.println(F("cm"));
  }
  if (senseMode == M_OFF || now < nextSense) return;
  nextSense = now + 120;
  uint16_t d = readCm();
  if (!d) return;                                    // 반사 없음 (멀거나 선 빠짐)
  switch (senseMode) {
    case M_GUARD:   // 앞으로 걷다가 15cm 안에 뭔가 있으면 멈추고 비켜 감
      if (curAct == ACT_WALK_FWD && d < 15) {
        home(300); clearQueue();
        setFace(F_SURPRISE); playSound(4);           // alarm
        Serial.print(F("> 앞에 장애물 ")); Serial.print(d); Serial.println(F("cm: 뒤로 → 왼쪽"));
        enqueue(ACT_WALK_BACK, 2); enqueue(ACT_TURN_LEFT, 2);
      }
      break;
    case M_FOLLOW:  // 손이 가까우면 뒤로, 조금 멀면 앞으로
      if (curAct != ACT_HOME || qLen) break;
      if (d < 8)                 { Serial.println(F("> 가까움: 뒤로")); startAction(ACT_WALK_BACK, 1); }
      else if (d > 14 && d < 40) { Serial.println(F("> 따라가기: 앞으로")); startAction(ACT_WALK_FWD, 1); }
      break;
    case M_REACT:   // 손을 10cm 안에 대면 놀라고 춤
      if (curAct != ACT_HOME || qLen || now < reactReady || d >= 10) break;
      reactReady = now + 5000;
      setFace(F_SURPRISE); playSound(1);             // hello
      Serial.println(F("> 반가워!"));
      enqueue(ACT_UPDOWN, 2); enqueue(ACT_SWING, 2);
      break;
    default: break;
  }
}

// ───── 확인용 ─────
void identify() {
  home();
  for (int i = 0; i < SERVO_COUNT; i++) {
    Serial.print(F("D")); Serial.print(PINS[i]); Serial.print(F(" → 코드상 "));
    Serial.print(NAMES[i]); Serial.println(F(" 이 흔들려야 함"));
    for (int k = 0; k < 2; k++) { writeServo(i, 70); delay(350); writeServo(i, 110); delay(350); }
    writeServo(i, 90);
    delay(1200);
  }
  Serial.println(F("다르면 PINS 배열 순서를 고치고 다시 업로드"));
}

void loadTrims() {
  if (EEPROM.read(0) != EE_MAGIC) return;
  for (int i = 0; i < SERVO_COUNT; i++) trim[i] = (int8_t)EEPROM.read(1 + i);
}

void saveTrims() {
  EEPROM.update(0, EE_MAGIC);
  for (int i = 0; i < SERVO_COUNT; i++) EEPROM.update(1 + i, (uint8_t)trim[i]);
  Serial.println(F("트림 저장함"));
}

void show() {
  Serial.print(F("트림: "));
  for (int i = 0; i < SERVO_COUNT; i++) { Serial.print(NAMES[i]); Serial.print('='); Serial.print(trim[i]); Serial.print(' '); }
  Serial.println();
  Serial.print(F("주기 ")); Serial.print(periodMs); Serial.print(F("ms, 진폭 ")); Serial.print(ampPct);
  Serial.print(F("%, 최고 속도 ")); Serial.print(maxSpeed); Serial.println(F("도/초"));
}

// ───── 명령 처리 ─────
char buf[24];
uint8_t len = 0;
uint32_t lastChar = 0;

void handle(char* line) {
  debugCmd(line);
  char* cmd = strtok(line, " ");
  if (!cmd) return;
  char* a1 = strtok(NULL, " ");
  char* a2 = strtok(NULL, " ");
  int n = a1 ? atoi(a1) : 4;

  if (!strcmp(cmd, "d")) { home(); startDemo(); return; }
  if (!strcmp(cmd, "demo") && a1) {
    demoAuto = !strcmp(a1, "on");
    EEPROM.update(EE_DEMO_ADDR, demoAuto ? EE_DEMO_ON : 0);
    Serial.println(demoAuto ? F("> 켤 때 자동 데모: 켬 (저장됨)") : F("> 켤 때 자동 데모: 끔 (저장됨)"));
    stopDemo();
    return;
  }
  stopDemo();   // 다른 명령이 오면 데모는 멈춘다

  if (!strcmp(cmd, "eco")) {   // 걸을 때 도트·소리 끄기
    if (a1) { ecoOn = !strcmp(a1, "on"); EEPROM.update(EE_ECO_ADDR, ecoOn ? EE_ECO_ON : 0); if (!ecoOn) ecoWake(); }
    Serial.println(ecoOn ? F("> 절전: 걸을 때 도트·소리 끔, 쉴 때 서보 힘 뺌 (저장됨)") : F("> 절전 끔: 걸을 때도 도트·소리 켬 (저장됨)"));
    return;
  }
  if (!strcmp(cmd, "dbg")) {   // 디버그 기록 켜기/끄기 (EEPROM 저장)
    if (a1 && !strcmp(a1, "reset")) { EEPROM.update(EE_BOOT_ADDR, 0); EEPROM.update(EE_BOOT_ADDR + 1, 0); Serial.println(F("> 켠 횟수 0으로")); return; }
    if (a1) { dbgOn = strcmp(a1, "off") != 0; EEPROM.update(EE_DBG_ADDR, dbgOn ? 0 : 0xDB); }
    sampleVcc();
    Serial.print(F("> 디버그 기록 ")); Serial.print(dbgOn ? F("켬") : F("끔"));
    Serial.print(F(", 지금 전압 ")); Serial.print(vccNow); Serial.println(F("mV"));
    return;
  }

  // 도트 · 부저 · 초음파
  if (!strcmp(cmd, "e") && a1) {
    if (!strcmp(a1, "rot") && a2) dotRot = atoi(a2) & 3;
    else if (!strcmp(a1, "flip")) dotFlip = !dotFlip;
    else if (!strcmp(a1, "bright") && a2) { dotWrite(0x0A, constrain(atoi(a2), 0, 15)); return; }
    else { int f = faceByName(a1); if (f < 0) { Serial.println(F("? 표정: eyes blink smile happy sad surprise heart up down left right x all off")); return; }
           idleBlink = (f == F_EYES); setFace((Face)f); Serial.print(F("> 표정 ")); Serial.println(a1); return; }
    EEPROM.update(EE_DOT_ADDR, 0xD0 | dotRot | (dotFlip << 2));
    setFace(curFace == F_OFF ? F_UP : curFace);
    Serial.print(F("> 도트 회전 ")); Serial.print(dotRot); Serial.print(F(", 거울 ")); Serial.println(dotFlip ? F("켬") : F("끔"));
    return;
  }
  if (!strcmp(cmd, "z") && a1) {
    if (a2 && soundByName(a1) >= 0) {   // z billie 10 → 10초 반복
      int sec = constrain(atoi(a2), 1, 30);
      playSoundFor(soundByName(a1), (uint32_t)sec * 1000);
      Serial.print(F("> 소리 ")); Serial.print(a1); Serial.print(' '); Serial.print(sec); Serial.println(F("초")); return;
    }
    if (a2) { tone(BUZZER, constrain(atoi(a1), 31, 8000), constrain(atoi(a2), 10, 3000)); Serial.println(F("> 삐")); return; }
    int i = soundByName(a1);
    if (i < 0) { Serial.println(F("? 소리: beep hello happy sad alarm ok song billie")); return; }
    playSound(i); Serial.print(F("> 소리 ")); Serial.println(a1); return;
  }
  // ── 웹 노래 (OTTO LIVE의 🎵 내 노래): 음은 n, 박자는 bpm, 춤은 dn ──
  if (!strcmp(cmd, "n") && a1) {   // n <Hz> <ms>  조용히 음 하나 (응답 줄 없음, 0Hz면 쉼표)
    int hz = atoi(a1);
    if (soundOn && !ecoOn) { sndNotes = nullptr; if (hz > 0) tone(BUZZER, constrain(hz, 31, 8000), a2 ? constrain(atoi(a2), 10, 3000) : 200); else noTone(BUZZER); }
    return;
  }
  if (!strcmp(cmd, "bpm") && a1) {   // bpm <n>  한 주기 = 2박
    endRoutine(); periodMs = constrain(120000L / constrain(atoi(a1), 40, 300), 400L, 3000L);
    Serial.print(F("> BPM ")); Serial.print(a1); Serial.print(F(", 주기 ")); Serial.print(periodMs); Serial.println(F("ms")); return;
  }
  if (!strcmp(cmd, "dn") && a1) { danceNext(atoi(a1), a2 ? (uint32_t)atol(a2) : periodMs); return; }   // dn <동작 번호> <ms>
  if (!strcmp(cmd, "test")) {   // 도트 · 부저 · 초음파 자동 점검
    Serial.println(F("[점검 1] 도트: 전부 켜기 2초 → 웃음 → 하트"));
    setFace(F_ALL); delay(2000); setFace(F_SMILE); delay(1500); setFace(F_HEART); delay(1500);
    Serial.println(F("[점검 2] 부저: 낮은 삐 → 높은 삐 → 1초 길게"));
    tone(BUZZER, 500, 300); delay(500); tone(BUZZER, 2000, 300); delay(500); tone(BUZZER, 1000, 1000); delay(1200);
    Serial.println(F("[점검 3] 초음파: 센서 앞 20cm쯤에 손을 대고 있으세요"));
    delay(1500);
    uint8_t ok = 0, swapped = 0; uint16_t last = 0, lastSw = 0;
    for (uint8_t k = 0; k < 5; k++) {
      uint16_t d = readCmPins(US_TRIG, US_ECHO); if (d) { ok++; last = d; } delay(60);
      uint16_t e = readCmPins(US_ECHO, US_TRIG); if (e) { swapped++; lastSw = e; } delay(60);
    }
    pinMode(US_TRIG, OUTPUT); pinMode(US_ECHO, INPUT);
    pinMode(US_ECHO, INPUT_PULLUP); delay(2); bool echoIdle = digitalRead(US_ECHO); pinMode(US_ECHO, INPUT);
    Serial.print(F("  정상 배선으로 잰 횟수 ")); Serial.print(ok); Serial.print(F("/5, 마지막 ")); Serial.print(last); Serial.println(F("cm"));
    Serial.print(F("  Trig·Echo 바꿔서 잰 횟수 ")); Serial.print(swapped); Serial.print(F("/5, 마지막 ")); Serial.print(lastSw); Serial.println(F("cm"));
    if (ok) Serial.println(F("  → 초음파 정상"));
    else if (swapped) Serial.println(F("  → Trig와 Echo 선이 서로 바뀜: 8번 줄 S와 9번 줄 S의 선을 맞바꾸세요"));
    else Serial.println(echoIdle ? F("  → 반응 없음: 초음파 VCC(8번 줄 V)·GND(9번 줄 G) 선, 핀 4개 순서를 확인하세요")
                                 : F("  → 반응 없음: 센서 전원·선 확인"));
    setFace(F_EYES);
    Serial.println(F("[점검 끝]"));
    return;
  }
  if (!strcmp(cmd, "mute"))   { soundOn = false; sndNotes = nullptr; noTone(BUZZER); Serial.println(F("> 소리 끔")); return; }
  if (!strcmp(cmd, "unmute")) { soundOn = true; playSound(0); Serial.println(F("> 소리 켬")); return; }
  if (!strcmp(cmd, "dist"))   { uint16_t d = readCm(); Serial.print(F("거리 ")); Serial.print(d); Serial.println(d ? F("cm") : F("cm (반사 없음: 너무 멀거나 선 확인)")); return; }
  if (!strcmp(cmd, "watch") && a1) { watchDist = !strcmp(a1, "on"); return; }
  if (!strcmp(cmd, "mode") && a1) {
    for (uint8_t i = 0; i < 4; i++) if (!strcmp(a1, MODE_NAMES[i])) {
      senseMode = (SenseMode)i; clearQueue(); playSound(5);
      Serial.print(F("> 센서 모드 ")); Serial.println(a1); return;
    }
    Serial.println(F("? mode off | guard | follow | react")); return;
  }

  if(!strcmp(cmd, "h") || !strcmp(cmd, "x")) { endRoutine(); clearQueue(); sndNotes = nullptr; noTone(BUZZER); home(); setFace(F_EYES); idleBlink = true; Serial.println(F("> 차렷")); }
  else if (!strcmp(cmd, "off"))  { curAct = ACT_HOME; detachAll(); Serial.println(F("> 서보 힘 뺌")); }
  else if (!strcmp(cmd, "i"))    identify();
  else if (!strcmp(cmd, "show")) show();
  else if (!strcmp(cmd, "save")) saveTrims();
  else if (!strcmp(cmd, "t") && a1 && a2) {
    int i = atoi(a1);
    if (i >= 0 && i < SERVO_COUNT) {
      trim[i] = constrain(atoi(a2), -MAX_TRIM, MAX_TRIM);
      home(200); show();
    }
  }
  else if (!strcmp(cmd, "p") && a1) { periodMs = constrain(atoi(a1), 400, 3000); show(); }
  else if (!strcmp(cmd, "a") && a1) { ampPct = constrain(atoi(a1), 0, 100); show(); }
  else if (!strcmp(cmd, "v") && a1) { maxSpeed = constrain(atoi(a1), 0, 1000); show(); }
  else if (!strcmp(cmd, "w")) startAction(ACT_WALK_FWD, n);
  else if (!strcmp(cmd, "b")) startAction(ACT_WALK_BACK, n);
  else if (!strcmp(cmd, "l")) startAction(ACT_TURN_LEFT, n);
  else if (!strcmp(cmd, "r")) startAction(ACT_TURN_RIGHT, n);
  else if (!strcmp(cmd, "u")) startAction(ACT_UPDOWN, n);
  else if (!strcmp(cmd, "s")) startAction(ACT_SWING, n);
  else if (!strcmp(cmd, "m")) startAction(ACT_MOONWALK_LEFT, n);
  else if (!strcmp(cmd, "mj")) {   // 빌리진 문워크: 초만큼 춤추고 음악도 그만큼
    int sec = a1 ? constrain(atoi(a1), 1, 30) : 4;
    startAction(ACT_MOONWALK_LEFT, (sec * 1000L + periodMs - 1) / periodMs);
    if (!ecoOn) playSoundFor(SND_BILLIE, (uint32_t)sec * 1000);
  }
  else if (!strcmp(cmd, "k")) startAction(ACT_TIPTOE, n);
  else if (!strcmp(cmd, "kp")) { clearQueue(); startRoutine(a1 ? atoi(a1) : 0); }   // K-pop 루틴 0~3
  else Serial.println(F("? 명령: h i t save show w b l r u s m mj k kp p a v x off d / e z mute dist watch mode dbg eco"));
}

void readSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    lastChar = millis();
    if (c == '\n' || c == '\r') {
      if (len) { buf[len] = 0; handle(buf); len = 0; }
    } else if (len < sizeof(buf) - 1) {
      buf[len++] = c;
    }
  }
  // 줄 끝을 안 보내는 BT 터미널 앱 대응: 150ms 조용하면 한 명령으로 처리
  if (len && millis() - lastChar > 150) { buf[len] = 0; handle(buf); len = 0; }
}
// ───── 쌍둥이 페이지용 각도 (web/otto-twin.html) ─────
// #A <왼다리> <오른다리> <왼발> <오른발> <표정>   각도는 트림·반전 전 값(90 = 차렷), 표정은 Face 번호
// 움직이는 동안 0.1초마다, 가만히 있으면 1초마다. 디버그 기록(dbg)이 켜져 있을 때만.
uint32_t nextAng = 0;
void tickAngles() {
  if (!dbgOn) return;
  uint32_t now = millis();
  if (now < nextAng) return;
  nextAng = now + (curAct != ACT_HOME ? 100 : 1000);
  if (Serial.availableForWrite() < 24) return;   // 보내는 버퍼가 차 있으면 건너뜀 (걷기 박자를 막지 않게)
  Serial.print(F("#A"));
  for (int i = 0; i < SERVO_COUNT; i++) { Serial.print(' '); Serial.print((int)(lastAngle[i] + 0.5f)); }
  Serial.print(' '); Serial.println((int)curFace);
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  debugBoot();
  demoAuto = (EEPROM.read(EE_DEMO_ADDR) == EE_DEMO_ON);
  ecoOn = (EEPROM.read(EE_ECO_ADDR) == EE_ECO_ON);
  loadTrims();
  uint8_t dv = EEPROM.read(EE_DOT_ADDR);
  if ((dv & 0xF0) == 0xD0) { dotRot = dv & 3; dotFlip = dv & 4; }
  extrasInit();
  readVccRaw(); delay(2);   // extrasInit의 analogRead가 ADC 기준을 바꿔 놓으므로 다시 맞춤
  setFace(F_EYES);
  Serial.println(F("서보 하나씩 켜는 중..."));
  attachAll();   // lastAngle 초기값 90° → 차렷으로 하나씩 켜짐
  Serial.println(F("otto_nano_walk 준비. 'show'로 설정 확인, 'w'로 걷기"));
  Serial.println(demoAuto ? F("자동 데모 켬: 5초간 명령 없으면 시작 ('demo off'로 끔)")
                          : F("자동 데모 끔: 'demo on'으로 켬, 'd'로 바로 시작"));
  show();
  playSound(1);   // hello
}

void loop() {
  readSerial();
  tickAction();
  tickQueue();
  tickDemo();
  tickSense();
  tickSound();
  tickBlink(curAct == ACT_HOME);
  tickEcoRelax();
  tickDebug(curAct);
  tickAngles();
}
