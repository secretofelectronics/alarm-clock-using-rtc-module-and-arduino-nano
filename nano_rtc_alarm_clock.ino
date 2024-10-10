#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS1307 rtc;

const int buzzerPin = 6;
const int buttonSet = 2;
const int buttonHour = 3;
const int buttonMinute = 4;
const int buttonSecond = 5;

int alarmHour = 7;
int alarmMinute = 0;
int alarmSecond = 0;
bool alarmEnabled = true;  // Ensure the alarm is enabled
bool settingAlarm = false;
bool settingClock = false;

unsigned long buttonPressStart = 0;
unsigned long alarmStartTime = 0;  // To track when the alarm starts
bool alarmPlaying = false;

unsigned long lastButtonPressTime = 0;  // For inactivity timeout
const unsigned long inactivityTimeout = 2000;  // 2 seconds timeout

// Global variables for clock setting
int newHour;
int newMinute;
int newSecond;

void setup() {
  pinMode(buttonSet, INPUT);
  pinMode(buttonHour, INPUT);
  pinMode(buttonMinute, INPUT);
  pinMode(buttonSecond, INPUT);
  pinMode(buzzerPin, OUTPUT);

  if (!rtc.begin()) {
    while (1); // Halt if RTC not found
  }
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1); // Halt if OLED not found
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  DateTime now = rtc.now();

  // Check if the Set button is pressed
  if (digitalRead(buttonSet) == HIGH) {
    buttonPressStart = millis();  // Start timing when button is pressed
    while (digitalRead(buttonSet) == HIGH);  // Wait for button release
    unsigned long buttonPressDuration = millis() - buttonPressStart;

    if (buttonPressDuration >= 1000 && buttonPressDuration < 4000) {
      // Enter alarm setting mode (pressed for 1 to 4 seconds)
      settingAlarm = true;
      settingClock = false;
      lastButtonPressTime = millis();  // Reset inactivity timer
    } else if (buttonPressDuration >= 4000) {
      // Enter clock setting mode (pressed for 4 seconds or more)
      settingClock = true;
      settingAlarm = false;
      lastButtonPressTime = millis();  // Reset inactivity timer
      // Initialize the newHour, newMinute, and newSecond
      newHour = now.hour();
      newMinute = now.minute();
      newSecond = now.second();
    }
  }

  // If in alarm setting mode, allow user to adjust alarm time
  if (settingAlarm) {
    adjustAlarmTime();
    if (millis() - lastButtonPressTime > inactivityTimeout) {
      // Auto-save alarm time after timeout
      settingAlarm = false;
    }
  }
  // If in clock setting mode, allow user to adjust clock time
  else if (settingClock) {
    adjustClockTime();
    if (millis() - lastButtonPressTime > inactivityTimeout) {
      // Auto-save clock time after timeout
      settingClock = false;
    }
  } else {
    // Display current time and alarm time
    displayCurrentAndAlarmTime(now);

    // Check if it's time to trigger the alarm and play buzzer for 60 seconds
    if (alarmEnabled && now.hour() == alarmHour && now.minute() == alarmMinute && now.second() == alarmSecond) {
      if (!alarmPlaying) {
        tone(buzzerPin, 1000);  // Play the buzzer
        alarmStartTime = millis();  // Record the start time
        alarmPlaying = true;
      }
    }

    // Stop the buzzer after 60 seconds
    if (alarmPlaying && (millis() - alarmStartTime >= 60000)) {
      noTone(buzzerPin);  // Stop the buzzer after 1 minute
      alarmPlaying = false;
    }
  }

  delay(500);  // Update display every half second
}

// Function to allow user to adjust the alarm time
void adjustAlarmTime() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Set Alarm:");
  display.setCursor(0, 20);
  display.print(alarmHour < 10 ? "0" : "");
  display.print(alarmHour);
  display.print(":");
  display.print(alarmMinute < 10 ? "0" : "");
  display.print(alarmMinute);
  display.print(":");
  display.print(alarmSecond < 10 ? "0" : "");
  display.print(alarmSecond);

  // Button for adjusting hours
  if (digitalRead(buttonHour) == HIGH) {
    alarmHour = (alarmHour + 1) % 24;  // Wrap around after 23
    lastButtonPressTime = millis();    // Reset inactivity timer
    delay(200);  // Debounce
  }

  // Button for adjusting minutes
  if (digitalRead(buttonMinute) == HIGH) {
    alarmMinute = (alarmMinute + 1) % 60;  // Wrap around after 59
    lastButtonPressTime = millis();        // Reset inactivity timer
    delay(200);  // Debounce
  }

  // Button for adjusting seconds
  if (digitalRead(buttonSecond) == HIGH) {
    alarmSecond = (alarmSecond + 1) % 60;  // Wrap around after 59
    lastButtonPressTime = millis();        // Reset inactivity timer
    delay(200);  // Debounce
  }

  display.display();
}

// Function to allow user to adjust the clock time
void adjustClockTime() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Set Clock:");
  display.setCursor(0, 20);
  display.print(newHour < 10 ? "0" : "");
  display.print(newHour);
  display.print(":");
  display.print(newMinute < 10 ? "0" : "");
  display.print(newMinute);
  display.print(":");
  display.print(newSecond < 10 ? "0" : "");
  display.print(newSecond);

  // Button for adjusting hours
  if (digitalRead(buttonHour) == HIGH) {
    newHour = (newHour + 1) % 24;  // Wrap around after 23
    lastButtonPressTime = millis();  // Reset inactivity timer
    delay(200);  // Debounce
  }

  // Button for adjusting minutes
  if (digitalRead(buttonMinute) == HIGH) {
    newMinute = (newMinute + 1) % 60;  // Wrap around after 59
    lastButtonPressTime = millis();    // Reset inactivity timer
    delay(200);  // Debounce
  }

  // Button for adjusting seconds
  if (digitalRead(buttonSecond) == HIGH) {
    newSecond = (newSecond + 1) % 60;  // Wrap around after 59
    lastButtonPressTime = millis();    // Reset inactivity timer
    delay(200);  // Debounce
  }

  // Auto-save after inactivity timeout
  if (millis() - lastButtonPressTime > inactivityTimeout) {
    rtc.adjust(DateTime(rtc.now().year(), rtc.now().month(), rtc.now().day(), newHour, newMinute, newSecond));
    settingClock = false;  // Exit clock setting mode
  }

  display.display();
}

// Function to display the current time and alarm time
void displayCurrentAndAlarmTime(DateTime now) {
  display.clearDisplay();
  
  // Display current time
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(now.hour(), DEC);
  display.print(":");
  display.print(now.minute(), DEC);
  display.print(":");
  display.print(now.second(), DEC);
  
  // Display alarm time
  display.setTextSize(1);
  display.setCursor(0, 30);
  display.print("Alarm: ");
  display.print(alarmHour < 10 ? "0" : "");
  display.print(alarmHour);
  display.print(":");
  display.print(alarmMinute < 10 ? "0" : "");
  display.print(alarmMinute);
  display.print(":");
  display.print(alarmSecond < 10 ? "0" : "");
  display.print(alarmSecond);

  display.display();
}