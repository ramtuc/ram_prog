#include <SPI.h>

const int latchPin = 10;

byte patterns[][8] = {
  // 90度回転させたハート
  {0x00, 0x66, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00},
  // チェック
  {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55},
  // Xマーク
  {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81},
  // ボーダー
  {0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF}
};

void setup() {
  SPI.begin();
  pinMode(latchPin, OUTPUT);
}

void loop() {
  for (int p = 0; p < 4; p++) {
    int totalRepeats = (p == 1) ? 120 : 120; // チェックの場合は繰り返し回数を増やす
    for (int repeat = 0; repeat < totalRepeats; repeat++) {
      for (int row = 0; row < 8; row++) {
        digitalWrite(latchPin, LOW);
        // チェックパターンの場合、15回の繰り返し毎に反転
        if (p == 1 && (repeat / 15) % 2 == 0) {
          SPI.transfer(~patterns[p][row]);
        } else {
          SPI.transfer(patterns[p][row]);
        }
        SPI.transfer(1 << row);
        digitalWrite(latchPin, HIGH);
        delay(2);
      }
    }
    delay(500);
  }
}
