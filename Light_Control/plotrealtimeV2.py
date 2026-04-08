#!/usr/bin/env python3
"""Plot realtime V2: mostra 3 luminarias ao mesmo tempo com cores diferentes."""

import argparse
import os
import time
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

WINDOW_SECONDS = 20
WINDOW_MS = WINDOW_SECONDS * 1000
MIN_X_SPAN_SECONDS = 0.5

LUM_COLORS = {
    0: "#e63946",
    1: "#2a9d8f",
    2: "#457b9d",
}


def parse_args():
    parser = argparse.ArgumentParser(description="Plot realtime V2")
    parser.add_argument("--csv-file", required=True, help="CSV de stream unificado")
    parser.add_argument("--wait", action="store_true", help="Espera o ficheiro existir")
    return parser.parse_args()


def update_plot(_frame):
    try:
        if not os.path.exists(csv_file):
            return

        df = pd.read_csv(csv_file)
        if df.empty:
            return

        df["time"] = pd.to_numeric(df["time"], errors="coerce")
        df["luminaire_id"] = pd.to_numeric(df["luminaire_id"], errors="coerce")
        if "lux_ref" in df.columns:
            df["lux_ref"] = pd.to_numeric(df["lux_ref"], errors="coerce")
        else:
            df["lux_ref"] = float("nan")
        df["lux_meas"] = pd.to_numeric(df["lux_meas"], errors="coerce")
        df["duty_cycle"] = pd.to_numeric(df["duty_cycle"], errors="coerce")
        df = df.dropna(subset=["time", "luminaire_id", "lux_meas", "duty_cycle"])
        if df.empty:
            return

        t_max = df["time"].iloc[-1]
        t_min = max(0, t_max - WINDOW_MS)
        window = df[df["time"] >= t_min].sort_values(["luminaire_id", "time"])
        if window.empty:
            return

        ax1.clear()
        ax2.clear()

        lum_ids = sorted(int(v) for v in window["luminaire_id"].unique())
        for lum_id in lum_ids:
            lum = window[window["luminaire_id"] == lum_id]
            x_s = lum["time"] / 1000.0
            color = LUM_COLORS.get(lum_id, None)

            ax1.plot(x_s, lum["lux_meas"], linewidth=2, color=color, label=f"Lux L{lum_id}")
            if lum["lux_ref"].notna().any():
                ax1.plot(
                    x_s,
                    lum["lux_ref"],
                    linewidth=1.8,
                    linestyle="--",
                    color=color,
                    alpha=0.85,
                    label=f"Ref L{lum_id}",
                )
            ax2.plot(x_s, lum["duty_cycle"], linewidth=2, color=color, label=f"Duty L{lum_id}")

        ax1.set_xlabel("Tempo (s)")
        ax1.set_ylabel("Luminancia (lux)")
        ax1.set_title("Luminancia Medida (L0, L1, L2)")
        ax1.grid(True, alpha=0.3)
        ax1.legend(loc="upper left")

        ax2.set_xlabel("Tempo (s)")
        ax2.set_ylabel("Duty Cycle")
        ax2.set_title("Ciclo de Trabalho (L0, L1, L2)")
        ax2.grid(True, alpha=0.3)
        ax2.legend(loc="upper left")
        ax2.set_ylim([0, 1])

        x_min = t_min / 1000.0
        x_max = t_max / 1000.0
        if x_max <= x_min:
            x_max = x_min + MIN_X_SPAN_SECONDS
        ax1.set_xlim([x_min, x_max])
        ax2.set_xlim([x_min, x_max])

    except Exception as exc:
        print(f"Erro ao atualizar plot: {exc}")


args = parse_args()
csv_file = args.csv_file

if args.wait:
    while not os.path.exists(csv_file):
        time.sleep(0.2)

print(f"A ler dados de: {csv_file}")
print("Plot realtime V2 ativo (Ctrl+C para sair)")

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8))
plt.subplots_adjust(hspace=0.3)

ani = FuncAnimation(fig, update_plot, interval=200, cache_frame_data=False)
plt.show()
