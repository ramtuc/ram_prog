#include <Wire.h>
#include <Adafruit_MCP23X17.h>
Adafruit_MCP23X17 mcp;
void setup() {
  Wire.begin(); // I2Cバスを初期化
  mcp.begin_I2C();  // デフォルトのアドレス0x20でMCP23017を初期化

  // すべてのピンを出力として設定
  for (int i = 0; i < 16; i++) {
    mcp.pinMode(i, OUTPUT);
  }
}

void loop() {
  // 各ピンを順番にHIGHにし、残りをLOWに設定
  for (int i = 0; i < 16; i++) {
    // 現在のピンをHIGHに設定
    mcp.digitalWrite(i, HIGH);

    // 他のピンをLOWに設定
    for (int j = 0; j < 16; j++) {
      if (j != i) {
        mcp.digitalWrite(j, LOW);
      }
    }

    delay(200); // 500ミリ秒待機
  }
}