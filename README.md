# Smart Medication Reminder (Pressure Sensing)

![Platform](https://img.shields.io/badge/Platform-Arduino%20UNO-blue.svg)
![Sensor](https://img.shields.io/badge/Sensor-FSR402%20Pressure-orange.svg)
![Module](https://img.shields.io/badge/Module-DS3231%20RTC%20%7C%20LCD%20I2C-green.svg)
![License](https://img.shields.io/badge/License-CC%20BY--NC%204.0-lightgrey.svg)

## 📖 Introduction

**Smart Medication Reminder** is a prototype device aimed at improving medication adherence for elderly patients or individuals with chronic conditions. 

Unlike traditional alarms that can be simply silenced with a button press, this system enforces compliance through physical interaction. It features a **Force Sensitive Resistor (FSR)** pad where the medicine bottle sits. When the alarm rings, the user **must physically lift the bottle** to stop the buzzer, ensuring the medication is at least handled at the correct time.

### ✨ Key Features
* **Force-Feedback Alarm Stop:** The buzzer will persist until the pressure sensor detects the removal of the medicine bottle (`pressureValue < 20`).
* **Precision Timing:** Uses a **DS3231 RTC (Real Time Clock)** module for accurate timekeeping, even during power loss (battery backed).
* **Interactive UI:** Features a 16x2 LCD and a 3-button menu system for easy time setting and status monitoring.
* **Daily Recurrence:** Automatically schedules the next alarm for the following day once the current dose is taken.
* **State Machine Architecture:** Robust firmware design manages different modes (Clock Display, Set Hour, Set Minute) without logic conflicts.

---

## 🛠️ Hardware & Bill of Materials (BOM)

| Component | Type | Function |
| :--- | :--- | :--- |
| **Microcontroller** | Arduino Uno R3 | Central Processing Unit |
| **Timekeeping** | DS3231 RTC Module | Real-time clock with battery backup |
| **Display** | LCD 1602 (I2C) | Interface for time display and settings |
| **Sensor** | FSR402 (Force Sensitive Resistor) | Detects the presence/weight of the pill bottle |
| **Audio Output** | Active Buzzer Module | Audible alarm alert |
| **Input** | 3x Tactile Buttons | Menu Navigation (+, -, Confirm) |
| **Resistor** | 1x 10kΩ | Pull-down resistor for FSR sensor |
| **Wiring** | Breadboard & Jumpers | Circuit assembly |

---

## 🔌 Circuit Diagram

The following Fritzing diagram illustrates the connections between the RTC, LCD, FSR Sensor, and the User Interface buttons.

![Alarm clock _bb](https://github.com/user-attachments/assets/cee1c3d9-cabf-4107-8d29-ca10dfa5f06a)


### Pin Configuration
* **FSR Sensor:** Analog Pin `A0` (Voltage Divider Circuit)
* **Buzzer:** Digital Pin `11`
* **Buttons:**
    * Plus: Pin `8`
    * Minus: Pin `9`
    * Confirm: Pin `10`
* **I2C Bus (LCD & RTC):** `SDA` (A4), `SCL` (A5)

---

## 💻 How It Works (Logic Flow)

1.  **Idle State:** The LCD displays the current time (from RTC) and the next scheduled alarm.
2.  **Alarm Trigger:** When `Current Time == Alarm Time`, the system activates the buzzer.
3.  **Compliance Check:** The system continuously polls the FSR sensor (`analogRead(A0)`).
    * If **Pressure > Threshold**: Bottle is still on the pad -> **Keep Ringing**.
    * If **Pressure < Threshold**: Bottle is lifted -> **Stop Alarm**.
4.  **Reset:** Once the bottle is lifted, the alarm is deactivated, and the next reminder is automatically set for the same time tomorrow.

---

## 📸 Prototype Gallery

### 1. Hardware Setup

<img width="257" height="211" alt="屏幕截图 2025-11-24 003002" src="https://github.com/user-attachments/assets/3a4002bc-406a-49d3-9a8f-0cb1ea01b869" />


---

## ©️ Intellectual Property & License

**Copyright © 2025 Chen Junxu. All Rights Reserved.**

### ⚠️ Disclaimer (Prototype Use Only)
This repository contains **prototype firmware and schematics** developed for educational purposes. It is not a certified medical device.

### 📜 Usage Policy (CC BY-NC 4.0)
This project is licensed under the **Creative Commons Attribution-NonCommercial 4.0 International License**.

1.  **Non-Commercial Use:** Free for personal learning and academic research.
2.  **No Commercial Deployment:** Strictly prohibited for commercial use without permission.
3.  **Attribution:** Please credit the author (**Chen Junxu**) when using this work.
