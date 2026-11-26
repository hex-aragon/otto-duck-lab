// debug.h — 걷다가 재시작하는 원인을 찾기 위한 기록 (웹 디버그 페이지 web/otto-debug.html이 읽음)
//
// 보드가 내보내는 줄 (사람용 줄과 섞여도 '#'으로 시작해서 웹이 골라낸다):
//   #B boot=<켠 횟수> rst=<리셋 원인 16진> vcc=<mV>     켜질 때 한 번
//   #S <서보번호> <mV>                                   서보 하나 켤 때마다 (켜는 순간 전압)
//   #C <받은 명령>                                       명령을 받자마자 (블루투스로 잘 왔는지)
//   #D <ms> <지금 mV> <최저 mV> <동작> <루프 최대 ms>     가만히 1초마다, 움직일 땐 0.2초마다
//   #A <서보 4개 각도> <표정 번호>                        움직일 땐 0.1초마다 (otto_nano_walk.ino tickAngles, 쌍둥이 페이지용)
//
// 전압은 보드 5V(AVcc)를 내부 1.1V 기준으로 거꾸로 잰 값이다. 추가 배선이 필요 없다.
// 최저 mV가 4300 아래로 떨어진 뒤 #B가 다시 나오면 → 전원 부족으로 재시작한 것.
// rst 비트: 1=전원 켬, 4=저전압(BOD), 8=리셋 핀(버튼·USB 연결·업로드). 0이면 알 수 없음.
//   optiboot는 리셋 핀으로 켜지면 부트로더를 돌고 워치독으로 앱을 시작하므로 8(WDRF)로 보인다.
//   걷다가 재시작했는데 rst=4면 전원 부족, rst=8이면 리셋 핀에 잡음(선 흔들림·DTR)이 들어간 것.
#pragma once
#include <Arduino.h>
#include <EEPROM.h>

// optiboot 부트로더는 리셋 원인(MCUSR)을 r2 레지스터에 남겨 준다. C 코드가 시작되기 전에 건져 둔다.
uint8_t bootResetFlags __attribute__((section(".noinit")));
void grabResetFlags() __attribute__((naked, used, section(".init0")));
void grabResetFlags() { __asm__ __volatile__("sts bootResetFlags, r2\n"); }

static const uint8_t EE_DBG_ADDR = 7;    // 0xDB = 기록 끔 (기본은 켬)
static const uint8_t EE_BOOT_ADDR = 8;   // 8~9번지: 켠 횟수

bool     dbgOn = true;
uint16_t vccNow = 0, vccMin = 0xFFFF;
uint32_t nextDbg = 0, loopPrev = 0;
uint16_t loopMax = 0;

// 내부 1.1V 기준 전압을 AVcc 기준으로 재면 AVcc를 거꾸로 알 수 있다
uint16_t readVccRaw() {
  ADMUX = _BV(REFS0) | 0x0E;
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC)) {}
  uint16_t r = ADC;
  return r ? (uint16_t)(1125300UL / r) : 0;
}

// 자주 불러도 된다 (한 번에 0.1ms쯤). 최저값을 기억해 둔다.
void sampleVcc() {
  vccNow = readVccRaw();
  if (vccNow && vccNow < vccMin) vccMin = vccNow;
}

void debugBoot() {
  dbgOn = EEPROM.read(EE_DBG_ADDR) != 0xDB;
  uint16_t boots = EEPROM.read(EE_BOOT_ADDR) | (EEPROM.read(EE_BOOT_ADDR + 1) << 8);
  if (boots == 0xFFFF) boots = 0;
  boots++;
  EEPROM.update(EE_BOOT_ADDR, boots & 0xFF);
  EEPROM.update(EE_BOOT_ADDR + 1, boots >> 8);
  uint8_t rst = bootResetFlags | MCUSR;
  MCUSR = 0;
  readVccRaw(); delay(2);          // 기준 전압 바꾼 뒤 첫 값은 버린다
  sampleVcc();
  Serial.print(F("#B boot=")); Serial.print(boots);
  Serial.print(F(" rst=")); Serial.print(rst, HEX);
  Serial.print(F(" vcc=")); Serial.println(vccNow);
  Serial.print(F("켠 횟수 ")); Serial.print(boots); Serial.print(F(", 리셋 원인 "));
  if (rst & 0x04)      Serial.println(F("저전압(BOD)"));
  else if (rst & 0x0A) Serial.println(F("리셋 핀(버튼·USB 연결·업로드)"));
  else if (rst & 0x01) Serial.println(F("전원 켬"));
  else                 Serial.println(F("모름(부트로더가 지움)"));
}

// 서보 하나 켠 직후 전압
void debugServo(uint8_t i) {
  if (!dbgOn) return;
  delay(30); sampleVcc();
  Serial.print(F("#S ")); Serial.print(i); Serial.print(' '); Serial.println(vccNow);
}

void debugCmd(const char* line) {
  if (!dbgOn) return;
  Serial.print(F("#C ")); Serial.println(line);
  Serial.flush();                   // 명령 직후 재시작해도 이 줄은 나가도록
}

// loop()마다 부른다. act = 지금 동작 번호(0이면 차렷)
void tickDebug(uint8_t act) {
  uint32_t now = millis();
  if (loopPrev) { uint16_t g = now - loopPrev; if (g > loopMax) loopMax = g; }
  loopPrev = now;
  sampleVcc();
  if (!dbgOn || now < nextDbg) return;
  nextDbg = now + (act ? 200 : 1000);
  Serial.print(F("#D ")); Serial.print(now);
  Serial.print(' '); Serial.print(vccNow);
  Serial.print(' '); Serial.print(vccMin);
  Serial.print(' '); Serial.print(act);
  Serial.print(' '); Serial.println(loopMax);
  vccMin = 0xFFFF; loopMax = 0;
}
