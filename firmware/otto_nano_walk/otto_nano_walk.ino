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
 *   x            멈추고 차렷           off  서보 힘 빼기
 */
#include <Servo.h>
#include <EEPROM.h>
#include "gait.h"

// ───── 핀 · 방향 (servo_center 또는 'i' 명령으로 확인 후 고칠 것) ─────
// Otto 표준: 왼다리 D2, 오른다리 D3, 왼발 D4, 오른발 D5. 왼쪽/오른쪽 = 로봇 자신 기준.
static const uint8_t PINS[SERVO_COUNT]    = { 2, 3, 4, 5 };
// 서보를 거울처럼 반대로 붙였으면 true (표준 조립이면 모두 false)
static const bool    REVERSE[SERVO_COUNT] = { false, false, false, false };
static const char* const NAMES[SERVO_COUNT] = { "왼다리(골반)", "오른다리(골반)", "왼발", "오른발" };

static const int     MAX_TRIM   = 30;
static const uint8_t EE_MAGIC   = 0xA7;  // EEPROM 0번지: 저장 여부 표시, 1~4번지: 트림

Servo servo[SERVO_COUNT];
int8_t trim[SERVO_COUNT] = { 0, 0, 0, 0 };
float  lastAngle[SERVO_COUNT] = { 90, 90, 90, 90 };
bool   attached = false;

uint16_t periodMs = 1000;
uint8_t  ampPct   = 100;

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

void attachAll() {
  if (attached) return;
  for (int i = 0; i < SERVO_COUNT; i++) servo[i].attach(PINS[i]);
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
    delay(step);
  }
  for (int i = 0; i < SERVO_COUNT; i++) writeServo(i, target[i]);
}

void home(uint16_t ms = 500) {
  float t[SERVO_COUNT] = { 90, 90, 90, 90 };
  moveTo(t, ms);
  curAct = ACT_HOME;
}

// 진폭 비율 적용한 파라미터
GaitParams scaled(Action a) {
  GaitParams p = gaitFor(a);
  for (int i = 0; i < SERVO_COUNT; i++) p.A[i] *= ampPct / 100.0f;
  return p;
}

void startAction(Action a, int steps) {
  if (steps < 1) steps = 1;
  GaitParams p = scaled(a);
  float first[SERVO_COUNT];
  gaitAngles(p, 0, periodMs, first);
  moveTo(first, 300);              // 시작 자세로 부드럽게 들어간 뒤 사인파 시작
  curAct = a;
  actStart = millis();
  actDuration = (uint32_t)steps * periodMs;
  Serial.print(F("> ")); Serial.print(ACTION_NAMES[a]);
  Serial.print(F(" x")); Serial.print(steps);
  Serial.print(F(", 주기 ")); Serial.print(periodMs);
  Serial.print(F("ms, 진폭 ")); Serial.print(ampPct); Serial.println('%');
}

void tickAction() {
  if (curAct == ACT_HOME) return;
  uint32_t now = millis();
  if (now - lastTick < 20) return;  // 50Hz
  lastTick = now;
  uint32_t t = now - actStart;
  if (t >= actDuration) { home(); Serial.println(F("> 끝, 차렷")); return; }
  GaitParams p = scaled(curAct);
  float out[SERVO_COUNT];
  gaitAngles(p, t, periodMs, out);
  for (int i = 0; i < SERVO_COUNT; i++) writeServo(i, out[i]);
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
  Serial.print(F("주기 ")); Serial.print(periodMs); Serial.print(F("ms, 진폭 ")); Serial.print(ampPct); Serial.println('%');
}

// ───── 명령 처리 ─────
char buf[24];
uint8_t len = 0;
uint32_t lastChar = 0;

void handle(char* line) {
  char* cmd = strtok(line, " ");
  if (!cmd) return;
  char* a1 = strtok(NULL, " ");
  char* a2 = strtok(NULL, " ");
  int n = a1 ? atoi(a1) : 4;

  if      (!strcmp(cmd, "h") || !strcmp(cmd, "x")) { home(); Serial.println(F("> 차렷")); }
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
  else if (!strcmp(cmd, "w")) startAction(ACT_WALK_FWD, n);
  else if (!strcmp(cmd, "b")) startAction(ACT_WALK_BACK, n);
  else if (!strcmp(cmd, "l")) startAction(ACT_TURN_LEFT, n);
  else if (!strcmp(cmd, "r")) startAction(ACT_TURN_RIGHT, n);
  else if (!strcmp(cmd, "u")) startAction(ACT_UPDOWN, n);
  else if (!strcmp(cmd, "s")) startAction(ACT_SWING, n);
  else if (!strcmp(cmd, "m")) startAction(ACT_MOONWALK_LEFT, n);
  else if (!strcmp(cmd, "k")) startAction(ACT_TIPTOE, n);
  else Serial.println(F("? 명령: h i t save show w b l r u s m k p a x off"));
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

void setup() {
  Serial.begin(9600);
  loadTrims();
  attachAll();
  for (int i = 0; i < SERVO_COUNT; i++) writeServo(i, 90);
  Serial.println(F("otto_nano_walk 준비. 'show'로 설정 확인, 'w'로 걷기"));
  show();
}

void loop() {
  readSerial();
  tickAction();
}
