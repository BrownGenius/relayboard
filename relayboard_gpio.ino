#include <ArduinoJson.h>

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

{"command":"info"}

#endif

#define VERSION 0x20220202

StaticJsonDocument<32*RELAY_COUNT> status;
StaticJsonDocument<32*RELAY_COUNT> input;

void printInfo()
{
    StaticJsonDocument<256> info;

    info["model"] = "GPIO Relay Board";
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
    digitalWrite(relay2PinMapping[relay], enable ? RELAY_ENABLE : RELAY_DISABLE);
    status["relays"][relay]["port"] = relay;
    status["relays"][relay]["enable"] = enable;
}

void initRelays(bool initialize)
{
    uint8_t relay;

    for (relay = 0; relay < RELAY_COUNT; relay++)
    {
        if (initialize) {
            pinMode(relay2PinMapping[relay], OUTPUT);
        }
        setRelay(relay, true);
    }
}

void setup()
{
    status.createNestedArray("relays");

    initRelays(true);

    // start serial port at 9600 bps and wait for port to open:
    Serial.begin(9600);
    while (!Serial)
        ; // wait for serial port to connect. Needed for Leonardo only
    printInfo();
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
        }
        if (input.containsKey("command"))
        {
            if (0 == strcmp(input["command"], "status")) {printStatus();}
            else if (0 == strcmp(input["command"], "info")) { printInfo();}
        }
    }
}