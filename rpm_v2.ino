#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

volatile unsigned long count = 0;  // Raw interrupt counts
unsigned long lastDebounceTime = 0;
const int sample_tim = 2000;
const int debounce_delay = 60;    // 200ms debounce in loop
int value = 0;
float rpm = 0;
const int ir = 16;
unsigned long lastTime = 0;
int lineCount = 0;
int lastState = HIGH;              // Track sensor state
unsigned long debouncedCount = 0;  // Debounced pulse count

void IRAM_ATTR countPulse() {
  count++;  // Raw count on every FALLING edge
}

void setup() {
  pinMode(ir, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize OLED, no error check
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  //display.println("RPM Meter");
  display.display();
  attachInterrupt(digitalPinToInterrupt(ir), countPulse, FALLING);
  lastTime = millis();
  lastDebounceTime = millis();
}

void loop() {
  int currentState = digitalRead(ir);  // Read sensor

  // Debounce logic in loop
  if (currentState == LOW && lastState == HIGH) {  // FALLING edge
    if ((millis() - lastDebounceTime) >= debounce_delay) {
      debouncedCount++;
      lastDebounceTime = millis();
    }
  }
  lastState = currentState;

  // Update RPM and OLED every 2 seconds
  if (millis() - lastTime >= sample_tim) {
    noInterrupts();
    value = debouncedCount;
    debouncedCount = 0;
    interrupts();
    rpm = (float)value * (60000 / sample_tim);

    if (lineCount >= 4) {
      display.clearDisplay();
      lineCount = 0;
    }
    display.setCursor(0, lineCount * 16);
    display.print("RPM: ");
    display.println(rpm, 1);
    display.display();
    lineCount++;

    lastTime = millis();
  }
}