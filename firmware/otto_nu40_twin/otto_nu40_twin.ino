/*
 * otto_nu40_twin — 오또 몸에 얹는 NU40 DK + IMU: 움직임 센서 + 오또 지갑(서명)
 *
 * 서보는 Nano가 움직이고, 이 보드는 서보를 건드리지 않는다. 하는 일은 세 가지:
 *   1. IMU로 몸 기울기(pitch/roll)와 도는 방향(yaw)을 재서 웹 쌍둥이 페이지(web/otto-pet.html)로 보낸다
 *   2. 오또 지갑: 처음 켤 때 보드 안에서 ed25519 개인키를 만들어 내부 플래시에 저장한다.
 *      개인키는 밖으로 내보내지 않는다. 공개키 = 솔라나 지갑 주소 = 오또 신분증(DID).
 *      웹이 보낸 트랜잭션 메시지에 서명만 해서 돌려준다 (토큰 발행할 때 오또가 직접 서명).
 *   3. 보드 버튼(BUTTON1~4)을 누르면 "신호"를 보낸다 → 웹이 그 순간까지의 움직임을 토큰으로 발행
 *
 * 연결: USB 시리얼(115200) 또는 BLE(Nordic UART, 이름 "Otto-Twin-XXXX" = 지갑 공개키 앞 4자리). 둘 다 같은 글자 명령.
 *       여러 대를 한 웹에 띄울 때 이 이름과 지갑 주소로 구분한다.
 *
 * 보내는 줄
 *   #I <공개키 hex 64자> <IMU 종류>      켤 때, 'id' 명령 때
 *   #T <ms> <pitch> <roll> <yaw> <자이로크기> <가속도크기 mg>   USB 25Hz, BLE 10Hz (각도는 도)
 *   #K <버튼번호>                          버튼 눌림 = 발행 신호
 *   #G <서명 hex 128자>                    sign 결과
 *   #E <내용>                              오류
 * 받는 명령
 *   id                  공개키 다시 보내기
 *   sign <메시지 hex>   ed25519 서명 (최대 1232바이트)
 *   zero                지금 방향을 yaw 0으로
 *
 * 주의: devnet(시험망)용이다. NU40에는 보안 칩이 없어서 보드를 가져가면 키를 꺼낼 수 있다.
 *       실제 돈이 있는 지갑으로 쓰지 말 것.
 */
#include <bluefruit.h>
#include <Wire.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <Ed25519.h>
#include "imu.h"

using namespace Adafruit_LittleFS_Namespace;

static char           bleName[20] = "Otto-Twin";
static const char*    KEY_FILE  = "/otto_key";
static const uint8_t  BUTTONS[] = { PIN_BUTTON1, PIN_BUTTON2, PIN_BUTTON3, PIN_BUTTON4 };
static const uint16_t MAX_MSG   = 1232;

BLEUart bleuart(2048);
bool     bleOn = false;
uint8_t  priv[32], pub[32];
bool     imuOk = false;
Tilt     tilt;
float    yaw = 0, gzBias = 0, gMag = 0, aMag = 1;

// ───── 키 ─────
static void randomBytes(uint8_t* out, uint8_t n) {
  // SoftDevice가 켜진 뒤에는 하드웨어 난수기를 SoftDevice를 통해 쓴다
  uint8_t got = 0;
  while (got < n) {
    uint8_t avail = 0;
    sd_rand_application_bytes_available_get(&avail);
    if (!avail) { delay(1); continue; }
    uint8_t take = min<uint8_t>(avail, n - got);
    if (sd_rand_application_vector_get(out + got, take) == NRF_SUCCESS) got += take;
  }
}

static void loadOrMakeKey() {
  InternalFS.begin();
  File f(InternalFS);
  if (f.open(KEY_FILE, FILE_O_READ) && f.read(priv, 32) == 32) {
    f.close();
  } else {
    if (f) f.close();
    randomBytes(priv, 32);
    InternalFS.remove(KEY_FILE);
    if (f.open(KEY_FILE, FILE_O_WRITE)) { f.write(priv, 32); f.close(); }
  }
  Ed25519::derivePublicKey(pub, priv);
}

// ───── 출력: USB와 BLE에 같은 줄 ─────
static void out(const char* s, bool toBle = true) {
  Serial.println(s);
  if (toBle && bleOn) { bleuart.print(s); bleuart.print('\n'); }
}
static void hexStr(char* dst, const uint8_t* b, uint16_t n) {
  static const char H[] = "0123456789abcdef";
  for (uint16_t i = 0; i < n; i++) { dst[i * 2] = H[b[i] >> 4]; dst[i * 2 + 1] = H[b[i] & 15]; }
  dst[n * 2] = 0;
}
static int hexVal(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

static void sendId() {
  char line[100], h[65];
  hexStr(h, pub, 32);
  snprintf(line, sizeof(line), "#I %s %s", h, g_imuType == IMU_LSM6DS ? "LSM6DS" : g_imuType == IMU_MPU ? "MPU6050" : "none");
  out(line);
}

// ───── 명령 ─────
static uint8_t msgBuf[MAX_MSG];
static void handleLine(char* s) {
  while (*s == ' ') s++;
  if (!strcmp(s, "id")) { sendId(); return; }
  if (!strcmp(s, "zero")) { yaw = 0; out("#E yaw 0"); return; }
  if (!strncmp(s, "sign ", 5)) {
    const char* h = s + 5;
    size_t L = strlen(h);
    if (L % 2 || L / 2 > MAX_MSG || L == 0) { out("#E sign 길이 오류"); return; }
    for (size_t i = 0; i < L / 2; i++) {
      int a = hexVal(h[i * 2]), b = hexVal(h[i * 2 + 1]);
      if (a < 0 || b < 0) { out("#E sign hex 오류"); return; }
      msgBuf[i] = (a << 4) | b;
    }
    uint8_t sig[64];
    digitalWrite(LED_BLUE, HIGH);
    Ed25519::sign(sig, priv, pub, msgBuf, L / 2);
    digitalWrite(LED_BLUE, LOW);
    char line[140];
    strcpy(line, "#G ");
    hexStr(line + 3, sig, 64);
    out(line);
    return;
  }
  if (*s) out("#E 모르는 명령");
}

// USB · BLE 각각 줄 모으기 (서명할 메시지가 길어서 넉넉히)
struct LineIn {
  char buf[MAX_MSG * 2 + 16];
  uint16_t n = 0;
  void feed(char c) {
    if (c == '\r') return;
    if (c == '\n') { buf[n] = 0; handleLine(buf); n = 0; return; }
    if (n < sizeof(buf) - 1) buf[n++] = c;
  }
};
LineIn usbIn, bleIn;

void onConnect(uint16_t) { bleOn = true; }
void onDisconnect(uint16_t, uint8_t) { bleOn = false; }

void setup() {
  Serial.begin(115200);
  pinMode(LED_RED, OUTPUT); pinMode(LED_BLUE, OUTPUT);
  for (uint8_t b : BUTTONS) pinMode(b, INPUT_PULLUP);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  loadOrMakeKey();   // 난수는 SoftDevice가 켜진 뒤에 쓸 수 있다
  snprintf(bleName, sizeof(bleName), "Otto-Twin-%02X%02X", pub[0], pub[1]);
  Bluefruit.setName(bleName);
  Bluefruit.Periph.setConnectCallback(onConnect);
  Bluefruit.Periph.setDisconnectCallback(onDisconnect);
  bleuart.begin();
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.start(0);


  Wire.begin();
  Wire.setClock(400000);
  imuOk = imuBegin();
  // 자이로 영점: 켜고 1초 가만히 있는 동안 평균
  if (imuOk) {
    float sum = 0; int n = 0;
    uint32_t t0 = millis();
    while (millis() - t0 < 1000) { ImuSample s; if (imuRead(s)) { sum += s.gz; n++; } delay(5); }
    if (n) gzBias = sum / n;
  }
  sendId();
}

void loop() {
  static uint32_t tImu = 0, tUsb = 0, tBle = 0, lastUs = 0;
  static uint8_t btnPrev = 0xFF;
  uint32_t now = millis();

  // IMU 100Hz
  if (imuOk && now - tImu >= 10) {
    tImu = now;
    ImuSample s;
    if (imuRead(s)) {
      uint32_t us = micros();
      float dt = lastUs ? (us - lastUs) * 1e-6f : 0.01f;
      lastUs = us;
      tilt.update(s, dt);
      yaw += (s.gz - gzBias) * dt;
      if (yaw > 180) yaw -= 360; else if (yaw < -180) yaw += 360;
      gMag = sqrtf(s.gx * s.gx + s.gy * s.gy + s.gz * s.gz);
      aMag = sqrtf(s.ax * s.ax + s.ay * s.ay + s.az * s.az);
    }
  }

  // 움직임 줄: USB 25Hz, BLE 10Hz
  bool usbDue = now - tUsb >= 40, bleDue = bleOn && now - tBle >= 100;
  if (imuOk && (usbDue || bleDue)) {
    char line[80];
    snprintf(line, sizeof(line), "#T %lu %.1f %.1f %.1f %d %d", (unsigned long)now, tilt.pitch, tilt.roll, yaw, (int)gMag, (int)(aMag * 1000));
    if (usbDue) { tUsb = now; Serial.println(line); }
    if (bleDue) { tBle = now; bleuart.print(line); bleuart.print('\n'); }
  }

  // 버튼 = 발행 신호 (눌리는 순간 한 번)
  uint8_t btn = 0;
  for (uint8_t i = 0; i < 4; i++) if (digitalRead(BUTTONS[i]) == LOW) btn |= 1 << i;
  static uint32_t tBtn = 0;
  if (btn != btnPrev && now - tBtn > 30) {
    for (uint8_t i = 0; i < 4; i++)
      if ((btn & (1 << i)) && !(btnPrev & (1 << i))) {
        char line[8]; snprintf(line, sizeof(line), "#K %d", i + 1); out(line);
        digitalWrite(LED_RED, HIGH); delay(40); digitalWrite(LED_RED, LOW);
      }
    btnPrev = btn; tBtn = now;
  }

  while (Serial.available()) usbIn.feed((char)Serial.read());
  while (bleuart.available()) bleIn.feed((char)bleuart.read());
}
