# Leitor de ADC do Raspberry Pi Pico

Sistema para ler tensão do pino A0 do ADC do Raspberry Pi Pico e gravar os dados num ficheiro no PC.

## Hardware

- **Pino ADC**: A0 (GPIO26)
- **Pino LED**: GPIO9
- **Tensão de referência**: 3.3V

## Funcionalidades

- Lê continuamente a tensão do pino A0
- Envia dados pela porta série (115200 baud)
- Acende LED no GPIO9 após 2 segundos de funcionamento
- Amostragem a cada 100ms

## Como usar

### 1. Compilar e carregar o código no Pico

```bash
cd /home/velo/Documentos/Fac/5ano/SCDTR/projeto/systemtau
pio run -t upload
```

### 2. Instalar dependências Python (primeira vez)

```bash
pip install pyserial
```

### 3. Executar o script de captura

```bash
python3 read_adc_data.py
```

O script irá:
- Procurar automaticamente a porta do Pico
- Criar um ficheiro CSV com timestamp (ex: `adc_data_20260304_143025.csv`)
- Mostrar os dados no terminal em tempo real
- Gravar continuamente no ficheiro

### 4. Parar a captura

Pressione `Ctrl+C` para parar. Os dados serão gravados no ficheiro CSV.

## Formato dos dados

O ficheiro CSV contém:
```
timestamp_ms,adc_value,voltage_v
1234,512,1.6500
2345,768,2.4750
...
```

- **timestamp_ms**: Milissegundos desde o início
- **adc_value**: Valor bruto do ADC (0-1023)
- **voltage_v**: Tensão em Volts (0-3.3V)

## Ligações

```
Raspberry Pi Pico
├─ GPIO9  → LED (+ resistência 220Ω)
└─ A0/GPIO26 → Sinal analógico (0-3.3V)
```

⚠️ **Atenção**: Nunca aplique tensões superiores a 3.3V no pino A0!
