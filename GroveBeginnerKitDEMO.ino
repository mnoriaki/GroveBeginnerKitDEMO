//
// This sketch is a demo program for Grove Beginner Kit for Arduino by Seeed Studio.
// 
// To run this sketch you need to install libraries below;
// - Grove Temperature and Humidity Sensor library by Seeed Studio,
// - Grove Barometer Sensor library BMP280 by Seeed Studio, and
// - U8g2 library by oliver. 
// You can install these three libraries through Arduino's library manager.
// You also need Seeed_Arduino_LIS3DHTR library. You can download it from
//    https://github.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR
//
#include <DHT.h>            // Grove Temperature and Humidity Sensor library by Seeed Studio
#include <Seeed_BMP280.h>   // Grove Barometer Sensor library BMP280 by Seeed Studio 
#include <U8x8lib.h>        // U8g2 library by oliver
#include <LIS3DHTR.h>       // Seeed_Arduino_LIS3DHTR at https://github.com/Seeed-Studio/Seeed_Arduino_LIS3DHTR

// Define fixed values
const uint8_t DHT_PIN = 3;     // Pin which DHT11 is connected to
const uint8_t OLED_ADDR =  0x3C;
// Declarations
DHT dht(DHT_PIN, DHT11);
BMP280 bmp280;
LIS3DHTR<TwoWire> LIS;
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

// Global variables
unsigned long prev = 0;
unsigned long prev2 = 0;
int mode = 0;
bool prevSW = LOW;
int t = 0;
uint8_t ymin = 63;
uint8_t ymax = 0;

// Function declarations
void disp2(const char *p, int val, uint8_t y);
void dispACC_X();
void dispADC(const char *p, int pin);
void dispValues();
void oledPutByte(uint8_t x, uint8_t yy, uint8_t c);
inline void oledPutPixel(uint8_t x, uint8_t y) {
  oledPutByte(x, y / 8, 1 << (y % 8));
}
void oledVLine(uint8_t x, uint8_t y1, uint8_t y2);

void setup(void) {
  pinMode(4, OUTPUT);
  pinMode(13, OUTPUT);
  dht.begin();
  bmp280.init();
  LIS.begin(Wire, 0x19);

  u8x8.setBusClock(100000);
  u8x8.begin();
  u8x8.setFlipMode(1);   // Comment out this line if you want to display upside down

  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 2);
  u8x8.print(F("Grove"));
  u8x8.setCursor(2, 3);
  u8x8.print(F("Beginner Kit"));
  u8x8.setCursor(8, 4);
  u8x8.print(F("DEMO"));

  delay(1000);
  u8x8.clearDisplay();
}

void loop(void) {
  // Show sensor values and/or a graph
  switch (mode) {
    case 0:
      dispValues();
      break;
    case 1:
      dispACC_X();
      break;
    case 2:
      dispADC("Pot", A0);
      break;
    case 3:
      dispADC("Light", A6);
      break;
    case 4:
      dispADC("Sound", A2);
      break;
    default:
      mode = 0;
  }

  // Handle the switch
  if (digitalRead(6) == HIGH) {
    if (prevSW == LOW) {
      mode ++;
      if (mode >= 5)
        mode = 0;
      t = 0;
      ymin = 63;
      ymax = 0;
      u8x8.clearDisplay();
    }
    prevSW = HIGH;
    digitalWrite(4, HIGH);
    digitalWrite(13, HIGH);
    tone(5, 1000);
  } else {
    prevSW = LOW;
    digitalWrite(4, LOW);
    digitalWrite(13, LOW);
    noTone(5);
  }

  delay(20);
}

void oledPutByte(uint8_t x, uint8_t yy, uint8_t c) {
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0b10000000); //control byte, Co bit = 1 (1byte only), D/C# = 0 (command) Max=31byte
  Wire.write(0xB0 | (yy & 0x7)); //set page start address←垂直開始位置はここで決める(B0～B7)
  Wire.write(0b00000000); //control byte, Co bit = 0, D/C# = 0 (command)
  Wire.write(x & 0xf); //set lower column address
  Wire.write(0x10 | (x >> 4)); // set higher column address
  Wire.endTransmission();

  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0b01000000); //control byte, Co bit = 0 (continue), D/C# = 1 (data) Max=31byte
  Wire.write(c); //SSD1306のGDRAM にデータ書き込み
  Wire.endTransmission(); //これが送信されて初めてディスプレイに表示される
}

void oledVLine(uint8_t x, uint8_t y1, uint8_t y2) {
  if (y1 >= y2) {
    uint8_t tmp = y1;
    y1 = y2;
    y2 = tmp;
  }

  uint8_t buf[8] = {0};

  for (int i = y1; i <= y2; i ++) {
    buf[i / 8] |= (1 << (i % 8));
  }
  for (int i = 0; i < 8; i ++) {
    if (buf[i] != 0) {
      oledPutByte(x, i, buf[i]);
    }
  }
}


void disp2(const char *p, int val, uint8_t y) {
  u8x8.setCursor(0, 0);
  u8x8.print(p);
  u8x8.print(" ");
  u8x8.print(val);
  u8x8.print(" ");

  ymin = min(y, ymin);
  ymax = max(y, ymax);
  oledPutPixel(t, y);

  if (millis() - prev > 200) {
    oledVLine(t, ymin, ymax);
    ymin = ymax = y;
    prev += 200;
    t ++;
    if (t >= 128) {
      t = 0;
      u8x8.clearDisplay();
    }
  }
}

void dispACC_X() {
  float x = LIS.getAccelerationX();
  u8x8.setCursor(0, 0);
  u8x8.print(F("ACC X: "));
  u8x8.print(x);
  u8x8.print(" ");
  //u8x8.print(LIS.getAccelerationY());
  //u8x8.print(" ");
  //u8x8.print(LIS.getAccelerationZ());

  uint8_t y = 36.0 - x * 28.0;
  ymin = min(y, ymin);
  ymax = max(y, ymax);
  oledPutPixel(t, y);

  if (millis() - prev > 200) {
    oledVLine(t, ymin, ymax);
    ymin = ymax = y;
    prev += 200;
    t ++;
    if (t >= 128) {
      t = 0;
      u8x8.clearDisplay();
    }
  }
}

void dispADC(const char *p, int pin) {
  int a = analogRead(pin);
  uint8_t y = (int)(63.0 - a * 55.0 / 1024.0 + .5);

  disp2(p, a, y);
}


void dispValues() {
  if (millis() - prev > 200) {
    prev = millis();
    float temp = dht.readTemperature();
    float humi = dht.readHumidity();

    u8x8.setCursor(0, 1);
    u8x8.print(temp);
    u8x8.print("C ");
    u8x8.print(humi);
    u8x8.print("%");
  }

  if (millis() - prev2 > 1000) {
    prev2 = millis();
    float t = bmp280.getTemperature();
    float p = bmp280.getPressure();

    u8x8.setCursor(0, 3);
    u8x8.print("Temp:");
    u8x8.print(t);
    u8x8.print("C");

    u8x8.setCursor(0, 4);
    u8x8.print("P:");
    u8x8.print(p);
    u8x8.println("Pa");

    u8x8.setCursor(0, 5);
    u8x8.print("Alt:");
    u8x8.print(bmp280.calcAltitude(p));
    u8x8.println("m");
  }

  u8x8.setCursor(0, 0);
  u8x8.print(analogRead(A0));
  u8x8.print("    ");
  u8x8.setCursor(6, 0);
  u8x8.print(analogRead(A2));
  u8x8.print("    ");
  u8x8.setCursor(12, 0);
  u8x8.print(analogRead(A6));
  u8x8.print(" ");

  u8x8.setCursor(0, 7);
  u8x8.print(LIS.getAccelerationX());
  u8x8.print(" ");
  u8x8.print(LIS.getAccelerationY());
  u8x8.print(" ");
  u8x8.print(LIS.getAccelerationZ());
}
