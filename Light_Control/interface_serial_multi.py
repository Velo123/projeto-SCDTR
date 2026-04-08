#!/usr/bin/env python3
"""Interface serial unificada para 3 Raspberry Pi Pico.

- Lê stream dos 3 nós e grava num único CSV para plot realtime.
- Mostra respostas no terminal com prefixo [rpiX].
- Permite enviar comandos para um nó específico:
    @rpi0 g d 1
    @rpi1 g L 0
  @all status
- Sem prefixo, envia para o nó default (por omissão: rpi0).
"""

import argparse
import glob
import os
import threading
import time
from datetime import datetime

import serial


RPI_USB_IDS = {
    "rpi0": os.getenv("RPI0_USB_ID", os.getenv("RPI1_USB_ID", "E66118604B886021")),
    "rpi1": os.getenv("RPI1_USB_ID", os.getenv("RPI2_USB_ID", "E66118604B497921")),
    "rpi2": os.getenv("RPI2_USB_ID", os.getenv("RPI3_USB_ID", "E66118604B296221")),
}


def parse_args():
    parser = argparse.ArgumentParser(description="Interface serial unificada (3 RPis)")
    parser.add_argument(
        "--csv-file",
        help="Caminho do ficheiro CSV para gravar o stream em tempo real",
    )
    parser.add_argument(
        "--default-rpi",
        choices=["rpi0", "rpi1", "rpi2"],
        default="rpi0",
        help="Destino por defeito quando comando não tem prefixo",
    )
    parser.add_argument(
        "--list",
        action="store_true",
        help="Lista portas /dev/serial/by-id detetadas e termina",
    )
    parser.add_argument(
        "--clock-source",
        choices=["host", "device"],
        default="host",
        help="Fonte de tempo para CSV: host (PC, sincronizado) ou device (timestamp do Pico)",
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
    return matches[0] if matches else None


def wait_for_rpi_port(rpi_name, timeout_s=20.0, poll_s=0.2):
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


def parse_targeted_command(raw_cmd, default_target):
    cmd = raw_cmd.strip()
    if not cmd:
        return None, None

    if cmd.lower() == "/help":
        return "__help__", ""

    if cmd.startswith("@"):
        parts = cmd.split(" ", 1)
        target = parts[0][1:].strip().lower()
        payload = parts[1].strip() if len(parts) > 1 else ""
        if not payload:
            return None, None
        return target, payload

    return default_target, cmd


def build_csv_line(raw_stream_line, host_ms, clock_source):
    parts = [p.strip() for p in raw_stream_line.split(",")]
    # raw format from firmware:
    # - old: time,luminaire_id,lux_meas,duty_cycle
    # - new: time,luminaire_id,lux_ref,lux_meas,duty_cycle
    if len(parts) == 4:
        parts = [parts[0], parts[1], "nan", parts[2], parts[3]]
    elif len(parts) != 5:
        return None

    if clock_source == "host":
        parts[0] = str(host_ms)

    return ",".join(parts)


def main():
    args = parse_args()

    if args.list:
        list_serial_by_id()
        return

    filename = args.csv_file or f"adc_data_multi_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
    t0_host = time.monotonic()

    serial_links = {}
    serial_ports = {}

    for rpi in ["rpi0", "rpi1", "rpi2"]:
        port = resolve_rpi_port_by_id(rpi)
        if not port:
            print(f"A aguardar porta do {rpi} aparecer...")
            port = wait_for_rpi_port(rpi, timeout_s=20.0)
        if not port:
            print(f"Erro: não encontrei porta para {rpi}")
            list_serial_by_id()
            return
        serial_links[rpi] = port

    try:
        for rpi in ["rpi0", "rpi1", "rpi2"]:
            ser = serial.Serial(serial_links[rpi], 115200, timeout=0.05)
            serial_ports[rpi] = ser

        time.sleep(2.0)

        print("Conectado aos 3 RPis:")
        for rpi, link in serial_links.items():
            print(f"  - {rpi}: {link}")

        print(f"A gravar stream em: {filename}")
        print(f"Clock para CSV: {args.clock_source}")
        print("Comandos:")
        print("  - @rpi0 <cmd>  (envia para rpi0)")
        print("  - @rpi1 <cmd>  (envia para rpi1)")
        print("  - @rpi2 <cmd>  (envia para rpi2)")
        print("  - @all <cmd>   (envia para os 3)")
        print(f"  - sem prefixo: envia para {args.default_rpi}")
        print("  - /help para reimprimir ajuda")
        print("Ctrl+C para sair.\n")

        running = True
        write_lock = threading.Lock()

        with open(filename, "w", buffering=1) as csv_file:
            csv_file.write("time,luminaire_id,lux_ref,lux_meas,duty_cycle\n")

            def read_commands():
                nonlocal running
                while running:
                    try:
                        cmd_in = input()
                    except EOFError:
                        break
                    except KeyboardInterrupt:
                        running = False
                        break

                    target, payload = parse_targeted_command(cmd_in, args.default_rpi)
                    if target is None:
                        continue

                    if target == "__help__":
                        print("Uso: @rpi0 <cmd> | @rpi1 <cmd> | @rpi2 <cmd> | @all <cmd>")
                        print(f"Sem prefixo, envia para {args.default_rpi}")
                        continue

                    if target == "all":
                        for rpi, ser in serial_ports.items():
                            try:
                                ser.write((payload + "\n").encode("utf-8"))
                                print(f"[Enviado->{rpi}] {payload}")
                            except Exception as exc:
                                print(f"Erro ao enviar para {rpi}: {exc}")
                        continue

                    if target not in serial_ports:
                        print(f"Destino inválido: {target}")
                        continue

                    try:
                        serial_ports[target].write((payload + "\n").encode("utf-8"))
                        print(f"[Enviado->{target}] {payload}")
                    except Exception as exc:
                        print(f"Erro ao enviar para {target}: {exc}")

            cmd_thread = threading.Thread(target=read_commands, daemon=True)
            cmd_thread.start()

            while running:
                activity = False
                for rpi, ser in serial_ports.items():
                    try:
                        if ser.in_waiting <= 0:
                            continue
                        line = ser.readline().decode("utf-8", errors="ignore").strip()
                        if not line:
                            continue

                        activity = True
                        if is_stream_csv_line(line):
                            host_ms = int((time.monotonic() - t0_host) * 1000.0)
                            out_line = build_csv_line(line, host_ms, args.clock_source)
                            if out_line is None:
                                continue
                            with write_lock:
                                csv_file.write(out_line + "\n")
                        else:
                            print(f"[{rpi}] {line}")
                    except Exception as exc:
                        print(f"Erro a ler de {rpi}: {exc}")

                if not activity:
                    time.sleep(0.01)

    except KeyboardInterrupt:
        print("\nInterrompido pelo utilizador.")
    finally:
        for ser in serial_ports.values():
            try:
                if ser.is_open:
                    ser.close()
            except Exception:
                pass
        print(f"Portas serial fechadas. CSV: {filename}")


if __name__ == "__main__":
    main()
