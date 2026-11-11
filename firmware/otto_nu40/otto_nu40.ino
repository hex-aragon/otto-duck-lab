/*
 * otto_nu40 — 누코드 NU40 DK(nRF52840)로 움직이는 오또봇 펌웨어
 *
 * 하는 일
 *   1. 서보 4개(왼다리 · 오른다리 · 왼발 · 오른발)를 사인파 조합으로 움직인다 (gait.h)
 *   2. BLE(Nordic UART)로 웹/폰/에어밴드 스틱 중계에서 명령을 받는다
 *   3. IMU로 기울기를 재고, 1초 창의 특징으로 상태(서 있음/걷기/넘어짐/들림)를 분류한다 (posture.h)
 *      → 넘어지거나 들리면 즉시 멈추고 서보 힘을 뺀다 (Open Duck Mini의 안전 동작에 해당)
 *   4. 텔레메트리 · 특징값을 BLE로 보내 웹에서 TinyML 학습 데이터를 모은다
 *
 * 흐름
 *   loop() ─┬─ 100Hz  IMU 읽기 → 기울기(상보 필터)
 *           ├─ 25Hz   특징 창에 넣기 → 200ms마다 상태 분류 → 안전 처리
 *           ├─ 50Hz   동작 중이면 서보 목표 각도 갱신
 *           ├─ 10Hz   텔레메트리 notify
 *           └─ 수시   BLE / 시리얼 명령 처리
 *   loop 안에 delay()를 쓰지 않는다. 모든 주기는 millis()/micros() 경과시간으로 맞춘다.
 *
 * 하드웨어 사실 중 확인하지 못한 것은 "TODO: 확인 필요"로 표시했다. 추측으로 채우지 말 것.
 */
#include <bluefruit.h>
#include <Wire.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include "gait.h"
#include "imu.h"
#include "posture.h"

using namespace Adafruit_LittleFS_Namespace;

// ─────────────────────────────────────────────
// 핀 설정
// ─────────────────────────────────────────────
// 이 BSP는 아두이노 핀 번호 = nRF GPIO 번호 (P0.xx → xx, P1.xx → 32+xx).
// 버튼(11,12,24,25) · LED(13~16) · I2C(26,27)와 겹치지 않는 핀으로 골랐다.
// TODO: 확인 필요 — P0.02~P0.05(variant.h의 A0~A3)가 NU40 DK 헤더 어디에 나와 있는지 실크/핀맵으로 확인.
static const uint8_t SERVO_PINS[SERVO_COUNT] = {
  2,  // LEFT_LEG   (왼쪽 고관절, 좌우 회전)
  3,  // RIGHT_LEG
  4,  // LEFT_FOOT  (왼쪽 발목, 발 기울임)
  5,  // RIGHT_FOOT
};
// 서보를 거울처럼 반대로 붙였으면 true. Otto 표준 조립이면 모두 false.
static const bool SERVO_REVERSE[SERVO_COUNT] = { false, false, false, false };  // TODO: 확인 필요

static const uint8_t PIN_STATUS_LED = LED_BUILTIN;

// ─────────────────────────────────────────────
// 동작 · 안전 설정
// ─────────────────────────────────────────────
static const uint16_t DEFAULT_PERIOD_MS   = 1000;  // 걸음 한 주기
static const uint16_t CMD_TIMEOUT_MS      = 1000;  // 연속 동작(steps=0) 중 명령이 이만큼 끊기면 멈춤
static const uint8_t  FALL_CONFIRM_COUNT  = 2;     // 넘어짐/들림이 연속 이만큼(200ms 간격) 나오면 확정
static const float    MAX_TRIM_DEG        = 30;    // 트림 허용 범위 ±
static const char*    TRIM_FILE           = "/otto_trims";
static const char*    BLE_NAME            = "Otto-NU";

// 디버그 플래그
#define DEBUG_LOG 1   // 시리얼에 상태 로그
#define DEBUG_RAW 0   // 1이면 시리얼에 ax,ay,az,gx,gy,gz CSV (25Hz) — Edge Impulse Data Forwarder용

// ─────────────────────────────────────────────
// BLE 패킷 (little-endian). 자세한 표는 docs/03-소프트웨어-구조.md
// ─────────────────────────────────────────────
enum : uint8_t {
  CMD_ACTION = 0x01,  // [1]action [2]steps(0=계속) [3]period/10ms(0=기본)
  CMD_STOP   = 0x02,
  CMD_TRIM   = 0x03,  // [1]servo [2]int8 트림(도)
  CMD_SAVE   = 0x04,  // 트림 저장
  CMD_STREAM = 0x05,  // [1]bit0 텔레메트리 · bit1 특징값 · bit2 원시 IMU
  CMD_RELAX  = 0x06,  // 서보 힘 빼기
  CMD_PING   = 0x07,  // 연속 동작 유지용 (명령 끊김 감지 리셋)
};
enum : uint8_t {
  PKT_TELEMETRY = 0x90,  // 12바이트
  PKT_FEATURES  = 0x92,  // 1 + 8×int16(×1000) = 17바이트
  PKT_RAW       = 0x93,  // 1 + 6×int16 = 13바이트 (가속 mg, 자이로 °/s×10)
  PKT_EVENT     = 0x94,  // [1]event [2]값
  PKT_TRIMS     = 0x95,  // [1..4] int8 트림
};
enum : uint8_t { EVT_FALLEN = 1, EVT_LIFTED = 2, EVT_UPRIGHT = 3, EVT_DONE = 4, EVT_TIMEOUT = 5 };
enum RobotState : uint8_t { ST_IDLE = 0, ST_MOVING = 1, ST_FALLEN = 2, ST_LIFTED = 3, ST_RELAXED = 4 };

// ─────────────────────────────────────────────
// 전역 상태
// ─────────────────────────────────────────────
BLEUart  bleuart;
HardwarePWM* g_pwm = nullptr;  // 서보 4개가 같이 쓰는 하드웨어 PWM 모듈 (nRF52840에는 4개, 모듈당 4채널)
bool     g_attached = false;
int8_t   g_trim[SERVO_COUNT] = { 0, 0, 0, 0 };

RobotState g_state = ST_IDLE;
Action     g_action = ACT_HOME;
GaitParams g_params;
uint32_t   g_actionStart = 0, g_period = DEFAULT_PERIOD_MS, g_lastCmd = 0;
uint8_t    g_steps = 0;  // 0 = 명령이 계속 오는 동안 반복

bool      g_imuOk = false;
Tilt      g_tilt;
ImuSample g_last;
FeatureWindow g_window;
uint8_t   g_posture = POSTURE_STANDING, g_badCount = 0;
float     g_postureConf = 0;
uint8_t   g_streamMask = 0b001;
bool      g_connected = false;

// ─────────────────────────────────────────────
// 서보
// ─────────────────────────────────────────────
// [설명] 서보 신호 = 20ms(50Hz)마다 0.5~2.4ms 길이의 펄스. 펄스 길이가 각도다.
// 아두이노 Servo 라이브러리 대신 nRF52 하드웨어 PWM을 직접 쓴다.
//   - PC에 일반 Servo 라이브러리가 깔려 있으면 BSP 것 대신 잡혀 컴파일이 깨지는 문제를 피한다
//   - 1MHz 클럭 → 1µs 단위로 펄스를 정할 수 있다 (BSP Servo는 8µs)
// tone()은 HwPWM2를 쓰므로 부저를 쓰려면 서보가 다른 모듈을 잡아야 한다 → 빈 모듈을 찾아 쓴다.
static const uint32_t PWM_TOKEN      = 0x4F74746F;  // 'Otto' — PWM 모듈 소유 표시
static const uint16_t SERVO_MIN_US   = 500;   // 0°   TODO: 확인 필요 — 쓰는 서보(SG90 등) 규격으로 맞출 것
static const uint16_t SERVO_MAX_US   = 2400;  // 180°
static const uint16_t SERVO_FRAME_US = 20000; // 50Hz

static bool servosInit() {
  for (int m = 0; m < HWPWM_MODULE_NUM && !g_pwm; m++) {
    if (HwPWMx[m]->takeOwnership(PWM_TOKEN)) g_pwm = HwPWMx[m];
  }
  if (!g_pwm) return false;
  g_pwm->setClockDiv(PWM_PRESCALER_PRESCALER_DIV_16);  // 16MHz / 16 = 1MHz
  g_pwm->setMaxValue(SERVO_FRAME_US);
  for (int i = 0; i < SERVO_COUNT; i++) {
    pinMode(SERVO_PINS[i], OUTPUT);
    digitalWrite(SERVO_PINS[i], LOW);
    if (!g_pwm->addPin(SERVO_PINS[i])) return false;
    g_pwm->writePin(SERVO_PINS[i], 0);  // 0 = 펄스 없음 = 힘 빠짐
  }
  return true;
}

// 힘 주기: 다음 servoWrite부터 펄스가 나간다
static void servosAttach() { g_attached = (g_pwm != nullptr); }

// 힘 빼기: 펄스를 끊으면 아날로그 서보(SG90)는 위치 유지를 멈춘다
static void servosDetach() {
  if (g_pwm) for (int i = 0; i < SERVO_COUNT; i++) g_pwm->writePin(SERVO_PINS[i], 0);
  g_attached = false;
}

// angle: 90° 중심 목표 각도. 트림 · 방향 · 범위 제한을 여기서 한 번에 처리한다
static void servoWrite(int i, float angle) {
  float a = angle + g_trim[i];
  if (SERVO_REVERSE[i]) a = 180 - a;
  if (a < 0) a = 0;
  if (a > 180) a = 180;
  if (!g_attached) return;
  uint16_t us = SERVO_MIN_US + (uint16_t)((SERVO_MAX_US - SERVO_MIN_US) * a / 180.0f + 0.5f);
  g_pwm->writePin(SERVO_PINS[i], us);
}

static void writeHome() {
  for (int i = 0; i < SERVO_COUNT; i++) servoWrite(i, 90);
}

// ─────────────────────────────────────────────
// 트림 저장 (내부 플래시)
// ─────────────────────────────────────────────
static void loadTrims() {
  File f(InternalFS);
  if (f.open(TRIM_FILE, FILE_O_READ)) {
    if (f.read(g_trim, sizeof(g_trim)) != sizeof(g_trim)) memset(g_trim, 0, sizeof(g_trim));
    f.close();
  }
}

static void saveTrims() {
  InternalFS.remove(TRIM_FILE);
  File f(InternalFS);
  if (f.open(TRIM_FILE, FILE_O_WRITE)) {
    f.write((const uint8_t*)g_trim, sizeof(g_trim));
    f.close();
  }
}

// ─────────────────────────────────────────────
// BLE 전송
// ─────────────────────────────────────────────
static void send(const uint8_t* buf, uint16_t n) {
  if (g_connected) bleuart.write(buf, n);
}

static void sendEvent(uint8_t evt, uint8_t val = 0) {
  uint8_t p[3] = { PKT_EVENT, evt, val };
  send(p, sizeof(p));
#if DEBUG_LOG
  Serial.printf("[event] %u %u\n", evt, val);
#endif
}

static void sendTrims() {
  uint8_t p[1 + SERVO_COUNT] = { PKT_TRIMS };
  memcpy(p + 1, g_trim, SERVO_COUNT);
  send(p, sizeof(p));
}

static void put16(uint8_t* p, int16_t v) { p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF; }
static int16_t clamp16(float v) { return (int16_t)(v > 32767 ? 32767 : (v < -32768 ? -32768 : v)); }

static void sendTelemetry() {
  uint8_t p[12] = { PKT_TELEMETRY, (uint8_t)g_state, (uint8_t)g_action, g_posture };
  put16(p + 4, clamp16(g_tilt.pitch * 100));
  put16(p + 6, clamp16(g_tilt.roll * 100));
  float mag = sqrtf(g_last.ax * g_last.ax + g_last.ay * g_last.ay + g_last.az * g_last.az);
  put16(p + 8, clamp16(mag * 1000));
  p[10] = (uint8_t)(g_postureConf * 255);
  p[11] = (g_imuOk ? 1 : 0) | (POSTURE_TINYML ? 2 : 0) | (g_attached ? 4 : 0);
  send(p, sizeof(p));
}

// ─────────────────────────────────────────────
// 동작 시작 · 정지
// ─────────────────────────────────────────────
static void startAction(Action a, uint8_t steps, uint16_t period) {
  if (a >= ACT_COUNT) return;
  // 넘어지거나 들린 상태에서는 움직이지 않는다 (다시 세워 놓으면 자동으로 풀림)
  if (g_state == ST_FALLEN || g_state == ST_LIFTED) { sendEvent(g_state == ST_FALLEN ? EVT_FALLEN : EVT_LIFTED); return; }
  servosAttach();
  g_action = a;
  g_params = gaitFor(a);
  g_steps = steps;
  g_period = period ? period : DEFAULT_PERIOD_MS;
  g_actionStart = millis();
  g_lastCmd = g_actionStart;
  g_state = (a == ACT_HOME) ? ST_IDLE : ST_MOVING;
  if (a == ACT_HOME) writeHome();
}

static void stopAction(uint8_t evt = EVT_DONE) {
  g_action = ACT_HOME;
  if (g_attached) writeHome();
  if (g_state == ST_MOVING) { g_state = ST_IDLE; sendEvent(evt); }
}

static void relax(RobotState st) {
  g_action = ACT_HOME;
  servosDetach();
  g_state = st;
}

static void updateMotion(uint32_t now) {
  if (g_state != ST_MOVING) return;
  uint32_t t = now - g_actionStart;
  if (g_steps > 0 && t >= (uint32_t)g_steps * g_period) { stopAction(EVT_DONE); return; }
  if (g_steps == 0 && now - g_lastCmd > CMD_TIMEOUT_MS) { stopAction(EVT_TIMEOUT); return; }
  float ang[SERVO_COUNT];
  gaitAngles(g_params, t, g_period, ang);
  for (int i = 0; i < SERVO_COUNT; i++) servoWrite(i, ang[i]);
}

// ─────────────────────────────────────────────
// 상태 분류 → 안전 처리
// ─────────────────────────────────────────────
static void updatePosture() {
  if (!g_window.full()) return;
  float f[FEATURE_COUNT];
  g_window.features(f);
  g_posture = classifyPosture(f, g_postureConf);

  if (g_streamMask & 0b010) {
    uint8_t p[1 + FEATURE_COUNT * 2] = { PKT_FEATURES };
    for (int i = 0; i < FEATURE_COUNT; i++) put16(p + 1 + i * 2, clamp16(f[i] * 1000));
    send(p, sizeof(p));
  }

  bool fallen = postureIsFallen(g_posture);
  bool lifted = (g_posture == POSTURE_LIFTED);
  if (fallen || lifted) {
    if (g_badCount < 255) g_badCount++;
    if (g_badCount == FALL_CONFIRM_COUNT && g_state != ST_FALLEN && g_state != ST_LIFTED) {
      relax(fallen ? ST_FALLEN : ST_LIFTED);  // 넘어진 채 다리를 계속 움직이면 서보 · 기어가 상한다
      sendEvent(fallen ? EVT_FALLEN : EVT_LIFTED, g_posture);
    }
  } else {
    g_badCount = 0;
    if (g_state == ST_FALLEN || g_state == ST_LIFTED) { g_state = ST_RELAXED; sendEvent(EVT_UPRIGHT); }
  }
}

// ─────────────────────────────────────────────
// 명령 처리 (BLE · 시리얼 공통)
// ─────────────────────────────────────────────
// 한 글자 명령 (시리얼 모니터 · 간단한 BLE 앱용)
//   w 앞 · s 뒤 · a 왼쪽 · d 오른쪽 (각 4걸음) · x 정지 · h 차렷 · r 힘 빼기
//   1 위아래 · 2 흔들기 · 3 문워크 · 4 까치발 · t 트림 출력
static void handleChar(char c) {
  switch (c) {
    case 'w': startAction(ACT_WALK_FWD, 4, 0); break;
    case 's': startAction(ACT_WALK_BACK, 4, 0); break;
    case 'a': startAction(ACT_TURN_LEFT, 4, 0); break;
    case 'd': startAction(ACT_TURN_RIGHT, 4, 0); break;
    case '1': startAction(ACT_UPDOWN, 4, 0); break;
    case '2': startAction(ACT_SWING, 4, 0); break;
    case '3': startAction(ACT_MOONWALK_LEFT, 4, 0); break;
    case '4': startAction(ACT_TIPTOE, 4, 0); break;
    case 'x': stopAction(); break;
    case 'h': startAction(ACT_HOME, 0, 0); break;
    case 'r': relax(ST_RELAXED); break;
    case 't': Serial.printf("trims %d %d %d %d\n", g_trim[0], g_trim[1], g_trim[2], g_trim[3]); sendTrims(); break;
    default: break;
  }
}

static void handlePacket(const uint8_t* p, uint16_t n) {
  if (n == 0) return;
  g_lastCmd = millis();
  if (n == 1 && p[0] >= 0x20) { handleChar((char)p[0]); return; }
  switch (p[0]) {
    case CMD_ACTION:
      if (n >= 4) {
        // 같은 연속 동작을 계속 보내면 처음부터 다시 시작하지 않고 유지만 한다 (걸음이 끊기지 않게)
        if (g_state == ST_MOVING && p[2] == 0 && g_steps == 0 && p[1] == g_action) break;
        startAction((Action)p[1], p[2], (uint16_t)p[3] * 10);
      }
      break;
    case CMD_STOP:  stopAction(); break;
    case CMD_TRIM:
      if (n >= 3 && p[1] < SERVO_COUNT) {
        int8_t v = (int8_t)p[2];
        if (v > MAX_TRIM_DEG) v = MAX_TRIM_DEG;
        if (v < -MAX_TRIM_DEG) v = -MAX_TRIM_DEG;
        g_trim[p[1]] = v;
        servosAttach();
        if (g_state != ST_MOVING) writeHome();  // 트림은 차렷 자세에서 눈으로 맞춘다
        sendTrims();
      }
      break;
    case CMD_SAVE:   saveTrims(); sendTrims(); break;
    case CMD_STREAM: if (n >= 2) g_streamMask = p[1]; break;
    case CMD_RELAX:  relax(ST_RELAXED); break;
    case CMD_PING:   break;  // g_lastCmd 갱신만
    default: break;
  }
}

// ─────────────────────────────────────────────
// BLE 콜백
// ─────────────────────────────────────────────
static void onConnect(uint16_t) {
  g_connected = true;
  digitalWrite(PIN_STATUS_LED, HIGH);
#if DEBUG_LOG
  Serial.println("[ble] connected");
#endif
}

static void onDisconnect(uint16_t, uint8_t) {
  g_connected = false;
  digitalWrite(PIN_STATUS_LED, LOW);
  stopAction(EVT_TIMEOUT);  // 연결이 끊기면 바로 멈춘다
#if DEBUG_LOG
  Serial.println("[ble] disconnected");
#endif
}

static void bleBegin() {
  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.setName(BLE_NAME);
  Bluefruit.Periph.setConnectCallback(onConnect);
  Bluefruit.Periph.setDisconnectCallback(onDisconnect);
  Bluefruit.Periph.setConnInterval(6, 12);  // 7.5~15ms 요청
  bleuart.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
}

// ─────────────────────────────────────────────
void setup() {
  pinMode(PIN_STATUS_LED, OUTPUT);
  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 1500) {}  // USB 시리얼을 잠깐만 기다린다 (배터리 구동이면 그냥 진행)

  InternalFS.begin();
  loadTrims();

  Wire.begin();
  Wire.setClock(400000);
  g_imuOk = imuBegin();
#if DEBUG_LOG
  Serial.printf("[imu] %s addr=0x%02X\n", g_imuOk ? (g_imuType == IMU_LSM6DS ? "LSM6DS" : "MPU") : "none", g_imuAddr);
  Serial.printf("[posture] %s\n", POSTURE_TINYML ? "TinyML model" : "rule");
#endif

  bool pwmOk = servosInit();
#if DEBUG_LOG
  Serial.printf("[servo] %s\n", pwmOk ? "ok" : "PWM 모듈을 잡지 못함");
#endif
  servosAttach();
  writeHome();
  bleBegin();
}

void loop() {
  static uint32_t tImu = 0, tWin = 0, tPost = 0, tServo = 0, tTele = 0;
  static uint8_t lineLen = 0;
  uint32_t now = millis();

  // 100Hz IMU
  if (g_imuOk && now - tImu >= 10) {
    float dt = (tImu == 0) ? 0.01f : (now - tImu) / 1000.0f;
    tImu = now;
    if (imuRead(g_last)) g_tilt.update(g_last, dt);
  }

  // 25Hz 특징 창
  if (g_imuOk && now - tWin >= 40) {
    tWin = now;
    g_window.push(g_last);
#if DEBUG_RAW
    Serial.printf("%.4f,%.4f,%.4f,%.2f,%.2f,%.2f\n", g_last.ax, g_last.ay, g_last.az, g_last.gx, g_last.gy, g_last.gz);
#endif
    if (g_streamMask & 0b100) {
      uint8_t p[13] = { PKT_RAW };
      put16(p + 1, clamp16(g_last.ax * 1000)); put16(p + 3, clamp16(g_last.ay * 1000)); put16(p + 5, clamp16(g_last.az * 1000));
      put16(p + 7, clamp16(g_last.gx * 10));   put16(p + 9, clamp16(g_last.gy * 10));   put16(p + 11, clamp16(g_last.gz * 10));
      send(p, sizeof(p));
    }
  }

  // 5Hz 상태 분류
  if (g_imuOk && now - tPost >= 200) { tPost = now; updatePosture(); }

  // 50Hz 서보
  if (now - tServo >= 20) { tServo = now; updateMotion(now); }

  // 10Hz 텔레메트리
  if ((g_streamMask & 0b001) && now - tTele >= 100) { tTele = now; sendTelemetry(); }

  // BLE 수신: 패킷 경계가 없으므로 한 번에 들어온 바이트를 한 명령으로 본다 (명령은 모두 20바이트 이하)
  if (bleuart.available()) {
    uint8_t buf[20];
    uint16_t n = 0;
    while (bleuart.available() && n < sizeof(buf)) buf[n++] = (uint8_t)bleuart.read();
    handlePacket(buf, n);
  }

  // 시리얼: 한 글자 명령
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') { lineLen = 0; continue; }
    if (lineLen++ == 0) handleChar(c);
  }
}
