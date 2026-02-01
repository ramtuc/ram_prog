# 74HC595 LED Test Board

74HC595シフトレジスタを5個カスケード接続して40個のLEDを制御するArduino用テストボードプロジェクトです。わずか3本の信号線で大量のLEDを制御できます。

## 特徴

- **40個のLED制御**: 74HC595を5個カスケード接続
- **3本の信号線のみ**: DATA、CLOCK、LATCHの3本で制御
- **2種類の制御方法**:
  - GPIO制御（shiftOut関数）
  - SPI通信（高速転送）
- **多彩なアニメーションパターン**: ウェーブ、チェイス、ランダムなど
- **拡張性**: さらに74HC595を追加して無限に拡張可能

## ハードウェア

### 回路図

`schematic/` フォルダにKiCadプロジェクトファイルが含まれています：

- `74HC595_testboard.kicad_sch` - 回路図
- `74HC595_testboard.kicad_pcb` - PCBレイアウト
- `74HC595_testboard.pdf` - 回路図PDF
- `74HC595_testboardBOM_JLCSMT.xlsx` - 部品表（BOM）
- `74HC595_testboard_CPL1.xlsx` - 部品配置リスト
- `74HC595_testboard.zip` - Gerberファイル一式

### 74HC595について

74HC595は、シリアル入力をパラレル出力に変換する8ビットシフトレジスタです。

**主な仕様:**
- 8ビットのパラレル出力
- カスケード接続可能（Q7'ピンで次段へデータ転送）
- 3線式制御（SER、SRCLK、RCLK）
- 出力電流: 最大35mA per pin
- 動作電圧: 2V～6V
- 高速動作: 最大100MHz（5V時）

**ピン説明:**
- **SER (DS)**: シリアルデータ入力
- **SRCLK (SHCP)**: シフトレジスタクロック
- **RCLK (STCP)**: ストレージレジスタクロック（ラッチ）
- **Q0-Q7**: パラレル出力
- **Q7'**: 次段へのシリアル出力（カスケード接続用）
- **OE**: 出力イネーブル（通常はGNDに接続）
- **SRCLR**: クリア（通常はVCCに接続）

### カスケード接続

5個の74HC595を数珠つなぎに接続することで、40個の独立した出力を制御：

```
Arduino → [74HC595 #1] → [74HC595 #2] → [74HC595 #3] → [74HC595 #4] → [74HC595 #5]
  (3pin)       ↓              ↓              ↓              ↓              ↓
             Q0-Q7         Q8-Q15        Q16-Q23        Q24-Q31        Q32-Q39
```

### 対応ボード

- Arduino UNO R3
- Arduino UNO R4 Minima
- Arduino UNO R4 WiFi
- その他のArduino互換ボード

## サンプルプログラム

### 1. 74HC595_test
GPIOピンでの制御（shiftOut関数を使用）

**ピン接続:**
- データピン (SER): D2
- ラッチピン (RCLK): D3
- クロックピン (SRCLK): D4

**アニメーションパターン:**

1. **ウェーブパターン** - 1つのLEDが端から端まで移動
2. **ダブルチェイスパターン** - 2つのLEDが追いかけっこ
3. **中央から放射パターン** - 中央から両端へ展開
4. **チャーサーパターン** - 4つのLEDが連なって移動
5. **プログレスバー** - LEDが順次点灯してバーが伸びる
6. **ランダムパターン** - LEDがランダムに点滅

**コード構造:**
```cpp
const int dataPin = 2;     // SERピン
const int latchPin = 3;    // RCLKピン
const int clockPin = 4;    // SRCLKピン

void updateShiftRegisters(uint64_t data) {
  digitalWrite(latchPin, LOW);
  // 5バイト分のデータを送信（MSBから）
  for (int i = 4; i >= 0; i--) {
    shiftOut(dataPin, clockPin, MSBFIRST, (data >> (8 * i)) & 0xFF);
  }
  digitalWrite(latchPin, HIGH);
}
```

**主要パターン関数:**
```cpp
// ウェーブ: 単一LEDが順次移動
void wavePattern() {
  for (int j = 0; j < 40; j++) {
    uint64_t pattern = 1ULL << j;
    updateShiftRegisters(pattern);
    delay(100);
  }
}

// 中央から放射
void middleOutPattern() {
  int middle = 20;
  for (int j = 0; j <= middle; j++) {
    uint64_t pattern = (1ULL << (middle - j)) | (1ULL << (middle + j));
    updateShiftRegisters(pattern);
    delay(100);
  }
}

// ランダム点灯
void randomPattern() {
  for (int j = 0; j < 40; j++) {
    uint64_t pattern = random(0x10000000000ULL);
    updateShiftRegisters(pattern);
    delay(100);
  }
}
```

### 2. 74HC595_SPI_test
SPI通信での制御（高速転送）

**ピン接続（SPI標準ピン）:**
- データピン (MOSI): D11
- ラッチピン (SS): D10
- クロックピン (SCK): D13

**特徴:**
- SPI.transfer()を使用した高速データ転送
- シンプルなシーケンシャル点灯
- 転送速度が向上（最大8MHz）

**コード:**
```cpp
#include <SPI.h>

const int latchPin = 10;

void setup() {
  SPI.begin();
  pinMode(latchPin, OUTPUT);
}

void loop() {
  for (int i = 0; i < 40; i++) {
    digitalWrite(latchPin, LOW);
    
    // 5個の74HC595にデータ送信
    for (int j = 4; j >= 0; j--) {
      if (i/8 == j) {
        SPI.transfer(1 << (i % 8));  // 該当するICにビットを送信
      } else {
        SPI.transfer(0);             // 他のICは0
      }
    }
    
    digitalWrite(latchPin, HIGH);
    delay(20);
  }
}
```

## セットアップ

### 1. ハードウェアの準備

1. PCBを製造（Gerberファイルを使用）
2. 部品表（BOM）を参照して部品を実装
3. Arduinoに接続

### 2. スケッチのアップロード

**GPIO制御版:**
```bash
Arduino_Schetch/74HC595_test/74HC595_test.ino
```

**SPI制御版:**
```bash
Arduino_Schetch/74HC595_SPI_test/74HC595_SPI_test.ino
```

Arduino IDEで開いて、ボードとポートを選択してアップロードします。

## 技術的詳細

### データ転送の仕組み

74HC595は2段階でデータを転送します：

1. **シフト段階（SRCLK）**: 
   - SRCLKの立ち上がりエッジでSERピンのデータを取り込み
   - データは内部シフトレジスタにシフトされる
   - 8ビット分シフトすると、次の74HC595へQ7'から出力

2. **ラッチ段階（RCLK）**:
   - RCLKの立ち上がりエッジでシフトレジスタの内容を出力レジスタへコピー
   - 出力ピン（Q0-Q7）に反映される

```cpp
digitalWrite(latchPin, LOW);    // ラッチを無効化
shiftOut(dataPin, clockPin, MSBFIRST, data); // データ転送
digitalWrite(latchPin, HIGH);   // ラッチを有効化→出力に反映
```

### 40ビットデータの扱い

Arduinoの`uint64_t`型（64ビット）を使用して40ビット分のLEDパターンを表現：

```cpp
uint64_t pattern = 0x0000000001; // LED 0のみ点灯
uint64_t pattern = 0xFFFFFFFFFF; // 全40個のLED点灯
uint64_t pattern = 1ULL << 20;   // LED 20のみ点灯
```

### shiftOut vs SPI

| 方式 | 速度 | 柔軟性 | 使用ピン |
|------|------|--------|----------|
| shiftOut | 遅い | 任意のピン使用可能 | 任意の3ピン |
| SPI | 高速（最大8MHz） | 固定ピンのみ | D10, D11, D13 |

**速度比較:**
- shiftOut: 約5kHz
- SPI: 最大8MHz（約1600倍高速）

## カスタマイズ

### アニメーション速度の調整

```cpp
delay(100);  // この値を変更して速度調整
```

### 新しいパターンの追加

```cpp
void myCustomPattern() {
  // 独自のパターンを実装
  uint64_t pattern = 0x5555555555ULL; // 交互パターン
  updateShiftRegisters(pattern);
}
```

### さらに74HC595を追加

74HC595を追加する場合は、データ型を変更：

```cpp
// 6個（48ビット）の場合
uint64_t data; // 64ビットでOK

// 9個以上（72ビット以上）の場合
// 配列やバイト配列を使用
byte data[10]; // 10バイト = 80ビット
```

## 応用例

### PWM制御でLED輝度調整

```cpp
// ソフトウェアPWMで輝度制御
void dimPattern(uint64_t pattern, int brightness) {
  for (int i = 0; i < 100; i++) {
    updateShiftRegisters(pattern);
    delayMicroseconds(brightness * 10);
    updateShiftRegisters(0);
    delayMicroseconds((100 - brightness) * 10);
  }
}
```

### マルチプレクス表示

複数のパターンを高速切り替えで表示：

```cpp
uint64_t frames[10]; // 10フレーム分
void displayFrames() {
  for (int i = 0; i < 10; i++) {
    updateShiftRegisters(frames[i]);
    delay(50); // フレームレート: 20fps
  }
}
```

## トラブルシューティング

### LEDが点灯しない

1. 電源電圧を確認（74HC595: 2-6V）
2. 配線を確認（SER、SRCLK、RCLK）
3. OEピンがGNDに接続されているか確認
4. SRCLRピンがVCCに接続されているか確認

### 一部のLEDが点灯しない

1. カスケード接続の配線を確認
2. 各74HC595の電源を確認
3. LED用の電流制限抵抗値を確認

### データが正しく転送されない

1. クロック周波数を下げる（delay追加）
2. GNDがArduinoと74HC595で共通か確認
3. 配線の長さを短くする（ノイズ対策）

## 製造データ

### Gerberファイル

`schematic/74HC595_testboard.zip` に製造用ファイルが含まれています。

### 部品表

詳細は `schematic/74HC595_testboardBOM_JLCSMT.xlsx` を参照してください。

**主要部品:**
- 74HC595 x5
- LED x40
- 抵抗（LED用電流制限）
- コンデンサ（デカップリング）

## 技術仕様

**74HC595:**
- 出力電流: 最大35mA per pin (70mA合計推奨)
- クロック周波数: 最大100MHz（5V時）
- 伝搬遅延: 14ns typ（5V時）
- 動作電圧: 2V～6V
- 動作温度: -40°C～+85°C

**消費電力:**
- 全LED点灯時: 約800mA（LED 40個 x 20mA）
- 74HC595 x5: 約10mA

## 参考資料

### 解説記事

- [74HC595シフトレジスタでLEDボード製作 - 40個のLED制御方法](https://electwork.net/posts/arduino-74hc595-led-board/)

### 技術資料

- [74HC595 データシート (Texas Instruments)](https://www.ti.com/product/SN74HC595)
- [Arduino shiftOut() Reference](https://www.arduino.cc/reference/en/language/functions/advanced-io/shiftout/)
- [Arduino SPI Library](https://www.arduino.cc/en/reference/SPI)
- [シフトレジスタの基礎](https://learn.sparkfun.com/tutorials/shift-registers)

## ライセンス

MITライセンス

## 開発環境

- Arduino IDE 2.x
- KiCad 7.x
- Arduino UNO R3/R4
