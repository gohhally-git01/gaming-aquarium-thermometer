#include <OneWire.h>
#include <DallasTemperature.h>

// ===== ピン設定 =====
// GPIO1とGPIO2は入れ替え済み
#define RED_PIN    0
#define BLUE_PIN   1
#define GREEN_PIN  2
#define TEMP_PIN   4

// ===== 温度設定 =====
#define TEMP_BLUE       24.0   // 24℃以下：青
#define TEMP_CYAN       26.0   // 24〜26℃：水色
#define TEMP_GRAD_LOW   26.0   // 26〜28℃：グラデーション
#define TEMP_GRAD_HIGH  28.0
#define TEMP_ORANGE     32.0   // 28〜32℃：オレンジ
                              // 32℃以上：赤点滅

#define NORMAL_PERIOD 10000
#define FADE_SPEED    0.08

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

float curR = 0, curG = 0, curB = 0;
float targetR = 0, targetG = 0, targetB = 0;
float tempC = 26.0;

unsigned long lastTempRead = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  sensors.begin();

  Serial.println("RGB Temp Controller Custom Range START");
}

void loop() {
  unsigned long now = millis();

  if (now - lastTempRead >= 1000) {
    lastTempRead = now;

    sensors.requestTemperatures();
    float t = sensors.getTempCByIndex(0);

    if (t != DEVICE_DISCONNECTED_C && t > -50 && t < 100) {
      tempC = t;
    }

    Serial.print("Temp: ");
    Serial.println(tempC);
  }

  decideTargetColor(now);
  smoothFade();
  outputRGB();

  delay(20);
}

void decideTargetColor(unsigned long now) {

  if (tempC <= TEMP_BLUE) {
    // 24℃以下：青
    targetR = 0;
    targetG = 0;
    targetB = 220;
  }
  else if (tempC <= TEMP_CYAN) {
    // 24〜26℃：水色
    targetR = 0;
    targetG = 180;
    targetB = 220;
  }
  else if (tempC < TEMP_GRAD_HIGH) {
    // 26〜28℃：青緑黄グラデーション
    float phase = (now % NORMAL_PERIOD) / (float)NORMAL_PERIOD;

    if (phase < 0.33) {
      // 水色 → 緑
      float x = phase / 0.33;
      targetR = 0;
      targetG = 180 + (220 - 180) * x;
      targetB = 220 * (1.0 - x);
    }
    else if (phase < 0.66) {
      // 緑 → 黄
      float x = (phase - 0.33) / 0.33;
      targetR = 220 * x;
      targetG = 220;
      targetB = 0;
    }
    else {
      // 黄 → 水色
      float x = (phase - 0.66) / 0.34;
      targetR = 220 * (1.0 - x);
      targetG = 220 - 40 * x;
      targetB = 220 * x;
    }
  }
  else if (tempC < TEMP_ORANGE) {
    // 28〜32℃：オレンジ
    targetR = 255;
    targetG = 80;
    targetB = 0;
  }
  else {
    // 32℃以上：赤点滅
    if ((now / 300) % 2 == 0) {
      targetR = 255;
      targetG = 0;
      targetB = 0;
    } else {
      targetR = 0;
      targetG = 0;
      targetB = 0;
    }
  }
}

void smoothFade() {
  curR += (targetR - curR) * FADE_SPEED;
  curG += (targetG - curG) * FADE_SPEED;
  curB += (targetB - curB) * FADE_SPEED;
}

void outputRGB() {
  analogWrite(RED_PIN, constrain((int)curR, 0, 255));
  analogWrite(GREEN_PIN, constrain((int)curG, 0, 255));
  analogWrite(BLUE_PIN, constrain((int)curB, 0, 255));
}