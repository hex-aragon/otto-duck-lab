// posture.h — 로봇 상태 분류 (TinyML 자리)
//
// 입력: 최근 1초(25Hz × 25샘플) IMU에서 뽑은 특징 8개
//   [0..2] 가속도 x,y,z 평균 (g)       → 중력 방향 = 서 있나, 어느 쪽으로 넘어졌나
//   [3..5] 가속도 x,y,z 표준편차 (g)   → 흔들림 크기 = 걷는 중인가, 들려서 흔들리나
//   [6]    자이로 크기 평균 (°/s ÷ 100) → 회전 많음
//   [7]    가속도 크기 최대 (g)          → 충격 (넘어지는 순간, 부딪힘)
//
// 출력 class (웹 web/index.html의 POSTURE_CLASSES와 번호가 같아야 한다)
//   0 서 있음 · 1 걷는 중 · 2 앞으로 넘어짐 · 3 뒤로 넘어짐 · 4 옆으로 넘어짐 · 5 들어 올림
//
// posture_model.h 가 이 폴더에 있으면 신경망(웹에서 학습 → 내보내기)으로 분류하고,
// 없으면 아래 규칙(임계값)으로 분류한다. 규칙은 "들어 올림"을 구분하지 못한다 → TinyML이 필요한 이유.
#pragma once
#include <math.h>
#include <stdint.h>
#include "imu.h"

enum PostureClass : uint8_t {
  POSTURE_STANDING = 0,
  POSTURE_WALKING,
  POSTURE_FALL_FRONT,
  POSTURE_FALL_BACK,
  POSTURE_FALL_SIDE,
  POSTURE_LIFTED,
  POSTURE_COUNT
};

static const uint8_t FEATURE_COUNT = 8;
static const uint8_t FEATURE_WINDOW = 25;  // 25Hz × 1초

static inline bool postureIsFallen(uint8_t c) {
  return c == POSTURE_FALL_FRONT || c == POSTURE_FALL_BACK || c == POSTURE_FALL_SIDE;
}

// 25Hz 샘플을 쌓아 두는 원형 버퍼
struct FeatureWindow {
  ImuSample s[FEATURE_WINDOW];
  uint8_t head = 0, count = 0;
  void push(const ImuSample& x) {
    s[head] = x;
    head = (head + 1) % FEATURE_WINDOW;
    if (count < FEATURE_WINDOW) count++;
  }
  bool full() const { return count == FEATURE_WINDOW; }

  void features(float f[FEATURE_COUNT]) const {
    float sum[3] = {0}, sq[3] = {0}, gyro = 0, peak = 0;
    for (uint8_t i = 0; i < count; i++) {
      const ImuSample& x = s[i];
      float a[3] = { x.ax, x.ay, x.az };
      for (int k = 0; k < 3; k++) { sum[k] += a[k]; sq[k] += a[k] * a[k]; }
      gyro += sqrtf(x.gx * x.gx + x.gy * x.gy + x.gz * x.gz);
      float mag = sqrtf(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
      if (mag > peak) peak = mag;
    }
    float n = count ? (float)count : 1;
    for (int k = 0; k < 3; k++) {
      float m = sum[k] / n;
      f[k] = m;
      float v = sq[k] / n - m * m;
      f[3 + k] = v > 0 ? sqrtf(v) : 0;
    }
    f[6] = gyro / n / 100.0f;
    f[7] = peak;
  }
};

#if defined(__has_include)
  #if __has_include("posture_model.h")
    #include "posture_model.h"
    #define POSTURE_TINYML 1
  #endif
#endif
#ifndef POSTURE_TINYML
  #define POSTURE_TINYML 0
#endif

// 규칙 기반 (모델이 없을 때)
static uint8_t classifyPostureRule(const float f[FEATURE_COUNT], float& conf) {
  conf = 1.0f;
  // ax = -sin(앞으로 숙인 각) → 앞으로 넘어지면 ax ≈ -1
  if (f[0] < -0.6f) return POSTURE_FALL_FRONT;
  if (f[0] > 0.6f)  return POSTURE_FALL_BACK;
  if (fabsf(f[1]) > 0.6f) return POSTURE_FALL_SIDE;
  if (f[6] > 0.4f || f[3] > 0.08f || f[4] > 0.08f) return POSTURE_WALKING;
  return POSTURE_STANDING;
}

#if POSTURE_TINYML
// 2층 신경망: 입력(정규화) → 은닉(ReLU) → 출력(softmax). 가중치 배열은 posture_model.h에 있다.
static uint8_t classifyPostureModel(const float f[FEATURE_COUNT], float& conf) {
  static_assert(POSTURE_MODEL_INPUTS == FEATURE_COUNT, "posture_model.h 입력 개수가 다릅니다 - 웹에서 다시 내보내세요");
  float x[POSTURE_MODEL_INPUTS], h[POSTURE_MODEL_HIDDEN], o[POSTURE_MODEL_OUTPUTS];
  for (int i = 0; i < POSTURE_MODEL_INPUTS; i++) x[i] = (f[i] - POSTURE_MODEL_MEAN[i]) / POSTURE_MODEL_STD[i];
  for (int j = 0; j < POSTURE_MODEL_HIDDEN; j++) {
    float a = POSTURE_MODEL_B1[j];
    for (int i = 0; i < POSTURE_MODEL_INPUTS; i++) a += x[i] * POSTURE_MODEL_W1[i * POSTURE_MODEL_HIDDEN + j];
    h[j] = a > 0 ? a : 0;
  }
  float mx = -1e30f;
  for (int k = 0; k < POSTURE_MODEL_OUTPUTS; k++) {
    float a = POSTURE_MODEL_B2[k];
    for (int j = 0; j < POSTURE_MODEL_HIDDEN; j++) a += h[j] * POSTURE_MODEL_W2[j * POSTURE_MODEL_OUTPUTS + k];
    o[k] = a;
    if (a > mx) mx = a;
  }
  float sum = 0; int best = 0;
  for (int k = 0; k < POSTURE_MODEL_OUTPUTS; k++) { o[k] = expf(o[k] - mx); sum += o[k]; if (o[k] > o[best]) best = k; }
  conf = o[best] / sum;
  return POSTURE_MODEL_CLASSES[best];
}
#endif

static uint8_t classifyPosture(const float f[FEATURE_COUNT], float& conf) {
#if POSTURE_TINYML
  return classifyPostureModel(f, conf);
#else
  return classifyPostureRule(f, conf);
#endif
}
