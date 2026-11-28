"""
bt_dual_test.py — 오또 몸(Nano + JDY-31 클래식 블루투스)과 오또 트윈(NU40 BLE)을
노트북 한 대에서 동시에 블루투스로 연결해 버티는지 보는 시험.

Windows Python에서 실행:  python tools\\bt_dual_test.py [--com COM15] [--sec 60]
필요:  pip install bleak pyserial

하는 일
  1. NU40을 BLE로 찾아 연결 (이름 Otto-Twin-*, Nordic UART) → #T(자세) 줄 받기
  2. Nano를 JDY-31 COM 포트로 연결 → 걷기 · 춤 명령 보내고 #A(각도) 줄 받기
  3. 둘이 동시에 도는 동안 NU40에 sign(서명)을 여러 번 보내 왕복 시간 재기
  4. 초마다 수신 줄 수, 끊김, 서명 성공률을 표로 출력
"""
import argparse, asyncio, os, statistics, sys, time, threading

import serial
from bleak import BleakClient, BleakScanner

NUS_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"   # 우리가 쓰는 쪽
NUS_TX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"   # 보드가 알려 주는 쪽

NANO_SCRIPT = [  # (보낼 시각 초, 명령)
    (1, "h"), (3, "w 4"), (12, "h"), (14, "kp 0"), (40, "h"), (42, "w 4"), (52, "h"),
]


class Counter:
    def __init__(self):
        self.lines = 0; self.per_sec = {}; self.last = None; self.gaps = []; self.kinds = {}

    def hit(self, line, t0):
        now = time.monotonic(); s = int(now - t0)
        self.lines += 1; self.per_sec[s] = self.per_sec.get(s, 0) + 1
        if self.last is not None: self.gaps.append(now - self.last)
        self.last = now
        k = line.split(" ", 1)[0] if line.startswith("#") else "글"
        self.kinds[k] = self.kinds.get(k, 0) + 1


def nano_thread(port, sec, t0, cnt, log, stop):
    try:
        ser = serial.Serial(port, 9600, timeout=0.1)
    except Exception as e:
        log.append(f"Nano 포트 {port} 열기 실패: {e}"); return
    log.append(f"Nano {port} 열림")
    buf = b""; script = list(NANO_SCRIPT)
    while not stop.is_set() and time.monotonic() - t0 < sec:
        el = time.monotonic() - t0
        while script and script[0][0] <= el:
            _, c = script.pop(0); ser.write((c + "\n").encode()); log.append(f"{el:5.1f}s Nano ← {c}")
        buf += ser.read(256)
        while b"\n" in buf:
            ln, buf = buf.split(b"\n", 1)
            s = ln.decode("utf-8", "replace").strip()
            if not s: continue
            cnt.hit(s, t0)
            if not s.startswith("#A") and not s.startswith("#D"): log.append(f"{time.monotonic()-t0:5.1f}s Nano → {s[:60]}")
            if "rst=" in s or "부팅" in s or "OTTO" in s.upper() and "준비" in s: log.append(f"⚠ Nano 재시작 의심: {s[:60]}")
    try: ser.write(b"h\n"); ser.close()
    except Exception: pass


async def main():
    ap = argparse.ArgumentParser(); ap.add_argument("--com", default="COM15"); ap.add_argument("--sec", type=int, default=60)
    ap.add_argument("--no-nano", action="store_true"); ap.add_argument("--no-nu40", action="store_true")
    a = ap.parse_args()
    log = []; nano = Counter(); nu = Counter(); sign_rtt = []; sign_fail = 0

    dev = None
    if not a.no_nu40:
        print("NU40 찾는 중 (Otto-Twin-*) …", flush=True)
        dev = await BleakScanner.find_device_by_filter(lambda d, ad: (d.name or ad.local_name or "").startswith("Otto-Twin"), timeout=15)
        if not dev: print("✕ NU40 BLE를 못 찾음: 전원, 또는 다른 기기(폰 · 크롬)가 이미 붙어 있는지 확인"); a.no_nu40 = True
        else: print(f"찾음: {dev.name} {dev.address}", flush=True)

    t0 = time.monotonic(); stop = threading.Event()
    th = None
    if not a.no_nano:
        th = threading.Thread(target=nano_thread, args=(a.com, a.sec, t0, nano, log, stop), daemon=True); th.start()

    if not a.no_nu40:
        rx = bytearray(); pending = {}
        def on_tx(_, data):
            rx.extend(data)
            while b"\n" in rx:
                i = rx.index(b"\n"); s = rx[:i].decode("utf-8", "replace").strip(); del rx[:i + 1]
                if not s: continue
                nu.hit(s, t0)
                if s.startswith("#G") and "t" in pending: sign_rtt.append(time.monotonic() - pending.pop("t"))
                elif s.startswith("#I"): log.append(f"{time.monotonic()-t0:5.1f}s NU40 → {s}")
                elif s.startswith("#E"): log.append(f"NU40 오류: {s}")
        async with BleakClient(dev, timeout=20) as cl:
            log.append(f"NU40 BLE 연결 (MTU {cl.mtu_size})")
            await cl.start_notify(NUS_TX, on_tx)
            await cl.write_gatt_char(NUS_RX, b"id\n", response=False)
            msg = os.urandom(200).hex()  # 트랜잭션 메시지 크기쯤
            next_sign = 5.0
            while time.monotonic() - t0 < a.sec:
                el = time.monotonic() - t0
                if el >= next_sign:
                    if "t" in pending: sign_fail += 1; pending.pop("t")
                    pending["t"] = time.monotonic()
                    data = f"sign {msg}\n".encode()
                    ch = max(20, cl.mtu_size - 3)
                    for i in range(0, len(data), ch): await cl.write_gatt_char(NUS_RX, data[i:i + ch], response=False)
                    next_sign += 5.0
                await asyncio.sleep(0.05)
            if "t" in pending: sign_fail += 1
            await cl.stop_notify(NUS_TX)
    else:
        while time.monotonic() - t0 < a.sec: await asyncio.sleep(0.2)
    stop.set()
    if th: th.join(2)

    print("\n── 기록 ──"); print("\n".join(log))
    print("\n── 초당 수신 줄 (Nano #A 등 / NU40 #T 등) ──")
    for s in range(0, a.sec, 5):
        n1 = sum(nano.per_sec.get(i, 0) for i in range(s, s + 5)) / 5; n2 = sum(nu.per_sec.get(i, 0) for i in range(s, s + 5)) / 5
        print(f"{s:3d}~{s+5:3d}s  Nano {n1:5.1f}/s   NU40 {n2:5.1f}/s")
    def gap(c): return f"최대 공백 {max(c.gaps):.2f}s" if c.gaps else "공백 없음"
    print(f"\nNano 합계 {nano.lines}줄 {nano.kinds} {gap(nano)}")
    print(f"NU40 합계 {nu.lines}줄 {nu.kinds} {gap(nu)}")
    if sign_rtt or sign_fail:
        print(f"서명 {len(sign_rtt)}번 성공 / {sign_fail}번 응답 없음, 왕복 평균 {statistics.mean(sign_rtt)*1000:.0f}ms 최대 {max(sign_rtt)*1000:.0f}ms" if sign_rtt else f"서명 0번 성공 / {sign_fail}번 응답 없음")


if __name__ == "__main__":
    asyncio.run(main())
