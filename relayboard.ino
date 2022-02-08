
#define BUILD_TYPE_GPIO 1
#define BUILD_TYPE_I2C 0

#include <ArduinoJson.h>
/* Required the following Arduino Libraries
 - ArduinoJSON (v6.19.1)
*/


#if BUILD_TYPE_I2C

#define MODEL "I2C Relay Board"

#include <Wire.h>
#include <Adafruit_MCP23X08.h>

/* Required the following Arduino Libraries
 - Adafruit MCP23017 library (v2.0.2)
*/

// I2C Relayboard test
// connect VDD to power 5V
// connect GND to power GND
// connect SDA to digital SDA (I2C DATA)
// connect SCL to digital ACL (I2C CLOCK)

#define RELAY_COUNT 8
#define RELAY_ENABLE 0
#define RELAY_DISABLE 1
#define I2C_BOARD_COUNT 1

uint8_t relay2PinMapping[RELAY_COUNT] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
#if 0
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7
#endif
};

uint8_t relay2BoardMapping[RELAY_COUNT] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
#if 0
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
#endif
};

Adafruit_MCP23X08 mcp[I2C_BOARD_COUNT];
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

#define VERSION 0x20220207

StaticJsonDocument<32*RELAY_COUNT> status;
StaticJsonDocument<32*RELAY_COUNT> input;

void printInfo()
{
    StaticJsonDocument<256> info;

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
    /* TODO: Validate relay */
#if BUILD_TYPE_I2C
    mcp[relay2BoardMapping[relay]].digitalWrite(relay2PinMapping[relay], enable ? RELAY_ENABLE : RELAY_DISABLE);
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
    for (board = 0; board < I2C_BOARD_COUNT; board++)
    {
        Serial.println("configuring MCP board");
//        mcp[board].begin_I2C(board);
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

    Serial.println("Initializing...");

    status.createNestedArray("relays");
    initRelays();

    Serial.println("Initialized!");
}

void loop()
{
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