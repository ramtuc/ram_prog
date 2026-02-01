# ESP32 Azure Demo

ESP32からAzure Functions（HTTP Trigger）にJSONデータをPOSTするデモプロジェクト

## プロジェクト構成

```
esp32-azure-demo/
├── esp32-azure/          # ESP32 (ESP-IDF) プロジェクト
│   └── main/
│       ├── main.c
│       ├── config.h.example
│       └── CMakeLists.txt
└── azure_http/           # Azure Functions (Python) プロジェクト
    ├── function_app.py
    ├── requirements.txt
    └── README.md
```

## 必要な環境

### ESP32側
- ESP-IDF v5.x以降
- ESP32-S3または互換ボード
- WiFi接続環境

### Azure側
- Azureアカウント
- Azure Functions (Python 3.9以降)
- Azure CLI（デプロイ時）

## セットアップ

### 1. ESP32プロジェクトの設定

1. `esp32-azure/main/config.h.example` を `config.h` にコピー
   ```bash
   cd esp32-azure/main
   cp config.h.example config.h
   ```

2. `config.h` を編集し、以下の情報を設定
   - `WIFI_SSID`: WiFiのSSID
   - `WIFI_PASS`: WiFiのパスワード
   - `AZURE_URL`: Azure FunctionのURL（後で取得）

3. ビルドとフラッシュ
   ```bash
   cd esp32-azure
   idf.py build
   idf.py flash monitor
   ```

### 2. Azure Functionsのデプロイ

詳しくは [azure_http/README.md](azure_http/README.md) を参照

1. Azure Function Appの作成
2. コードのデプロイ
3. Function URLの取得
4. ESP32の `config.h` にURLを設定

## 動作確認

ESP32が正常に動作すると、15秒ごとに以下のJSONデータがAzureに送信されます：

```json
{
  "temperature": 24.8,
  "humidity": 55,
  "device_id": "ESP32-S3-Azure"
}
```

Azure Functions側でログを確認できます。

## セキュリティに関する注意

- `config.h` は `.gitignore` に含まれており、Gitにコミットされません
- WiFiパスワードやAPIキーなどの機密情報は絶対にGitHubにプッシュしないでください
- 実運用環境では、Azure Functionsの認証レベルを適切に設定してください

## ライセンス

MIT License

## 作者

[RamTuckey]
