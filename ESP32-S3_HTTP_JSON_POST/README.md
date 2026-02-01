# ESP32-S3 HTTP JSON POST Sample

ESP32-S3からHTTP経由でJSON形式のセンサーデータをPOSTし、FastAPIサーバーで受信するサンプルプロジェクトです。

## プロジェクト構成

```
ESP32-S3_HTTP_JSON_POST/
├── esp32_http_json_sample/   # ESP32-S3クライアント（ESP-IDF）
└── server_json/              # FastAPIサーバー（Python）
```

## 機能概要

- **ESP32-S3（クライアント側）**
  - Wi-Fiに接続
  - 10秒ごとにセンサーデータ（温度、湿度、デバイスID）をJSON形式でPOST
  - ESP-IDF（FreeRTOS）で実装

- **FastAPIサーバー（サーバー側）**
  - HTTP POSTリクエストを受信
  - JSON形式のセンサーデータを解析して表示
  - Pythonで実装

## 必要な環境

### ESP32-S3側

- ESP-IDF v5.0以降
- ESP32-S3開発ボード
- Wi-Fi環境

### サーバー側

- Python 3.8以降
- uv（Pythonパッケージマネージャー）

## セットアップ

### 1. サーバーの起動

```bash
cd server_json
uv run main.py
```

サーバーは `http://0.0.0.0:8000` でリッスンを開始します。

### 2. ESP32-S3の設定

`esp32_http_json_sample/main/main.c` を編集して、以下の設定を環境に合わせて変更します：

```c
#define WIFI_SSID      "your-wifi-ssid"      // Wi-FiのSSID
#define WIFI_PASS      "your-wifi-password"  // Wi-Fiのパスワード
#define WEB_URL        "http://192.168.1.100:8000/api/data"  // サーバーのURL
```

### 3. ESP32-S3のビルドと書き込み

```bash
cd esp32_http_json_sample
idf.py build
idf.py -p COM3 flash monitor  # COM3はお使いのポートに合わせて変更
```

## データフォーマット

### POSTリクエスト

- **URL**: `http://[サーバーIP]:8000/api/data`
- **Method**: POST
- **Content-Type**: application/json

**ボディ例:**
```json
{
  "temperature": 26.54,
  "humidity": 60,
  "device_id": "ESP32-S3-001"
}
```

### レスポンス

```json
{
  "status": "success",
  "message": "Data received"
}
```

## 動作確認

1. サーバーを起動
2. ESP32-S3を起動してシリアルモニタで確認
3. サーバー側のコンソールに受信データが表示される

```
--- 届いたデータ ---
デバイスID: ESP32-S3-001
温度: 26.54 ℃
湿度: 60 %
```

## 使用ライブラリ

### ESP32-S3
- ESP-IDF標準ライブラリ
- esp_http_client
- cJSON
- FreeRTOS

### サーバー
- FastAPI
- Uvicorn
- Pydantic

## カスタマイズ

- センサーデータの種類を追加する場合は、`main.c`と`main.py`の両方でデータモデルを変更してください
- 送信間隔を変更する場合は、`main.c`の`vTaskDelay()`の値を調整してください
- サーバーのポート番号を変更する場合は、`main.py`と`main.c`の両方で変更してください

## 参考資料

### 解説記事

- [ESP32-S3でESP-IDFを使ったHTTP JSON POST通信 - センサーデータ送信の実装](https://electwork.net/posts/esp32-s3-esp-idf-http-json-post/)

### 技術資料

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)
- [ESP HTTP Client](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/protocols/esp_http_client.html)
- [FastAPI Documentation](https://fastapi.tiangolo.com/)
- [cJSON Library](https://github.com/DaveGamble/cJSON)

## ライセンス

MITライセンス

## 開発環境

- ESP-IDF v5.0+
- Python 3.8+
- Windows/Linux/macOS
