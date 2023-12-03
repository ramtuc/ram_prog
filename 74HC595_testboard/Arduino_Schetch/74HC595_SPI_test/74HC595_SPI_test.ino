// 74HC595を5個カスケード接続した場合の動作サンプル(SPI通信を使用した場合)
//SERピン Arduinoの11番ピン
//RCLKピン Arduinoの10番ピン
//SRCLKピン Arduinoの13番ピン
#include <SPI.h>

const int latchPin = 10;

void setup() {
  SPI.begin();
  pinMode(latchPin, OUTPUT);
}

void loop() {
  for (int i = 0; i < 40; i++) {
    digitalWrite(latchPin, LOW);
    
    for (int j = 4; j >= 0; j--) {
      if (i/8 == j) {
        SPI.transfer(1 << (i % 8));
      } else {
        SPI.transfer(0);
      }
    }
    
    digitalWrite(latchPin, HIGH);
    delay(20);
  }
}
