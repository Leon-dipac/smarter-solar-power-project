# Portable Solar Smart Control System

**Learn Like Leon** · Arduino Uno · IR remote · capacitor-bank mode selection · PWM load control

![Learn Like Leon — Portable Solar Smart Control System promotional flyer](https://github-portfolio-images.s3.us-east-1.amazonaws.com/smarter-solar-power-project/Portable+Solar+Smart+Control+System.png)

**Website:** [learnlikeleon.com](https://learnlikeleon.com/)  
**Status:** Educational prototype / Simulation

## Project overview

An Arduino-controlled educational prototype exploring solar-powered capacitor storage, user-requested output voltages, relay mode selection, PWM transistor drive, and voltage monitoring on a 16×2 I²C LCD.

A foldable solar panel is a **proposed portability upgrade** to the fixed solar source shown in the original Tinkercad simulation. Its output and charging interface would need to be checked before integrating it with the physical circuit.

## Key features

- IR remote accepts two-digit voltage commands, such as `2` then `5` → **2.50 V**.
- LCD immediately shows the first key as `Input = 2.00 V` and waits for the second key.
- Completed input, such as `Input = 3.20 V`, remains on the bottom row for **one second** before normal readings return.
- For target voltages **below 3.50 V**, software requests **PARALLEL**; at **3.50 V or above**, it requests **SERIES**.
- Arduino **D12** commands the relay driver; **D5** produces open-loop PWM to the output transistor.
- **A0**, **A1**, and **A3** monitor bank, individual-capacitor, and load-output voltages through voltage dividers.
- I²C LCD (address `0x27`) displays requested capacitor mode, filtered load voltage, capacitor readings, and the `Emani` label.

## Project files

```text
learn-like-leon-solar-controller/
├── README.md
└── src/
    └── smarter_solar_power_controller.ino
```

## Hardware overview

| Hardware                           | Arduino interface   |
| ---------------------------------- | ------------------- |
| IR receiver OUT                    | D3                  |
| DPDT relay transistor driver       | D12                 |
| PWM transistor base driver         | D5                  |
| Total capacitor-bank voltage sense | A0                  |
| Individual capacitor voltage sense | A1                  |
| Load-output voltage sense          | A3                  |
| I²C LCD SDA                        | A4                  |
| I²C LCD SCL                        | A5                  |
| LCD power                          | Arduino 5 V and GND |

The design documentation has used a 20 V solar-cell simulation and two **2,200 µF / 20 V** polarized storage capacitors. Component ratings alone do **not** establish a safe charging or series/parallel switching design.

## Supported IR commands

| Keys      | Requested output | Software mode request | D12 command |
| --------- | ---------------: | --------------------- | ----------- |
| `2` → `0` |           2.00 V | Parallel              | LOW         |
| `2` → `5` |           2.50 V | Parallel              | LOW         |
| `3` → `2` |           3.20 V | Parallel              | LOW         |
| `3` → `5` |           3.50 V | Series                | HIGH        |

The included code recognizes raw remote-button codes for digits `0`, `2`, `3`, and `5`. Other remotes may transmit different codes.

## How the sketch works

1. `setup()` initializes the display, IR receiver, serial port, relay output, PWM output, and default target of **2.00 V**.
2. `checkIRRemote()` decodes an IR button and debounces accepted input by 120 ms.
3. `processDigit()` displays the first digit and waits for the second. The completed value is shown for one second, then the normal LCD display returns.
4. `selectCapacitorMode()` decides the **requested** series or parallel configuration at a **3.50 V** threshold.
5. `updateRelayOutput()` drives D12 HIGH for requested series or LOW for requested parallel.
6. `analogWrite(D5, pwmValue)` applies a PWM duty cycle calculated from `targetVoltage × 51`, limited to 0–255.
7. `readVoltage()` averages 100 ADC samples for each of A0, A1, and A3. Software smooths the A3 reading before showing it on the LCD.

![Learn Like Leon — Portable Solar Smart Control System - Output view](https://github-portfolio-images.s3.us-east-1.amazonaws.com/smarter-solar-power-project/Smarter+Solar+Power+Project_Output.png)

### Important limitation: PWM is not voltage regulation

The sketch uses **open-loop PWM**. It does not adjust the duty cycle based on `loadVoltage`, and the NPN emitter-follower stage cannot be assumed to provide a regulated 0–20 V DC output. For accurate regulated DC output, use an appropriately rated power converter and validated closed-loop feedback.

### Safety and electrical validation

**The schematic and sketch are educational prototype material, not a validated high-energy switching design.** Before connecting charged capacitors, verify relay contact identification, contact transitions, current limiting, charging regulation, and protection against short-circuit/equalization currents. The D12 command does not verify the actual relay contact state. A 20 V solar panel should not directly charge capacitors without appropriate voltage/current control. Measure floating series capacitors using suitable sensing circuitry; an Arduino analog pin measures relative to Arduino GND. If the load negative is not ground-referenced, A3 alone does not measure the true voltage across the load.

---

**Learn Like Leon** · [learnlikeleon.com](https://learnlikeleon.com/)  
_Portable solar smart control — educational prototype._
