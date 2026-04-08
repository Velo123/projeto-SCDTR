#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

declare -A RPI_USB_IDS=(
  [rpi0]="E66118604B886021"
  [rpi1]="E66118604B497921"
  [rpi2]="E66118604B296221"
)

resolve_port_for_target() {
  local target="$1"
  local usb_id="${RPI_USB_IDS[$target]}"
  local pattern="/dev/serial/by-id/usb-Raspberry_Pi_Pico_${usb_id}*"
  local matches=()

  shopt -s nullglob
  matches=( $pattern )
  shopt -u nullglob

  if [[ ${#matches[@]} -gt 0 ]]; then
    printf '%s\n' "${matches[0]}"
    return 0
  fi
  return 1
}

wait_for_port() {
  local port="$1"
  local timeout_s="${2:-20}"
  local end=$((SECONDS + timeout_s))

  while (( SECONDS < end )); do
    if [[ -e "$port" ]]; then
      return 0
    fi
    sleep 0.2
  done
  return 1
}

upload_with_retry() {
  local env="$1"
  local port="$2"

  for attempt in 1 2 3; do
    echo "[1/3] Upload para $TARGET ($env) - tentativa $attempt/3..."
    if "$PIO_CMD" run -e "$env" -t upload --upload-port "$port"; then
      return 0
    fi
    echo "Upload falhou (tentativa $attempt)."
    echo "A aguardar reconexão da porta no hub..."
    wait_for_port "$port" 20 || true
  done

  return 1
}

if [[ $# -ne 1 ]]; then
  echo "Uso: $0 rpi0|rpi1|rpi2"
  exit 1
fi

TARGET="$1"
case "$TARGET" in
  rpi0)
    PIO_ENV="pico_rpi1"
    ;;
  rpi1)
    PIO_ENV="pico_rpi2"
    ;;
  rpi2)
    PIO_ENV="pico_rpi3"
    ;;
  *)
    echo "Alvo invalido: $TARGET"
    echo "Use: rpi0, rpi1 ou rpi2"
    exit 1
    ;;
esac

if [[ -x "$HOME/.platformio/penv/bin/pio" ]]; then
  PIO_CMD="$HOME/.platformio/penv/bin/pio"
elif command -v pio >/dev/null 2>&1; then
  PIO_CMD="pio"
else
  echo "PlatformIO nao encontrado."
  exit 1
fi

SERIAL_PORT=""
if ! SERIAL_PORT="$(resolve_port_for_target "$TARGET")"; then
  echo "Porta do $TARGET nao encontrada em /dev/serial/by-id"
  echo "Dispositivos disponiveis:"
  ls -1 /dev/serial/by-id 2>/dev/null || echo "  (nenhum)"
  exit 1
fi

echo "Porta resolvida para $TARGET: $SERIAL_PORT"
if ! wait_for_port "$SERIAL_PORT" 20; then
  echo "Porta nao ficou disponivel a tempo: $SERIAL_PORT"
  exit 1
fi

if ! upload_with_retry "$PIO_ENV" "$SERIAL_PORT"; then
  echo "Erro: upload falhou apos 3 tentativas."
  exit 1
fi

CSV_FILE="adc_data_${TARGET}_$(date +%Y%m%d_%H%M%S).csv"

echo "[2/3] A abrir plot realtime..."
python3 plot_realtime.py --csv-file "$CSV_FILE" --wait &
PLOT_PID=$!

cleanup() {
  if kill -0 "$PLOT_PID" >/dev/null 2>&1; then
    kill "$PLOT_PID" >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT INT TERM

echo "[3/3] A abrir interface serial de $TARGET..."
python3 interface_serial.py --rpi "$TARGET" --port "$SERIAL_PORT" --csv-file "$CSV_FILE"
