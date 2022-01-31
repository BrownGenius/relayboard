
// GPIO Relayboard test
// connect VDD to power 5V
// connect GND to power GND

#define NUM_RELAYS 16
#define INITIAL_VALUE 1

uint8_t relay2PinMapping[NUM_RELAYS] = {
    9,  /* Relay 0 */
    11, /* Relay 1 */
    8,  /* Relay 2 */
    10, /* Relay 3 */
    7,  /* Relay 4 */
    A0, /* Relay 5 */
    6,  /* Relay 6 */
    A1, /* Relay 7 */
    5,  /* Relay 8 */
    A2, /* Relay 9 */
    4,  /* Relay a (10) */
    A3, /* Relay b (11) */
    3,  /* Relay c (1)2 */
    A4, /* Relay d (13) */
    2,  /* Relay e (14) */
    A5, /* Relay f (15) */
};

static uint16_t olat;

void resetPins(bool initialize)
{
    uint8_t relay;

    for (relay = 0; relay < NUM_RELAYS; relay++)
    {
        olat &= ~(1 << relay);
        olat |= (INITIAL_VALUE << relay);
        if (initialize) {
            pinMode(relay2PinMapping[relay], OUTPUT);
        }
        digitalWrite(relay2PinMapping[relay], INITIAL_VALUE);
    }
}

void setup()
{
    resetPins(true);

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

    if (Serial.available() > 0)
    {
        input = Serial.read();
        if ((input >= '0') && (input <= '9'))
        {
            relay = input - '0';
        }
        else if ((input >= 'a') && (input <= 'z'))
        {
            relay = (input - 'a')+10;
        }
        if (input == '*')
        {
            Serial.println("Clear");
            resetPins(false);
        }
        else if (relay < NUM_RELAYS)
        {
            olat ^= (1 << relay);
            digitalWrite(relay2PinMapping[relay], (olat >> relay) & 0x1);
        }

        for (relay = 0; relay < NUM_RELAYS; relay++)
        {
            Serial.print(relay);
            Serial.print((olat & (1 << relay) ? ": ON  " : ": OFF "));
        }
        Serial.println();
    }
}
