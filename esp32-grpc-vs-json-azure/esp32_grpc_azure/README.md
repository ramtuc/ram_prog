# ESP32 gRPC Azure Client

ESP32（ESP-IDF）からAzure FunctionsにセンサーデータをPOSTするプロジェクト。JSON形式とProtobuf（gRPC）形式の両方をサポートし、性能を比較できます。

## 📋 機能

- WiFi接続管理
- JSON形式でのデータ送信
- Protobuf形式でのデータ送信（Nanopb使用）
- シリアライズ時間の計測
- ネットワーク送信時間の計測
- HTTPS通信（Azure証明書バンドル使用）

## 🛠️ 必要な環境

- **ESP-IDF**: v5.0 以降
- **ハードウェア**: ESP32-S3 または ESP32
- **開発環境**: VS Code + ESP-IDF Extension（推奨）

## 🚀 セットアップ

### 1. 設定ファイルの作成

```bash
cd main
cp config.h.example config.h
```

### 2. config.h の編集

```c
#define WIFI_SSID      "your-wifi-ssid"       // WiFiのSSID
#define WIFI_PASS      "your-wifi-password"   // WiFiのパスワード
#define AZURE_DOMAIN   "https://your-app.azurewebsites.net/api"
#define FUNCTION_KEY   "your-function-key"    // Azure Functionsのキー
```

**重要**: `config.h` は `.gitignore` に含まれており、Gitにコミットされません。

### 3. ビルド

```bash
# ターゲットの設定（初回のみ）
idf.py set-target esp32s3  # または esp32

# ビルド
idf.py build
```

### 4. フラッシュと実行

```bash
# フラッシュしてモニター起動
idf.py flash monitor

# または個別に
idf.py flash
idf.py monitor
```

## 📊 動作確認

正常に動作すると、以下のようなログが出力されます：

```
I (5234) GRPC_VS_JSON: wifi_init_sta finished.
I (5456) GRPC_VS_JSON: got ip:192.168.1.100
I (5457) GRPC_VS_JSON: [JSON] Serialize Time: 120 us
I (5812) GRPC_VS_JSON: HTTPS POST OK. Network Time: 350000 us, Status: 200, Size: 78 bytes
I (10457) GRPC_VS_JSON: [gRPC] Serialize Time: 45 us
I (10723) GRPC_VS_JSON: HTTPS POST OK. Network Time: 280000 us, Status: 200, Size: 28 bytes
```

### 送信サイクル

1. JSON送信 → 5秒待機
2. Protobuf送信 → 10秒待機
3. 繰り返し

## 🔧 プロジェクト構成

```
esp32_grpc_azure/
├── CMakeLists.txt
├── main/
│   ├── CMakeLists.txt
│   ├── main.c                 # メインプログラム
│   ├── config.h.example       # 設定テンプレート
│   ├── config.h               # 実際の設定（.gitignore）
│   ├── certs/
│   │   └── azure_root_ca.pem  # Azure証明書
│   └── proto/
│       └── sensor.proto       # Protobufスキーマ
└── components/
    └── nanopb/                # Nanopbライブラリ
```

## 📝 Protobufスキーマ

```protobuf
syntax = "proto3";

message SensorData {
  string device_id = 1;
  float temperature = 2;
  int32 humidity = 3;
}
```

### Nanopbコードの生成

スキーマを変更した場合：

```bash
cd main/proto
protoc --nanopb_out=. sensor.proto
```

生成されるファイル:
- `sensor.pb.h`
- `sensor.pb.c`

## 🔒 セキュリティ

### WiFi設定の保護

- `config.h` はGitリポジトリに含まれません
- 代わりに `config.h.example` をテンプレートとして提供

### HTTPS通信

- ESP-IDFの証明書バンドルを使用
- Azure証明書を `main/certs/azure_root_ca.pem` に配置

## 🐛 トラブルシューティング

### WiFiに接続できない

```
E (xxx) wifi:Failed to connect to AP
```

**解決策**:
- SSID/パスワードが正しいか確認
- WiFiが2.4GHz帯か確認（ESP32は5GHz非対応）
- WiFiの電波強度を確認

### Azureに接続できない

```
E (xxx) GRPC_VS_JSON: HTTPS POST Failed: ESP_ERR_...
```

**解決策**:
- `AZURE_DOMAIN` と `FUNCTION_KEY` が正しいか確認
- インターネット接続を確認
- Azure Function Appが起動しているか確認
- ファイアウォールでHTTPSポート443が開放されているか確認

### ビルドエラー

```
fatal error: config.h: No such file or directory
```

**解決策**:
```bash
cd main
cp config.h.example config.h
# config.h を編集
```

### Nanopbエンコードエラー

```
E (xxx) GRPC_VS_JSON: Nanopb Encoding failed!
```

**解決策**:
- `sensor.pb.h` と `sensor.pb.c` が最新か確認
- バッファサイズ（`pb_buffer`）が十分か確認

## 📊 性能比較のポイント

### データサイズ

- **JSON**: 通常 70-100 バイト
- **Protobuf**: 通常 25-35 バイト
- **削減率**: 約60-70%

### シリアライズ時間

- **JSON**: sprintf使用、通常 100-150 us
- **Protobuf**: Nanopb使用、通常 40-60 us
- **高速化**: 約60-80%

### ネットワーク時間

データサイズが小さいため、Protobufの方が若干高速（ネットワーク環境に依存）

## 📚 参考資料

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [Nanopb Documentation](https://jpa.kapsi.fi/nanopb/docs/)
- [Protocol Buffers](https://developers.google.com/protocol-buffers)

## 💡 カスタマイズ

### 送信間隔の変更

`main.c` の以下の部分を編集：

```c
vTaskDelay(pdMS_TO_TICKS(5000));  // JSON後の待機時間（ミリ秒）
// ...
vTaskDelay(pdMS_TO_TICKS(10000)); // Protobuf後の待機時間（ミリ秒）
```

### センサーデータの変更

ダミーデータの部分を実際のセンサー読み取りに置き換え：

```c
float temp = 25.5f;  // ← ここを実際のセンサー読み取りに
int hum = 60;        // ← ここを実際のセンサー読み取りに
```

## 👤 作者

RamTuckey
- Website: https://electwork.net/
