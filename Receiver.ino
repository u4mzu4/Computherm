#include <ComputhermRF.h>

#define BUTTONPIN 0
#define RELAYPIN 12
#define LEDPIN 13
#define INPUT_PIN 4
#define DELAY 500
#define AUTO 0
#define MANUAL 1


const String ADDR1 = "84BF7";
const String ADDR2A = "E3DF7";
const String ADDR2B = "DECAF";

ComputhermRF rf = ComputhermRF(INPUT_PIN, 255);

void setup() {
  pinMode(BUTTONPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
  digitalWrite(RELAYPIN, LOW);
  rf.startReceiver();
}

void loop() {
  String receivedAddress;
  bool receivedCommand;
  static bool command1;
  static bool command2;
  static bool modeState = AUTO;
  static unsigned long ledTimer;

  if (!modeState)
  {
    if (rf.isDataAvailable()) {
      rf.getData(receivedAddress, receivedCommand);
      if (ADDR1 == receivedAddress)
      {
        command1 = receivedCommand;
        digitalWrite(LEDPIN, LOW);
        ledTimer = millis();
      }
      if ((ADDR2A == receivedAddress) || (ADDR2B == receivedAddress))
      {
        command2 = receivedCommand;
        digitalWrite(LEDPIN, LOW);
        ledTimer = millis();
      }
      if (command1 || command2)
      {
        digitalWrite(RELAYPIN, HIGH);
      }
      else
      {
        digitalWrite(RELAYPIN, LOW);
      }
    }
    if ((millis() > ledTimer + DELAY) && !digitalRead(LEDPIN))
    {
      digitalWrite(LEDPIN, HIGH);
    }
    if (!digitalRead(BUTTONPIN))
    {
      modeState = MANUAL;
      digitalWrite(LEDPIN, LOW);
      digitalWrite(RELAYPIN, HIGH);
      delay(DELAY);
    }
  }
  else
  {
    if (!digitalRead(BUTTONPIN))
    {
      modeState = AUTO;
      digitalWrite(LEDPIN, HIGH);
      digitalWrite(RELAYPIN, LOW);
      delay(DELAY);
    }
  }
}
