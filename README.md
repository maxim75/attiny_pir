# ATtiny PIR Motion Sensor

A battery-powered PIR motion sensor node built on the ATtiny3224 microcontroller. Detects motion events and wirelessly transmits sensor data (motion, battery voltage, ambient light) via an nRF24L01 radio module.

## Features

- **Ultra-low power** — sleeps in `POWER_DOWN` mode between motion events; ADC disabled during sleep
- **PIR motion detection** — wakes on falling-edge interrupt from PIR sensor
- **Ambient light sensing** — suppresses RF transmissions in bright conditions (configurable threshold)
- **Battery monitoring** — reads and reports battery voltage in millivolts
- **nRF24L01 wireless** — 250 kbps data rate, low power (PA_LOW) transmit

## Hardware

| Component | Pin |
|-----------|-----|
| nRF24L01 CE | PA5 |
| nRF24L01 CSN | PA7 |
| Battery voltage (ADC) | PA4 |
| PIR sensor interrupt | PA6 |
| Status LED | PB0 |
| Light sensor (ADC) | PB1 |

## Message Format

Sensor data is transmitted as a 32-byte ASCII string:

```
PIR14_S:1_B:3300_L:512
```

| Field | Description |
|-------|-------------|
| `PIR14` | Device ID |
| `S:1` | Status flag |
| `B:XXXX` | Battery voltage in mV (zero-padded to 4 digits) |
| `L:XXXX` | Raw ADC light sensor reading |

A start message (`PIR14_start`) is sent once on startup.

## Configuration

Key constants in [src/main.cpp](src/main.cpp):

| Constant | Default | Description |
|----------|---------|-------------|
| `DEVICE_ID` | `"PIR14"` | Unique node identifier |
| `RADIO_ADDRESS` | `0xFAB7C2F0E2` | nRF24L01 pipe address |
| `LIGHT_THRESHOLD` | `950` | ADC value above which TX is skipped (daytime suppression) |
| `BATTERY_VOLTAGE_MULTIPLIER` | `6.46` | ADC-to-millivolt scaling factor |
| `RF24_POWER_LEVEL` | `RF24_PA_LOW` | Transmit power level |

## Build & Flash

Requires [PlatformIO](https://platformio.org/) and `pymcuprog`.

```bash
# Build
pio run

# Flash via UART (pymcuprog)
pio run --target upload

# Serial monitor (9600 baud)
pio device monitor
```

Target board: **ATtiny3224** at 5 MHz (internal oscillator), uploaded over UART at 115200 baud.

## Dependencies

- [nrf24/RF24](https://github.com/nRF24/RF24) `^1.4.10`
