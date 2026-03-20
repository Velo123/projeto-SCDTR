#!/usr/bin/env python3
#time,luminaire ID,lux_ref,lux_meas,LDRvoltage,LDRresistance,duty cycle,windup state,feedback state
"""
Script para ler dados do ADC do Raspberry Pi Pico e gravar num ficheiro.
"""

import serial
import serial.tools.list_ports
import time
from datetime import datetime

def find_pico_port():
    """Procura automaticamente a porta do Raspberry Pi Pico."""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        # O Pico geralmente aparece como "USB Serial Device" ou similar
        if "USB" in port.description or "ACM" in port.device or "ttyUSB" in port.device:
            return port.device
    return None

def main():
    # Tenta encontrar a porta automaticamente
    port = find_pico_port()
    
    if not port:
        print("Porta USB não encontrada automaticamente.")
        port = input("Insira a porta serial (ex: /dev/ttyACM0 ou /dev/ttyUSB0): ")
    else:
        print(f"Porta encontrada: {port}")
    
    # Nome do ficheiro com timestamp
    filename = f"adc_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
    
    try:
        # Abre conexão serial
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(2)  # Aguarda inicialização
        
        print(f"Conectado a {port}")
        print(f"A gravar dados em: {filename}")
        print("Pressione Ctrl+C para parar...\n")
        
        # Abre ficheiro para escrita
        with open(filename, 'w') as f:
            # Escreve cabeçalho
            f.write("timestamp_ms,adc_value,voltage_v\n")
            
            while True:
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    
                    if line:
                        print(line)  # Mostra no terminal
                        
                        # Grava no ficheiro se for uma linha de dados válida
                        if ',' in line and not line.startswith("Sistema") and not line.startswith("LED"):
                            f.write(line + "\n")
                            f.flush()  # Garante que os dados são escritos imediatamente
                
    except KeyboardInterrupt:
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
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Porta serial fechada.")

if __name__ == "__main__":
    main()
