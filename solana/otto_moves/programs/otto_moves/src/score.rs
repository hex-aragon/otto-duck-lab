//! 움직임 점수 → MOVE 보상 계산 (체인 밖에서도 똑같이 계산해 볼 수 있게 순수 함수로 둔다)

/// 한 번 기록에 줄 수 있는 최대 보상
pub const MAX_REWARD: u64 = 100;
/// 이보다 짧은 기록은 보상 0 (버튼 연타 막기)
pub const MIN_SECONDS: u32 = 10;

/// 웹(otto-pet.html)이 발행할 때 쓰는 움직임 요약과 같은 값
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct Motion {
    pub sec: u32,       // 기록 길이 (초)
    pub steps: u32,     // 걸음 수
    pub dance_sec: u32, // 춤춘 시간 (초)
    pub max_tilt: u16,  // 가장 크게 기운 각도 (도)
    pub falls: u16,     // 넘어진 횟수
    pub lines: u32,     // 원본 기록 줄 수 (센서 · 동작)
}

/// 점수 규칙 (단순하고 예측 가능하게)
/// - 걸음 2개당 1점, 최대 40점
/// - 춤 3초당 1점, 최대 40점
/// - 센서 기록이 충분하면(초당 2줄 이상) +20점: 실물이 진짜 움직였다는 근거
/// - 넘어질 때마다 −10점
/// - 10초보다 짧거나, 기록 줄이 없으면 0점
pub fn reward(m: &Motion) -> u64 {
    if m.sec < MIN_SECONDS || m.lines == 0 {
        return 0;
    }
    let walk = (m.steps / 2).min(40) as i64;
    let dance = (m.dance_sec / 3).min(40) as i64;
    let sensor = if m.lines >= m.sec.saturating_mul(2) { 20 } else { 0 };
    let penalty = 10 * m.falls as i64;
    (walk + dance + sensor - penalty).clamp(0, MAX_REWARD as i64) as u64
}

#[cfg(test)]
mod tests {
    use super::*;

    fn m(sec: u32, steps: u32, dance_sec: u32, falls: u16, lines: u32) -> Motion {
        Motion { sec, steps, dance_sec, max_tilt: 10, falls, lines }
    }

    #[test]
    fn short_or_empty_is_zero() {
        assert_eq!(reward(&m(9, 100, 100, 0, 1000)), 0);
        assert_eq!(reward(&m(60, 100, 100, 0, 0)), 0);
    }

    #[test]
    fn walk_and_dance() {
        // 걸음 30 → 15, 춤 30초 → 10, 센서 충분 → 20
        assert_eq!(reward(&m(60, 30, 30, 0, 200)), 45);
        // 센서 줄 부족하면 보너스 없음
        assert_eq!(reward(&m(60, 30, 30, 0, 100)), 25);
    }

    #[test]
    fn caps_and_falls() {
        assert_eq!(reward(&m(600, 10_000, 10_000, 0, 5000)), 100);
        assert_eq!(reward(&m(600, 10_000, 10_000, 3, 5000)), 70);
        assert_eq!(reward(&m(60, 2, 0, 5, 200)), 0); // 음수가 되면 0
    }
}
