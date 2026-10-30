#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define MPU_ADDR 0x68

#define PIN_LED 4
#define PIN_SWITCH 13

// BATAS BARU
#define BATAS_AMAN 2
#define BATAS_KELELAHAN 10

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool alarmReset = false;

int16_t axLama = 0;
int16_t ayLama = 0;
int16_t azLama = 0;

void setup() {

  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_SWITCH, INPUT_PULLUP);

  Wire.begin();

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED GAGAL");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 20);
  display.println("SIGMA");
  display.display();

  delay(2000);

  // Bangunkan MPU6500
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(15, 25);
  display.println("Sensor Aktif");
  display.display();

  Serial.println("SISTEM SIGMA AKTIF");

  delay(1000);
}

void loop() {

  int16_t ax, ay, az;

  // Baca accelerometer
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 6);

  if (Wire.available() < 6) {
    return;
  }

  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();

  // Nilai gerakan (diperkecil)
  float gerakan =
    (abs(ax - axLama) +
     abs(ay - ayLama) +
     abs(az - azLama)) / 1000.0;

  axLama = ax;
  ayLama = ay;
  azLama = az;

  // Tombol Reset
  if (digitalRead(PIN_SWITCH) == LOW) {

    alarmReset = true;

    digitalWrite(PIN_LED, LOW);

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(15, 20);
    display.println("RESET");
    display.display();

    delay(1000);
  }

  Serial.print("AX=");
  Serial.print(ax);

  Serial.print(" AY=");
  Serial.print(ay);

  Serial.print(" AZ=");
  Serial.print(az);

  Serial.print(" G=");
  Serial.print(gerakan, 1);

  Serial.print(" STATUS=");

  display.clearDisplay();

  // =====================
  // AMAN
  // =====================
  if (gerakan < BATAS_AMAN) {

    alarmReset = false;

    digitalWrite(PIN_LED, LOW);

    Serial.println("AMAN");

    display.setTextSize(2);
    display.setCursor(20, 15);
    display.println("AMAN");

    display.setTextSize(1);
    display.setCursor(0, 55);
    display.print("G=");
    display.print(gerakan, 1);
  }

  // =====================
  // WASPADA
  // =====================
  else if (gerakan < BATAS_KELELAHAN) {

    alarmReset = false;

    digitalWrite(PIN_LED, HIGH);
    delay(80);
    digitalWrite(PIN_LED, LOW);

    Serial.println("WASPADA");

    display.setTextSize(2);
    display.setCursor(0, 15);
    display.println("WASPADA");

    display.setTextSize(1);
    display.setCursor(0, 55);
    display.print("G=");
    display.print(gerakan, 1);
  }

  // =====================
  // KELELAHAN
  // =====================
  else {

    Serial.println("KELELAHAN");

    if (!alarmReset) {
      digitalWrite(PIN_LED, HIGH);
    }

    display.setTextSize(1);
    display.setCursor(10, 15);
    display.println("STATUS:");

    display.setTextSize(2);
    display.setCursor(0, 30);
    display.println("LELAH");

    display.setTextSize(1);
    display.setCursor(80, 0);
    display.print("G=");
    display.print(gerakan, 1);
  }

  display.display();

  delay(200);
}
