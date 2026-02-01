# Azure Functions - ESP32データ受信API

ESP32から送信されるセンサーデータ（JSON）を受信するAzure Functions HTTPトリガー

## 機能

- ESP32からのHTTP POSTリクエストを受信
- JSONデータ（温度、湿度、デバイスID）を処理
- ログに記録

## ローカル開発

### 必要な環境

- Python 3.9以降
- Azure Functions Core Tools v4
- Azure CLI (デプロイ時)

### セットアップ

1. 仮想環境の作成と有効化
   ```bash
   python -m venv .venv
   # Windows
   .venv\Scripts\activate
   # macOS/Linux
   source .venv/bin/activate
   ```

2. 依存パッケージのインストール
   ```bash
   pip install -r requirements.txt
   ```

3. ローカル実行
   ```bash
   func start
   ```

   ローカルURLが表示されます（例: `http://localhost:7071/api/esp_data`）

## Azureへのデプロイ

### 1. Function Appの作成

```bash
# リソースグループの作成
az group create --name esp32-demo-rg --location japanwest

# ストレージアカウントの作成
az storage account create --name esp32demostorage --location japanwest --resource-group esp32-demo-rg --sku Standard_LRS

# Function Appの作成
az functionapp create --resource-group esp32-demo-rg --consumption-plan-location japanwest --runtime python --runtime-version 3.9 --functions-version 4 --name esp32-function-app --storage-account esp32demostorage
```

### 2. コードのデプロイ

```bash
func azure functionapp publish esp32-function-app
```

### 3. Function URLの取得

Azure ポータルで以下の手順で取得：
1. Function App を開く
2. 「関数」→「esp_data」を選択
3. 「関数の URL の取得」をクリック
4. URLとアクセスキーをコピー

このURLをESP32の `config.h` に設定します。

## データフォーマット

### リクエスト（ESP32から）

```json
{
  "temperature": 24.8,
  "humidity": 55,
  "device_id": "ESP32-S3-Azure"
}
```

### レスポンス（正常時）

```
Success: Received data from ESP32-S3-Azure
```
ステータスコード: 200

### レスポンス（エラー時）

```
Invalid JSON
```
ステータスコード: 400

## ログの確認

Azure ポータルで以下の手順で確認：
1. Function App を開く
2. 「ログストリーム」を選択
3. リアルタイムでログが表示されます

または、Application Insights を有効にして詳細な監視が可能です。

## セキュリティ設定

- デフォルトでは `FUNCTION` レベルの認証が有効
- URLにアクセスキー（`?code=...`）が必要
- 実運用環境では、適切な認証方法を検討してください：
  - Azure AD認証
  - IPアドレス制限
  - VNet統合

## カスタマイズ

### データベースへの保存

受信したデータをCosmos DBやAzure Storage Tableに保存する場合：

```python
# Cosmos DB バインディングの例
@app.route(route="esp_data")
@app.cosmos_db_output(arg_name="outputDocument", database_name="esp32db", collection_name="sensor_data", connection_string_setting="CosmosDbConnectionString")
def http_trigger(req: func.HttpRequest, outputDocument: func.Out[func.Document]) -> func.HttpResponse:
    req_body = req.get_json()
    outputDocument.set(func.Document.from_dict(req_body))
    return func.HttpResponse("Data saved", status_code=200)
```

### アラート通知

特定の条件でアラートを送る場合：

```python
if temp > 30:
    # SendGridやLogic Appsで通知
    send_alert(f"High temperature: {temp}°C")
```

## トラブルシューティング

### ESP32から接続できない

1. Function URLが正しいか確認
2. アクセスキーが含まれているか確認
3. Azure ポータルでFunction Appが実行中か確認
4. ログで詳細なエラーを確認

### デプロイエラー

```bash
# Azure CLIで再ログイン
az login

# サブスクリプションの確認
az account show
```

## ライセンス

MIT License
