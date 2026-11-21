// extras.h — 오또 키트의 도트(8x8, MAX7219) · 부저 · 초음파(HC-SR04)
//
// 핀은 docs/otto-조립-지도.html 의 "핀 지도"와 같다.
//   도트  DIN D12, CLK D11, CS D10   (라이브러리 없이 직접 신호를 만든다)
//   부저  A0                         (능동 부저면 소리 높낮이 없이 "삐"만 남)
//   초음파 Trig D8, Echo D9
//
// NU40로 옮길 때는 이 파일의 핀 번호만 바꾸면 된다. 단 NU40은 3.3V라서
// 초음파 Echo(5V)는 저항 분압(예: 1kΩ/2kΩ)으로 낮춰서 넣어야 한다.
#pragma once
#include <Arduino.h>
#include <avr/pgmspace.h>

static const uint8_t DOT_DIN = 12, DOT_CLK = 11, DOT_CS = 10;
static const uint8_t BUZZER  = A0;
static const uint8_t US_TRIG = 8, US_ECHO = 9;

// ───── 도트 ─────
enum Face : uint8_t { F_OFF, F_EYES, F_BLINK, F_SMILE, F_HAPPY, F_SAD, F_SURPRISE, F_HEART,
                      F_UP, F_DOWN, F_LEFT, F_RIGHT, F_X, F_ALL, F_COUNT };
static const char* const FACE_NAMES[F_COUNT] = {
  "off", "eyes", "blink", "smile", "happy", "sad", "surprise", "heart",
  "up", "down", "left", "right", "x", "all" };

// 위 줄부터 아래 줄. 비트 7 = 왼쪽 칸
static const uint8_t FACES[F_COUNT][8] PROGMEM = {
  {0,0,0,0,0,0,0,0},
  {0x00,0x66,0x66,0x66,0x00,0x00,0x3C,0x00},  // 눈 뜸
  {0x00,0x00,0x00,0x66,0x00,0x00,0x3C,0x00},  // 눈 감음
  {0x00,0x66,0x66,0x00,0x00,0x42,0x3C,0x00},  // 웃음
  {0x00,0x66,0x99,0x00,0x00,0x7E,0x3C,0x00},  // 신남 (^^)
  {0x00,0x66,0x66,0x00,0x00,0x3C,0x42,0x00},  // 슬픔
  {0x00,0x66,0x66,0x00,0x18,0x24,0x24,0x18},  // 놀람
  {0x00,0x66,0xFF,0xFF,0x7E,0x3C,0x18,0x00},  // 하트
  {0x18,0x3C,0x7E,0xDB,0x18,0x18,0x18,0x18},  // ↑
  {0x18,0x18,0x18,0x18,0xDB,0x7E,0x3C,0x18},  // ↓
  {0x10,0x30,0x60,0xFF,0xFF,0x60,0x30,0x10},  // ←
  {0x08,0x0C,0x06,0xFF,0xFF,0x06,0x0C,0x08},  // →
  {0x81,0x42,0x24,0x18,0x18,0x24,0x42,0x81},  // X
  {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF},
};

uint8_t dotRot = 0;      // 0~3: 도트 모듈을 돌려 붙였을 때 90°씩 돌림
bool    dotFlip = false; // 좌우 거울
Face    curFace = F_OFF;

static void dotWrite(uint8_t reg, uint8_t val) {
  digitalWrite(DOT_CS, LOW);
  shiftOut(DOT_DIN, DOT_CLK, MSBFIRST, reg);
  shiftOut(DOT_DIN, DOT_CLK, MSBFIRST, val);
  digitalWrite(DOT_CS, HIGH);
}

static void dotInit(uint8_t bright = 3) {
  pinMode(DOT_DIN, OUTPUT); pinMode(DOT_CLK, OUTPUT); pinMode(DOT_CS, OUTPUT);
  digitalWrite(DOT_CS, HIGH);
  dotWrite(0x0F, 0);       // 테스트 모드 끔
  dotWrite(0x0C, 1);       // 켜기
  dotWrite(0x0B, 7);       // 8줄 모두
  dotWrite(0x09, 0);       // 숫자 해석 안 함
  dotWrite(0x0A, bright);  // 밝기 0~15 (전류를 아끼려고 낮게)
}

static void dotShow(const uint8_t rows[8]) {
  uint8_t out[8] = {0};
  for (uint8_t r = 0; r < 8; r++)
    for (uint8_t c = 0; c < 8; c++) {
      bool on = rows[r] & (0x80 >> c);
      uint8_t cc = dotFlip ? 7 - c : c, rr = r;
      for (uint8_t k = 0; k < dotRot; k++) { uint8_t t = rr; rr = cc; cc = 7 - t; }
      if (on) out[rr] |= (0x80 >> cc);
    }
  for (uint8_t r = 0; r < 8; r++) dotWrite(r + 1, out[r]);
}

static void setFace(Face f) {
  if (f >= F_COUNT) return;
  uint8_t rows[8];
  memcpy_P(rows, FACES[f], 8);
  dotShow(rows);
  curFace = f;
}

static int faceByName(const char* s) {
  for (uint8_t i = 0; i < F_COUNT; i++) if (!strcmp(s, FACE_NAMES[i])) return i;
  return -1;
}

// 가만히 있을 때 3~5초마다 눈 깜빡임
uint32_t nextBlink = 0;
bool     idleBlink = true;
static void tickBlink(bool idle) {
  if (!idle || !idleBlink || (curFace != F_EYES && curFace != F_BLINK)) return;
  uint32_t now = millis();
  if (curFace == F_BLINK && now >= nextBlink) { setFace(F_EYES); nextBlink = now + 3000 + random(2000); }
  else if (curFace == F_EYES && now >= nextBlink) { setFace(F_BLINK); nextBlink = now + 150; }
}

// ───── 부저 ─────
// 소리 = (주파수 Hz, 길이 ms) 목록. 0 Hz = 쉼
struct Note { uint16_t hz; uint16_t ms; };
static const Note S_BEEP[]  PROGMEM = {{1760,120}};
static const Note S_HELLO[] PROGMEM = {{1047,90},{1319,90},{1568,140}};
static const Note S_HAPPY[] PROGMEM = {{1319,100},{1568,100},{2093,100},{0,40},{2093,160}};
static const Note S_SAD[]   PROGMEM = {{784,200},{698,200},{587,350}};
static const Note S_ALARM[] PROGMEM = {{2200,120},{0,60},{2200,120},{0,60},{2200,120}};
static const Note S_OK[]    PROGMEM = {{1568,80},{2093,140}};
// 짧은 동요 한 소절 (학교종: 솔솔라라 솔솔미)
static const Note S_SONG[]  PROGMEM = {{784,250},{784,250},{880,250},{880,250},{784,250},{784,250},{659,500},
                                       {0,100},{784,250},{784,250},{659,250},{659,250},{587,700}};
struct Sound { const char* name; const Note* notes; uint8_t n; };
#define SND(nm, arr) { nm, arr, (uint8_t)(sizeof(arr) / sizeof(arr[0])) }
static const Sound SOUNDS[] = {
  SND("beep", S_BEEP), SND("hello", S_HELLO), SND("happy", S_HAPPY), SND("sad", S_SAD),
  SND("alarm", S_ALARM), SND("ok", S_OK), SND("song", S_SONG) };
static const uint8_t SOUND_COUNT = sizeof(SOUNDS) / sizeof(SOUNDS[0]);

bool        soundOn = true;      // 'mute'로 끔
const Note* sndNotes = nullptr;
uint8_t     sndLen = 0, sndIdx = 0;
uint32_t    sndNext = 0;

static void playSound(uint8_t i) {
  if (!soundOn || i >= SOUND_COUNT) return;
  sndNotes = SOUNDS[i].notes; sndLen = SOUNDS[i].n; sndIdx = 0; sndNext = 0;
}
static int soundByName(const char* s) {
  for (uint8_t i = 0; i < SOUND_COUNT; i++) if (!strcmp(s, SOUNDS[i].name)) return i;
  return -1;
}
// 멈추지 않고(loop를 막지 않고) 음을 하나씩 낸다
static void tickSound() {
  if (!sndNotes) return;
  uint32_t now = millis();
  if (now < sndNext) return;
  if (sndIdx >= sndLen) { noTone(BUZZER); digitalWrite(BUZZER, LOW); sndNotes = nullptr; return; }
  Note nt; memcpy_P(&nt, &sndNotes[sndIdx++], sizeof(Note));
  if (nt.hz) tone(BUZZER, nt.hz, nt.ms); else noTone(BUZZER);
  sndNext = now + nt.ms + 20;
}

// ───── 초음파 ─────
// 거리(cm). 30ms 안에 반사가 없으면(약 5m 이상이거나 선이 빠짐) 0
static uint16_t readCm() {
  digitalWrite(US_TRIG, LOW); delayMicroseconds(2);
  digitalWrite(US_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);
  unsigned long us = pulseIn(US_ECHO, HIGH, 30000UL);
  return us ? (uint16_t)(us / 58) : 0;
}

static void extrasInit() {
  pinMode(US_TRIG, OUTPUT); pinMode(US_ECHO, INPUT);
  pinMode(BUZZER, OUTPUT);
  dotInit();
  randomSeed(analogRead(A7));
}
