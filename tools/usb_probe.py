"""usb_probe.py — USB로 꽂힌 오또 보드에 한 줄 보내고 답 보기.  python tools\\usb_probe.py COM11 9600 i [기다릴초]"""
import sys, time, serial
port, baud = sys.argv[1], int(sys.argv[2])
args = sys.argv[3:]; wait = float(args.pop()) if args and args[-1].replace(".", "").isdigit() else 3
cmd = " ".join(args) or "i"
s = serial.Serial(port, baud, timeout=0.2); s.dtr = True; time.sleep(2.2)
pre = s.read(4096)
s.write((cmd + "\n").encode()); end = time.time() + wait; out = b""
while time.time() < end: out += s.read(512)
print(f"[{port}] 열자마자 {len(pre)}바이트, {cmd!r} 뒤 {len(out)}바이트")
print((pre[-300:] + b"\n---\n" + out[:1500]).decode("utf-8", "replace")); s.close()
