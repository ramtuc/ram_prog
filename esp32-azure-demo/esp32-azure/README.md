# ESP32 Azure HTTP Post Demo

ESP32からAzure FunctionsにHTTPS経由でJSONデータをPOSTするサンプルプロジェクト

## 概要

このプロジェクトは、ESP32（ESP-IDF使用）からWiFi経由でAzure FunctionsにセンサーデータをHTTP POSTで送信するデモです。

## 特徴

- WiFi接続
- HTTPS通信（TLS/SSL）
- JSON形式でのデータ送信
- Azure Functions との連携
- 15秒間隔での自動送信

## 必要な環境

- ESP-IDF v5.x以降
- ESP32-S3または互換性のあるESP32ボード
- WiFiアクセスポイント
- Azure Functionsアカウント

## セットアップ

### 1. 設定ファイルの準備

```bash
cd main
cp config.h.example config.h
```

### 2. config.h の編集

```c
#define WIFI_SSID      "your-wifi-ssid"      // WiFiのSSID
#define WIFI_PASS      "your-wifi-password"  // WiFiのパスワード
#define AZURE_URL      "https://your-function-app.azurewebsites.net/api/esp_data?code=YOUR_KEY"
```

Azure URLは、Azure ポータルから以下の手順で取得：
1. Function App → 関数 → esp_data を選択
2. 「関数の URL の取得」をクリック
3. URLをコピーして `AZURE_URL` に設定

### 3. ビルドとフラッシュ

```bash
# プロジェクトのビルド
idf.py build

# ESP32にフラッシュ
idf.py flash

# シリアルモニタで動作確認
idf.py monitor
```

または一括実行：
```bash
idf.py build flash monitor
```

## 送信データフォーマット

ESP32から以下の形式のJSONデータを送信します：

```json
{
  "temperature": 24.8,
  "humidity": 55,
  "device_id": "ESP32-S3-Azure"
}
```

## 動作確認

シリアルモニタに以下のようなログが表示されれば正常に動作しています：

```
I (xxx) AZURE_HTTPS_POST: Waiting for Wi-Fi connection...
I (xxx) AZURE_HTTPS_POST: Connected!
I (xxx) AZURE_HTTPS_POST: Azure Success! Status = 200
```

## トラブルシューティング

### WiFiに接続できない
- SSID/パスワードが正しいか確認
- WiFiの電波強度を確認
- 2.4GHz帯を使用しているか確認（5GHz非対応）

### Azureに接続できない
- Azure URLが正しいか確認（`?code=...` も含む）
- Function Appが起動しているか確認
- HTTPSポート443が開放されているか確認

### 証明書エラー
- `esp_crt_bundle_attach` を使用して証明書バンドルを有効化
- テスト時は `skip_cert_common_name_check = true` で証明書チェックをスキップ可能（本番では非推奨）

## カスタマイズ

### 送信間隔の変更

main.c の以下の行を編集：
```c
vTaskDelay(pdMS_TO_TICKS(15000)); // 15秒 → 任意のミリ秒に変更
```

### センサーデータの追加

実際のセンサーを接続する場合：
```c
// 例：DHT22センサー
float temp = dht_read_temperature();
float humi = dht_read_humidity();

cJSON_AddNumberToObject(root, "temperature", temp);
cJSON_AddNumberToObject(root, "humidity", humi);
```

### デバイスIDの変更

```c
cJSON_AddStringToObject(root, "device_id", "YOUR-DEVICE-NAME");
```

## プロジェクト構成

```
esp32-azure/
├── CMakeLists.txt
├── main/
│   ├── CMakeLists.txt
│   ├── main.c              # メインコード
│   ├── config.h.example    # 設定ファイルのテンプレート
│   └── config.h            # 実際の設定（.gitignoreで除外）
└── README.md
```

## セキュリティに関する注意

⚠️ **重要**: `config.h` には機密情報が含まれるため、Gitにコミットしないでください。

- `config.h` は `.gitignore` に含まれています
- `config.h.example` のみをGitで管理します
- 実運用では環境変数やSecure Storageの使用を検討してください

## ライセンス

MIT License

## 関連リンク

- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [Azure Functions Documentation](https://docs.microsoft.com/azure/azure-functions/)
