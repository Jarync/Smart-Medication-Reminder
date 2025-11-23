#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>  // I2C LCD Library

// Initialize LCD (Address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

RTC_DS3231 rtc;

// Define Button Pins
const int buttonPlusPin = 8;      // "+" Button
const int buttonMinusPin = 9;     // "-" Button
const int buttonConfirmPin = 10;  // "Confirm" Button

// Buzzer Pin
const int buzzerPin = 11;

// Pressure Sensor Pin (FSR)
const int pressureSensorPin = A0;

// Reminder Time Variables
int reminderHour = 0;
int reminderMin = 0;
bool reminderSet = false;  // Flag: Is the reminder time set?
bool alarmActive = false;  // Flag: Is the alarm currently ringing?

// State Tracking Variables
bool bottleLifted = false;         // Flag: Has the bottle been lifted?
bool alarmTriggeredToday = false;  // Flag: Has today's alarm already triggered?
int previousDay = -1;              // To track day changes

DateTime nextReminderTime;  // Timestamp for the next scheduled alarm

// Finite State Machine (FSM) for Menu System
enum State {
  NORMAL,     // Normal Clock Display Mode
  SET_HOUR,   // Set Hour Mode
  SET_MINUTE  // Set Minute Mode
};
State currentState = NORMAL;

// Button Debouncing Variables
int ButtonPlusState;
int ButtonMinusState;
int ButtonConfirmState;

int lastButtonPlusState = HIGH;
int lastButtonMinusState = HIGH;
int lastButtonConfirmState = HIGH;

unsigned long lastDebounceTimePlus = 0;
unsigned long lastDebounceTimeMinus = 0;
unsigned long lastDebounceTimeConfirm = 0;
const unsigned long debounceDelay = 10;  // Debounce delay (ms)

void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);

  // Initialize LCD
  lcd.init();       // Init LCD
  lcd.backlight();  // Turn on backlight

  // Initialize RTC Module
  if (!rtc.begin()) {
    lcd.print("Couldn't find RTC");
    while (1); // Halt if RTC missing
  }

  if (rtc.lostPower()) {
    lcd.print("RTC lost power!");
    // Automatically set time to compile time
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }

  // Set Pin Modes
  pinMode(buttonPlusPin, INPUT_PULLUP);
  pinMode(buttonMinusPin, INPUT_PULLUP);
  pinMode(buttonConfirmPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(pressureSensorPin, INPUT);

  // Ensure buzzer is OFF initially (Active LOW)
  digitalWrite(buzzerPin, HIGH); 
}

void loop() {
  DateTime now = rtc.now();

  // Read Pressure Sensor (Force Sensitive Resistor)
  int pressureValue = analogRead(pressureSensorPin);
  Serial.println(pressureValue);

  // Check Button Inputs
  checkButtons();

  // Day Change Logic: Reset daily alarm flags at midnight
  if (now.day() != previousDay) {
    previousDay = now.day();
    alarmTriggeredToday = false;  // Reset trigger flag for new day
    bottleLifted = false;         // Reset bottle status
  }

  // State Machine Logic
  switch (currentState) {
    case NORMAL:
      // Display Current Time
      lcd.setCursor(0, 0);
      lcd.print("T:");
      printDigits(now.hour());
      lcd.print(":");
      printDigits(now.minute());
      lcd.print(":");
      printDigits(now.second());
      lcd.print(" ");

      // Display Date (MM/DD)
      printDigits(now.month());
      lcd.print("/");
      printDigits(now.day());

      // Display Alarm Time
      lcd.setCursor(0, 1);
      lcd.print("Alarm:");
      if (reminderSet) {
        printDigits(reminderHour);
        lcd.print(":");
        printDigits(reminderMin);
        lcd.print("   ");  // Clear trailing characters
      } else {
        lcd.print("Not Set   ");
      }

      // Alarm Trigger Logic
      if (reminderSet && !alarmTriggeredToday) {
        if (now.unixtime() >= nextReminderTime.unixtime()) {
          alarmActive = true;          // Activate Alarm
          alarmTriggeredToday = true;  // Mark as triggered for today
          bottleLifted = false;        // Reset bottle lift status

          // Schedule next alarm for tomorrow
          nextReminderTime = nextReminderTime + TimeSpan(1, 0, 0, 0);
        }
      }

      // Alarm Active Logic (Ringing)
      if (alarmActive) {
        // Sound the buzzer pattern
        buzzer2();

        // Check if bottle is lifted (Threshold < 20 means no pressure)
        if (pressureValue < 20 && !bottleLifted) {
          // Bottle Lifted Action
          digitalWrite(buzzerPin, HIGH);  // Turn OFF Buzzer
          bottleLifted = true;            // Mark bottle as lifted
          alarmActive = false;            // Deactivate Alarm state
        }
      } else {
        // Ensure buzzer stays off when not active
        digitalWrite(buzzerPin, HIGH);
      }
      break;

    case SET_HOUR:
      // UI for Setting Hour
      lcd.setCursor(0, 0);
      lcd.print("Set Hour: ");
      printDigits(reminderHour);
      lcd.print("    ");  

      lcd.setCursor(0, 1);
      lcd.print("Press Confirm");
      break;

    case SET_MINUTE:
      // UI for Setting Minute
      lcd.setCursor(0, 0);
      lcd.print("Set Min:  ");
      printDigits(reminderMin);
      lcd.print("    ");  

      lcd.setCursor(0, 1);
      lcd.print("Press Confirm");
      break;
  }
}

// Helper: Print digits with leading zero if needed
void printDigits(int digits) {
  if (digits < 10)
    lcd.print('0');
  lcd.print(digits);
}

// Function to handle button inputs and state transitions
void checkButtons() {
  int readingPlus = digitalRead(buttonPlusPin);
  int readingMinus = digitalRead(buttonMinusPin);
  int readingConfirm = digitalRead(buttonConfirmPin);

  // --- "+" Button Logic ---
  if (readingPlus != lastButtonPlusState) {
    lastDebounceTimePlus = millis();
  }
  if ((millis() - lastDebounceTimePlus) > debounceDelay) {
    if (readingPlus != ButtonPlusState) {
      ButtonPlusState = readingPlus;
      if (ButtonPlusState == LOW) { // Pressed
        if (currentState == SET_HOUR) {
          reminderHour = (reminderHour + 1) % 24;
        } else if (currentState == SET_MINUTE) {
          reminderMin = (reminderMin + 1) % 60;
        }
      }
    }
  }
  lastButtonPlusState = readingPlus;

  // --- "-" Button Logic ---
  if (readingMinus != lastButtonMinusState) {
    lastDebounceTimeMinus = millis();
  }
  if ((millis() - lastDebounceTimeMinus) > debounceDelay) {
    if (readingMinus != ButtonMinusState) {
      ButtonMinusState = readingMinus;
      if (ButtonMinusState == LOW) { // Pressed
        if (currentState == SET_HOUR) {
          reminderHour = (reminderHour + 23) % 24;  // Handle negative wrapping
        } else if (currentState == SET_MINUTE) {
          reminderMin = (reminderMin + 59) % 60;
        }
      }
    }
  }
  lastButtonMinusState = readingMinus;

  // --- "Confirm" Button Logic (State Switcher) ---
  if (readingConfirm != lastButtonConfirmState) {
    lastDebounceTimeConfirm = millis();
  }
  if ((millis() - lastDebounceTimeConfirm) > debounceDelay) {
    if (readingConfirm != ButtonConfirmState) {
      ButtonConfirmState = readingConfirm;
      if (ButtonConfirmState == LOW) { // Pressed
        if (currentState == NORMAL) {
          currentState = SET_HOUR;  // Enter Setup Mode
        } else if (currentState == SET_HOUR) {
          currentState = SET_MINUTE;  // Move to Minute Setup
        } else if (currentState == SET_MINUTE) {
          currentState = NORMAL;  // Save and Exit
          reminderSet = true;     // Flag set
          alarmActive = false;
          bottleLifted = false;
          alarmTriggeredToday = false;  // Reset daily flag for new time

          // Calculate initial next reminder time
          DateTime now = rtc.now();
          nextReminderTime = DateTime(now.year(), now.month(), now.day(), reminderHour, reminderMin, 0);
          
          // If time passed today, schedule for tomorrow
          if (now.unixtime() >= nextReminderTime.unixtime()) {
            nextReminderTime = nextReminderTime + TimeSpan(1, 0, 0, 0);
          }
        }
      }
    }
  }
  lastButtonConfirmState = readingConfirm;
}

// Buzzer Pattern Generator
void buzzer2() {
  digitalWrite(buzzerPin, LOW);  // ON
  delay(60);
  digitalWrite(buzzerPin, HIGH); // OFF
  delay(60);
}