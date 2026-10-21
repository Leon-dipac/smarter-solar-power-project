#include <LiquidCrystal_I2C.h>
#include <IRremote.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte IR_RECEIVE_PIN = 3;
const byte RELAY_PIN = 12;
const byte PWM_PIN = 5;
const byte BANK_PIN = A0;
const byte CAP_PIN = A1;
const byte LOAD_PIN = A3;

float bankVoltage = 0.0;
float capVoltage = 0.0;
float loadVoltage = 0.0;
float targetVoltage = 2.0;
float filteredLoadVoltage = 0.0;
int pwmValue = 0;

enum CapacitorMode { PARALLEL_MODE, SERIES_MODE };
CapacitorMode requestedMode = PARALLEL_MODE;
bool relayCommand = false;

int firstDigit = -1;
unsigned long lastIRTime = 0;
bool showInput = false;
unsigned long messageTime = 0;
const unsigned long INPUT_DISPLAY_TIME = 1000;


byte parallelLeft[8] = {
  B00001, B01111, B01001, B11000,
  B11000, B01001, B01111, B00001
};
byte parallelRight[8] = {
  B10000, B11110, B10010, B00011,
  B00011, B10010, B11110, B10000
};
byte seriesLeft[8] = {
  B01010, B01010, B01010, B11011,
  B01010, B01010, B01010, B00000
};
byte seriesRight[8] = {
  B01010, B01010, B01010, B11011,
  B01010, B01010, B01010, B00000
};

float readVoltage(byte pin) {
  long total = 0;
  for (int i = 0; i < 100; i++) {
    total += analogRead(pin);
  }
  return (total / 100.0) * 5.0 / 1023.0 * 5.02439;
}

void displayInput(float voltage) {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("Input = ");
  lcd.print(voltage, 2);
  lcd.print(" V");
}

int decodeDigit(unsigned long code) {
  if (code == 0xF30CBF00UL) return 0;
  if (code == 0xEE11BF00UL) return 2;
  if (code == 0xED12BF00UL) return 3;
  if (code == 0xEA15BF00UL) return 5;
  return -1;
}

void selectCapacitorMode() {
  if (targetVoltage >= 3.5) {
    requestedMode = SERIES_MODE;
  } else {
    requestedMode = PARALLEL_MODE;
  }
  relayCommand = (requestedMode == SERIES_MODE);
}

void processDigit(int digit) {
  if (firstDigit == -1) {
    firstDigit = digit;
    showInput = true;
    displayInput((float)digit);
    Serial.print("First digit: ");
    Serial.println(digit);
    return;
  }

  float newTarget = firstDigit + digit / 10.0;
  firstDigit = -1;
  showInput = true;
  messageTime = millis();

  if (newTarget > 4.3) {
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("Invalid voltage");
    Serial.println("Invalid voltage");
    return;
  }

  targetVoltage = newTarget;
  selectCapacitorMode();
  displayInput(targetVoltage);

  Serial.print("Target voltage: ");
  Serial.println(targetVoltage, 2);
  if (requestedMode == SERIES_MODE) {
    Serial.println("SERIES requested");
  } else {
    Serial.println("PARALLEL requested");
  }
}

void checkIRRemote() {
  if (IrReceiver.decode()) {
    unsigned long code = IrReceiver.decodedIRData.decodedRawData;
    IrReceiver.resume();
    int digit = decodeDigit(code);
    if (digit >= 0 && millis() - lastIRTime >= 120) {
      lastIRTime = millis();
      processDigit(digit);
    }
  }
}

void updateRelayOutput() {
  if (relayCommand == true) {
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
}

void displayCapacitorMode() {
  // Keep first-digit input visible until another recognized digit arrives.
  if (firstDigit != -1) return;

  // After the second digit, keep the completed input visible for 1 second.
  if (showInput) {
    if (millis() - messageTime < INPUT_DISPLAY_TIME) return;
    showInput = false;
  }

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);

  if (requestedMode == SERIES_MODE) {
    lcd.write((uint8_t)2);
    lcd.write((uint8_t)3);
  } else {
    lcd.write((uint8_t)0);
    lcd.write((uint8_t)1);
  }

  lcd.setCursor(3, 0);
  lcd.print(filteredLoadVoltage, 2);
  lcd.print("V");
  lcd.setCursor(9, 0);
  lcd.print("Emani");

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(bankVoltage, 2);
  lcd.print("V");
  lcd.setCursor(7, 1);
  lcd.print(capVoltage, 2);
  lcd.print("V");
}

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  analogWrite(PWM_PIN, 0);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, parallelLeft);
  lcd.createChar(1, parallelRight);
  lcd.createChar(2, seriesLeft);
  lcd.createChar(3, seriesRight);
  lcd.clear();

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  selectCapacitorMode();
}

void loop() {
  checkIRRemote();
  updateRelayOutput();

  pwmValue = (int)(targetVoltage * 51.0);
  pwmValue = constrain(pwmValue, 0, 255);
  analogWrite(PWM_PIN, pwmValue);

  bankVoltage = readVoltage(BANK_PIN);
  capVoltage = readVoltage(CAP_PIN);
  loadVoltage = readVoltage(LOAD_PIN);

  filteredLoadVoltage =
    0.8 * filteredLoadVoltage + 0.2 * loadVoltage;

  displayCapacitorMode();
  delay(100);
}