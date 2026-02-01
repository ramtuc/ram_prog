# Arduino LED Shield

Arduino UNO用の20個のLEDを制御するシールド基板プロジェクトです。デジタルピン(D0-D13)とアナログピン(A0-A5)をすべて使用してLEDアニメーションを実現します。

## 特徴

- **20個のLED制御**: デジタルピン14個 + アナログピン6個
- **ソフトウェアPWM**: 全ピンでPWM制御が可能
- **3種類のデモプログラム**:
  - LED動作テスト
  - 波形アニメーション
  - セルオートマトン

## ハードウェア

### 回路図

`schematic/` フォルダにKiCadプロジェクトファイルが含まれています：

- `Arduino_LED_Shield.kicad_sch` - 回路図
- `Arduino_LED_Shield.kicad_pcb` - PCBレイアウト
- `Arduino_LED_Shield.pdf` - 回路図PDF
- `bom.xls` - 部品表（BOM）
- `CPL.xlsx` - 部品配置リスト

### LED配置

- **D0-D13**: 14個のLED（波形表示用）
- **A0-A5**: 6個のLED（メーター/カウンター表示用）

### 対応ボード

- Arduino UNO R3
- Arduino UNO R4 Minima
- Arduino UNO R4 WiFi

## サンプルプログラム

### 1. LED_test
基本的な動作確認用プログラム

**機能:**
- 全LED一斉点滅
- シーケンシャル点灯（順次点灯）

**用途:**
- 基板の動作確認
- 各LEDの配線チェック

**詳細記事:**
- [Arduino全PINデバッグ - 20個のLEDで全ピン動作確認](https://electwork.net/posts/arduino-led-shield/)

```arduino
void loop() {
  for(int i = 0; i < 3; i++) {
    blinkAllLEDs();       // 全LED点滅
    sequentialBlink();    // 順次点灯
  }
}
```

### 2. LED_wave
3パターンのPWM波形アニメーション

**パターン（10秒ごとに自動切り替え）:**
1. **Deep Ocean** - ゆったりとした大きな波
2. **Cyber Pulse** - 高速で細かい逆方向の波
3. **Sonar Ripple** - 中心から外へ広がる波

**特徴:**
- ソフトウェアPWM実装
- ガンマ補正による滑らかな輝度変化
- A0-A5ピンでリアルタイムメーター表示

**詳細記事:**
- [ArduinoでソフトウェアPWM波形アニメーション - 3パターンのLEDエフェクト](https://electwork.net/posts/arduino-software-pwm-wave/)

```arduino
// パターンは10秒ごとに自動切り替え
int pattern = (millis() / 10000) % 3;
```

### 3. cellular-automaton
セルオートマトンによる生命的パターン生成

**ルール（自動切り替え）:**
- **Rule 30** - カオス的パターン
- **Rule 90** - フラクタル構造
- **Rule 110** - 複雑な振る舞い

**機能:**
- D0-D13: セルの状態表示
- A0-A5: 世代カウンターのバイナリ表示
- 64世代ごとにルール切り替え
- ルール切り替え時に5秒間の点滅演出

**詳細記事:**
- [Arduinoでセルオートマトン - Rule 30/90/110の視覚化](https://electwork.net/posts/arduino-cellular-automaton/)

```arduino
int rules[] = { 30, 90, 110 }; 
// 各ルールを64世代ずつ実行
```

## セットアップ

### 1. ハードウェアの準備

1. PCBを製造（Gerberファイルを使用）
2. 部品表（BOM）を参照して部品を実装
3. Arduino UNOに取り付け

### 2. ソフトウェアのアップロード

```bash
# Arduino IDEで任意のスケッチを開く
Arduino_Schetch/LED_test/LED_test.ino
Arduino_Schetch/LED_wave/LED_wave.ino
Arduino_Schetch/cellular-automaton/cellular-automaton.ino

# ボードを選択: Arduino UNO R3/R4
# ポートを選択
# アップロード
```

## 回路設計について

### KiCadプロジェクト

プロジェクトファイルは `schematic/` フォルダに配置されています。

**ファイル構成:**
- `.kicad_pro` - プロジェクト設定
- `.kicad_sch` - 回路図
- `.kicad_pcb` - PCBレイアウト
- `.kicad_prl` - プロジェクトローカル設定

### 製造データ

製造用のGerberファイルとドキュメント：
- `bom.xls` - 部品表
- `CPL.xlsx` - 部品配置リスト（SMD実装用）
- `Arduino_LED_Shield.pdf` - 回路図PDF
- `Arduino_LED_Shield.zip` - Gerberファイル一式

## 技術的詳細

### ソフトウェアPWM

Arduino UNOはハードウェアPWMピンが限られているため、全20ピンでソフトウェアPWMを実装しています。

```cpp
unsigned long pwmStart = micros();
while (micros() - pwmStart < 5000) { // 5ms PWMサイクル
  int pwmCycle = micros() & 0xFF; // 0-255
  digitalWrite(pin, (brightness > pwmCycle) ? HIGH : LOW);
}
```

### ガンマ補正

LEDの視覚的な輝度を改善するため、2乗カーブでガンマ補正を適用：

```cpp
int corrected = (brightness * brightness) / 255;
```

### セルオートマトンアルゴリズム

8通りの近傍パターンをルール番号のビット列で判定：

```cpp
int pattern = (left << 2) | (center << 1) | right; // 0-7
nextState = (ruleNumber >> pattern) & 1;
```

## カスタマイズ

### 速度調整

```cpp
delay(120);  // LED_waveやcellular-automatonの更新間隔
```

### パターン追加

新しい波形パターンやセルオートマトンのルールを追加可能：

```cpp
// LED_wave.inoに新しいパターンを追加
if (pattern == 3) {
  // 新しいアニメーション
}

// cellular-automaton.inoに新しいルールを追加
int rules[] = { 30, 90, 110, 184 }; // Rule 184を追加
```

## 必要な材料

- Arduino UNO R3/R4
- LED x20
- 抵抗（LED用）
- カスタムシールドPCB

詳細は `schematic/bom.xls` を参照してください。

## ライセンス

MITライセンス

## 参考資料

### 解説記事

- [Arduino全PINデバッグ - 20個のLEDで全ピン動作確認](https://electwork.net/posts/arduino-led-shield/)
- [ArduinoでソフトウェアPWM波形アニメーション](https://electwork.net/posts/arduino-software-pwm-wave/)
- [Arduinoでセルオートマトン - Rule 30/90/110の視覚化](https://electwork.net/posts/arduino-cellular-automaton/)

### 技術資料

- [セルオートマトン - Wikipedia](https://ja.wikipedia.org/wiki/セルオートマトン)
- [Elementary Cellular Automaton](http://mathworld.wolfram.com/ElementaryCellularAutomaton.html)
- [Arduino Software PWM](https://docs.arduino.cc/)

## 開発環境

- Arduino IDE 2.x
- KiCad 7.x
- Arduino UNO R3/R4
