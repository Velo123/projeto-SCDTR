#!/usr/bin/env python3
#time,luminaire_ID,lux_ref,lux_meas,ldrvoltage,ldrresistance,duty_cycle,windup_state,setpoint-weight
"""
Script para ler dados do ADC do Raspberry Pi Pico e gravar num ficheiro.
"""

import serial
import serial.tools.list_ports
import time
from datetime import datetime
import threading
import sys
import argparse
import os
import glob

RPI_USB_IDS = {
    "rpi0": "E66118604B886021",
    "rpi1": "E66118604B497921",
    "rpi2": "E66118604B296221",
}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Interface serial para Raspberry Pi Pico"
    )
    parser.add_argument(
        "--rpi",
            choices=["rpi0", "rpi1", "rpi2"],
            help="Seleciona automaticamente a porta USB do RPI 0, RPI 1 ou RPI 2",
    )
    parser.add_argument(
        "--port",
        help="Define explicitamente a porta serial (ex: /dev/ttyACM0)",
    )
    parser.add_argument(
        "--csv-file",
        help="Caminho do ficheiro CSV para gravar o stream em tempo real",
    )
    parser.add_argument(
        "--list",
        action="store_true",
        help="Lista portas /dev/serial/by-id detetadas e termina",
    )
    return parser.parse_args()


def list_serial_by_id():
    entries = sorted(glob.glob("/dev/serial/by-id/*"))
    if not entries:
        print("Nenhuma porta em /dev/serial/by-id")
        return
    print("Portas em /dev/serial/by-id:")
    for entry in entries:
        print(f"  - {entry}")


def resolve_rpi_port_by_id(rpi_name):
    usb_id = RPI_USB_IDS[rpi_name]
    pattern = f"/dev/serial/by-id/usb-Raspberry_Pi_Pico_{usb_id}*"
    matches = sorted(glob.glob(pattern))
    if matches:
        return matches[0]
    return None


def wait_for_rpi_port(rpi_name, timeout_s=20.0, poll_s=0.2):
    """Espera a porta do RPI por ID único reaparecer (útil com hubs USB)."""
    deadline = time.time() + timeout_s
    while time.time() < deadline:
        port = resolve_rpi_port_by_id(rpi_name)
        if port and os.path.exists(port):
            return port
        time.sleep(poll_s)
    return None


def is_stream_csv_line(line):
    parts = [p.strip() for p in line.split(",")]
    if len(parts) not in (4, 5):
        return False
    try:
        for part in parts:
            float(part)
        return True
    except ValueError:
        return False


def normalize_stream_csv_line(line):
    parts = [p.strip() for p in line.split(",")]
    # old: time,luminaire_id,lux_meas,duty_cycle
    # new: time,luminaire_id,lux_ref,lux_meas,duty_cycle
    if len(parts) == 4:
        parts = [parts[0], parts[1], "nan", parts[2], parts[3]]
    return ",".join(parts)

def find_pico_port():
    """Procura automaticamente a porta do Raspberry Pi Pico."""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        # O Pico geralmente aparece como "USB Serial Device" ou similar
        if "USB" in port.description or "ACM" in port.device or "ttyUSB" in port.device:
            return port.device
    return None


def resolve_port(args):
    """Resolve a porta serial conforme prioridade: --port > --rpi > auto-deteção."""
    if args.port:
        return args.port

    if args.rpi:
        selected = resolve_rpi_port_by_id(args.rpi)
        return selected

    return find_pico_port()


def wait_for_port(port_path, timeout_s=12.0, poll_s=0.2):
    """Espera a porta aparecer (útil após reset/upload do Pico)."""
    deadline = time.time() + timeout_s
    while time.time() < deadline:
        if os.path.exists(port_path):
            return True
        time.sleep(poll_s)
    return False

def main():
    args = parse_args()

    if args.list:
        list_serial_by_id()
        return

    # Tenta resolver porta por argumentos e fallback automático
    port = resolve_port(args)

    # Com --rpi, se o dispositivo ainda não estiver presente, espera reaparecer.
    if args.rpi and not port:
        print(f"A aguardar porta do {args.rpi} aparecer no hub...")
        port = wait_for_rpi_port(args.rpi, timeout_s=20.0)

    # Quando uma porta específica é pedida, aguarda reaparecer após upload/reset.
    if port and (args.rpi or args.port) and not os.path.exists(port):
        print(f"A aguardar porta {port} ficar disponível...")
        if not wait_for_port(port):
            print(f"Porta não apareceu a tempo: {port}")
            if args.rpi:
                print("Não vou usar auto-deteção para evitar ligar ao RPI errado.")
                print("Confirme se a board está ligada e se o ID no script está correto.")
            port = None
    
    if not port:
        if args.rpi:
            print("Porta USB do RPI selecionado não encontrada.")
            list_serial_by_id()
            return
        print("Porta USB não encontrada automaticamente.")
        port = input("Insira a porta serial (ex: /dev/ttyACM0 ou /dev/ttyUSB0): ")
    else:
        print(f"Porta encontrada: {port}")
    
    # Nome do ficheiro com timestamp (ou nome fornecido)
    filename = args.csv_file or f"adc_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
    
    try:
        # Abre conexão serial
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(2)  # Aguarda inicialização
        
        print(f"Conectado a {port}")
        print(f"A gravar dados em: {filename}")
        print("Digite comandos e pressione Enter para enviar ao Pico")
        print("Pressione Ctrl+C para parar...\n")
        
        # Flag para controlar threads
        running = True
        
        def read_commands():
            """Thread para ler comandos do utilizador"""
            nonlocal running
            try:
                while running:
                    try:
                        cmd = input()
                        if cmd.strip():
                            # Envia comando para o Pico
                            ser.write((cmd + "\n").encode('utf-8'))
                            print(f"[Enviado] {cmd}")
                    except EOFError:
                        break
                    except Exception as e:
                        if running:
                            print(f"Erro ao enviar comando: {e}")
            except KeyboardInterrupt:
                pass
        
        # Inicia thread de leitura de comandos
        cmd_thread = threading.Thread(target=read_commands, daemon=True)
        cmd_thread.start()
        
        # Abre ficheiro para escrita
        with open(filename, 'w', buffering=1) as f:
            # Cabeçalho do stream compacto: time,luminaire_id,lux_ref,lux_meas,duty_cycle
            f.write("time,luminaire_id,lux_ref,lux_meas,duty_cycle\n")

            while running:
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()

                    if line:
                        if is_stream_csv_line(line):
                            # Grava stream para plot sem poluir o terminal.
                            f.write(normalize_stream_csv_line(line) + "\n")
                        else:
                            # Mostra apenas respostas/comandos no terminal.
                            print(line)
                
    except KeyboardInterrupt:
        running = False
        print("\n\nInterrompido pelo utilizador.")
        print(f"Dados gravados em: {filename}")
        
    except serial.SerialException as e:
        print(f"Erro ao abrir porta serial: {e}")
        print("\nPortas disponíveis:")
        ports = serial.tools.list_ports.comports()
        for port in ports:
            print(f"  - {port.device}: {port.description}")
    
    except Exception as e:
        print(f"Erro: {e}")
    
    finally:
        running = False
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Porta serial fechada.")

if __name__ == "__main__":
    main()
