#!/usr/bin/env python3
"""
Script para desenhar gráficos em tempo real do ficheiro CSV.
Mostra lux_ref vs lux_meas num gráfico e duty_cycle noutro.
"""

import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import glob
import os
from datetime import datetime

WINDOW_SECONDS = 20
WINDOW_MS = WINDOW_SECONDS * 1000

# Encontra o ficheiro CSV mais recente
def find_latest_csv():
    csv_files = glob.glob("adc_data_*.csv")
    if not csv_files:
        return None
    return max(csv_files, key=os.path.getctime)

def update_plot(frame):
    """Atualiza os gráficos com os dados mais recentes"""
    try:
        # Lê o ficheiro CSV
        df = pd.read_csv(csv_file)
        
        if len(df) == 0:
            return
        
        # Garante que o tempo é numérico (ms)
        df['time'] = pd.to_numeric(df['time'], errors='coerce')
        df = df.dropna(subset=['time'])

        if len(df) == 0:
            return

        # Mantém apenas os últimos 20 segundos
        t_max = df['time'].iloc[-1]
        t_min = max(0, t_max - WINDOW_MS)
        df_window = df[df['time'] >= t_min]

        if len(df_window) == 0:
            return

        x_seconds = df_window['time'] / 1000.0
        
        # Limpa os axes anteriores
        ax1.clear()
        ax2.clear()
        
        # Gráfico 1: Lux Ref vs Lux Meas
        ax1.plot(x_seconds, df_window['lux_ref'], 'b-', label='Lux Ref', linewidth=2)
        ax1.plot(x_seconds, df_window['lux_meas'], 'r-', label='Lux Meas', linewidth=2)
        ax1.set_xlabel('Tempo (s)')
        ax1.set_ylabel('Luminância (lux)')
        ax1.set_title('Luminância: Referência vs Medida')
        ax1.legend(loc='upper left')
        ax1.grid(True, alpha=0.3)
        
        # Gráfico 2: Duty Cycle
        ax2.plot(x_seconds, df_window['duty_cycle'], 'g-', linewidth=2)
        ax2.set_xlabel('Tempo (s)')
        ax2.set_ylabel('Duty Cycle (%)')
        ax2.set_title('Ciclo de Trabalho')
        ax2.grid(True, alpha=0.3)
        ax2.set_ylim([0, 1])
        ax2.set_xlim([t_min / 1000.0, t_max / 1000.0])
        
        # Atualiza o título com a hora
        plt.suptitle(f'Gráficos em Tempo Real - {datetime.now().strftime("%H:%M:%S")}', fontsize=12)
        
    except Exception as e:
        print(f"Erro ao atualizar gráfico: {e}")

# Procura o ficheiro CSV mais recente
csv_file = find_latest_csv()

if not csv_file:
    print("Nenhum ficheiro CSV encontrado!")
    print("Certifique-se que o interface_serial.py está a correr e a gravar dados.")
    exit(1)

print(f"A ler dados de: {csv_file}")
print("A atualizar gráficos em tempo real... (Ctrl+C para sair)")

# Cria a figura com 2 subplots
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8))
plt.subplots_adjust(hspace=0.3)

# Inicia a animação (atualiza a cada 150ms)
ani = FuncAnimation(fig, update_plot, interval=200, cache_frame_data=False)

plt.show()
