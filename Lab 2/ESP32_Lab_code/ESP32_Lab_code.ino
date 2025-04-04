#include <Wire.h>
#include <ESP32Encoder.h>

// MMA8452Q I2C address is 0x1C (28)
#define Addr 0x1C

#define CLK 18 // CLK ENCODER
#define DT 19 // DT ENCODER

int ENA = 15;
int IN1 = 2;
int IN2 = 4;
char incomingByte;
int movement = 0;

ESP32Encoder encoder;
boolean data_stream = false;
int sign = 1;

// Timer variables
unsigned long previousMillis = 0;
const unsigned long interval = 5; // 5ms interval

void movemotor(int p1, int p2, int num) {
    digitalWrite(IN1, p1);
    digitalWrite(IN2, p2); 
    delay(num);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
}

void setup() {
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    digitalWrite(ENA, HIGH);
    
    // Initialise I2C communication as MASTER
    Wire.begin();
    
    // Initialise Serial Communication, set baud rate = 115200
    Serial.begin(115200);
    
    // Initialize Accelerometer
    Wire.beginTransmission(Addr);
    Wire.write(0x2A);
    Wire.write((byte)0x00);
    Wire.endTransmission();

    Wire.beginTransmission(Addr);
    Wire.write(0x2A);
    Wire.write(0x01);
    Wire.endTransmission();

    Wire.beginTransmission(Addr);
    Wire.write(0x0E);
    Wire.write((byte)0x00);
    Wire.endTransmission();
    
    delay(300);

    encoder.attachHalfQuad(DT, CLK);
    encoder.setCount(0);
}

void loop() {
    if (Serial.available() > 0) {
        incomingByte = Serial.read();
        
        if (!data_stream) {
            if (incomingByte == '-') {
                sign = -1;
                movement = 0;
            } else {
                sign = 1;
                movement = incomingByte - '0';
            }
            data_stream = true;
        } else if (isDigit(incomingByte)) {
            movement = movement * 10 + int(incomingByte - '0');
        }
    } else if (data_stream) {
        data_stream = false;
        if (sign == -1) {
            movemotor(HIGH, LOW, movement);
        } else {
            movemotor(LOW, HIGH, movement);
        }
    }

    // Non-blocking 5ms timer check
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        unsigned int data[7];

        // Request 7 bytes of data
        Wire.requestFrom(Addr, 7);

        // Read 7 bytes of data
        if (Wire.available() == 7) {
            for (int i = 0; i < 7; i++) {
                data[i] = Wire.read();
            }
        }

        // Convert the data to 12-bits
        int xAccl = ((data[1] * 256) + data[2]) / 16;
        if (xAccl > 2047) xAccl -= 4096;

        int yAccl = ((data[3] * 256) + data[4]) / 16;
        if (yAccl > 2047) yAccl -= 4096;

        int zAccl = ((data[5] * 256) + data[6]) / 16;
        if (zAccl > 2047) zAccl -= 4096;

        long newPosition = encoder.getCount();

        // Output data to serial monitor
        Serial.print(xAccl);
        Serial.print(",");
        Serial.print(yAccl);
        Serial.print(",");
        Serial.print(zAccl);
        Serial.print(",");
        Serial.print(newPosition);
        Serial.println(",");
    }
}
