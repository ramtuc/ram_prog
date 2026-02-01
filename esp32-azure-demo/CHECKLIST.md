# GitHub公開前のチェックリスト

## ✅ 完了した作業

### 機密情報の削除
- [x] ESP32の `main.c` からWiFi SSID/パスワードを削除
- [x] ESP32の `main.c` からAzure Function URLとアクセスキーを削除
- [x] `config.h.example` を作成し、設定テンプレートを提供
- [x] Azure Functions の `local.settings.json` を .gitignore に追加

### ドキュメント作成
- [x] プロジェクト全体の README.md を作成
- [x] ESP32プロジェクトの README.md を作成
- [x] Azure Functions プロジェクトの README.md を作成

### .gitignore設定
- [x] プロジェクトルートに .gitignore を作成
- [x] esp32-azure/ に .gitignore を作成
- [x] azure_http/ の .gitignore を更新

### 不要ファイルの削除
- [x] `main.py` を削除（function_app.pyと重複）
- [x] `sdkconfig` を削除（ビルド固有の設定）

### 追加ファイル
- [x] `local.settings.json.example` を作成

## 📋 GitHub公開前の最終確認事項

### 1. 機密情報の確認
```bash
# 以下のコマンドで機密情報が残っていないか確認
cd esp32-azure-demo
grep -r "password\|secret\|key" --include="*.c" --include="*.h" --include="*.py"
```

### 2. .gitignore の動作確認
```bash
git status
# config.h, local.settings.json, sdkconfig などが表示されないことを確認
```

### 3. READMEの確認
- [ ] すべてのREADMEが読みやすいか
- [ ] セットアップ手順が明確か
- [ ] ライセンス情報が含まれているか

### 4. コードの動作確認
- [ ] ESP32側のコードがビルドできるか（config.h設定後）
- [ ] Azure Functions側のコードがデプロイできるか

## 🚀 GitHubにアップロードする手順

### 1. Gitリポジトリの初期化
```bash
cd d:\git\ram_prog\esp32-azure-demo
git init
```

### 2. ファイルの追加
```bash
git add .
git status  # .gitignoreが正しく機能しているか確認
```

### 3. 初回コミット
```bash
git commit -m "Initial commit: ESP32 to Azure Functions HTTP POST demo"
```

### 4. GitHubリポジトリの作成
GitHubで新しいリポジトリを作成（例: `esp32-azure-demo`）

### 5. リモートリポジトリの設定とプッシュ
```bash
git remote add origin https://github.com/YOUR_USERNAME/esp32-azure-demo.git
git branch -M main
git push -u origin main
```

## ⚠️ 注意事項

### 絶対にコミットしてはいけないファイル
- `esp32-azure/main/config.h` （実際の設定値）
- `azure_http/local.settings.json` （実際の設定値）
- `esp32-azure/sdkconfig` （ビルド固有の設定）
- `esp32-azure/build/` （ビルド生成物）
- `.venv/`, `__pycache__/` （Python環境）

### 万が一機密情報をプッシュしてしまった場合
1. すぐにWiFiパスワード、Azure関数キーを変更
2. Git履歴から機密情報を削除（git filter-branch等）
3. GitHubリポジトリを削除して作り直す（最も確実）

## 📝 推奨される追加作業（オプション）

- [ ] LICENSE ファイルの追加（MIT、Apache 2.0など）
- [ ] GitHub Actions でCI/CDを設定
- [ ] Issues/PRテンプレートの作成
- [ ] CODE_OF_CONDUCT.md の追加
- [ ] CONTRIBUTING.md の追加
- [ ] 実際のセンサー実装例の追加
- [ ] データベース連携の例を追加

## ✨ 完成後の構造

```
esp32-azure-demo/
├── .gitignore
├── README.md
├── CHECKLIST.md （このファイル）
├── esp32-azure/
│   ├── .gitignore
│   ├── README.md
│   ├── CMakeLists.txt
│   └── main/
│       ├── CMakeLists.txt
│       ├── main.c              （機密情報削除済み）
│       └── config.h.example    （設定テンプレート）
└── azure_http/
    ├── .gitignore
    ├── README.md
    ├── function_app.py
    ├── requirements.txt
    ├── host.json
    ├── pyproject.toml
    ├── uv.lock
    └── local.settings.json.example
```
