using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using Microsoft.Playwright;
using Newtonsoft.Json.Linq;
using OxyPlot;
using OxyPlot.Axes;
using OxyPlot.Series;

namespace BrightData_WPF
{
    public partial class MainWindow : Window
    {
        // データバインディング用プロパティ
        public ObservableCollection<RepositoryData> Repositories { get; set; } = new ObservableCollection<RepositoryData>();
        public PlotModel MyPlotModel { get; set; }
        public List<string> RepoNames { get; set; } = new List<string>();

        // ▼ Bright Data設定 (ISP Proxy)
        // 以下の値は必ずご自身の認証情報に置き換えてください
        private const string ProxyHost = "brd.superproxy.io:33335";
        private const string Username = "brd-customer-hl_YOUR_CUSTOMER_ID-zone-isp_proxy1";
        private const string Password = "YOUR_BRIGHTDATA_PASSWORD";

        // ▼ Gemini APIキー (ご自身のキーを入力してください)
        private const string GeminiApiKey = "YOUR_GEMINI_API_KEY";

        public MainWindow()
        {
            InitializeComponent();
            RepoGrid.ItemsSource = Repositories;

            // グラフ初期設定
            MyPlotModel = new PlotModel { Title = "" };

            // Y軸（リポジトリ名）
            MyPlotModel.Axes.Add(new CategoryAxis
            {
                Position = AxisPosition.Left,
                Key = "RepoAxis",
                FontSize = 11,
                IsZoomEnabled = false
            });

            // X軸（スター数）
            MyPlotModel.Axes.Add(new LinearAxis
            {
                Position = AxisPosition.Bottom,
                Minimum = 0,
                Title = "Stars",
                FontSize = 10
            });

            DataContext = this;
        }

        // --- ボタン1: 接続テスト ---
        private async void TestButton_Click(object sender, RoutedEventArgs e)
        {
            TestButton.IsEnabled = false;
            Log("【接続テスト】開始...");
            try
            {
                await RunTestConnection();
            }
            catch (Exception ex)
            {
                Log($"エラー: {ex.Message}");
            }
            finally
            {
                TestButton.IsEnabled = true;
            }
        }

        // --- ボタン2: GitHubトレンド取得 ---
        private async void ScrapeButton_Click(object sender, RoutedEventArgs e)
        {
            ScrapeButton.IsEnabled = false;
            AnalyzeButton.IsEnabled = false; // データ取得中は分析ボタンも押せないようにする

            Repositories.Clear();
            RepoNames.Clear();
            AiSummaryText.Text = "データ取得後に「AI分析」ボタンを押してください。";

            Log("【GitHubトレンド】取得開始...");
            try
            {
                await RunGitHubScraping();
            }
            catch (Exception ex)
            {
                Log($"エラー: {ex.Message}");
            }
            finally
            {
                ScrapeButton.IsEnabled = true;
                AnalyzeButton.IsEnabled = true;
            }
        }

        // --- ボタン3: AI分析 (新規追加) ---
        private async void AnalyzeButton_Click(object sender, RoutedEventArgs e)
        {
            // データがない場合は実行しない
            if (Repositories.Count == 0)
            {
                MessageBox.Show("先に「GitHubトレンド取得」を行ってください。", "データなし", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            AnalyzeButton.IsEnabled = false;
            Log("Geminiによる分析を開始します...");
            AiSummaryText.Text = "🤖 Geminiがトレンドを分析中...お待ちください...";

            try
            {
                await AnalyzeWithGemini();
            }
            catch (Exception ex)
            {
                Log($"AIエラー: {ex.Message}");
                AiSummaryText.Text = "AI分析中にエラーが発生しました。";
            }
            finally
            {
                AnalyzeButton.IsEnabled = true;
            }
        }

        // ==========================================
        // ロジック部分
        // ==========================================

        private async Task RunTestConnection()
        {
            using var playwright = await Playwright.CreateAsync();
            var browser = await LaunchBrowser(playwright);
            var page = await browser.NewPageAsync();

            Log("接続テストページへアクセス中...");
            await page.GotoAsync("https://geo.brdtest.com/welcome.txt?product=isp&method=native");
            var content = await page.Locator("body").InnerTextAsync();

            Log($"結果:\n{content}");
            await browser.CloseAsync();
        }

        private async Task RunGitHubScraping()
        {
            using var playwright = await Playwright.CreateAsync();
            var browser = await LaunchBrowser(playwright);
            var page = await browser.NewPageAsync();

            // 画面サイズ設定
            await page.SetViewportSizeAsync(1920, 1080);

            Log("GitHub Trendingへアクセス中...");
            await page.GotoAsync("https://github.com/trending?since=monthly", new PageGotoOptions
            {
                Timeout = 120000,
                WaitUntil = WaitUntilState.DOMContentLoaded
            });

            Log("ページ読み込み待機中...");
            await page.WaitForLoadStateAsync(LoadState.NetworkIdle);
            await page.EvaluateAsync("window.scrollTo(0, document.body.scrollHeight)");
            await Task.Delay(2000);

            Log("HTML解析中...");
            var rows = await page.Locator("article.Box-row").AllAsync();
            if (rows.Count == 0) Log("【注意】データが見つかりませんでした。");

            var starValues = new List<double>();
            int rank = 1;

            foreach (var row in rows)
            {
                var name = await row.Locator("h2 a").InnerTextAsync();

                var descLocator = row.Locator("p");
                var description = (await descLocator.CountAsync() > 0) ? await descLocator.InnerTextAsync() : "";

                var langLocator = row.Locator("[itemprop='programmingLanguage']");
                var language = (await langLocator.CountAsync() > 0) ? await langLocator.InnerTextAsync() : "-";

                var starText = await row.Locator("div.f6 a").First.InnerTextAsync();

                // 数値変換
                double starNum = ParseStars(starText.Trim());
                starValues.Add(starNum);

                Dispatcher.Invoke(() =>
                {
                    var cleanName = name.Trim().Replace("\n", "").Replace(" ", "");
                    Repositories.Add(new RepositoryData
                    {
                        Rank = rank.ToString(),
                        Name = cleanName,
                        Description = description.Trim(),
                        Language = language,
                        Stars = starText.Trim()
                    });
                    RepoNames.Add(cleanName);
                });
                rank++;
            }

            // グラフ更新
            Dispatcher.Invoke(() =>
            {
                var barSeries = new BarSeries
                {
                    Title = "Stars",
                    StrokeThickness = 1,
                    FillColor = OxyColors.DodgerBlue
                };

                for (int i = 0; i < starValues.Count; i++)
                {
                    barSeries.Items.Add(new BarItem { Value = starValues[i] });
                }

                MyPlotModel.Series.Clear();
                MyPlotModel.Series.Add(barSeries);

                if (MyPlotModel.Axes.Count > 0 && MyPlotModel.Axes[0] is CategoryAxis axis)
                {
                    axis.Labels.Clear();
                    foreach (var name in RepoNames) axis.Labels.Add(name);
                }
                MyPlotModel.InvalidatePlot(true);
            });

            Log($"完了！ {rows.Count} 件取得しました。");
            await browser.CloseAsync();
        }

       private async Task AnalyzeWithGemini()
        {
            // 1. プロンプト作成
            var sb = new StringBuilder();
            sb.AppendLine("あなたはベテランの技術ジャーナリスト兼CTOです。");
            sb.AppendLine("以下の「GitHub Trending (本日の急上昇リポジトリ)」のリストを分析し、開発者向けの専門的なトレンドレポートを作成してください。");
            sb.AppendLine("");
            sb.AppendLine("【分析の指針】");
            sb.AppendLine("1. **市場の潮流 (Overview):** リスト全体から読み取れる、現在の技術界の大きな流れ（例: 自律型AIエージェントの実用化、Rustのツール採用など）を詳しく解説してください。");
            sb.AppendLine("2. **カテゴリ別深掘り:** リストのリポジトリを「AI/LLM」「インフラ/DevOps」「Webフロントエンド」「その他」などの適切なカテゴリに分類し、それぞれの動向を述べてください。");
            sb.AppendLine("3. **注目のHidden Gem:** スター数がトップクラスでなくとも、アイデアが斬新で将来性があるリポジトリを1つ挙げ、なぜ注目すべきか熱く語ってください。");
            sb.AppendLine("4. **結論:** 開発者は今、何を学ぶべきか？");
            sb.AppendLine("");
            sb.AppendLine("【入力データ】");
            foreach (var repo in Repositories)
            {
                // 言語情報がない場合は補足するなど、情報をリッチに渡す
                string langInfo = string.IsNullOrEmpty(repo.Language) ? "Language not specified" : repo.Language;
                sb.AppendLine($"- 名前: {repo.Name} | 言語: {langInfo} | ★: {repo.Stars} | 説明: {repo.Description}");
            }

            // 2. HTTPリクエスト準備
            var url = $"https://generativelanguage.googleapis.com/v1beta/models/gemini-3-pro-preview:generateContent?key={GeminiApiKey}";

            var jsonBody = new JObject(
                new JProperty("contents", new JArray(
                    new JObject(
                        new JProperty("parts", new JArray(
                            new JObject(
                                new JProperty("text", sb.ToString())
                            )
                        ))
                    )
                ))
            );

            using var client = new HttpClient();
            var content = new StringContent(jsonBody.ToString(), Encoding.UTF8, "application/json");

            // 3. 送信
            var response = await client.PostAsync(url, content);
            var responseString = await response.Content.ReadAsStringAsync();

            // 4. 結果解析
            try
            {
                var jsonResponse = JObject.Parse(responseString);
                var resultText = jsonResponse["candidates"]?[0]?["content"]?["parts"]?[0]?["text"]?.ToString();

                // 5. 表示
                Dispatcher.Invoke(() =>
                {
                    if (!string.IsNullOrEmpty(resultText))
                    {
                        AiSummaryText.Text = resultText;
                    }
                    else
                    {
                        // エラーハンドリング: APIキーの間違いや制限など
                        var errorMsg = jsonResponse["error"]?["message"]?.ToString();
                        AiSummaryText.Text = $"分析できませんでした。\n詳細: {errorMsg ?? "不明なエラー"}";
                        Log("API Response: " + responseString);
                    }
                });
            }
            catch (Exception ex)
            {
                Dispatcher.Invoke(() => AiSummaryText.Text = $"解析エラー: {ex.Message}");
                Log($"Parse Error: {responseString}");
            }

            Log("Gemini分析完了。");
        }

        private async Task<IBrowser> LaunchBrowser(IPlaywright playwright)
        {
            return await playwright.Chromium.LaunchAsync(new BrowserTypeLaunchOptions
            {
                Headless = false,
                Proxy = new Proxy { Server = $"http://{ProxyHost}", Username = Username, Password = Password }
            });
        }

        private double ParseStars(string starText)
        {
            var cleanText = starText.Replace(",", "").Trim();
            double multiplier = 1;
            if (cleanText.EndsWith("k", StringComparison.OrdinalIgnoreCase))
            {
                multiplier = 1000;
                cleanText = cleanText.Substring(0, cleanText.Length - 1);
            }
            if (double.TryParse(cleanText, out double result)) return result * multiplier;
            return 0;
        }

        private void Log(string message)
        {
            LogText.AppendText($"[{DateTime.Now:HH:mm:ss}] {message}\n");
            LogText.ScrollToEnd();
        }
    }
}