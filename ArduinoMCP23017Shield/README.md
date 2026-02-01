# Arduino MCP23017 Shield

MCP23017 I2C GPIOエキスパンダーを使用したArduino UNO用シールド基板プロジェクトです。I2C通信で16個のGPIOピンを追加し、デジタル入出力を拡張できます。

## 特徴

- **16ピンのGPIO拡張**: MCP23017により16個のデジタルI/Oピンを追加
- **I2C通信**: わずか2本の信号線(SDA/SCL)で制御
- **柔軟な入出力設定**: 各ピンを独立して入力/出力に設定可能
- **アドレス選択可能**: 最大8個のMCP23017を接続可能(アドレス0x20-0x27)
- **2種類のサンプルプログラム**:
  - 出力テスト（シーケンシャルLED点灯）
  - 入出力テスト（入力→出力ループバック）

## ハードウェア

### 回路図

`schematic/` フォルダにKiCadプロジェクトファイルが含まれています：

- `mcp23017_testboard_arduino.kicad_sch` - 回路図
- `mcp23017_testboard_arduino.kicad_pcb` - PCBレイアウト
- `mcp23017_testboard_arduino.pdf` - 回路図PDF
- `bom .xls` - 部品表（BOM）
- `CPL.xlsx` - 部品配置リスト

### MCP23017について

MCP23017は、I2Cインターフェースで制御できる16ビットGPIOエキスパンダーです。

**主な仕様:**
- 16個の双方向I/Oピン（2つの8ビットポート: GPIOA, GPIOB）
- I2C通信（400kHzまで対応）
- 各ピンに内部プルアップ抵抗搭載
- 割り込み機能搭載
- 動作電圧: 1.8V～5.5V

**ピン配置:**
- **GPIOA (PA0-PA7)**: ポートA（ピン0-7）
- **GPIOB (PB0-PB7)**: ポートB（ピン8-15）

### 対応ボード

- Arduino UNO R3
- Arduino UNO R4 Minima
- Arduino UNO R4 WiFi
- ESP32（I2C対応ボード）

## サンプルプログラム

### 1. Arduino_mcp23017_testboard
MCP23017の出力機能テスト - 16ピンをシーケンシャルに点灯

**機能:**
- 16個の出力ピン（PA0-PA7, PB0-PB7）を順番にHIGH/LOW制御
- 各ピンを順次点灯してLEDテストが可能
- 200msごとに次のピンへ移動

**用途:**
- MCP23017の動作確認
- 16個のLED順次点灯
- 出力ポートの配線チェック

**コード例:**
```cpp
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

Adafruit_MCP23X17 mcp;

void setup() {
  Wire.begin();
  mcp.begin_I2C();  // アドレス0x20
  
  // 全16ピンを出力に設定
  for (int i = 0; i < 16; i++) {
    mcp.pinMode(i, OUTPUT);
  }
}

void loop() {
  // 各ピンを順番に点灯
  for (int i = 0; i < 16; i++) {
    mcp.digitalWrite(i, HIGH);
    // 他のピンをLOWに
    for (int j = 0; j < 16; j++) {
      if (j != i) mcp.digitalWrite(j, LOW);
    }
    delay(200);
  }
}
```

### 2. Arduino_mcp23017_testboard_inout
MCP23017の入出力機能テスト - 入力読み取りと出力制御

**機能:**
- **ポートA (PA0-PA7)**: 出力ポート
- **ポートB (PB0-PB7)**: 入力ポート
- ポートBの入力状態をリアルタイムでポートAに反映
- Arduinoのデジタルピン0-7をランダムに制御

**用途:**
- 入力機能の動作確認
- スイッチ入力とLED出力の連動テスト
- ループバックテスト

**コード例:**
```cpp
void setup() {
  Wire.begin();
  mcp.begin_I2C();
  
  // ポートA (PA0-PA7): 出力
  for (int i = 0; i < 8; i++) {
    mcp.pinMode(i, OUTPUT);
  }
  
  // ポートB (PB0-PB7): 入力
  for (int i = 8; i < 16; i++) {
    mcp.pinMode(i, INPUT);
  }
}

void loop() {
  // ポートBの入力をポートAに反映
  for (int i = 0; i < 8; i++) {
    int state = mcp.digitalRead(i + 8);  // PB読み取り
    mcp.digitalWrite(i, state);          // PAに出力
  }
  delay(500);
}
```

## セットアップ

### 1. 必要なライブラリ

Arduino IDEのライブラリマネージャーからインストール：

- **Adafruit MCP23017 Library** (by Adafruit)
- **Wire** (Arduino標準ライブラリ)

```
スケッチ → ライブラリをインクルード → ライブラリを管理
「Adafruit MCP23017」で検索してインストール
```

### 2. ハードウェア接続

**I2C接続:**
- SDA → Arduino A4 (UNO) / GPIO21 (ESP32)
- SCL → Arduino A5 (UNO) / GPIO22 (ESP32)
- VDD → 5V (Arduino) / 3.3V (ESP32)
- GND → GND

**アドレス設定:**
- A0, A1, A2ピンでI2Cアドレスを設定
- デフォルト: 0x20 (A0=A1=A2=GND)

### 3. スケッチのアップロード

```bash
# Arduino IDEでスケッチを開く
Arduino_Schetch/Arduino_mcp23017_testboard/Arduino_mcp23017_testboard.ino
# または
Arduino_Schetch/Arduino_mcp23017_testboard_inout/Arduino_mcp23017_testboard_inout.ino

# ボードを選択: Arduino UNO R3/R4
# ポートを選択
# アップロード
```

## I2Cアドレス設定

MCP23017のI2Cアドレスは、A0、A1、A2ピンで設定します：

| A2 | A1 | A0 | アドレス |
|----|----|----|----------|
| 0  | 0  | 0  | 0x20     |
| 0  | 0  | 1  | 0x21     |
| 0  | 1  | 0  | 0x22     |
| 0  | 1  | 1  | 0x23     |
| 1  | 0  | 0  | 0x24     |
| 1  | 0  | 1  | 0x25     |
| 1  | 1  | 0  | 0x26     |
| 1  | 1  | 1  | 0x27     |

```cpp
// カスタムアドレスで初期化
mcp.begin_I2C(0x21);  // A0=HIGH, A1=A2=LOW
```

## 応用例

### 複数のMCP23017を使用

```cpp
Adafruit_MCP23X17 mcp1;
Adafruit_MCP23X17 mcp2;

void setup() {
  Wire.begin();
  mcp1.begin_I2C(0x20);  // 1つ目
  mcp2.begin_I2C(0x21);  // 2つ目
  
  // 合計32ピンの拡張が可能
}
```

### プルアップ抵抗の有効化

```cpp
// 内部プルアップを有効化（スイッチ入力に便利）
mcp.pinMode(0, INPUT_PULLUP);
```

### 割り込み機能の使用

```cpp
// ピン変化時に割り込みを発生
mcp.setupInterruptPin(0, CHANGE);
```

## トラブルシューティング

### I2Cデバイスが見つからない

1. 配線を確認（SDA/SCL、電源、GND）
2. I2Cスキャナーでアドレスを確認
3. プルアップ抵抗（4.7kΩ）がSDA/SCLに接続されているか確認

```cpp
// I2Cスキャナーコード
Wire.begin();
for (byte address = 1; address < 127; address++) {
  Wire.beginTransmission(address);
  if (Wire.endTransmission() == 0) {
    Serial.print("Found device at 0x");
    Serial.println(address, HEX);
  }
}
```

### ESP32で使用する場合

ESP32では、I2Cピンを明示的に指定する必要があります：

```cpp
Wire.begin(21, 22); // SDA=GPIO21, SCL=GPIO22
mcp.begin_I2C();
```

## 製造データ

### Gerberファイル

`schematic/` フォルダにPCB製造用のGerberファイルが含まれています：

- `mcp23017_testboard_arduino-F_Cu.gbr` - 表面銅箔
- `mcp23017_testboard_arduino-B_Cu.gbr` - 裏面銅箔
- `mcp23017_testboard_arduino-F_Mask.gbr` - 表面レジスト
- `mcp23017_testboard_arduino-B_Mask.gbr` - 裏面レジスト
- `mcp23017_testboard_arduino.drl` - ドリルデータ

### 部品表

詳細な部品リストは `schematic/bom .xls` を参照してください。

**主要部品:**
- MCP23017-E/SP (DIP-28)
- 抵抗、コンデンサ
- LEDインジケーター（オプション）

## 技術仕様

**MCP23017:**
- I2C通信速度: 100kHz (標準), 400kHz (Fast), 1.7MHz (High Speed)
- 出力電流: 最大25mA per pin
- 入力電圧: VDD+0.6V以内
- 動作温度: -40°C～+85°C

**基板サイズ:**
- Arduino UNO互換シールドフォームファクタ

## 参考資料

### 解説記事

- [ArduinoでMCP23017を使う - I2C GPIO拡張の基本](https://electwork.net/posts/arduino-mcp23017/)
- [ESP32でMCP23017を使う - I2C GPIO拡張の実践](https://electwork.net/posts/esp32-mcp23017/)

### 技術資料

- [MCP23017 データシート (Microchip)](https://www.microchip.com/en-us/product/MCP23017)
- [Adafruit MCP23017 Library Documentation](https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library)
- [I2C通信の基礎](https://www.arduino.cc/en/Reference/Wire)

## ライセンス

MITライセンス

## 開発環境

- Arduino IDE 2.x
- KiCad 7.x
- Adafruit MCP23017 Library
- Arduino UNO R3/R4 / ESP32
