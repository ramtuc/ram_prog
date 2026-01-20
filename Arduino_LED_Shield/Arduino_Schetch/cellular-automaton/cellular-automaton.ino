/*
  Demo: 3-Rule Cellular Automaton (30 -> 90 -> 110)
  Board: Arduino Uno R4
  
  D0-D13: Automaton Display
  A0-A5 : Generation Counter
*/

const int cellPins[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
const int numCells = sizeof(cellPins) / sizeof(cellPins[0]);
const int countPins[] = { A0, A1, A2, A3, A4, A5 };
const int numCountBits = sizeof(countPins) / sizeof(countPins[0]);

byte currentCells[14];
byte nextCells[14];

int generation = 0;
const int maxGenerations = 64; // 各ルールの実行時間（世代数）

// 順番に表示したいルールのリスト
int rules[] = { 30, 90, 110 }; 
int currentRuleIndex = 0;      // 現在どのルールを使っているか

void setup() {
  for (int i = 0; i < numCells; i++) pinMode(cellPins[i], OUTPUT);
  for (int i = 0; i < numCountBits; i++) pinMode(countPins[i], OUTPUT);
  resetWorld();
}

void loop() {
  // 1. 世界の表示
  for (int i = 0; i < numCells; i++) digitalWrite(cellPins[i], currentCells[i]);

  // 2. カウンター表示
  for (int i = 0; i < numCountBits; i++) digitalWrite(countPins[i], (generation >> i) & 1);

  delay(120); // 少しテンポアップ

  // 3. 次の世代を計算（現在のルール番号を使用）
  calculateNextGeneration(rules[currentRuleIndex]);

  // 4. 世代更新と切り替え判定
  generation++;
  if (generation >= maxGenerations) {
    finishedEffect(); // 5秒間の点滅演出
    
    // 次のルールへ切り替え
    currentRuleIndex++;
    if (currentRuleIndex >= 3) { // 3つ終わったら最初(30)に戻る
      currentRuleIndex = 0;
    }
    
    resetWorld();
  }
}

// 汎用計算ロジック：ルール番号から自動でビット演算します
void calculateNextGeneration(int ruleNumber) {
  for (int i = 0; i < numCells; i++) {
    int leftIdx = (i - 1 + numCells) % numCells;
    int rightIdx = (i + 1) % numCells;
    
    byte left = currentCells[leftIdx];
    byte center = currentCells[i];
    byte right = currentCells[rightIdx];

    // 周囲3つの状態を3ビットの整数(0〜7)にする
    // 例: left=1, center=0, right=1 なら "101" = 5
    int pattern = (left << 2) | (center << 1) | right;

    // ルール番号のビットを見て、次の状態を決める
    // 例: Rule 30 (00011110) で patternが5なら、5ビット目は1なので次は1
    nextCells[i] = (ruleNumber >> pattern) & 1;
  }

  for (int i = 0; i < numCells; i++) currentCells[i] = nextCells[i];
}

void resetWorld() {
  generation = 0;
  for (int i = 0; i < numCells; i++) currentCells[i] = 0;
  currentCells[numCells / 2] = 1; // 中央一点スタート
}

void finishedEffect() {
  // 5秒間のクライマックス演出
  for(int k=0; k<10; k++){
    for (int i = 0; i < numCells; i++) digitalWrite(cellPins[i], HIGH);
    for (int i = 0; i < numCountBits; i++) digitalWrite(countPins[i], HIGH);
    delay(250);
    for (int i = 0; i < numCells; i++) digitalWrite(cellPins[i], LOW);
    for (int i = 0; i < numCountBits; i++) digitalWrite(countPins[i], LOW);
    delay(250);
  }
  delay(500);
}