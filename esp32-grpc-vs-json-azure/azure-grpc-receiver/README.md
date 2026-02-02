# Azure Functions - gRPC/JSON レシーバー

ESP32から送信されるセンサーデータを受信するAzure Functions。JSON形式とProtobuf（gRPC）形式の両方に対応。

## 📋 機能

### エンドポイント

1. **`/api/esp_data_json`** - JSON形式のデータを受信
2. **`/api/esp_data_grpc`** - Protobuf形式のデータを受信

## 🚀 ローカルでの開発

### 1. 環境のセットアップ

```bash
# 仮想環境の作成と有効化
python -m venv .venv
source .venv/bin/activate  # Windows: .venv\Scripts\activate

# 依存関係のインストール
pip install -r requirements.txt
```

### 2. 設定ファイルの準備

```bash
cp local.settings.json.example local.settings.json
```

### 3. ローカルでの実行

```bash
func start
```

実行後、以下のURLでアクセス可能：
- JSON: `http://localhost:7071/api/esp_data_json`
- gRPC: `http://localhost:7071/api/esp_data_grpc`

### 4. テスト送信

#### JSONテスト

```bash
curl -X POST http://localhost:7071/api/esp_data_json \
  -H "Content-Type: application/json" \
  -d '{"device_id":"TEST","temperature":25.5,"humidity":60}'
```

#### Protobufテスト（Python）

```python
import requests
import sensor_pb2

# Protobufメッセージの作成
msg = sensor_pb2.SensorData()
msg.device_id = "TEST"
msg.temperature = 25.5
msg.humidity = 60

# バイナリ化して送信
response = requests.post(
    'http://localhost:7071/api/esp_data_grpc',
    headers={'Content-Type': 'application/x-protobuf'},
    data=msg.SerializeToString()
)
print(response.text)
```

## ☁️ Azureへのデプロイ

### 1. Azure CLIでのデプロイ

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

### 2. デプロイ後の確認

Azure Portalで以下を確認：
1. Function App → 関数 → `esp_data_json` / `esp_data_grpc`
2. 「関数のURLの取得」でエンドポイントURLを確認
3. 「監視」→「ログストリーム」でリアルタイムログを確認

## 📊 ログの確認

### ローカル開発時

ターミナルに直接出力されます。

### Azure環境

1. Azure Portal → Function App
2. 「監視」→「ログストリーム」
3. または Application Insights を有効化して詳細な分析

## 🔧 依存関係

- `azure-functions`: Azure Functions ランタイム
- `protobuf`: Protobuf シリアライゼーション

## 📝 Protobufスキーマの更新

`.proto` ファイルを変更した場合：

```bash
# sensor_pb2.py の再生成
protoc --python_out=. sensor.proto
```

## 🔒 セキュリティ設定

### 認証レベル

デフォルトでは `FUNCTION` レベルの認証が有効（URLにアクセスキーが必要）

```python
@app.route(route="esp_data_json", auth_level=func.AuthLevel.FUNCTION)
```

### 実運用での推奨設定

- Azure AD認証の使用
- IPアドレス制限の設定
- VNet統合
- Azure Key Vault での機密情報管理

## 🐛 トラブルシューティング

### Protobufデコードエラー

```
Decode Error: ...
```

**原因**:
- ESP32側と異なるスキーマを使用している
- データが破損している

**解決策**:
- `sensor_pb2.py` を最新の `.proto` から再生成
- ESP32側のNanopbバージョンを確認

### Function が起動しない

**原因**:
- Python バージョンの不一致
- 依存関係が正しくインストールされていない

**解決策**:
```bash
pip install --upgrade -r requirements.txt
```

## 📚 参考資料

- [Azure Functions Python Developer Guide](https://docs.microsoft.com/azure/azure-functions/functions-reference-python)
- [Protocol Buffers Python Tutorial](https://developers.google.com/protocol-buffers/docs/pythontutorial)

## 👤 作者

RamTuckey
- Website: https://electwork.net/
