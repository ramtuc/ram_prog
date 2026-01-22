/*
  Demo: 3-Pattern PWM Wave Animation
  Board: Arduino Uno R4 Minima / WiFi
  
  Patterns (Switch every 10 seconds):
  1. Deep Ocean (Slow, Wide wave)
  2. Cyber Pulse (Fast, Short wave)
  3. Sonar Ripple (Center-out expanding wave)
*/

const int wavePins[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
const int numWavePins = sizeof(wavePins) / sizeof(wavePins[0]);
const int meterPins[] = { A0, A1, A2, A3, A4, A5 };
const int numMeterPins = sizeof(meterPins) / sizeof(meterPins[0]);

int waveBrightness[14];
int meterBrightness[6];

void setup() {
  for (int i = 0; i < numWavePins; i++) pinMode(wavePins[i], OUTPUT);
  for (int i = 0; i < numMeterPins; i++) pinMode(meterPins[i], OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // --- 10秒ごとにパターン切り替え ---
  // 10000msで割って、3で割った余りを求める (0 -> 1 -> 2 -> 0...)
  int pattern = (currentMillis / 10000) % 3;
  
  float val = 0;
  int sampleForMeter = 0; // メーター用のサンプリング値

  // --- パターンごとの計算 ---
  for (int i = 0; i < numWavePins; i++) {
    
    if (pattern == 0) {
      // Pattern 1: Deep Ocean (ゆったり、波長長い)
      float speed = 0.002;
      float waveLen = 0.5;
      val = sin(currentMillis * speed + i * waveLen);
      sampleForMeter = waveBrightness[6]; // 中央付近をサンプリング
      
    } else if (pattern == 1) {
      // Pattern 2: Cyber Pulse (高速、波長短い、逆再生)
      float speed = -0.008; // マイナスで逆方向、数値大きく
      float waveLen = 1.2;  // 波を細かく
      val = sin(currentMillis * speed + i * waveLen);
      sampleForMeter = waveBrightness[3]; // 端の方をサンプリングして動きを出す

    } else {
      // Pattern 3: Sonar Ripple (中心から外へ)
      float speed = 0.005;
      float waveLen = 0.8;
      // 中心(6.5)からの距離を使って計算
      float dist = abs(i - 6.5); 
      // distを引くことで中心から外へ動くように見える
      val = sin(currentMillis * speed - dist * waveLen);
      sampleForMeter = waveBrightness[6]; // 中心の輝度と連動
    }

    // -1〜1 を 0〜255 に変換
    int b = (int)((val + 1.0) * 127.5);
    
    // ガンマ補正 (2乗カーブでコントラスト強調)
    waveBrightness[i] = (b * b) / 255;
  }

  // --- メーター(A0-A5)の計算 ---
  // サンプリングした明るさに応じてバーを上下させる
  int level = map(sampleForMeter, 0, 255, 0, numMeterPins);
  for (int i = 0; i < numMeterPins; i++) {
    meterBrightness[i] = (i <= level) ? 255 : 0;
  }

  // --- ソフトウェアPWM描画ループ (約5ms回す) ---
  unsigned long pwmStart = micros();
  while (micros() - pwmStart < 5000) { 
    int pwmCycle = micros() & 0xFF; // 0-255 Loop
    
    // Wave Pins Output
    for (int i = 0; i < numWavePins; i++) {
      digitalWrite(wavePins[i], (waveBrightness[i] > pwmCycle) ? HIGH : LOW);
    }
    // Meter Pins Output
    for (int i = 0; i < numMeterPins; i++) {
      digitalWrite(meterPins[i], (meterBrightness[i] > pwmCycle) ? HIGH : LOW);
    }
  }
}