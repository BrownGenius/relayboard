#include <Wire.h>

// GPIO Relayboard test
// connect VDD to power 5V
// connect GND to power GND

#define NUM_RELAYS 16

uint8_t relay2PinMapping[NUM_RELAYS] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    14,
    15,
    16,
    17,
    18,
    19};

void setup()
{
    uint8_t relay;

    for (relay = 0; relay < NUM_RELAYS; relay++)
    {
        pinMode(relay2PinMapping[relay], OUTPUT);
        digitalWrite(relay2PinMapping[relay], 0);
    }

    // start serial port at 9600 bps and wait for port to open:
    Serial.begin(9600);
    while (!Serial)
        ; // wait for serial port to connect. Needed for Leonardo only
    Serial.println("I2C Relayboard test - press keys 0123456789abcdef` (toggle relay) * (clear all)");
}

void loop()
{
    char input;
    uint8_t relay;
    int i;
    static uint16_t olat;

    if (Serial.available() > 0)
    {
        input = Serial.read();
        if ((input >= '0') && (input <= '9'))
        {
            relay = input - '0';
        }
        else if ((input >= 'a') && (input <= 'z'))
        {
            relay = input - 'a';
        }
        if (input == '*')
        {
            Serial.println("Clear");
            olat = 0;
            for (relay = 0; relay < NUM_RELAYS; relay++)
            {
                digitalWrite(relay2PinMapping[relay], 0);
            }
        }
        else if (relay < NUM_RELAYS)
        {
            olat ^= 1 << relay;
            digitalWrite(relay2PinMapping[relay], (olat >> relay) & 0x1);
        }

        for (relay = 0; relay < NUM_RELAYS; relay++)
        {
            Serial.print(relay);
            Serial.print((olat & (1 << i) ? ": ON  " : ": OFF "));
        }
        Serial.println();
    }
}
