// 74HC595のピン接続
#include <SPI.h>
const int latchPin = 10;    // RCLK
void setup() {
  SPI.begin();
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
}


void loop() {
  wavePattern();

  doubleChasePattern();
  middleOutPattern();

  chaserPattern();
  for (int j = 0; j <= 40; j++) {
    uint64_t pattern = (j == 40) ? 0xFFFFFFFFFF : ((1ULL << j) - 1);
    updateShiftRegisters(pattern);
    delay(50);
  }
  delay(100);
  randomPattern();
  //singleChasePattern();

  //rotateChasePattern();
}

void updateShiftRegisters(uint64_t data) {
  digitalWrite(latchPin, LOW);
  for (int i = 4; i >= 0; i--) {
    shiftOut(dataPin, clockPin, MSBFIRST, (data >> (8 * i)) & 0xFF);
  }
  digitalWrite(latchPin, HIGH);
}


// 1. ウェーブ
void wavePattern() {
  for (int j = 0; j < 40; j++) {
    uint64_t pattern = 1ULL << j;
    updateShiftRegisters(pattern);
    delay(100);
  }
  delay(500);
}

// 2. 中央から放射
void middleOutPattern() {
  int middle = 20;
  for (int j = 0; j <= middle; j++) {
    uint64_t pattern = (1ULL << (middle - j)) | (1ULL << (middle + j));
    updateShiftRegisters(pattern);
    delay(100);
  }
  delay(500);
}

// 3. ランダム
void randomPattern() {
  for (int j = 0; j < 40; j++) {
    uint64_t upperBits = (uint64_t)random(0xFFFF) << 32;  // 16ビットを生成して、32ビット左にシフト
    uint64_t middleBits = (uint64_t)random(0xFFFF) << 16; // 16ビットを生成して、16ビット左にシフト
    uint64_t lowerBits = random(0xFFFF);  // 下位16ビット
    uint64_t pattern = upperBits | middleBits | lowerBits;
    updateShiftRegisters(pattern);
    delay(100);
  }
  delay(500);
}

// 4. チャーサー
void chaserPattern() {
  uint64_t pattern = 0xF;  // 初期の4つのLEDを点灯
  for (int j = 0; j < 37; j++) {
    pattern <<= 1;
    updateShiftRegisters(pattern);
    delay(100);
  }
  delay(500);
}
// 1. 単一LEDチェイス
void singleChasePattern() {
  for (int j = 0; j < 40; j++) {
    uint64_t pattern = 1ULL << j;
    updateShiftRegisters(pattern);
    delay(100);
  }
  for (int j = 38; j >= 0; j--) {
    uint64_t pattern = 1ULL << j;
    updateShiftRegisters(pattern);
    delay(100);
  }
  delay(500);
}

// 2. ダブルLEDチェイス
void doubleChasePattern() {
  for (int j = 0; j < 38; j++) {
    uint64_t pattern = (1ULL << j) | (1ULL << (j + 2));
    updateShiftRegisters(pattern);
    delay(100);
  }
  for (int j = 38; j > 0; j--) {
    uint64_t pattern = (1ULL << j) | (1ULL << (j - 2));
    updateShiftRegisters(pattern);
    delay(100);
  }
  delay(500);
}

// 3. 回転チェイス
void rotateChasePattern() {
  for (int j = 0; j < 40; j++) {
    uint64_t pattern = 1ULL << (j % 40);
    updateShiftRegisters(pattern);
    delay(100);
  }
  delay(500);
}
