/*
  ╔══════════════════════════════════════════════════════════╗
  ║       Happy Birthday – Full Code                        ║
  ║  Triggers: Touch sensor (GPIO4) OR MPU-6050 tilt        ║
  ║  Output:   SH1106 OLED + Passive buzzer (GPIO5)         ║
  ║                                                          ║
  ║  I2C Bus (shared):                                       ║
  ║    SDA → GPIO21   SCL → GPIO22                          ║
  ║                                                          ║
  ║  Libraries needed (install via Arduino Library Manager): ║
  ║    • U8g2        by oliver                              ║
  ║    • MPU6050     by Electronic Cats                     ║
  ╚══════════════════════════════════════════════════════════╝

  Wiring summary
  ──────────────
  ESP32 3.3V  → OLED VCC, MPU VCC
  ESP32 GND   → OLED GND, MPU GND, MPU AD0
  ESP32 GPIO21→ OLED SDA, MPU SDA   (shared I2C row)
  ESP32 GPIO22→ OLED SCL, MPU SCL   (shared I2C row)
  ESP32 GPIO4 → Touch sensor signal
  ESP32 GPIO5 → Buzzer +
*/

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <MPU6050.h>       // "MPU6050 by Electronic Cats"

// ── Pin definitions ───────────────────────────────────────────
#define SDA_PIN     21
#define SCL_PIN     22
#define TOUCH_PIN    4
#define BUZZER_PIN   5

// ── MPU-6050 ─────────────────────────────────────────────────
MPU6050 mpu;
bool    mpuOk = false;

// Tilt threshold in raw accelerometer units.
// At ±2g range: 16384 = 1g.  8000 ≈ 0.49g (gentle tilt).
// Increase to 12000 if it triggers too easily.
#define TILT_THRESHOLD 8000

int16_t ax, ay, az, gx, gy, gz;

// ── OLED ─────────────────────────────────────────────────────
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ── Notes ─────────────────────────────────────────────────────
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_C5  523
#define REST       0
#define TEMPO    140      // BPM

int melody[] = {
  NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4, REST,
  NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_G4, NOTE_F4, REST,
  NOTE_C4, NOTE_C4, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_E4, NOTE_D4, REST,
  NOTE_AS4, NOTE_AS4, NOTE_A4, NOTE_F4, NOTE_G4, NOTE_F4
};

float noteDurations[] = {
  0.75, 0.25, 1.0, 1.0, 1.0, 2.0, 0.5,
  0.75, 0.25, 1.0, 1.0, 1.0, 2.0, 0.5,
  0.75, 0.25, 1.0, 1.0, 1.0, 1.0, 2.0, 0.5,
  0.75, 0.25, 1.0, 1.0, 1.5, 3.0
};

const int noteCount = sizeof(melody) / sizeof(melody[0]);

// ── Music state ───────────────────────────────────────────────
int           noteIndex = 0;
unsigned long noteStart = 0;
unsigned long noteDurMs = 0;
bool          noteOn    = false;

// ── Animation state ───────────────────────────────────────────
bool          wasHeld   = false;
unsigned long lastFrame = 0;
#define FRAME_MS 40        // ~25 fps

int textX      = 128;      // scrolling text X position
int globalTick = 0;        // animation tick counter

struct Balloon { int x, y, speed; };
Balloon balloons[3] = {
  { 20,  70, 1 },
  { 64,  90, 2 },
  { 105, 50, 1 },
};

struct Sparkle { int x, y, phase; };
Sparkle sparkles[8] = {
  { 10,  5,  0 }, { 45, 12, 1 }, { 80,  3, 2 }, { 115,  8, 3 },
  { 30, 55,  1 }, { 70, 50, 0 }, { 100, 58, 2 }, {   5, 35, 3 },
};

// ═════════════════════════════════════════════════════════════
//  HELPERS
// ═════════════════════════════════════════════════════════════

float bpmToMs(float beats) {
  return beats * (60000.0f / TEMPO);
}

// ── Trigger detection ─────────────────────────────────────────
// Returns true when touch pin is HIGH OR device is tilted.
bool isTriggerActive() {
  bool touch = (digitalRead(TOUCH_PIN) == HIGH);
  if (!mpuOk) return touch;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  bool tilt = (abs(ax) > TILT_THRESHOLD) || (abs(ay) > TILT_THRESHOLD);
  return touch || tilt;
}

// ── Music playback ────────────────────────────────────────────
void startMusic() {
  noteIndex = 0;
  noteStart = millis();
  noteDurMs = (unsigned long)bpmToMs(noteDurations[0]);
  noteOn    = true;
  if (melody[0] != REST) tone(BUZZER_PIN, melody[0]);
}

void updateMusic() {
  unsigned long now     = millis();
  unsigned long elapsed = now - noteStart;

  // Cut note slightly early for a crisp staccato gap
  if (noteOn && elapsed >= (unsigned long)(noteDurMs * 0.85f)) {
    noTone(BUZZER_PIN);
    noteOn = false;
  }

  // Advance to next note
  if (elapsed >= noteDurMs) {
    noteIndex = (noteIndex + 1) % noteCount;
    noteStart = now;
    noteDurMs = (unsigned long)bpmToMs(noteDurations[noteIndex]);
    noteOn    = true;
    if (melody[noteIndex] != REST) tone(BUZZER_PIN, melody[noteIndex]);
    else                           noTone(BUZZER_PIN);
  }
}

// ── Reset animation to initial state ─────────────────────────
void resetAnimation() {
  textX      = 128;
  globalTick = 0;
  balloons[0] = { 20,  70, 1 };
  balloons[1] = { 64,  90, 2 };
  balloons[2] = { 105, 50, 1 };
}

// ═════════════════════════════════════════════════════════════
//  DRAW FUNCTIONS
// ═════════════════════════════════════════════════════════════

// Balloon: oval body + knotted string
void drawBalloon(int x, int y) {
  if (y > 68 || y < -15) return;
  u8g2.drawEllipse(x, y, 7, 9, U8G2_DRAW_ALL);
  u8g2.drawPixel(x, y + 9);                        // knot
  u8g2.drawLine(x,     y + 10, x - 1, y + 13);
  u8g2.drawLine(x - 1, y + 13, x + 1, y + 16);
  u8g2.drawLine(x + 1, y + 16, x - 1, y + 19);
}

// Sparkle: dot → small plus → big star → off, cycling every 6 ticks
void drawSparkle(int x, int y, int phase) {
  int state = (globalTick + phase) % 6;
  switch (state) {
    case 0:
      u8g2.drawPixel(x, y);
      break;
    case 1:
    case 2:
      u8g2.drawPixel(x,     y);
      u8g2.drawPixel(x - 1, y);
      u8g2.drawPixel(x + 1, y);
      u8g2.drawPixel(x,     y - 1);
      u8g2.drawPixel(x,     y + 1);
      break;
    case 3:
      u8g2.drawPixel(x,     y);
      u8g2.drawPixel(x - 2, y);
      u8g2.drawPixel(x + 2, y);
      u8g2.drawPixel(x,     y - 2);
      u8g2.drawPixel(x,     y + 2);
      u8g2.drawPixel(x - 1, y - 1);
      u8g2.drawPixel(x + 1, y - 1);
      u8g2.drawPixel(x - 1, y + 1);
      u8g2.drawPixel(x + 1, y + 1);
      break;
    // cases 4, 5: off — nothing drawn
  }
}

// Tilt bar: tiny sliding indicator in top-right corner.
// The filled block shifts left/right with X-axis tilt.
void drawTiltBar() {
  if (!mpuOk) return;
  int cx     = 118;
  int cy     =   6;
  int offset = constrain(map(ax, -16384, 16384, -7, 7), -7, 7);
  u8g2.drawFrame(cx - 9, cy - 3, 18, 6);           // bar outline
  u8g2.drawBox(cx + offset - 1, cy - 2, 3, 4);     // sliding block
}

// Full celebration frame
void drawCelebration() {
  u8g2.clearBuffer();

  // Sparkles
  for (int i = 0; i < 8; i++)
    drawSparkle(sparkles[i].x, sparkles[i].y, sparkles[i].phase);

  // Balloons
  for (int i = 0; i < 3; i++)
    drawBalloon(balloons[i].x, balloons[i].y);

  // Scrolling "Happy Birthday!" — two copies for seamless loop
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.drawStr(textX,       38, "Happy Birthday!");
  u8g2.drawStr(textX + 140, 38, "Happy Birthday!");

  // Decorative second line
  u8g2.setFont(u8g2_font_helvR08_tr);
  u8g2.drawStr(textX + 20, 50, "* * * * * * * * * * * *");

  // MPU tilt indicator
  drawTiltBar();

  u8g2.sendBuffer();

  // ── Advance state ──────────────────────────────────────────
  globalTick++;

  // Scroll left; wrap when first copy is fully off-screen
  textX -= 2;
  if (textX <= -140) textX += 140;

  // Float balloons upward; wrap back to bottom
  for (int i = 0; i < 3; i++) {
    balloons[i].y -= balloons[i].speed;
    if (balloons[i].y < -20) balloons[i].y = 72;
  }
}

// Idle screen shown when no trigger is active
void drawIdle() {
  u8g2.clearBuffer();

  if (mpuOk) {
    // MPU detected: show tilt hint
    u8g2.setFont(u8g2_font_helvR08_tr);
    u8g2.drawStr(10, 18, "Touch or tilt to wish");
    u8g2.setFont(u8g2_font_helvB12_tr);
    u8g2.drawStr(14, 38, "Happy B-Day!");
    u8g2.setFont(u8g2_font_helvR08_tr);
    u8g2.drawStr(34, 54, "< tilt me >");
  } else {
    // Touch-only fallback
    u8g2.setFont(u8g2_font_helvR10_tr);
    u8g2.drawStr(18, 28, "Touch to wish");
    u8g2.setFont(u8g2_font_helvB12_tr);
    u8g2.drawStr(22, 48, "Happy B-Day!");
    // Small "touch" icon
    u8g2.drawCircle(64, 57, 4, U8G2_DRAW_ALL);
    u8g2.drawLine(64, 53, 64, 50);
  }

  u8g2.sendBuffer();
}

// ═════════════════════════════════════════════════════════════
//  SETUP
// ═════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);

  pinMode(TOUCH_PIN,  INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Start I2C — shared by OLED and MPU-6050
  Wire.begin(SDA_PIN, SCL_PIN);

  // ── MPU-6050 init ─────────────────────────────────────────
  mpu.initialize();
  if (mpu.testConnection()) {
    mpuOk = true;
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);  // ±2g, most sensitive
    mpu.setDLPFMode(MPU6050_DLPF_BW_5);             // 5 Hz low-pass, smooth out noise
    Serial.println(F("MPU-6050: connected"));
  } else {
    mpuOk = false;
    Serial.println(F("MPU-6050: not found — touch-only mode"));
  }

  // ── OLED init ─────────────────────────────────────────────
  u8g2.begin();
  u8g2.setContrast(200);

  drawIdle();
}

// ═════════════════════════════════════════════════════════════
//  LOOP
// ═════════════════════════════════════════════════════════════
void loop() {
  unsigned long now  = millis();
  bool          held = isTriggerActive();

  if (held) {
    // ── Trigger just started ───────────────────────────────
    if (!wasHeld) {
      resetAnimation();
      startMusic();
      lastFrame = now;
    }

    // Keep music ticking
    updateMusic();

    // Draw next animation frame at ~25 fps
    if (now - lastFrame >= FRAME_MS) {
      drawCelebration();
      lastFrame = now;
    }

  } else {
    // ── Trigger just released ──────────────────────────────
    if (wasHeld) {
      noTone(BUZZER_PIN);
      drawIdle();
    }
  }

  wasHeld = held;
  delay(5);
}
