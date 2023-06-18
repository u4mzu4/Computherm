#include <ComputhermRF.h>
#include <ESPAsyncWebServer.h>

#define BUTTONPIN 10
#define RELAYPIN1 12  //pump
#define RELAYPIN2 5   //boiler
#define LEDPIN 13
#define INPUT_PIN 9
#define DELAY 500
#define NRTRANS 3
#define AFTERCIRCTIME 360000  //6 min
#define TIMEOUT 900000        //15 min
#define PORT 80               //HTTP

//Enum
enum MAIN_SM {
  INIT = 0,
  OFF = 1,
  HEATING = 2,
  AFTERCIRC = 3,
  MANUAL = 4,
  RESERVED = 9
};


const String senderAdresses[NRTRANS] = { "84BF7", "E3DF7", "DECAF" };
const char* ssid = "Thermostat";
const char* password = "123456789";
bool heatingCommand[NRTRANS];
unsigned long lastCommandTime[NRTRANS];
const char index_html[] PROGMEM = "<!DOCTYPE html><head><title>Termosztat</title></head><body><p>Ado 84BF7: %0PH%<br />Ido: %3PH%<br />Ado E3DF7: %1PH%<br />Ido: %4PH%<br />Erosito DECAF: %2PH%<br />Ido: %5PH%</p></p></body></html>";

ComputhermRF rfReceiver = ComputhermRF(INPUT_PIN, 255);
AsyncWebServer server(PORT);

bool ProcessRFData() {
  String receivedAddress;
  bool receivedCommand;
  static unsigned long ledTimer;

  if (rfReceiver.isDataAvailable()) {
    rfReceiver.getData(receivedAddress, receivedCommand);
    for (int i = 0; i < NRTRANS; i++) {
      if (senderAdresses[i] == receivedAddress) {
        heatingCommand[i] = receivedCommand;
        digitalWrite(LEDPIN, LOW);
        ledTimer = millis();
        lastCommandTime[i] = millis();
      }
    }
  }
  if ((millis() - ledTimer > DELAY) && !digitalRead(LEDPIN)) {
    digitalWrite(LEDPIN, HIGH);
  }
  for (int j = 0; j < NRTRANS; j++) {
    if (millis() - lastCommandTime[j] > TIMEOUT) {
      heatingCommand[j] = false;
    }
  }

  return ((heatingCommand[0] || heatingCommand[1]) || heatingCommand[2]);
}

// Replaces placeholder with button section in your web page
String processor(const String& var) {
  String tempvar = var;
  tempvar.remove(2);
  int numberofPH = tempvar.toInt();
  if (numberofPH < NRTRANS) {
    return ((heatingCommand[numberofPH] == 0) ? "OFF" : "ON");
  } else {
        unsigned long cmdTime = (millis() - lastCommandTime[numberofPH - 3])>>10;
    return String(cmdTime);
  }
}

void setup() {
  pinMode(BUTTONPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(RELAYPIN1, OUTPUT);
  pinMode(RELAYPIN2, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
  digitalWrite(RELAYPIN1, LOW);
  digitalWrite(RELAYPIN2, LOW);
  rfReceiver.startReceiver();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  delay(500);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.begin();
}

void loop() {
  static MAIN_SM heatingState = INIT;
  bool heatingNeeded;
  static unsigned long afterCircStart;

  switch (heatingState) {
    case INIT:
      {
        heatingState = OFF;
        heatingNeeded = 0;
        break;
      }
    case OFF:
      {
        heatingNeeded = ProcessRFData();
        if (heatingNeeded) {
          digitalWrite(RELAYPIN1, HIGH);
          digitalWrite(RELAYPIN2, HIGH);
          heatingState = HEATING;
        }
        if (!digitalRead(BUTTONPIN)) {
          digitalWrite(LEDPIN, LOW);
          digitalWrite(RELAYPIN1, HIGH);
          digitalWrite(RELAYPIN2, HIGH);
          delay(DELAY);
          heatingState = MANUAL;
        }
        break;
      }
    case HEATING:
      {
        heatingNeeded = ProcessRFData();
        if (!heatingNeeded) {
          digitalWrite(RELAYPIN2, LOW);
          heatingState = AFTERCIRC;
          afterCircStart = millis();
        }
        break;
      }
    case AFTERCIRC:
      {
        if (millis() - afterCircStart > AFTERCIRCTIME) {
          digitalWrite(RELAYPIN1, LOW);
          heatingState = OFF;
        }
        break;
      }
    case MANUAL:
      {
        if (!digitalRead(BUTTONPIN)) {
          digitalWrite(LEDPIN, HIGH);
          digitalWrite(RELAYPIN1, LOW);
          digitalWrite(RELAYPIN2, LOW);
          delay(DELAY);
          heatingState = OFF;
        }
        break;
      }
    default:
      // empty
      break;
  }
}