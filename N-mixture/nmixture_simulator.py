#!/usr/bin/env python3
"""
ターゲットAIカウント & N-mixture推定シミュレータ
Target AI Count & N-mixture Estimation Simulator

モデル構成:
  世界モジュール  : 100x100 空間でN個体がランダムウォーク
  観測モジュール  : 中央50x50のカメラFOVで個体数をカウント
  推論モジュール  : N-mixtureモデル（二項分布MLE）で真のNとpを推定
"""

import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.font_manager as fm
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button
from scipy.special import gammaln

# ------------------------------------------------------------
# 日本語フォント設定 (フォントファイルを直接指定)
# ------------------------------------------------------------
_JP_FONT_CANDIDATES = [
    r"C:\Windows\Fonts\YuGothM.ttc",
    r"C:\Windows\Fonts\YuGothR.ttc",
    r"C:\Windows\Fonts\YuGothB.ttc",
    r"C:\Windows\Fonts\meiryo.ttc",
    r"C:\Windows\Fonts\msgothic.ttc",
]
for _fp in _JP_FONT_CANDIDATES:
    if os.path.exists(_fp):
        fm.fontManager.addfont(_fp)
        plt.rcParams["font.family"] = fm.FontProperties(fname=_fp).get_name()
        break

# ============================================================
# パラメータ設定
# ============================================================
TRUE_N           = 300      # 真の総個体数（推論モジュールには秘密）
SPACE_SIZE       = 100.0    # 空間サイズ（正方形の一辺）
FOV_SIZE         = 50.0     # カメラ視野サイズ（中央正方形の一辺）
SPEED            = 1.5      # 個体の移動速度（1フレームあたり）
NUM_SNAPSHOTS    = 500      # 観測（シャッター）回数
SNAPSHOT_INTERVAL = 30      # 撮影間隔（フレーム数）
RANDOM_SEED      = None     # Noneで毎回異なる結果、整数値で固定再現

# 派生パラメータ（変更不要）
FOV_X       = (SPACE_SIZE - FOV_SIZE) / 2   # FOV左端 = 25
FOV_Y       = (SPACE_SIZE - FOV_SIZE) / 2   # FOV下端 = 25
AREA_RATIO  = (FOV_SIZE / SPACE_SIZE) ** 2   # 面積比率 = 0.25
TOTAL_FRAMES = NUM_SNAPSHOTS * SNAPSHOT_INTERVAL + 60


# ============================================================
# ① 世界モジュール
# ============================================================
class World:
    """
    2D閉鎖空間における個体のランダムウォーク。
    壁では完全反射（入射角＝反射角）。
    """

    def __init__(self, n: int, space_size: float, speed: float):
        self.n = n
        self.space_size = space_size
        self.speed = speed

        # 初期位置: 空間全体にランダム配置
        self.positions = np.random.uniform(0.0, space_size, (n, 2))

        # 初期速度: ランダム方向・一定速さ
        angles = np.random.uniform(0.0, 2.0 * np.pi, n)
        self.velocities = speed * np.column_stack([np.cos(angles), np.sin(angles)])

    def step(self):
        """1フレーム分の移動と壁反射を適用する。"""
        self.positions += self.velocities

        for dim in range(2):
            low  = self.positions[:, dim] < 0.0
            high = self.positions[:, dim] > self.space_size

            # 壁を超えた分だけ折り返し、速度成分を反転
            self.positions[low,  dim] = -self.positions[low,  dim]
            self.positions[high, dim] = 2.0 * self.space_size - self.positions[high, dim]
            self.velocities[low,  dim] =  abs(self.velocities[low,  dim])
            self.velocities[high, dim] = -abs(self.velocities[high, dim])

    def randomize_directions(self):
        """速度方向をランダムリセット（位置は維持）。撮影ごとに相関を断ち切る。"""
        angles = np.random.uniform(0.0, 2.0 * np.pi, self.n)
        self.velocities = self.speed * np.column_stack([np.cos(angles), np.sin(angles)])


# ============================================================
# ② 観測モジュール（AIカメラ）
# ============================================================
class Camera:
    """
    FOV（Field of View）内の個体を100%検知するAIカメラ。
    第一段階仮定：見落とし・重複カウントなし。
    """

    def __init__(self, fov_x: float, fov_y: float, fov_size: float):
        self.x0 = fov_x
        self.y0 = fov_y
        self.x1 = fov_x + fov_size
        self.y1 = fov_y + fov_size

    def observe(self, positions: np.ndarray) -> tuple:
        """視野内の個体を検知してカウントを返す。
        Returns:
            count  : int        -- 視野内個体数
            in_fov : np.ndarray -- 各個体がFOV内かどうかのbool配列
        """
        in_fov = (
            (positions[:, 0] >= self.x0) & (positions[:, 0] <= self.x1) &
            (positions[:, 1] >= self.y0) & (positions[:, 1] <= self.y1)
        )
        return int(np.sum(in_fov)), in_fov


# ============================================================
# ③ 推論モジュール（N-mixture モデル MLE）
# ============================================================
def nmixture_mle(counts: list) -> tuple:
    """
    N-mixture モデル（閉鎖個体群 二項分布型）による最尤推定。

    モデル:  y_i ~ Binomial(N, p)  (i = 1, ..., T)

    最尤推定の手順:
      1. 各候補Nに対し、pのMLEは p = ȳ / N
      2. 対数尤度 ℓ(N) = Σ [logC(N,y_i) + y_i*log(p) + (N-y_i)*log(1-p)]
      3. ℓ(N) を最大化する整数Nを探索

    モーメント法による推定値をグリッド探索の中心に使用:
      ȳ = Np, s² = Np(1-p)  →  p_mom = 1 - s²/ȳ, N_mom = ȳ²/(ȳ - s²)

    Args:
        counts: 観測カウントのリスト [y_1, ..., y_T]

    Returns:
        N_hat: 推定総個体数 (float)
        p_hat: 推定発見率   (float)
    """
    y = np.array(counts, dtype=float)
    T = len(y)
    y_mean = float(np.mean(y))
    y_var  = float(np.var(y, ddof=1)) if T > 1 else 0.0
    y_max  = int(np.max(y))

    # --- モーメント法による初期推定 ---
    if y_mean > y_var > 0:
        # 二項分布の過少分散ケース: Var < Mean
        p_mom = 1.0 - y_var / y_mean
        N_mom = y_mean / p_mom
    else:
        # フォールバック（分散≥平均の場合）
        p_mom = AREA_RATIO
        N_mom = y_mean / p_mom if p_mom > 0 else y_mean * 4

    # --- 対数尤度グリッド探索 ---
    N_min = max(y_max, int(N_mom * 0.4))
    N_max = int(N_mom * 3.0) + 100

    best_ll = -np.inf
    best_N  = max(y_max, int(round(N_mom)))

    for N_cand in range(N_min, N_max + 1):
        p_cand = y_mean / N_cand
        if not (0.0 < p_cand < 1.0):
            continue
        # log C(N, y_i) を log-gamma で計算（オーバーフロー回避）
        log_binom = (
            gammaln(N_cand + 1)
            - gammaln(y + 1)
            - gammaln(N_cand - y + 1)
        )
        ll = float(np.sum(
            log_binom
            + y * np.log(p_cand)
            + (N_cand - y) * np.log(1.0 - p_cand)
        ))
        if ll > best_ll:
            best_ll = ll
            best_N  = N_cand

    p_hat = y_mean / best_N
    return float(best_N), p_hat


# ============================================================
# メインシミュレーション＆アニメーション
# ============================================================
def run_simulation():
    if RANDOM_SEED is not None:
        np.random.seed(RANDOM_SEED)

    world  = World(TRUE_N, SPACE_SIZE, SPEED)
    camera = Camera(FOV_X, FOV_Y, FOV_SIZE)

    counts = []          # 観測カウント蓄積リスト
    state  = {
        "frame":     0,
        "snapshots": 0,
        "flash":     0,   # シャッターフラッシュ残フレーム数
        "done":      False,
    }

    # ----------------------------------------------------------
    # レイアウト: 左=世界ビュー, 右=カウントグラフ, 下=ボタン
    # ----------------------------------------------------------
    fig = plt.figure(figsize=(15, 7.8))
    fig.patch.set_facecolor("#f5f5f5")
    fig.suptitle(
        "ターゲット N-mixture 推定シミュレータ",
        fontsize=15, fontweight="bold", y=0.99
    )

    # グラフ領域（下部にボタン用の余白を確保）
    ax_world  = fig.add_axes([0.05, 0.12, 0.42, 0.82])
    ax_counts = fig.add_axes([0.55, 0.12, 0.42, 0.82])

    # ボタン領域（中央下）
    ax_btn = fig.add_axes([0.40, 0.02, 0.20, 0.06])

    # --- 世界ビュー ---
    ax_world.set_facecolor("#eef4ff")
    ax_world.set_xlim(0, SPACE_SIZE)
    ax_world.set_ylim(0, SPACE_SIZE)
    ax_world.set_aspect("equal")
    ax_world.set_title(f"世界ビュー  (真のN = {TRUE_N}個, 非公開)", fontsize=11)
    ax_world.set_xlabel("X 座標")
    ax_world.set_ylabel("Y 座標")

    # 外枠（空間の境界）
    border = patches.FancyBboxPatch(
        (0, 0), SPACE_SIZE, SPACE_SIZE,
        boxstyle="square,pad=0", linewidth=2,
        edgecolor="black", facecolor="none", zorder=1
    )
    ax_world.add_patch(border)

    # カメラFOV（赤枠）
    fov_patch = patches.Rectangle(
        (FOV_X, FOV_Y), FOV_SIZE, FOV_SIZE,
        linewidth=2.5, edgecolor="crimson",
        facecolor="lightyellow", alpha=0.35, zorder=2,
        label=f"Camera FOV ({int(AREA_RATIO*100)}%)"
    )
    ax_world.add_patch(fov_patch)

    # カメララベル
    ax_world.text(
        FOV_X + FOV_SIZE / 2, FOV_Y + FOV_SIZE + 1.5,
        "AI Camera", ha="center", va="bottom",
        fontsize=8, color="crimson",
        bbox=dict(facecolor="white", edgecolor="crimson", alpha=0.7, pad=2)
    )

    # 散布図（視野外・視野内）
    sc_out = ax_world.scatter([], [], c="#3a7ebf", s=18, alpha=0.65, zorder=3, label="視野外")
    sc_in  = ax_world.scatter([], [], c="crimson",  s=28, alpha=0.95, zorder=4,
                               label="視野内（検知）", marker="o",
                               edgecolors="darkred", linewidths=0.5)
    ax_world.legend(loc="upper right", fontsize=8, framealpha=0.85)

    # ステータスボックス
    stat_text = ax_world.text(
        0.02, 0.02, "", transform=ax_world.transAxes,
        va="bottom", fontsize=9,
        bbox=dict(boxstyle="round,pad=0.4", facecolor="wheat", alpha=0.85)
    )

    # --- カウントグラフ ---
    ax_counts.set_facecolor("#f0fff0")
    ax_counts.set_xlim(0, NUM_SNAPSHOTS + 1)
    y_ceil = max(TRUE_N * AREA_RATIO * 2.5, 20)
    ax_counts.set_ylim(0, y_ceil)
    ax_counts.set_xlabel("撮影回数")
    ax_counts.set_ylabel("視野内検知数")
    ax_counts.set_title("観測データ（シャッターごとのカウント）", fontsize=11)
    ax_counts.grid(True, alpha=0.35)

    expected = TRUE_N * AREA_RATIO
    ax_counts.axhline(
        y=expected, color="seagreen", linestyle="--", linewidth=1.8,
        label=f"期待値 = N × 面積比率 = {expected:.1f}"
    )
    ax_counts.legend(fontsize=8, loc="upper right")

    count_line, = ax_counts.plot(
        [], [], "o-", markersize=5, linewidth=1.5,
        label="観測カウント", color="#2255cc"
    )

    result_text = ax_counts.text(
        0.02, 0.98, "", transform=ax_counts.transAxes,
        va="top", fontsize=9,
        bbox=dict(boxstyle="round,pad=0.5", facecolor="lightyellow",
                  edgecolor="goldenrod", alpha=0.95)
    )

    # ----------------------------------------------------------
    # アニメーション更新関数
    # ----------------------------------------------------------
    def animate(_frame):
        if state["done"] or not state["running"]:
            return

        world.step()
        state["frame"] += 1

        count, in_fov = camera.observe(world.positions)

        # ---- シャッタータイミング ----
        if (state["frame"] % SNAPSHOT_INTERVAL == 0 and
                state["snapshots"] < NUM_SNAPSHOTS):
            counts.append(count)
            state["snapshots"] += 1
            state["flash"] = 6
            world.randomize_directions()   # 位置はそのまま、向きだけリセットして相関を低減
            count_line.set_data(list(range(1, len(counts) + 1)), counts)

        # ---- FOVフラッシュ ----
        if state["flash"] > 0:
            fov_patch.set_facecolor("yellow")
            fov_patch.set_alpha(0.65)
            state["flash"] -= 1
        else:
            fov_patch.set_facecolor("lightyellow")
            fov_patch.set_alpha(0.35)

        # ---- 散布図更新 ----
        pos_out = world.positions[~in_fov]
        pos_in  = world.positions[ in_fov]
        sc_out.set_offsets(pos_out if len(pos_out) > 0 else np.empty((0, 2)))
        sc_in.set_offsets( pos_in  if len(pos_in)  > 0 else np.empty((0, 2)))

        # ---- ステータス更新 ----
        stat_text.set_text(
            f"フレーム   : {state['frame']}\n"
            f"撮影回数   : {state['snapshots']} / {NUM_SNAPSHOTS}\n"
            f"現在の検知数: {count}"
        )

        # ---- 推定（全撮影完了時） ----
        if state["snapshots"] == NUM_SNAPSHOTS and not state["done"]:
            state["done"] = True
            _print_and_display_results(counts, result_text)

    def _print_and_display_results(obs_counts, text_artist):
        """N-mixture MLE を実行し、結果をグラフとコンソールに出力する。"""
        N_hat, p_hat = nmixture_mle(obs_counts)
        err_N = abs(N_hat - TRUE_N) / TRUE_N * 100
        err_p = abs(p_hat - AREA_RATIO) / AREA_RATIO * 100
        y_arr = np.array(obs_counts, dtype=float)

        # グラフ内テキスト
        text_artist.set_text(
            f"【N-mixture 推定結果】\n"
            f"{'─'*24}\n"
            f"真の総数   : {TRUE_N} 個\n"
            f"推定総数   : {N_hat:.0f} 個  (誤差 {err_N:.1f}%)\n"
            f"{'─'*24}\n"
            f"面積比率   : {AREA_RATIO:.4f}\n"
            f"推定発見率 : {p_hat:.4f}  (誤差 {err_p:.1f}%)\n"
            f"{'─'*24}\n"
            f"観測平均   : {np.mean(y_arr):.2f}\n"
            f"観測分散   : {np.var(y_arr, ddof=1):.2f}"
        )

        # コンソール出力（答え合わせ）
        sep = "=" * 52
        print(f"\n{sep}")
        print("  【答え合わせ / N-mixture 推定結果】")
        print(sep)
        print(f"  真の総数         : {TRUE_N} 個")
        print(f"  推定総数 (N_hat) : {N_hat:.0f} 個  （誤差 {err_N:.1f}%）")
        print(f"  カメラ視野面積比 : {AREA_RATIO:.4f}")
        print(f"  推定発見率 (p)   : {p_hat:.4f}  （誤差 {err_p:.1f}%）")
        print("-" * 52)
        print(f"  観測回数         : {len(obs_counts)}")
        print(f"  カウント平均     : {np.mean(y_arr):.2f}")
        print(f"  カウント分散     : {np.var(y_arr, ddof=1):.2f}")
        print(f"  カウントデータ   : {obs_counts}")
        print(sep)

    state["running"] = False  # 初期は停止中（animate内のガードで制御）

    anim = FuncAnimation(  # noqa: F841
        fig, animate,
        frames=None,        # 無限フレーム: state["running"]とstate["done"]で制御
        interval=25,        # 25ms/frame ≈ 40fps（2倍速）
        blit=False,
        repeat=True,
        cache_frame_data=False
    )

    # ----------------------------------------------------------
    # ボタン（開始 / 一時停止 / 再開）
    # ----------------------------------------------------------
    btn = Button(ax_btn, "▶  開始", color="#4caf50", hovercolor="#66bb6a")
    btn.label.set_fontsize(12)
    btn.label.set_fontweight("bold")
    btn.label.set_color("white")

    def on_click(_event):
        if state["done"]:
            return
        state["running"] = not state["running"]
        if state["running"]:
            btn.label.set_text("||  一時停止")
            btn.ax.set_facecolor("#2196f3")
            btn.hovercolor = "#42a5f5"
        else:
            btn.label.set_text("▶  再開")
            btn.ax.set_facecolor("#f5a623")
            btn.hovercolor = "#f7b84b"
        fig.canvas.draw_idle()

    btn.on_clicked(on_click)

    plt.show()


# ============================================================
# エントリーポイント
# ============================================================
if __name__ == "__main__":
    run_simulation()
