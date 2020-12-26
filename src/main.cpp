#include <M5StickC.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoHttpClient.h>
#include "battery_status.h"
#include "credntials.h"

#define COUNT_ITEMS(arg) ((unsigned int)(sizeof(arg) / sizeof(arg[0])))
#define NETWORK_DELAY 100
#define NETWORK_TIMEOUT 500

struct XButton {
    const uint8_t PIN;
    uint32_t numberOfKeyPresses;
    bool pressed;
};

int status = WL_IDLE_STATUS;
int infected = 0;
int recovered = 0;
int deaths = 0;
int countryId = 0;

time_t nextFetchTime = 0;

XButton homeButton = {M5_BUTTON_HOME, 0, false};

const String countries[] = {
        "World",
        "Austria",
        "Denmark",
        "France",
        "Germany",
        "Greece",
        "Italy",
        "Spain",
        "Switzerland",
        "UK",
        "US",
};

WiFiClientSecure client;
HttpClient http(client, "www.worldometers.info", 443);

// - - - - - - - - - - - - - - -  - - - - - - - - - -  - - - - - - - - - - - - -
// function prototyping
void printAt(uint16_t x, uint16_t y, uint16_t color, uint8_t size, const String &text);

void fetchData();

void eraseCharactersFromString(String &str, char *characters);

bool wasHomeButtonPressed();

int parseInfected(String &text);

int parseDied(String &text);

int parseRecoverded(String &text);

void IRAM_ATTR isrHomeButton();

void setup() {
    Serial.begin(1152000);
    while (!Serial);

    pinMode(homeButton.PIN, INPUT_PULLUP);
    attachInterrupt(homeButton.PIN, isrHomeButton, FALLING);

    M5.begin();
    M5.Lcd.setRotation(3);

    WiFiClass::mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    M5.Lcd.fillScreen(BLACK);
    printAt(20, 25, WHITE, 2, "Connecting");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFiClass::status() != WL_CONNECTED) {
        printAt(20, 50, WHITE, 1, "Please Wait...");
        delay(1000);
    }

    M5.Lcd.fillScreen(BLACK);
    printAt(10, 20, WHITE, 2, "Connected");
    IPAddress ip = WiFi.localIP();
    printAt(10, 40, WHITE, 1, ip.toString());

    delay(2000);

    M5.Lcd.fillScreen(BLACK);
    printAt(0, 25, WHITE, 2, "Load Data");
}

void loop() {
    M5.update();

    while (wasHomeButtonPressed()) {
        countryId++;
        countryId = countryId %= COUNT_ITEMS(countries);
        nextFetchTime = 0;
    }

    if (millis() > nextFetchTime) {
        fetchData();
        nextFetchTime = millis() + 60 * 60 * 1000;
    }

    delay(100);
}

void showData(const String &sCountry) {
    char data[32];

    M5.Lcd.fillScreen(BLACK);
    battery_status();

    M5.Lcd.setRotation(3);

    printAt(0, 0, ORANGE, 2, sCountry);

    // infected
    sprintf(data, "I: %02d ", infected);
    printAt(5, 20, WHITE, 2, data);

    // recovered
    sprintf(data, "R: %02d ", recovered);
    printAt(5, 40, GREEN, 2, data);

    // deaths
    sprintf(data, "D: %02d ", deaths);
    printAt(5, 60, RED, 2, data);
}

void printAt(uint16_t x, uint16_t y, uint16_t color, uint8_t size, const String &text) {
    M5.Lcd.setCursor(x, y);
    M5.Lcd.setTextSize(size);
    M5.Lcd.setTextColor(color);
    M5.Lcd.print(text);
}

void eraseCharactersFromString(String &str, char *characters) {
    char c, r;

    for (int i = 0; i < strlen(characters) - 1; i++) {
        r = characters[i];
        for (int j = 0; j < str.length() - 1; j++) {
            c = str.charAt(j);
            if (c == r) {
                str.remove(j, 1);
            }
        }
    }
}

void fetchData() {
    int err = 0;
    int readCounter = 0;
    int readValueStep = 0;

    String previous = "";
    String current = "";
    String country = countries[countryId];

    if (country.equals("World")) {
        err = http.get("/coronavirus/");
    } else {
        err = http.get("/coronavirus/country/" + country + "/");
    }

    if (err == 0) {
        err = http.responseStatusCode();
        if (err >= 0) {
            unsigned long timeoutStart = millis();
            while ((http.connected() || http.available()) &&
                   (!http.endOfBodyReached()) &&
                   ((millis() - timeoutStart) < NETWORK_TIMEOUT) &&
                   readValueStep < 3) {

                if (http.available()) {
                    current += (char) http.read();;
                    if (readCounter < 300) {
                        readCounter++;
                    } else {
                        readCounter = 0;
                        String text = "";
                        text.concat(previous);
                        text.concat(current);

                        // check infected first
                        if (readValueStep == 0) {
                            readValueStep += parseInfected(text);
                        }

                        // check deaths
                        if (readValueStep == 1) {
                            readValueStep += parseDied(text);
                        }

                        // check recovered
                        if (readValueStep == 2) {
                            readValueStep += parseRecoverded(text);
                        }

                        previous = current;
                        current = "";
                    }
                    timeoutStart = millis();
                } else {
                    delay(NETWORK_DELAY);
                }
            }
        }
    }
    http.stop();
    showData(country);
}

int parseInfected(String &text) {
    int place = text.indexOf("Coronavirus Cases:");
    if ((place != -1) && (place < 350)) {
        String s2 = text.substring(place + 15);
        text = s2.substring(s2.indexOf("#aaa") + 6);
        String s1 = text.substring(0, (text.indexOf("</")));
        eraseCharactersFromString(s1, ",.");
        infected = s1.toInt();
        return 1;
    }
    return 0;
}

int parseRecoverded(String &text) {
    int place = text.indexOf("Recovered:");
    if ((place != -1) && (place < 350)) {
        String s2 = text.substring(place + 15);
        text = s2.substring(s2.indexOf("<span>") + 6);
        String s1 = text.substring(0, (text.indexOf("</")));
        eraseCharactersFromString(s1, ",.");
        recovered = s1.toInt();
        return 1;
    }
    return 0;
}

int parseDied(String &text) {
    int place = text.indexOf("Deaths:");
    if ((place != -1) && (place < 350)) {
        String s2 = text.substring(place + 15);
        text = s2.substring(s2.indexOf("<span>") + 6);
        String s1 = text.substring(0, (text.indexOf("</")));
        eraseCharactersFromString(s1, ",.");
        deaths = s1.toInt();
        return 1;
    }
    return 0;
}

void IRAM_ATTR isrHomeButton() {
    homeButton.pressed = true;
    homeButton.numberOfKeyPresses++;
}

bool wasHomeButtonPressed() {
    if (homeButton.numberOfKeyPresses) {
        homeButton.numberOfKeyPresses--;
        return true;
    }
    homeButton.pressed = false;
    return false;
}
