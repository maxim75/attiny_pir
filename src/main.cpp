#include <Arduino.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/sleep.h>

#define CE_PIN PIN_PA5
#define CSN_PIN PIN_PA7
#define BATTERY_PIN PIN_PA4
#define LED_PIN PIN_PB0
#define LIGHT_SENSOR_PIN PIN_PB1

char *strBuffer = new char[20];
const char *deviceId = "PIR11";

RF24 radio(CE_PIN, CSN_PIN); // CE, CSN

// RF24 settings
#define RF24_PA_LEVEL RF24_PA_LOW
#define LNA_ENABLE false
const uint64_t address = 0xFAB7C2F0E2LL;

const byte interruptPin = PIN_PA6;
volatile bool wakeUpFlag = false;

void wakeUp()
{
  wakeUpFlag = true; // ISR sets a flag
}

void send_message()
{
  Serial.printf(">>> %s", strBuffer);
  radio.write(strBuffer, strlen(strBuffer));
}

// int get_voltage_mv()
// {
//   PORTA.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc; // Disable digital buffer
//   ADC0.CTRLA = ADC_ENABLE_bm;                 // Enable ADC
//   ADC0.CTRLC = ADC_REFSEL_1024MV_gc;
//   ADC0.CTRLB = ADC_PRESC_DIV2_gc;       // use internal reference / prescaler: 16
//   ADC0.MUXPOS = ADC_MUXPOS_VDDDIV10_gc; // ADC_MUXPOS_AIN3_gc; // use A3 as input

//   ADC0.COMMAND = ADC_MODE_SINGLE_12BIT_gc | ADC_START_IMMEDIATE_gc; // Start Conversion
//   while ((ADC0.INTFLAGS & ADC_RESRDY_bm) == 0)
//   {
//   } // Waiting for the result
//   float result = ADC0.RESULT; // * 4096.0 / 4096.0 = 1
//   float v = result / .404f;
//   // Serial.print("Result: ");
//   // Serial.println(result);
//   // Serial.print("Voltage [mV]: ");
//   // Serial.println(v);
//   return (int)v;
// }

void setup()
{
  Serial.begin(9600, SERIAL_8N1);

  pinMode(interruptPin, INPUT);     // Ensure the pin is input
  pinMode(LIGHT_SENSOR_PIN, INPUT); // Ensure the pin is input
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  // pinMode(PIN_PB2, OUTPUT);
  // pinMode(PIN_PB3, OUTPUT);
  // pinMode(PIN_PB0, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), wakeUp, FALLING);

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
  radio.setPALevel(RF24_PA_LEVEL, LNA_ENABLE);
  radio.stopListening();

  Serial.println("Sending");
  sprintf(strBuffer, "%s_test", deviceId, 0);
  send_message();
  Serial.println("Sent");
  delay(5000);
}

int get_voltage_mv()
{
  ADC0.CTRLA = ADC_ENABLE_bm; // Enable ADC
  pinMode(BATTERY_PIN, INPUT); // Ensure the pin is input
  int result = analogRead(BATTERY_PIN);

  float v = (float)result * 6.46f;
  Serial.print("Result: ");
  Serial.println(result);
  Serial.print("Voltage [mV]: ");
  Serial.println(v);
  return (int)v;
}

void loop()
{
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

  int voltage_mv = get_voltage_mv();
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  int light_sensor_v = analogRead(LIGHT_SENSOR_PIN);
  digitalWrite(LED_PIN, LOW);
  sprintf(strBuffer, "%s_S:1_B:%04d_L:%d", deviceId, voltage_mv, light_sensor_v);
  Serial.printf(strBuffer);

  if (light_sensor_v < 800 || true)
  {
    radio.write(strBuffer, strlen(strBuffer));
    delay(200);
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
  }
}
