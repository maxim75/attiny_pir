#include <Arduino.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/sleep.h>

#define CE_PIN PIN_PA5
#define CSN_PIN PIN_PA7
#define BATTERY_PIN PIN_PB1
#define LED_PIN PIN_PB0

char *strBuffer = new char[20];

RF24 radio(CE_PIN, CSN_PIN); // CE, CSN

const uint64_t address = 0xFAB7C2F0E2LL;

const byte interruptPin = PIN_PA6;
volatile bool wakeUpFlag = false;

void wakeUp()
{
  wakeUpFlag = true; // ISR sets a flag
}

void setup()
{
  Serial.begin(9600, SERIAL_8N1);

  pinMode(interruptPin, INPUT); // Ensure the pin is input
  pinMode(PIN_PB0, OUTPUT);
  // pinMode(PIN_PB2, OUTPUT);
  // pinMode(PIN_PB3, OUTPUT);
  // pinMode(PIN_PB0, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), wakeUp, RISING);

  pinMode(0, OUTPUT);
  radio.begin();
  if (radio.isChipConnected())
  {
    Serial.println("Transmitter NF24 connected to SPI");
  }
  else
  {
    Serial.println("\n\nNF24 is NOT connected to SPI");
  }

  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX, false);
  radio.stopListening();

  Serial.println("Sending");
  const char text[] = "S";
  radio.write(&text, sizeof(text));
  Serial.println("Sent");
  delay(5000);
}

void loop()
{
  Serial.println("loop");
  if (!wakeUpFlag)
  {

    Serial.println("Sleeping now...");
    delay(100); // Small delay to allow Serial to flush

    ADC0.CTRLA = 0;
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    noInterrupts();
    sleep_enable();
    interrupts();
    sleep_cpu(); // Sleep until interrupt

    // MCU wakes here
    sleep_disable(); // Disable sleep
  }

  // Woke up from interrupt
  Serial.println("Woke up!");
  wakeUpFlag = false;

  // pinMode(BATTERY_PIN, INPUT); // Ensure the pin is input
  // int b = analogRead(BATTERY_PIN);
  // Serial.printf("Batt: %d", b);
  // pinMode(BATTERY_PIN, OUTPUT); // Ensure the pin is input

  sprintf(strBuffer, "PIR04_10%04d", 0);
  Serial.printf(strBuffer);
  radio.write(strBuffer, strlen(strBuffer));
  // Serial.println("1234");
  digitalWrite(PIN_PB0, HIGH);
  delay(500);
  digitalWrite(PIN_PB0, LOW);
}
