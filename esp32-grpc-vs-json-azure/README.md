# ESP32 gRPC vs JSON - Azure Functions連携デモ

ESP32（ESP-IDF）からAzure FunctionsにデータをPOSTする際の、**gRPC（Protobuf）** と **JSON** の性能比較プロジェクト。

## 📁 プロジェクト構成

```
esp32-grpc-vs-json-azure/
├── README.md                    # このファイル
├── esp32_grpc_azure/            # ESP32側のプロジェクト（ESP-IDF）
│   ├── main/
│   │   ├── main.c              # メインプログラム
│   │   ├── config.h.example    # 設定テンプレート
│   │   └── proto/              # Protobufスキーマ
│   ├── components/             # ESP-IDF コンポーネント
│   └── CMakeLists.txt
└── azure-grpc-receiver/         # Azure Functions側（Python）
    ├── function_app.py         # Azure Functions エンドポイント
    ├── sensor_pb2.py           # Protobuf生成コード
    ├── requirements.txt
    └── local.settings.json.example  # ローカル設定テンプレート
```

## 🎯 プロジェクトの目的

1. **データサイズ比較**: JSON vs Protobuf のペイロードサイズ
2. **シリアライズ速度比較**: エンコード処理にかかる時間
3. **通信効率**: ネットワーク送信時間の測定

## 🚀 セットアップ

### 必要な環境

- **ESP32側**
  - ESP-IDF v5.0 以降
  - ESP32-S3 または ESP32 開発ボード
  - WiFi環境

- **Azure側**
  - Azure サブスクリプション
  - Azure Functions（Python 3.9 以降）
  - Azure Functions Core Tools（ローカル開発時）

### 1. ESP32側のセットアップ

#### 1.1 設定ファイルの準備

```bash
cd esp32_grpc_azure/main
cp config.h.example config.h
```

#### 1.2 config.h の編集

```c
#define WIFI_SSID      "your-wifi-ssid"       // WiFiのSSID
#define WIFI_PASS      "your-wifi-password"   // WiFiのパスワード
#define AZURE_DOMAIN   "https://your-function-app.azurewebsites.net/api"
#define FUNCTION_KEY   "your-function-key"    // Azure Functionsのキー
```

**Azure Function Keyの取得方法**:
1. Azure Portal → Function App を開く
2. 「アプリキー」→「ホストキー」を選択
3. `default` キーの値をコピー

#### 1.3 ビルドとフラッシュ

```bash
cd esp32_grpc_azure
idf.py set-target esp32s3  # または esp32
idf.py build
idf.py flash monitor
```

### 2. Azure Functions側のセットアップ

#### 2.1 ローカル開発の準備

```bash
cd azure-grpc-receiver

# 仮想環境の作成
python -m venv .venv
source .venv/bin/activate  # Windows: .venv\Scripts\activate

# 依存関係のインストール
pip install -r requirements.txt

# 設定ファイルの準備
cp local.settings.json.example local.settings.json
```

#### 2.2 ローカルでの実行

```bash
func start
```

- JSONエンドポイント: `http://localhost:7071/api/esp_data_json`
- gRPCエンドポイント: `http://localhost:7071/api/esp_data_grpc`

#### 2.3 Azureへのデプロイ

```bash
# Azureにログイン
az login

# Function Appの作成（初回のみ）
az functionapp create \
  --resource-group your-resource-group \
  --consumption-plan-location japanwest \
  --runtime python \
  --runtime-version 3.9 \
  --functions-version 4 \
  --name your-function-app-name \
  --storage-account yourstorageaccount

# デプロイ
func azure functionapp publish your-function-app-name
```

## 📊 動作確認

ESP32が正常に動作すると、以下のようなログが出力されます：

```
I (12345) GRPC_VS_JSON: [JSON] Serialize Time: 120 us
I (12346) GRPC_VS_JSON: HTTPS POST OK. Network Time: 350000 us, Status: 200, Size: 78 bytes
I (17345) GRPC_VS_JSON: [gRPC] Serialize Time: 45 us
I (17346) GRPC_VS_JSON: HTTPS POST OK. Network Time: 280000 us, Status: 200, Size: 28 bytes
```

**結果の見方**:
- **Serialize Time**: データのエンコードにかかった時間（マイクロ秒）
- **Network Time**: HTTPリクエスト全体の時間（マイクロ秒）
- **Size**: 送信データのサイズ（バイト）

典型的な結果では、Protobufは以下の利点があります：
- データサイズが約50-70%削減
- シリアライズ時間が約60-80%短縮

## 🔒 セキュリティに関する注意

- `config.h` と `local.settings.json` は `.gitignore` に含まれており、Gitにコミットされません
- WiFiパスワード、Azure Function Key などの機密情報は絶対にGitHubにプッシュしないでください
- 実運用環境では以下を検討してください：
  - Azure Key Vaultの使用
  - Managed Identity の活用
  - IPアドレス制限の設定

## 📝 Protobufスキーマ

```protobuf
syntax = "proto3";

message SensorData {
  string device_id = 1;
  float temperature = 2;
  int32 humidity = 3;
}
```

## 🛠️ トラブルシューティング

### WiFiに接続できない
- SSID/パスワードが正しいか確認
- ESP32が2.4GHz帯をサポート（5GHz非対応）

### Azureに接続できない
- `AZURE_DOMAIN` と `FUNCTION_KEY` が正しいか確認
- Function Appが起動しているか確認
- HTTPSポート443が開放されているか確認

### Protobufのデコードエラー
- `sensor_pb2.py` が最新のスキーマから生成されているか確認
- ESP32側とAzure側で同じ `.proto` ファイルを使用しているか確認

## 📚 参考資料

- [ESP-IDF ドキュメント](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [Azure Functions ドキュメント](https://docs.microsoft.com/azure/azure-functions/)
- [Protocol Buffers](https://developers.google.com/protocol-buffers)
- [Nanopb](https://github.com/nanopb/nanopb)

## 📄 ライセンス

MIT License

## 👤 作者

RamTuckey
- Website: https://electwork.net/
