// imu.h — 6축 IMU 최소 드라이버 + 기울기(pitch/roll) 추정
//
// 에어 밴드(air-band) 펌웨어와 같은 두 계열을 WHO_AM_I로 자동 감지한다.
//   - ST LSM6DS3 / LSM6DS3TR-C / LSM6DSL / LSM6DSO (I2C 0x6A / 0x6B)
//   - InvenSense MPU-6050 / 6500 / 9250             (I2C 0x68 / 0x69)
// 다른 IMU를 쓰면 imuBegin() / imuRead() 두 함수에만 드라이버를 추가하면 된다.
//
// 축 방향은 로봇에 IMU를 어떻게 붙였는지에 따라 다르다.
// TODO: 확인 필요 — 실물에 붙인 뒤 앞으로 기울였을 때 pitch가 +가 되는지 확인하고 IMU_AXIS_* 값을 고칠 것.
#pragma once
#include <Arduino.h>
#include <Wire.h>

// 로봇 기준 축 = 센서 축 × 부호. (0=x, 1=y, 2=z)
static const uint8_t IMU_AXIS_FWD  = 0;  static const int8_t IMU_SIGN_FWD  = +1;  // TODO: 확인 필요
static const uint8_t IMU_AXIS_LEFT = 1;  static const int8_t IMU_SIGN_LEFT = +1;  // TODO: 확인 필요
static const uint8_t IMU_AXIS_UP   = 2;  static const int8_t IMU_SIGN_UP   = +1;  // TODO: 확인 필요

struct ImuSample {
  float ax, ay, az;  // g (로봇 기준: 앞, 왼쪽, 위)
  float gx, gy, gz;  // °/s
};

enum ImuType : uint8_t { IMU_NONE = 0, IMU_LSM6DS, IMU_MPU };

static ImuType g_imuType = IMU_NONE;
static uint8_t g_imuAddr = 0;

static bool i2cWrite(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

static bool i2cRead(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t n) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(addr, n) != n) return false;
  for (uint8_t i = 0; i < n; i++) buf[i] = Wire.read();
  return true;
}

// 감지 성공 시 true. 실패하면 IMU 없이 동작한다 (넘어짐 감지 · 자세 분류 꺼짐).
static bool imuBegin() {
  uint8_t who = 0;
  static const uint8_t LSM_ADDRS[] = { 0x6A, 0x6B };
  for (uint8_t addr : LSM_ADDRS) {
    if (i2cRead(addr, 0x0F, &who, 1) && (who == 0x69 || who == 0x6A || who == 0x6C)) {
      g_imuType = IMU_LSM6DS; g_imuAddr = addr;
      i2cWrite(addr, 0x12, 0x44);  // CTRL3_C: BDU=1, 주소 자동 증가
      i2cWrite(addr, 0x10, 0x48);  // CTRL1_XL: 104Hz, ±4g
      i2cWrite(addr, 0x11, 0x4C);  // CTRL2_G : 104Hz, ±2000°/s
      return true;
    }
  }
  static const uint8_t MPU_ADDRS[] = { 0x68, 0x69 };
  for (uint8_t addr : MPU_ADDRS) {
    if (i2cRead(addr, 0x75, &who, 1) && (who == 0x68 || who == 0x70 || who == 0x71)) {
      g_imuType = IMU_MPU; g_imuAddr = addr;
      i2cWrite(addr, 0x6B, 0x00);  // PWR_MGMT_1: 깨우기
      i2cWrite(addr, 0x1C, 0x08);  // ACCEL_CONFIG: ±4g
      i2cWrite(addr, 0x1B, 0x18);  // GYRO_CONFIG : ±2000°/s
      return true;
    }
  }
  g_imuType = IMU_NONE;
  return false;
}

static inline float pick(const float v[3], uint8_t axis, int8_t sign) { return v[axis] * sign; }

// 센서 원본 → 로봇 기준 축으로 바꿔 out에 넣는다
static bool imuRead(ImuSample& out) {
  float a[3], g[3];
  uint8_t b[14];
  if (g_imuType == IMU_LSM6DS) {
    if (!i2cRead(g_imuAddr, 0x22, b, 12)) return false;  // OUTX_L_G부터 자이로 6 + 가속도 6 바이트
    for (int i = 0; i < 3; i++) {
      g[i] = (int16_t)(b[i * 2] | (b[i * 2 + 1] << 8)) * 0.070f;       // 70 mdps/LSB
      a[i] = (int16_t)(b[6 + i * 2] | (b[6 + i * 2 + 1] << 8)) * 0.000122f;  // 0.122 mg/LSB
    }
  } else if (g_imuType == IMU_MPU) {
    if (!i2cRead(g_imuAddr, 0x3B, b, 14)) return false;  // 빅엔디언: 가속도 6, 온도 2, 자이로 6
    for (int i = 0; i < 3; i++) {
      a[i] = (int16_t)((b[i * 2] << 8) | b[i * 2 + 1]) / 8192.0f;
      g[i] = (int16_t)((b[8 + i * 2] << 8) | b[8 + i * 2 + 1]) / 16.4f;
    }
  } else {
    return false;
  }
  out.ax = pick(a, IMU_AXIS_FWD, IMU_SIGN_FWD);
  out.ay = pick(a, IMU_AXIS_LEFT, IMU_SIGN_LEFT);
  out.az = pick(a, IMU_AXIS_UP, IMU_SIGN_UP);
  out.gx = pick(g, IMU_AXIS_FWD, IMU_SIGN_FWD);
  out.gy = pick(g, IMU_AXIS_LEFT, IMU_SIGN_LEFT);
  out.gz = pick(g, IMU_AXIS_UP, IMU_SIGN_UP);
  return true;
}

// 상보 필터: 자이로 적분(빠르지만 오차가 쌓임) + 가속도 중력 방향(느리지만 오차가 안 쌓임)
//   pitch + = 앞으로 숙임, roll + = 오른쪽으로 기울어짐 (축 부호가 맞을 때)
struct Tilt {
  float pitch = 0, roll = 0;
  bool  init = false;
  void update(const ImuSample& s, float dt, float alpha = 0.98f) {
    const float RAD2DEG = 57.29578f;
    // 앞으로 θ만큼 숙이면 '위' 방향이 몸 x축에서 -sinθ로 보인다 → ax = -sinθ
    // 오른쪽으로 φ만큼 기울면 몸 y(왼쪽)축이 위를 향한다 → ay = +sinφ
    // 오른손 법칙: y축(왼쪽) + 회전 = 앞으로 숙임, x축(앞) + 회전 = 오른쪽으로 기움
    float accPitch = atan2f(-s.ax, sqrtf(s.ay * s.ay + s.az * s.az)) * RAD2DEG;
    float accRoll  = atan2f(s.ay, s.az) * RAD2DEG;
    if (!init) { pitch = accPitch; roll = accRoll; init = true; return; }
    pitch = alpha * (pitch + s.gy * dt) + (1 - alpha) * accPitch;
    roll  = alpha * (roll  + s.gx * dt) + (1 - alpha) * accRoll;
  }
};
