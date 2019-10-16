#include <SoftwareSerial.h>
#include <AccelStepper.h>

void decodeMsg(String data, String &componentType, String &componentNum, String &command);
String getValue(String data, char separator, int index);
void updateLED(String state);

typedef struct {
  int translateX;
  int translateY;
  int theta;
} DriveCommand;

SoftwareSerial Bluetooth(A8, 38); // Arduino(RX, TX) - HC-05 Bluetooth (TX, RX)
String msg = "";                  // string received by HC-05
String componentType = "";        // type of component to control, eg led, motor, servo, etc.
String componentNum = "";         // which component in type to control, eg led0, led1, etc.
String command = "";              // command to send to component

DriveCommand driveCommand;
AccelStepper WheelFL(1, 40, 41);  // (Type:driver, STEP, DIR) - Stepper 1
AccelStepper WheelFR(1, 42, 43);  // Stepper 2
AccelStepper WheelBL(1, 46, 47);  // Stepper 3
AccelStepper WheelBR(1, 44, 45);  // Stepper 4

int ledPins[] = {7, 37};

int speedMulti = 10;

/* decodes the string received by the HC-05 bluetooth module */
void decodeMsg(String msg, String &componentType, String &componentNum, String &command)
{
  msg.replace("(", "");
  msg.replace(")", "");
  componentType = getValue(msg, ' ', 0);
  componentNum = getValue(msg, ' ', 1);
  command = getValue(msg, ' ', 2);
}

/* Separate a string about a specific charactor */
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

/* decodes the command received for driving from mobile app */
void decodeDriveMsg(String msg, DriveCommand &driveCommand)
{
  driveCommand.translateX = getValue(msg, ',', 0).toInt();
  driveCommand.translateY = getValue(msg, ',', 1).toInt();
  driveCommand.theta      = getValue(msg, ',', 2).toInt();
}

/* updates stepper motor speeds */
void updateDrive(DriveCommand driveCommand)
{
  int speedFL, speedFR, speedBL, speedBR;

  speedFL = speedMulti * ( driveCommand.translateY + driveCommand.translateX + driveCommand.theta);
  speedFR = speedMulti * (-driveCommand.translateY + driveCommand.translateX + driveCommand.theta);
  speedBL = speedMulti * ( driveCommand.translateY - driveCommand.translateX + driveCommand.theta);
  speedBR = speedMulti * (-driveCommand.translateY - driveCommand.translateX + driveCommand.theta);

  Bluetooth.println((String)speedFL+" "+speedFR+"\n"+speedBL+" "+speedBR);

  WheelFL.setSpeed(speedFL);
  WheelFR.setSpeed(speedFR);
  WheelBL.setSpeed(speedBL);
  WheelBR.setSpeed(speedBR);
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

  // Set initial max speed for stepper motors
  WheelFL.setMaxSpeed(3000);
  WheelFR.setMaxSpeed(3000);
  WheelBL.setMaxSpeed(3000);
  WheelBR.setMaxSpeed(3000);

  Serial.begin(38400);    // Default communication rate of the Bluetooth module
  Bluetooth.begin(38400); // Default communication rate of the Bluetooth module
  Bluetooth.setTimeout(1);
}

void loop()
{
  // get Serial msg
  if (Bluetooth.available() > 0)
  {                               // Checks whether data is comming from the serial port
    msg = Bluetooth.readString(); // Reads the data from the serial port
    decodeMsg(msg, componentType, componentNum, command);
    Serial.println("msg: " + msg);
    Serial.println("component: " + componentType + " #" + componentNum);
    Serial.println("command: " + command);
    Serial.println();
  }

  // component controller
  if (componentType == "led")
  {
    int ledNum = componentNum.toInt();

    updateLED(ledPins[ledNum], command);
  }

  if (componentType == "drive")
  {
    decodeDriveMsg(command, driveCommand);
    updateDrive(driveCommand);
  }

  WheelFL.runSpeed();
  WheelFR.runSpeed();
  WheelBL.runSpeed();
  WheelBR.runSpeed();

  // reset msg
  componentType = "";
  componentNum = "";
  command = "";
}
