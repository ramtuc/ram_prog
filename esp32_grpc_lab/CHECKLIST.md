# GitHub公開前のチェックリスト - esp32_grpc_lab

## ✅ 完了した作業

### 機密情報の削除
- [x] `main.c` からWiFi SSID/パスワードを削除
- [x] `main.c` からローカルIPアドレスを削除
- [x] `config.h.example` を作成し、設定テンプレートを提供

### 不要ファイルの削除
- [x] `sdkconfig` を削除（ビルド固有の設定）
- [x] `build/` ディレクトリを削除（ビルド生成物）
- [x] `__pycache__/` を削除（Pythonキャッシュ）
- [x] `.clangd` を削除（IDE固有ファイル）

### .gitignore設定
- [x] `main/config.h` を .gitignore に追加
- [x] ビルドファイルとキャッシュが除外されることを確認

### ドキュメント作成
- [x] 詳細な README.md を作成

## 📋 GitHub公開前の最終確認事項

### 1. 機密情報の確認
```bash
cd esp32_grpc_lab
# WiFiパスワードやIPアドレスが残っていないか確認
grep -r "aterm\|192.168" --include="*.c" --include="*.h" --include="*.py"
```

期待する結果: 何も表示されないこと（または config.h.example のみ）

### 2. .gitignore の動作確認
```bash
git status
```

以下のファイルが表示されないことを確認:
- `main/config.h`
- `sdkconfig`
- `build/`
- `__pycache__/`
- `.clangd`

### 3. 必要なファイルの確認
以下のファイルが含まれていることを確認:
- [x] `README.md`
- [x] `main/config.h.example`
- [x] `main/main.c`（機密情報削除済み）
- [x] `server.py`
- [x] `main/proto/sensor.proto`
- [x] `.gitignore`

### 4. READMEの確認
- [x] プロジェクトの説明が明確
- [x] セットアップ手順が詳しく記載
- [x] 使い方が説明されている
- [x] トラブルシューティング情報がある

## 🚀 GitHubにアップロードする手順

### 1. Gitリポジトリの初期化
```bash
cd D:\git\ram_prog\esp32_grpc_lab
git init
```

### 2. ファイルの追加
```bash
git add .
git status  # .gitignoreが正しく機能しているか確認
```

### 3. 初回コミット
```bash
git commit -m "Initial commit: ESP32 Protobuf/gRPC Lab"
```

### 4. GitHubリポジトリの作成
GitHubで新しいリポジトリを作成（例: `esp32-grpc-lab`）

### 5. リモートリポジトリの設定とプッシュ
```bash
git remote add origin https://github.com/YOUR_USERNAME/esp32-grpc-lab.git
git branch -M main
git push -u origin main

# サブモジュールもプッシュ
git push --recurse-submodules=on-demand
```

**注意**: `.gitmodules` ファイルが自動的に作成されており、これによりサブモジュールの情報が管理されます。

## ⚠️ 注意事項

### 絶対にコミットしてはいけないファイル
- `main/config.h` （実際の設定値）
- `sdkconfig` （ビルド固有の設定）
- `build/` （ビルド生成物）
- `__pycache__/` （Pythonキャッシュ）
- `.clangd` （IDE設定）

これらは `.gitignore` で除外されています。

### 万が一機密情報をプッシュしてしまった場合
1. すぐにWiFiパスワードを変更
2. Git履歴から機密情報を削除（git filter-branch等）
3. GitHubリポジトリを削除して作り直す（最も確実）

## 📝 推奨される追加作業（オプション）

- [ ] LICENSE ファイルの追加（MIT、Apache 2.0など）
- [ ] GitHub Actions でビルドチェックを設定
- [ ] 実際のセンサー（DHT22など）を使った実装例を追加
- [ ] gRPC本体（HTTP/2ベース）への移行例を追加
- [ ] データベースへの保存例を追加
- [ ] 複数のESP32からのデータ集約例を追加
- [ ] requirements.txt を追加（Pythonサーバー用）

## ✨ 完成後の構造

```
esp32_grpc_lab/
├── .gitignore
├── README.md
├── CHECKLIST.md
├── CMakeLists.txt
├── main/
│   ├── CMakeLists.txt
│   ├── main.c              （機密情報削除済み）
│   ├── config.h.example    （設定テンプレート）
│   └── proto/
│       ├── sensor.proto
│       └── sensor.options
├── components/
│   └── nanopb/
├── server.py
└── sensor_pb2.py
```

## 🔍 最終チェック

プッシュ前に以下を実行してください:

```bash
# 1. 機密情報が含まれていないか確認
git grep -i "password\|ssid\|192.168"

# 2. 除外されるべきファイルが含まれていないか確認
git ls-files | grep -E "(sdkconfig|config.h|build/|__pycache__)"

# 3. 必要なファイルが含まれているか確認
git ls-files | grep -E "(README.md|config.h.example|.gitignore)"
```

すべて問題なければ、GitHubへのプッシュを実行してください！

## 🎉 公開完了後

- リポジトリのDescriptionを設定
- Topicsタグを追加（`esp32`, `protobuf`, `nanopb`, `iot`, `grpc`など）
- GitHubのREADMEが正しく表示されているか確認
- Issues機能を有効にする（フィードバック受付用）
