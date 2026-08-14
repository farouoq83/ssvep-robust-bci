# Bill of Materials — SSVEP/c-VEP Stimulus Panel

| Qty | Component | Spec / Part | Purpose |
|---|---|---|---|
| 1 | Microcontroller | STM32F103C8T6 ("Blue Pill") | Drives all 5 LED clusters via hardware timers (SSVEP + c-VEP modes) |
| 1 | Programmer | ST-Link V2 | Flashing/debugging the STM32 |
| 1 | Driver IC | ULN2003AG (Darlington transistor array) | Drives each LED cluster above the MCU's per-pin current limit |
| 17 | LEDs | 5 mm white "Hyperlight", V=3–3.2 V, I=15–20 mA | 3 per side key (Fwd/Back/Left/Right), 5 for STOP |
| — | Resistors | ~100–120 Ω, per LED (for 5 V rail) | Current limiting |
| 1 | Photoresistor (LDR) | — (any standard CdS type) | Verifies true LED flicker frequency via spare ADC input |
| 1 | Bluetooth module | HC-05 | Trial-synchronization marker to recording PC (**not yet implemented in firmware**) |
| 1 | Boost converter | XL6009E1 | Steps supply voltage up before regulation |
| 1 | Linear regulator | L7805CV | Provides clean, regulated 5 V rail |
| — | Diffuser material | Tracing paper or frosted acrylic | Even light distribution over each LED cluster |
| 1 | Panel base | 10 cm × 10 cm rigid board | Physical mount |
| — | Assembly tools | Breadboard/perfboard, soldering iron, multimeter, oscilloscope or logic analyzer | Assembly and timer-frequency verification before human-subject use |

## Firmware parameters (for reference — see `firmware/main_combined_ssvep_cvep.c`)

**SSVEP mode** (hardware timer Output Compare Toggle, TIM2/TIM3):
| Key | Frequency |
|---|---|
| Forward | 12 Hz |
| Backward | 8 Hz |
| Left | 13 Hz |
| Right | 9 Hz |
| STOP | 10 Hz |

**c-VEP mode** (TIM4, ~60 Hz update rate): 127-bit maximal-length sequence (7-bit LFSR,
primitive polynomial x⁷ + x⁶ + 1), cyclic shifts per key:
| Key | Shift (chips) |
|---|---|
| Forward | 0 |
| Backward | 25 |
| Left | 50 |
| Right | 75 |
| STOP | 100 |

## Safety note

⚠️ The XL6009E1 → L7805CV chain described above is a **mains-derived** power supply. Before
any actual human-subject EEG recording session, replace this with a **battery-powered** supply
for the stimulus panel, to eliminate any electrical connection between the subject and mains
power.
