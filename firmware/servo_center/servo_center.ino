// 오또 DIY 조립용: 서보 4개를 90도로 맞추고, 하나씩 흔들어 핀-서보 대응을 확인한다.
// 핀 번호(2번 서보 -> D2 ...)는 회로도 이름에서 추측한 값이니 실제 움직임으로 확인할 것.
#include <Servo.h>
Servo s[4];
const int PINS[4] = {2, 3, 4, 5};

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 4; i++) { s[i].attach(PINS[i]); s[i].write(90); }
  delay(3000);
  for (int i = 0; i < 4; i++) {
    Serial.print("지금 움직이는 핀: D"); Serial.println(PINS[i]);
    for (int k = 0; k < 2; k++) { s[i].write(60); delay(400); s[i].write(120); delay(400); }
    s[i].write(90);
    delay(1500);
  }
}
void loop() {}
