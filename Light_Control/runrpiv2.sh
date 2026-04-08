#!/usr/bin/env bash
clear
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

declare -A RPI_USB_IDS=(
  [rpi0]="${RPI0_USB_ID:-${RPI1_USB_ID:-E66118604B886021}}"
  [rpi1]="${RPI1_USB_ID:-${RPI2_USB_ID:-E66118604B497921}}"
  [rpi2]="${RPI2_USB_ID:-${RPI3_USB_ID:-E66118604B296221}}"
)

declare -A PIO_ENVS=(
  [rpi0]="pico_rpi1"
  [rpi1]="pico_rpi2"
  [rpi2]="pico_rpi3"
)

declare -A EFFECTIVE_USB_IDS=()
declare -A USED_USB_IDS=()

resolve_port_for_target() {
  local target="$1"
  local usb_id="${EFFECTIVE_USB_IDS[$target]:-${RPI_USB_IDS[$target]}}"
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

list_available_usb_ids() {
  local entries=()
  shopt -s nullglob
  entries=(/dev/serial/by-id/usb-Raspberry_Pi_Pico_*)
  shopt -u nullglob

  for path in "${entries[@]}"; do
    local base
    local tail
    local id
    base="$(basename "$path")"
    tail="${base#usb-Raspberry_Pi_Pico_}"
    id="${tail%%-*}"
    [[ -n "$id" ]] && printf '%s\n' "$id"
  done
}

choose_usb_id_for_target() {
  local target="$1"
  local expected="${RPI_USB_IDS[$target]}"
  shift
  local available=("$@")

  for id in "${available[@]}"; do
    if [[ "$id" == "$expected" && -z "${USED_USB_IDS[$id]:-}" ]]; then
      printf '%s\n' "$id"
      return 0
    fi
  done

  for id in "${available[@]}"; do
    if [[ -z "${USED_USB_IDS[$id]:-}" ]]; then
      printf '%s\n' "$id"
      return 0
    fi
  done

  return 1
}

wait_for_port() {
  local port="$1"
  local timeout_s="${2:-25}"
  local end=$((SECONDS + timeout_s))
  while (( SECONDS < end )); do
    [[ -e "$port" ]] && return 0
    sleep 0.2
  done
  return 1
}

if [[ -x "$HOME/.platformio/penv/bin/pio" ]]; then
  PIO_CMD="$HOME/.platformio/penv/bin/pio"
elif command -v pio >/dev/null 2>&1; then
  PIO_CMD="pio"
else
  echo "PlatformIO nao encontrado."
  exit 1
fi

mapfile -t AVAILABLE_USB_IDS < <(list_available_usb_ids)
if [[ ${#AVAILABLE_USB_IDS[@]} -lt 3 ]]; then
  echo "Erro: preciso de 3 RPis ligados para este launcher (encontrei ${#AVAILABLE_USB_IDS[@]})."
  ls -1 /dev/serial/by-id 2>/dev/null || true
  echo "Liga os 3 ou usa os scripts individuais ./run_rpi.sh rpiX."
  exit 1
fi

echo "IDs USB disponiveis: ${AVAILABLE_USB_IDS[*]}"
for target in rpi0 rpi1 rpi2; do
  chosen_id="$(choose_usb_id_for_target "$target" "${AVAILABLE_USB_IDS[@]}")" || {
    echo "Nao consegui mapear USB ID para $target"
    exit 1
  }
  EFFECTIVE_USB_IDS[$target]="$chosen_id"
  USED_USB_IDS[$chosen_id]=1

  if [[ "$chosen_id" != "${RPI_USB_IDS[$target]}" ]]; then
    echo "Aviso: $target esperado=${RPI_USB_IDS[$target]} -> mapeado para $chosen_id"
  fi
done

echo "[1/3] Programar rpi1, rpi2, rpi0"
for target in rpi1 rpi2 rpi0; do
  env_name="${PIO_ENVS[$target]}"

  echo "A detetar porta de $target (ID ${EFFECTIVE_USB_IDS[$target]})..."
  if ! port="$(resolve_port_for_target "$target")"; then
    echo "Porta de $target nao encontrada em /dev/serial/by-id"
    ls -1 /dev/serial/by-id 2>/dev/null || true
    exit 1
  fi

  echo " - $target -> $port ($env_name)"
  wait_for_port "$port" 25 || { echo "Porta indisponivel: $port"; exit 1; }

  ok=0
  for attempt in 1 2 3; do
    echo "   Upload tentativa $attempt/3"
    if "$PIO_CMD" run -e "$env_name" -t upload --upload-port "$port"; then
      ok=1
      break
    fi
    wait_for_port "$port" 25 || true
  done

  if [[ "$ok" -ne 1 ]]; then
    echo "Upload falhou para $target"
    exit 1
  fi
done

CSV_FILE="adc_data_all_$(date +%Y%m%d_%H%M%S).csv"

echo "[2/3] Abrir plotrealtimeV2"
python3 plotrealtimeV2.py --csv-file "$CSV_FILE" --wait &
PLOT_PID=$!

cleanup() {
  if kill -0 "$PLOT_PID" >/dev/null 2>&1; then
    kill "$PLOT_PID" >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT INT TERM

echo "[3/3] Abrir interface_serial_multi (terminal unico)"
RPI0_USB_ID="${EFFECTIVE_USB_IDS[rpi0]}" \
RPI1_USB_ID="${EFFECTIVE_USB_IDS[rpi1]}" \
RPI2_USB_ID="${EFFECTIVE_USB_IDS[rpi2]}" \
python3 interface_serial_multi.py --csv-file "$CSV_FILE" --default-rpi rpi0
