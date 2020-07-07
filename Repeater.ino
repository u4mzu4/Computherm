#include <ComputhermRF.h>

#define DELAY 1000 //1 sec
#define LEDPIN 2
#define INPUT_PIN 13
#define OUTPUT_PIN 14

const String ADDR2A = "E3DF7";
const String ADDR2B = "DECAF";


ComputhermRF rf = ComputhermRF(INPUT_PIN, OUTPUT_PIN);

void setup() {
  pinMode(OUTPUT_PIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(LEDPIN, LOW);
  rf.startReceiver();
}

void loop() {
  static unsigned long msgReceivedTime;
  static bool dataSent;
  static bool receivedCommand;
  String receivedAddress;

  if (rf.isDataAvailable()) {
    rf.getData(receivedAddress, receivedCommand);
    if (ADDR2A == receivedAddress)
    {
      msgReceivedTime = millis();
      dataSent = 0;
      pinMode(LEDPIN, HIGH);
    }
  }
  if ((!dataSent) && (millis() > (msgReceivedTime + DELAY)))
  {
    rf.sendMessage(ADDR2B, receivedCommand);
    dataSent = 1;
    pinMode(LEDPIN, LOW);
  }
}
