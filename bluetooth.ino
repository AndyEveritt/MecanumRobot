#include <SoftwareSerial.h>
#define ledPin 7

void decodeMsg(String data, String &component, String &command);
String getValue(String data, char separator, int index);
void updateLED(String state);

SoftwareSerial Bluetooth(A8, 38); // Arduino(RX, TX) - HC-05 Bluetooth (TX, RX)
String msg = "";
String component = "";
String command = "";

void decodeMsg(String msg, String &component, String &command)
{
  msg.replace("(", "");
  msg.replace(")", "");
  component = getValue(msg, ' ', 0);
  command = getValue(msg, ' ', 1);
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

void updateLED(String state)
{
  if (state == "off")
  {
    digitalWrite(ledPin, LOW);     // Turn LED OFF
    Bluetooth.println("LED: OFF"); // Send back, to the phone, the String "LED: ON"
  }
  else if (state == "on")
  {
    digitalWrite(ledPin, HIGH);
    Bluetooth.println("LED: ON");
  }
}

void setup()
{
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(38400);    // Default communication rate of the Bluetooth module
  Bluetooth.begin(38400); // Default communication rate of the Bluetooth module
  Bluetooth.setTimeout(1);
}

void loop()
{
  if (Bluetooth.available() > 0)
  {                               // Checks whether data is comming from the serial port
    msg = Bluetooth.readString(); // Reads the data from the serial port
    decodeMsg(msg, component, command);
    Serial.println("msg: " + msg);
    Serial.println("component: " + component);
    Serial.println("command: " + command);
    Serial.println();
  }

  if (component == "led")
  {
    updateLED(command);
  }

  component = "";
  command = "";
}
