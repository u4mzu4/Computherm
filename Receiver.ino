#include <ComputhermRF.h>

#define BUTTONPIN 10
#define RELAYPIN1 12  //pump
#define RELAYPIN2 5   //boiler
#define LEDPIN 13
#define INPUT_PIN 9
#define DELAY 500
#define NRTRANS 3
#define AFTERCIRCTIME 360000  //6 min

//Enum
enum MAIN_SM {
  INIT      = 0,
  OFF       = 1,
  HEATING   = 2,
  AFTERCIRC = 3,
  MANUAL    = 4,
  RESERVED  = 9
};


const String senderAdresses [NRTRANS] = {"84BF7", "E3DF7", "DECAF"};

ComputhermRF rfReceiver = ComputhermRF(INPUT_PIN, 255);

bool ProcessRFData ()
{
  String receivedAddress;
  bool receivedCommand;
  static bool heatingCommand [NRTRANS];
  static unsigned long ledTimer;

  if (rfReceiver.isDataAvailable()) {
    rfReceiver.getData(receivedAddress, receivedCommand);
    for (int i = 0; i < NRTRANS; i++)
    {
      if (senderAdresses[i] == receivedAddress)
      {
        heatingCommand[i] = receivedCommand;
        digitalWrite(LEDPIN, LOW);
        ledTimer = millis();
      }
    }
  }
  if ((millis() - ledTimer > DELAY) && !digitalRead(LEDPIN))
  {
    digitalWrite(LEDPIN, HIGH);
  }
  return ((heatingCommand[0] || heatingCommand[1]) || heatingCommand[2]);
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
}

void loop() {
  static MAIN_SM heatingState = INIT;
  bool heatingNeeded;
  static unsigned long afterCircStart;

  switch (heatingState)
  {
    case INIT:
      {
        heatingState = OFF;
        heatingNeeded = 0;
        break;
      }
    case OFF:
      {
        heatingNeeded = ProcessRFData();
        if (heatingNeeded)
        {
          digitalWrite(RELAYPIN1, HIGH);
          digitalWrite(RELAYPIN2, HIGH);
          heatingState = HEATING;
        }
        if (!digitalRead(BUTTONPIN))
        {
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
        if (!heatingNeeded)
        {
          digitalWrite(RELAYPIN2, LOW);
          heatingState = AFTERCIRC;
          afterCircStart = millis();
        }
        break;
      }
    case AFTERCIRC:
      {
        if (millis() - afterCircStart > AFTERCIRCTIME)
        {
          digitalWrite(RELAYPIN1, LOW);
          heatingState = OFF;
        }
        break;
      }
    case MANUAL:
      {
        if (!digitalRead(BUTTONPIN))
        {
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

 
