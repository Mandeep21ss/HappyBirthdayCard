# 🎂 Happy Birthday Greeting Device

An interactive Happy Birthday greeting device built with **ESP32**, an **SH1106 OLED display**, and a **passive buzzer**. Touch the sensor to trigger a real-time celebration animation with the Happy Birthday melody!

---

## 📷 Features

- 🎵 Plays **Happy Birthday to You** melody at 140 BPM
- 🎈 Animated **floating balloons** on OLED display
- ✨ **Twinkling sparkles** across the screen
- 📜 **Scrolling "Happy Birthday!"** text
- 👆 **Touch-activated** — starts on touch, stops on release
- ⚡ Fully **non-blocking** — music and animation run simultaneously

---

## 🛠 Hardware Requirements

| Component | Specification |
|---|---|
| Microcontroller | ESP32 (any 38-pin dev board) |
| Display | SH1106 128×64 OLED (I2C) |
| Input | Capacitive touch sensor module |
| Output | Passive buzzer |
| Power | 3.3V / 5V via USB |

---

## 🔌 Pin Configuration

| GPIO | Connected To | Mode |
|---|---|---|
| GPIO 4 | Touch Sensor OUT | INPUT |
| GPIO 5 | Buzzer (+) | OUTPUT |
| GPIO 21 | OLED SDA | I2C |
| GPIO 22 | OLED SCL | I2C |

---

## 📦 Dependencies

| Library | Purpose | Install |
|---|---|---|
| `U8g2lib` | SH1106 OLED driver | Arduino Library Manager |
| `Wire` | I2C communication | Built-in |
| `Arduino.h` | Core functions | Built-in |

Install U8g2 via **Arduino Library Manager** → search `U8g2` → Install.

---

## 🚀 Getting Started

### 1. Clone or download this repo

```bash
git clone https://github.com/your-username/happy-birthday-esp32.git
```

### 2. Open in Arduino IDE or PlatformIO

- **Arduino IDE:** Open `happy_birthday.ino`
- **PlatformIO:** Open the project folder

### 3. Install the U8g2 library

Go to **Sketch → Include Library → Manage Libraries** → search `U8g2` → Install.

### 4. Select your board

**Tools → Board → ESP32 Dev Module**

### 5. Upload

Connect your ESP32 via USB and click **Upload**.

---

## 🎮 How It Works

```
Touch Sensor HIGH  →  Celebration mode starts
                        ├── Happy Birthday melody plays on buzzer
                        ├── Balloons float upward on OLED
                        ├── Sparkles twinkle across screen
                        └── "Happy Birthday!" scrolls left

Touch Sensor LOW   →  Returns to idle screen
                        └── Melody stops immediately
```

The entire system is **non-blocking** — `millis()` handles all timing so music and animation run in parallel without any `delay()` freezing.

---

## ⚙️ Configuration

You can tweak these constants at the top of the file:

```cpp
#define TOUCH_PIN   4     // GPIO for touch sensor
#define BUZZER_PIN  5     // GPIO for passive buzzer
#define TEMPO       140   // BPM — increase for faster melody
#define FRAME_MS    40    // Animation frame interval (~25 FPS)
```

For display brightness:
```cpp
u8g2.setContrast(200);   // 0–255, higher = brighter
```

---

## 📁 Project Structure

```
happy-birthday-esp32/
├── happy_birthday.ino    # Main source file
└── README.md             # This file
```

---

## 🐛 Known Limitations

| # | Issue | Fix |
|---|---|---|
| 1 | Minimal touch debounce — noise may trigger false starts | Require N consecutive HIGH reads |
| 2 | Melody loops indefinitely while held | Add one-shot mode flag |
| 3 | Text wrap offset hardcoded at 140px | Use `u8g2.getStrWidth()` dynamically |
| 4 | No I2C error handling if OLED is missing | Check `u8g2.begin()` return value |
| 5 | Balloon reset positions are always identical | Use `esp_random()` for varied starts |

---

## 🧑‍💻 Author

**Mandeep Kafle**  
BIT Student — Bhaktapur Multiple Campus  
GitHub: [@Mandeep21ss](https://github.com/Mandeep21ss)

---

## 📄 License

This project is open source and available under the [MIT License](LICENSE).
