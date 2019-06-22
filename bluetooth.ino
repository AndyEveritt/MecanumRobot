#include <SoftwareSerial.h>

void decodeMsg(String data, String &componentType, String &componentNum, String &command);
String getValue(String data, char separator, int index);
void updateLED(String state);

SoftwareSerial Bluetooth(A8, 38); // Arduino(RX, TX) - HC-05 Bluetooth (TX, RX)
String msg = "";                  // string received by HC-05
String componentType = "";        // type of component to control, eg led, motor, servo, etc.
String componentNum = "";         // which component in type to control, eg led0, led1, etc.
String command = "";              // command to send to component

int ledPins[] = {7, 37};

/* decodes the string received by the HC-05 bluetooth module */
void decodeMsg(String msg, String &componentType, String &componentNum, String &command)
{
  msg.replace("(", "");
  msg.replace(")", "");
  componentType = getValue(msg, ' ', 0);
  componentNum = getValue(msg, ' ', 1);
  command = getValue(msg, ' ', 2);
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void updateLED(int pin, String state)
{
  if (state == "off")
  {
    digitalWrite(pin, LOW);     // Turn LED OFF
    Bluetooth.println("LED: OFF"); // Send back, to the phone, the String "LED: ON"
  }
  else if (state == "on")
  {
    digitalWrite(pin, HIGH);
    Bluetooth.println("LED: ON");
  }
}

void setup()
{
  // Setup LED pins
  for (int i=0; i<(sizeof(ledPins)/sizeof(ledPins[0])); i++)
  {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  Serial.begin(38400);    // Default communication rate of the Bluetooth module
  Bluetooth.begin(38400); // Default communication rate of the Bluetooth module
  Bluetooth.setTimeout(1);
}

void loop()
{
  if (Bluetooth.available() > 0)
  {                               // Checks whether data is comming from the serial port
    msg = Bluetooth.readString(); // Reads the data from the serial port
    decodeMsg(msg, componentType, componentNum, command);
    Serial.println("msg: " + msg);
    Serial.println("component: " + componentType + " #" + componentNum);
    Serial.println("command: " + command);
    Serial.println();
  }

  if (componentType == "led")
  {
    int ledNum = componentNum.toInt();

    updateLED(ledPins[ledNum], command);
  }

  componentType = "";
  componentNum = "";
  command = "";
}
