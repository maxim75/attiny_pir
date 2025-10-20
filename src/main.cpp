#include <Arduino.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/sleep.h>

// Pin Definitions
constexpr uint8_t CE_PIN = PIN_PA5;           // RF24 Chip Enable pin
constexpr uint8_t CSN_PIN = PIN_PA7;          // RF24 Chip Select pin
constexpr uint8_t BATTERY_PIN = PIN_PA4;      // Battery voltage monitoring pin
constexpr uint8_t LED_PIN = PIN_PB0;          // Status LED pin
constexpr uint8_t LIGHT_SENSOR_PIN = PIN_PB1; // Light sensor input pin
constexpr uint8_t INTERRUPT_PIN = PIN_PA6;     // PIR sensor interrupt pin

// RF24 Configuration
constexpr uint64_t RADIO_ADDRESS = 0xFAB7C2F0E2LL;
constexpr bool LNA_ENABLE = false;
constexpr rf24_pa_dbm_e RF24_POWER_LEVEL = RF24_PA_LOW;
constexpr uint8_t MESSAGE_BUFFER_SIZE = 32;

// Device Configuration
const char* DEVICE_ID = "PIR11";
constexpr int LIGHT_THRESHOLD = 800;           // Threshold for light sensor
constexpr float BATTERY_VOLTAGE_MULTIPLIER = 6.46f;

// Global variables
char* messageBuffer = new char[MESSAGE_BUFFER_SIZE];
RF24 radio(CE_PIN, CSN_PIN);
volatile bool wakeUpFlag = false;

// Interrupt Service Routine for PIR sensor
void handlePirInterrupt() {
    wakeUpFlag = true;
}

// Initialize all GPIO pins
void initializePins() {
    pinMode(INTERRUPT_PIN, INPUT);
    pinMode(LIGHT_SENSOR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(0, OUTPUT);  // Reserved pin configuration
}

// Configure the RF24 radio module
bool initializeRadio() {
    if (!radio.begin()) {
        Serial.println("Failed to initialize radio!");
        return false;
    }

    if (!radio.isChipConnected()) {
        Serial.println("NF24 is NOT connected to SPI!");
        return false;
    }

    Serial.println("Transmitter NF24 connected to SPI");
    
    radio.setDataRate(RF24_250KBPS);
    radio.openWritingPipe(RADIO_ADDRESS);
    radio.setPALevel(RF24_POWER_LEVEL, LNA_ENABLE);
    radio.stopListening();
    return true;
}

// Blink LED for visual feedback
void blinkLed(uint16_t duration) {
    digitalWrite(LED_PIN, HIGH);
    delay(duration);
    digitalWrite(LED_PIN, LOW);
}

// Send message through RF24
void sendMessage(const char* message) {
    Serial.printf(">>> %s", message);
    radio.write(message, strlen(message));
}

void setup() {
    Serial.begin(9600, SERIAL_8N1);
    
    // Initialize hardware
    initializePins();
    blinkLed(500);  // Power-on indicator
    
    // Setup PIR interrupt
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), handlePirInterrupt, FALLING);
    
    // Initialize radio
    if (!initializeRadio()) {
        while(1) {
            blinkLed(100);  // Error indicator - fast blinking
            delay(100);
        }
    }

    // Send initial test message
    Serial.println("Sending test message...");
    snprintf(messageBuffer, MESSAGE_BUFFER_SIZE, "%s_test", DEVICE_ID);
    sendMessage(messageBuffer);
    Serial.println("Test message sent");
    delay(5000);
}

// Read battery voltage in millivolts
int readBatteryVoltage() {
    ADC0.CTRLA = ADC_ENABLE_bm;  // Enable ADC
    pinMode(BATTERY_PIN, INPUT);
    
    int adcValue = analogRead(BATTERY_PIN);
    float voltage = static_cast<float>(adcValue) * BATTERY_VOLTAGE_MULTIPLIER;
    
    Serial.print("ADC Value: ");
    Serial.println(adcValue);
    Serial.print("Voltage [mV]: ");
    Serial.println(voltage);
    
    return static_cast<int>(voltage);
}

// Read light sensor value
int readLightSensor() {
    blinkLed(200);  // Brief LED flash while reading
    return analogRead(LIGHT_SENSOR_PIN);
}

// Enter sleep mode to conserve power
void enterSleepMode() {
    Serial.println("Entering sleep mode...");
    delay(100);  // Allow Serial to complete transmission
    
    ADC0.CTRLA = 0;  // Disable ADC to save power
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    
    noInterrupts();
    sleep_enable();
    interrupts();
    sleep_cpu();  // Sleep until interrupt
    
    sleep_disable();  // Disable sleep after waking
}

// Prepare and send sensor data
void sendSensorData(int batteryVoltage, int lightLevel) {
    snprintf(messageBuffer, MESSAGE_BUFFER_SIZE, 
             "%s_S:1_B:%04d_L:%d", 
             DEVICE_ID, batteryVoltage, lightLevel);
    
    Serial.println(messageBuffer);
    
    if (lightLevel < LIGHT_THRESHOLD || true) {  // TODO: Remove "|| true" after testing
        sendMessage(messageBuffer);
        blinkLed(200);  // Confirmation blink
    }
}

void loop() {
    // Check if we should enter sleep mode
    if (!wakeUpFlag) {
        enterSleepMode();
    }

    // Process wake-up event
    Serial.println("Wake-up event detected!");
    wakeUpFlag = false;
    
    // Read sensor values
    int batteryVoltage = readBatteryVoltage();
    int lightLevel = readLightSensor();
    
    // Process and send data
    sendSensorData(batteryVoltage, lightLevel);
}
