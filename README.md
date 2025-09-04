# 🤖 Self-Balancing Vertical Robot

<p align="left">
  <img src="media/robot_front.jpg" alt="Front View of Robot" width="600"/>
</p>

[![Build Status](https://img.shields.io/github/actions/workflow/status/BeratTezer/self-balancing-vertical-robot/build.yml?branch=main)](https://github.com/BeratTezer/self-balancing-vertical-robot/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Citation](https://img.shields.io/badge/Cite-APA%20Style-orange)](CITATION.cff)
[![Docs](https://img.shields.io/badge/Docs-📚-purple)](https://github.com/BeratTezer/self-balancing-vertical-robot)

The **Self-Balancing Vertical Robot** is a modular research platform featuring a 3D-printed chassis, low-cost electronic components, and open-source software, designed for both **academic research** and **industrial prototyping**.

---

## 📑 Table of Contents

- [✨ Features](#-features)<br>
- [⚙️ Hardware (BOM)](#️-hardware-bom)<br>
- [🖨️ 3D Printing](#️-3d-printing)<br>
- [💻 Software](#-software)<br>
- [🧠 Algorithms](#-algorithms)<br>
- [🧪 Tests & Results](#-tests--results)<br>
- [🗺 Roadmap](#-roadmap)<br>
- [🤝 Contribution & License](#-contribution--license)<br>
- [🔖 Citation](#-citation)

---

## ✨ Features

- ⚡ Arduino UNO-based control system
- 🎥 IMU-based motion sensing and orientation estimation
- 🔋 Modular chassis powered by a 2S Li-ion battery pack
- 🖨️ Fully customizable using 3D-printed components

---

## ⚙️ Hardware (BOM)

| Component    | Qty | Description     | Link                                                                                                                                                    |
| ------------ | --- | --------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Arduino UNO  | 1   | Main controller | [Datasheet](https://docs.arduino.cc/resources/datasheets/A000066-datasheet.pdf)                                                                         |
| Hobby Motor  | 2   | Actuator        | [Example Sale Link](https://www.botnroll.com/en/dc-motor/2975-hobby-gearmotor-200rpm-65mm-wheel.html)                                                   |
| Motor Wheel  | 2   | Wheels          | [Example Sale Link](https://www.botnroll.com/en/dc-motor/2975-hobby-gearmotor-200rpm-65mm-wheel.html)                                                   |
| MPU6050      | 1   | IMU sensor      | [Pinout](https://components101.com/sensors/mpu6050-module)                                                                                              |
| L298N        | 1   | Motor driver    | [Datasheet](https://www.handsontec.com/dataspecs/L298N%20Motor%20Driver.pdf)                                                                            |
| 18650 Li-ion | 2   | Power supply    | [Datasheet](https://www.mouser.com/datasheet/2/855/ASR00050_18650_2500mAh-3078640.pdf?srsltid=AfmBOoqWewHHNYpYeiml4Sg5m18XAfgiU2DQwppWHurnN8My09zVFvFn) |
| Chassi       | 1   | Robot Body      | [STL File](hardware/3d-model/robot_chassi.md) & [Link](https://makerworld.com/en/models/1538080-balancing-robot-chassi#profileId-1613796)               |

For the extended Bill of Materials, see: [`hardware/bom.md`](hardware/bom.md)

---

## 🖨️ 3D Printing

- STL/STEP files: [`hardware/3d-models/`](hardware/3d-models/)
- Recommended printing settings:
  - Layer height: **0.2 mm**
  - Infill: **20–40%**
  - Material: **PLA**

---

## 💻 Software

📂 Project structure:

```
software/
├── main.ino        # System initialization, setup(), loop(), main control loop (1 kHz)
```

---

### 🚀 Quick Start

**Arduino IDE:**

1. Install Arduino Uno board definition.
2. Open `src/main.cpp`.
3. Install libraries: `Wire`, `MPU6050`.
4. Upload to board.

---

## 🧠 Algorithms

- **PID Control** → Used for balancing, includes proportional, integral, and derivative terms with gyro-based derivative filtering.
- **Complementary Filter** → Combines accelerometer and gyroscope data for stable tilt estimation.
- **Slew Limiter** → Prevents sudden jumps in motor commands by limiting step changes.
- **Safety Cut-off** → Motors stop if tilt exceeds `±60°` (SAFE_TILT).

---

## 🧪 Tests & Results

- ⚡ **Maximum speed**: ~1.2 m/s
- 🔋 **Battery life**: ~40 minutes continuous use
- 🎯 **PID tilt error**: ±2°
- 🛠️ Includes built-in **motor self-test** and **IMU calibration** routines (run on startup).

---

## 🗺 Roadmap

(maybe)

- [ ] Implementing path finding algorithm
- [ ] ROS2 entegration

---

## 🤝 Contribution & License

Contributions are warmly welcomed. Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) file for guidelines.  
This project is released under the **MIT License**.

---

## 🔖 Citation

If you use this project in your work, please cite it according to the **CITATION.cff** file.
