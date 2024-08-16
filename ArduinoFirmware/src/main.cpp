#include <Arduino.h>
#include <AccelStepper.h>

constexpr auto DEVICE_GUID = "DC8EA937-C286-4896-A4BC-D2C945D9F4B6";
constexpr auto DIR_PIN = 2;
constexpr auto STEP_PIN = 3;
constexpr auto MS1_PIN = 5;
constexpr auto MS2_PIN = 4;
constexpr auto EN_PIN = 6;

AccelStepper myStepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

bool isConnected = false;

void setMicrostepping(int microstepping) {
  switch(microstepping) {
    case 1:
      digitalWrite(MS1_PIN, LOW);
      digitalWrite(MS2_PIN, LOW);
      break;
    case 2:
      digitalWrite(MS1_PIN, HIGH);
      digitalWrite(MS2_PIN, LOW);
      break;
    case 3:
      digitalWrite(MS1_PIN, LOW);
      digitalWrite(MS2_PIN, HIGH);
      break;
    case 4:
      digitalWrite(MS1_PIN, HIGH);
      digitalWrite(MS2_PIN, HIGH);
      break;
    default:
      digitalWrite(MS1_PIN, LOW);
      digitalWrite(MS2_PIN, LOW);
  }
}

String processCommand(String command) {
  char commandCode = command[0];

  String argString = command.substring(1);
  int argNumber = argString.toInt();
  bool isRunning = myStepper.isRunning() && myStepper.distanceToGo() != 0;

  switch(commandCode) {
    // CONNNECT
    case 'C':
      myStepper.enableOutputs();
      isConnected = true;
      return "OK#";
    // DISCONNECT
    case 'D':
      if(isRunning) {
        myStepper.setSpeed(0);
        myStepper.move(0);
        myStepper.runToPosition();
      }
      isConnected = false;
      myStepper.disableOutputs();
      return "OK#";
    // FORWARD steps
    case 'F':
      if(!isConnected) { return "NOK#"; }
      myStepper.move(-argNumber);
      myStepper.setSpeed(200);
      return "OK#";
    // BACKWARD steps
    case 'B':
      if(!isConnected) { return "NOK#"; }
      myStepper.move(argNumber);
      myStepper.setSpeed(200);
      return "OK#";
    // STOP
    case 'S':
      if(!isConnected) { return "NOK#"; }
      myStepper.setSpeed(0);
      myStepper.move(0);
      myStepper.runToPosition();
      return "OK#";
    // MICROSTEPPING
    case 'M':
      if(!isConnected) { return "NOK#"; }
      setMicrostepping(argNumber);
      return "OK#";
    // PING
    case 'P':
      return "OK:" + String(DEVICE_GUID) + "#";
    // STATUS (aka WHAT)
    case 'W':
      if(!isConnected) { return "NOK#"; }
      return isRunning ? "MOVING#" : "IDLE#";
    default:
      return "UNKNOWN#";
  }
}

void setup() {
  Serial.begin(57600, SERIAL_8N1);
  Serial.setTimeout(100);
  while (!Serial);
  Serial.println("INITIALIZED#");

  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  digitalWrite(MS1_PIN, LOW);
  digitalWrite(MS2_PIN, LOW);

  myStepper.setCurrentPosition(0);
	myStepper.setMaxSpeed(200.0);
  myStepper.setEnablePin(EN_PIN);
  myStepper.setPinsInverted(false, false, true);
  myStepper.disableOutputs();
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    String response = processCommand(command);
    Serial.println(response);
  }

  myStepper.runSpeedToPosition();
}