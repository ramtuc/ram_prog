#include <Wire.h>
#include <Adafruit_MCP23X17.h>
Adafruit_MCP23X17 mcp;
void setup() {
  Wire.begin();     // I2Cバスを初期化
  mcp.begin_I2C();  // デフォルトのアドレス0x20でMCP23017を初期化

  // MCP23017のAバンク（PA0-PA7）を出力として設定
  for (int i = 0; i < 8; i++) {
    mcp.pinMode(i, OUTPUT);
  }

  // MCP23017のBバンク（PB0-PB7）を入力として設定
  for (int i = 8; i < 16; i++) {
    mcp.pinMode(i, INPUT);
  }

  // Arduinoのピン0-7を出力として設定
  for (int i = 0; i < 8; i++) {
    pinMode(i, OUTPUT);
  }
  randomSeed(analogRead(0));
}

void loop() {
  int randomPin = random(0, 8); // 0から7のランダムなピンを選択

  digitalWrite(randomPin, HIGH);  // ランダムなピンをHIGHに設定
delay(500);
  // MCP23017のBバンク(PB0-PB7)の状態を読み取り、Aバンク(PA0-PA7)に反映させる
  for (int i = 0; i < 8; i++) {
    int state = mcp.digitalRead(i + 8);  // PB0-PB7の状態を読み取る
    mcp.digitalWrite(i, state);          // PA0-PA7に反映させる
  }

  delay(500);             // 1秒待機

  digitalWrite(randomPin, LOW); // ピンをLOWに設定
}