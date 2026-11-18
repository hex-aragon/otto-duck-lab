// gait.h — 오실레이터(사인파) 기반 오또봇 동작
//
// [원리] 서보 4개가 각자 사인파를 따라 움직인다.
//   각도(t) = 90° + 트림 + 오프셋 O + 진폭 A × sin(2π·t/T + 위상 φ)
//   네 서보의 A · O · φ 조합만 바꾸면 걷기 · 돌기 · 춤이 된다. (Otto DIY 라이브러리와 같은 방식)
//
// Open Duck Mini는 강화학습 정책(신경망)이 관절 목표를 50Hz로 내지만,
// 오또봇은 관절이 4개뿐이라 사인파 조합으로 충분하다. 대신 "파라미터를 어떻게 고르나"를
// 2단계 실험(파라미터 탐색, 시뮬레이션)으로 공부하면 Open Duck의 기준 걸음 생성기와 이어진다.
//
// 아래 파라미터 초기값은 Otto DIY 라이브러리(GPL-3.0)의 공개 동작을 참고해 고른 값이다.
// 코드는 이 저장소에서 새로 작성했다. 실물에서 반드시 다시 맞출 것.
//
// 이 파일은 Arduino 의존 없이 순수 계산이라 PC에서도 컴파일 · 시험할 수 있다.
#pragma once
#include <math.h>
#include <stdint.h>

enum ServoId : uint8_t { LEFT_LEG = 0, RIGHT_LEG = 1, LEFT_FOOT = 2, RIGHT_FOOT = 3, SERVO_COUNT = 4 };

enum Action : uint8_t {
  ACT_HOME = 0,
  ACT_WALK_FWD,
  ACT_WALK_BACK,
  ACT_TURN_LEFT,
  ACT_TURN_RIGHT,
  ACT_UPDOWN,
  ACT_SWING,
  ACT_MOONWALK_LEFT,
  ACT_MOONWALK_RIGHT,
  ACT_TIPTOE,
  ACT_COUNT
};

static const char* const ACTION_NAMES[ACT_COUNT] = {
  "home", "walk_fwd", "walk_back", "turn_left", "turn_right",
  "updown", "swing", "moonwalk_left", "moonwalk_right", "tiptoe",
};

struct GaitParams {
  float A[SERVO_COUNT];    // 진폭 (도)
  float O[SERVO_COUNT];    // 오프셋 (도)
  float phi[SERVO_COUNT];  // 위상 (도)
};

static const float DEG2RAD_F = 0.01745329f;

// 동작 → 파라미터. h = 동작 크기(춤 동작에서만 사용, 도)
static GaitParams gaitFor(Action a, float h = 25) {
  GaitParams p = {};
  switch (a) {
    case ACT_WALK_FWD:
    case ACT_WALK_BACK: {
      float dir = (a == ACT_WALK_FWD) ? 1 : -1;
      p = { { 30, 30, 20, 20 }, { 0, 0, 4, -4 }, { 0, 0, -90 * dir, -90 * dir } };
      break;
    }
    case ACT_TURN_LEFT:   // 한쪽 다리만 크게 → 방향 전환
      p = { { 30, 10, 20, 20 }, { 0, 0, 4, -4 }, { 0, 0, -90, -90 } };
      break;
    case ACT_TURN_RIGHT:
      p = { { 10, 30, 20, 20 }, { 0, 0, 4, -4 }, { 0, 0, -90, -90 } };
      break;
    case ACT_UPDOWN:      // 두 발목을 반대로 → 몸이 위아래
      p = { { 0, 0, h, h }, { 0, 0, h, -h }, { 0, 0, -90, 90 } };
      break;
    case ACT_SWING:       // 좌우로 흔들기
      p = { { 0, 0, h, h }, { 0, 0, h / 2, -h / 2 }, { 0, 0, 0, 0 } };
      break;
    case ACT_MOONWALK_LEFT:
    case ACT_MOONWALK_RIGHT: {
      float dir = (a == ACT_MOONWALK_LEFT) ? 1 : -1;
      float ph = -90 * dir;
      p = { { 0, 0, h, h }, { 0, 0, h / 2 + 2, -h / 2 - 2 }, { 0, 0, ph, ph - 60 * dir } };
      break;
    }
    case ACT_TIPTOE:      // 까치발 흔들기
      p = { { 0, 0, h, h }, { 0, 0, h, -h }, { 0, 0, 0, 0 } };
      break;
    default:              // HOME: 전부 0 → 차렷
      break;
  }
  return p;
}

// t_ms 시점(동작 시작 기준)의 각 서보 목표 각도(트림 제외, 90° 중심)
static void gaitAngles(const GaitParams& p, uint32_t t_ms, uint32_t period_ms, float out[SERVO_COUNT]) {
  float w = (period_ms > 0) ? (2.0f * (float)M_PI * (float)(t_ms % period_ms) / (float)period_ms) : 0;
  for (int i = 0; i < SERVO_COUNT; i++) {
    out[i] = 90.0f + p.O[i] + p.A[i] * sinf(w + p.phi[i] * DEG2RAD_F);
  }
}
