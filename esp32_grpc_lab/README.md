# ESP32 gRPC Lab

ESP32から Protocol Buffers (Nanopb) でシリアライズしたセンサーデータをHTTP経由でPythonサーバーに送信するデモプロジェクト

## 概要

このプロジェクトは、ESP32（ESP-IDF使用）とPythonサーバー間で、gRPC的なProtocol Buffersフォーマットを使ったデータ通信を行うサンプルです。

### 特徴

- **Protocol Buffers (Protobuf)**: Nanopbライブラリを使用してESP32でProtobufをシリアライズ
- **HTTP通信**: gRPC本体ではなく、HTTP POSTでバイナリデータを送信
- **WiFi接続**: ESP32からWiFi経由でローカルサーバーまたはクラウドサーバーと通信
- **Flaskサーバー**: Pythonで簡易的なデータ受信サーバーを実装

## プロジェクト構成

```
esp32_grpc_lab/
├── main/
│   ├── main.c              # ESP32メインコード
│   ├── config.h.example    # 設定ファイルのサンプル
│   └── proto/
│       ├── sensor.proto    # Protobuf定義ファイル
│       └── sensor.options  # Nanopbオプション
├── components/
│   └── nanopb/             # Nanopbライブラリ
├── server.py               # Pythonデータ受信サーバー
├── sensor_pb2.py           # Pythonで生成されたProtobufコード
├── CMakeLists.txt
└── README.md
```

## 必要な環境

### ESP32側
- ESP-IDF v5.x以降
- ESP32またはESP32-S3ボード
- WiFi接続環境

### Pythonサーバー側
- Python 3.7以降
- Flask
- protobuf

## セットアップ

### 1. Pythonサーバーの準備

```bash
# 必要なパッケージをインストール
pip install flask protobuf

# Protobufファイルから Python コードを生成（既に sensor_pb2.py があれば不要）
protoc --python_out=. main/proto/sensor.proto
```

### 2. ESP32プロジェクトの設定

#### 設定ファイルの作成

```bash
cd main
cp config.h.example config.h
```

#### config.h の編集

```c
#define WIFI_SSID      "your-wifi-ssid"      // WiFiのSSID
#define WIFI_PASS      "your-wifi-password"  // WiFiのパスワード
#define SERVER_URL     "http://192.168.x.x:5000/data"  // サーバーのURL
```

**SERVER_URL の設定方法**:
- ローカルPC上で `server.py` を実行する場合: `http://192.168.x.x:5000/data`（PCのローカルIPアドレスを使用）
- クラウドサーバーの場合: `https://your-server.com/data`

### 3. Nanopbコンポーネントの準備

このプロジェクトはNanopbライブラリをGitサブモジュールとして使用しています。

#### リポジトリをクローンした場合

```bash
# サブモジュールを初期化して取得
git submodule update --init --recursive
```

#### 既にクローン済みの場合

サブモジュールが既に取得されているか確認：
```bash
ls components/nanopb
```

空の場合は以下を実行：
```bash
git submodule update --init --recursive
```

### 4. ビルドとフラッシュ

```bash
# ESP32プロジェクトをビルド
idf.py build

# ESP32にフラッシュ
idf.py flash

# シリアルモニタで動作確認
idf.py monitor
```

または一括実行:
```bash
idf.py build flash monitor
```

## 使い方

### 1. Pythonサーバーの起動

```bash
python server.py
```

以下のようなメッセージが表示されます:
```
Starting gRPC Local Receiver...
 * Running on all addresses (0.0.0.0)
 * Running on http://127.0.0.1:5000
```

### 2. ESP32の起動

ESP32がWiFiに接続し、30秒ごとに以下の形式のデータを送信します:

```
Device ID: ESP32-S3-GRPC-LAB
Temperature: 24.5 °C
Humidity: 55 %
```

### 3. サーバー側でのデータ確認

Pythonサーバーのコンソールに以下のような出力が表示されます:

```
--- Received Binary (Size: 28 bytes) ---
Hex: 0a 13 45 53 50 33 32 2d 53 33 2d 47 52 50 43 2d 4c 41 42 15 00 00 c4 41 18 37
Device ID: ESP32-S3-GRPC-LAB
Temp:      24.5 °C
Humidity:  55 %
```

## データフォーマット

### Protobuf定義 (sensor.proto)

```protobuf
syntax = "proto3";

message SensorData {
    string device_id = 1;
    float temperature = 2;
    int32 humidity = 3;
}
```

### バイナリデータの流れ

1. **ESP32側**: Nanopbで `SensorData` をバイナリにシリアライズ
2. **HTTP POST**: `Content-Type: application/x-protobuf` でバイナリデータを送信
3. **Pythonサーバー側**: protobufライブラリでデシリアライズして元のデータを復元

## トラブルシューティング

### WiFiに接続できない
- SSID/パスワードが正しいか確認
- 2.4GHz帯のWiFiを使用しているか確認（ESP32は5GHz非対応）

### サーバーに接続できない
- サーバーが起動しているか確認
- `SERVER_URL` のIPアドレスが正しいか確認
- PCとESP32が同じネットワーク上にあるか確認
- ファイアウォールでポート5000が開放されているか確認

### Protobufのデコードエラー
- ESP32とPythonサーバーが同じ `.proto` ファイルを使用しているか確認
- Pythonの `sensor_pb2.py` が最新の `.proto` から生成されているか確認

## カスタマイズ

### 送信間隔の変更

`main/main.c` の以下の行を編集:
```c
vTaskDelay(pdMS_TO_TICKS(30000)); // 30秒 → 任意のミリ秒に変更
```

### センサーデータの追加

実際のセンサーを接続する場合:
```c
// 例: 実際のセンサーから読み取り
float temperature = read_temperature_sensor();
int32_t humidity = read_humidity_sensor();
```

### Protobuf定義の変更

1. `main/proto/sensor.proto` を編集
2. ESP32側のコードを再生成（ビルド時に自動）
3. Pythonコードを再生成:
   ```bash
   protoc --python_out=. main/proto/sensor.proto
   ```

## セキュリティに関する注意

⚠️ **重要**: `config.h` には機密情報（WiFiパスワード、サーバーURL）が含まれるため、Gitにコミットしないでください。

- `config.h` は `.gitignore` に含まれています
- `config.h.example` のみをGitで管理します
- 本番環境ではHTTPSを使用してください

## Gitサブモジュールについて

このプロジェクトは以下の外部ライブラリをGitサブモジュールとして使用しています：

- **nanopb**: Protocol Buffersのコンパクトな実装
  - リポジトリ: https://github.com/nanopb/nanopb
  - パス: `components/nanopb/`

サブモジュールを含めてクローンする場合：
```bash
git clone --recurse-submodules https://github.com/YOUR_USERNAME/esp32-grpc-lab.git
```

## 参考リンク

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [Nanopb - Protocol Buffers for Embedded Systems](https://jpa.kapsi.fi/nanopb/)
- [Protocol Buffers Documentation](https://protobuf.dev/)

## ライセンス

MIT License

## 作者

[あなたの名前]
