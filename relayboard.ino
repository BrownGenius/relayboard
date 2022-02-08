
#define BUILD_TYPE_GPIO 0
#define BUILD_TYPE_I2C 1

#include <ArduinoJson.h>
/* Required the following Arduino Libraries
 - ArduinoJSON (v6.19.1)
*/


#if BUILD_TYPE_I2C

#define MODEL "I2C Relay Board"

#include <Wire.h>

// I2C Relayboard test
// connect VDD to power 5V
// connect GND to power GND
// connect SDA to digital SDA (I2C DATA)
// connect SCL to digital ACL (I2C CLOCK)

#define RELAY_COUNT 16
#define RELAY_ENABLE 0
#define RELAY_DISABLE 1
#define I2C_BOARD_COUNT 2

uint8_t relay2PinMapping[RELAY_COUNT] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
#if RELAY_COUNT > 8
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
#endif
};

uint8_t board2AddressMapping[I2C_BOARD_COUNT] =
{
    0x20,
#if I2C_BOARD_COUNT > 1
    0x21,
#endif
};

uint8_t relay2BoardMapping(uint8_t relay)
{
    if (relay < 8) {
        return 0;
    } else {
        return 1;
    }
}

uint8_t writeI2CRegister(uint8_t deviceAddr, uint8_t registerAddr, uint8_t value)
{
#if 0
    Serial.print("I2C ");
    Serial.print(deviceAddr);
    Serial.print("@");
    Serial.print(registerAddr);
    Serial.print(" <-- ");
    Serial.println(value);
#endif
    Wire.beginTransmission(deviceAddr); //begins talking to the slave device
    Wire.write(registerAddr); //selects register
    Wire.write(value); // write value
    Wire.endTransmission(); //ends communication with the device
}

uint8_t readI2CRegister(uint8_t deviceAddr, uint8_t registerAddr)
{
    uint8_t value;
    Wire.beginTransmission(deviceAddr); //begins talking to the slave device
    Wire.write(registerAddr); //selects register
    Wire.endTransmission(); //ends communication with the device
    Wire.requestFrom(deviceAddr, (uint8_t)1);
    value = Wire.read();

#if 0
    Serial.print("I2C ");
    Serial.print(deviceAddr);
    Serial.print("@");
    Serial.print(registerAddr);
    Serial.print(" --> ");
    Serial.println(value);
#endif

    return value;
}
#else
#define MODEL "GPIO Relay Board"
// GPIO Relayboard test
// connect VDD to power 5V
// connect GND to power GND

#define RELAY_COUNT 16
#define RELAY_ENABLE 1
#define RELAY_DISABLE 0

uint8_t relay2PinMapping[RELAY_COUNT] = {
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
#endif

/* JSON Format */
#if 0
{
	"relays": [{
			"port":0,
			"enabled":true
		},
	]
}
{"relays":[{"port":0,"enable":false},{"port":2,"enable":false}]}
{"command":"status"}

{"command":"info","relays":[{"port":0,"enable":false},{"port":2,"enable":false}]}
{"relays":[{"port":0,"enable":false},{"port":2,"enable":false}]}
{"command":"info"}


{"relays":[{"port":0,"enable":false},{"port":1,"enable":true},{"port":2,"enable":false},{"port":3,"enable":true},{"port":4,"enable":true},{"port":5,"enable":true},{"port":6,"enable":true},{"port":7,"enable":true},{"port":8,"enable":true},{"port":9,"enable":true},{"port":10,"enable":true},{"port":11,"enable":true},{"port":12,"enable":true},{"port":13,"enable":true},{"port":14,"enable":true},{"port":15,"enable":true}]}

{"relays":[{"port":0,"enable":false},{"port":1,"enable":false}]}

{"relays":[{"port":2,"enable":false},{"port":3,"enable":false}]}
#endif

#define VERSION 0x20220208

StaticJsonDocument<32*RELAY_COUNT> status;

void printInfo()
{
    StaticJsonDocument<64> info;

    info["model"] = MODEL;
    info["version"] = VERSION;
    info["relay count"] = RELAY_COUNT;

    serializeJson(info, Serial);
    Serial.println();
}

void printStatus()
{
    serializeJson(status, Serial);
    Serial.println();
}

void setRelay(uint8_t relay, bool enable)
{
#if BUILD_TYPE_I2C
    uint8_t olat;
    olat = readI2CRegister(board2AddressMapping[relay2BoardMapping(relay)], 0x0A);
    olat &= ~((uint8_t)1 << relay2PinMapping[relay]);
    olat |= ((uint8_t)(enable ? RELAY_ENABLE : RELAY_DISABLE) << relay2PinMapping[relay]);
    writeI2CRegister(board2AddressMapping[relay2BoardMapping(relay)], 0x0A, olat);
#else
    digitalWrite(relay2PinMapping[relay], enable ? RELAY_ENABLE : RELAY_DISABLE);
#endif
    status["relays"][relay]["port"] = relay;
    status["relays"][relay]["enable"] = enable;
}

void initRelays()
{
    uint8_t relay;

#if BUILD_TYPE_I2C
    uint8_t board;
    Wire.begin();

    for (board = 0; board < I2C_BOARD_COUNT; board++)
    {
        // set I/O pins to outputs
        writeI2CRegister(board2AddressMapping[board], 0x00, 0x00);
    }
#elif BUILD_TYPE_GPIO
    for (relay = 0; relay < RELAY_COUNT; relay++)
    {
        pinMode(relay2PinMapping[relay], OUTPUT);
    }
#endif

    for (relay = 0; relay < RELAY_COUNT; relay++)
    {
        setRelay(relay, true);
    }
}

void setup()
{
    // start serial port at 9600 bps and wait for port to open:
    Serial.begin(9600);

    status.createNestedArray("relays");
    initRelays();
}

void loop()
{
    StaticJsonDocument<32*RELAY_COUNT> input;
    DeserializationError err;

    do
    {
        err = deserializeJson(input, Serial);
    } while (err == DeserializationError::EmptyInput);

    if (err)
    {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.f_str());
        return;
    }
    else
    {
        if (input.containsKey("relays"))
        {
            uint8_t i;
            int count = input["relays"].size();

            for (i=0; i<count; i++)
            {
                setRelay(
                    input["relays"][i]["port"],
                    input["relays"][i]["enable"]
                );
            }

            printStatus();
        }
        if (input.containsKey("command"))
        {
            if (0 == strcmp(input["command"], "status")) { printStatus(); }
            else if (0 == strcmp(input["command"], "info")) { printInfo(); }
        }
    }
}