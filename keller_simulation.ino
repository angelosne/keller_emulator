#include <Wire.h>
#include <stdlib.h>

uint16_t pressure = 49152;  // Variable to store the received number
uint16_t temperature = 24017;
uint8_t status = 67;  //Non busy, Normal mode, checksum okay
uint8_t i2cAddress = 0x41;
boolean isI2Cavailable = true;

void (*resetFunc)(void) = 0;  // create a standard reset function

void setup() {
  Serial.begin(9600);  // Initialize the serial communication
  Serial.setTimeout(10000);
  Wire.begin(i2cAddress);        // Initialize I2C communication with slave address
  Wire.onRequest(requestEvent);  // Register the request event handler READ command
  Wire.onReceive(receiveEvent);  // Register the request event handler WRITE command
  Serial.println("Keller I2C emulator");
  Serial.println("Press 'h' and ENTER to see the help menu");
  Serial.println("9600N81, Carriage Return only");
}

void loop() {
  if (Serial.available()) {
    String receivedCommand = Serial.readStringUntil('\r');  // Read the received command
    receivedCommand.toLowerCase();

    // Process the received command
    parseCommand(receivedCommand);
  }
}

void printMenu() {
  Serial.println("\nCommand Menu:");
  Serial.println("h: Prints this menu");
  Serial.println("p (number): Set the pressure. Values 0-65535");
  Serial.println("t (number): Set the temperature. Values 0-65535");
  Serial.println("s: Print the status of the registers");
  Serial.println("e: Enables the I2C sensor");
  Serial.println("d: Disables the I2C sensor");
  Serial.println("a (hex address): Change the I2C address. Provide number in hex");
  Serial.println("r: Software reset");
}
void printStatus() {
  addressStatus();
  Serial.print("Status: ");
  Serial.println((byte)status);  // Status
  pressureStatus();
  temperatureStatus();
}
void parseCommand(String command) {
  // Split the command into separate tokens
  int spaceIndex = command.indexOf(' ');
  String keyword = command.substring(0, spaceIndex);
  String argument = command.substring(spaceIndex + 1);

  // Process the command and update the variables
  if (keyword == "p") {
    // Convert the argument to an integer or float, if necessary

    uint16_t inputPressure = strtoul(argument.c_str(), NULL, 10);

    if (inputPressure > 65536 || inputPressure < 0) {
      Serial.println("Input is out of range. Try again...");
    } else {
      // Update the pressure variable
      pressure = inputPressure;
      Serial.println("\nPressure changed");
      pressureStatus();
    }
  } else if (keyword == "t") {
    // Convert the argument to an integer or byte, if necessary
    uint16_t inputTemperature = strtoul(argument.c_str(), NULL, 10); //unsigned long
    if (inputTemperature > 65536 || inputTemperature < 0) {
      Serial.println("Input is out of range. Try again...");
    } else {
      // Update the temperature variable
      temperature = inputTemperature;
      Serial.println("\nTemperature changed");
      temperatureStatus();
    }
  } else if (keyword == "a") {
    // Convert the argument to an integer or byte, if necessary
    uint8_t inputAddress = strtol(argument.c_str(), NULL, 16); //long
    // Update the address variable
    i2cAddress = inputAddress;
    Wire.end();
    Wire.begin(i2cAddress);
    Serial.print("\nAddress changed");
    addressStatus();
  } else if (keyword == "e") {
    Wire.begin(i2cAddress);
    isI2Cavailable = true;
    Serial.print("\nI2C is ");
    isI2Cavailable ? Serial.println("ENABLED") : Serial.println("DISABLED");
  } else if (keyword == "d") {
    Wire.end();
    isI2Cavailable = false;
    Serial.print("\nI2C is ");
    isI2Cavailable ? Serial.println("ENABLED") : Serial.println("DISABLED");
  } else if (keyword == "s") {
    printStatus();
  } else if (keyword == "h") {
    printMenu();
  } else if (keyword == "r") {
    resetFunc();
  } else {
    Serial.print("Wrong input\n");
  }
}

//I2C write command handler
void receiveEvent() {
  if (Wire.read() == 0xAC) {
    Serial.println("Data requested");
  }
}

//I2C read command handler
void requestEvent() {
  // Send pressure value over I2C
  Wire.write((byte)status);                // Status
  Wire.write((byte)(pressure >> 8));       // High byte
  Wire.write((byte)(pressure & 0xFF));     // Low byte
  Wire.write((byte)(temperature >> 8));    // High byte
  Wire.write((byte)(temperature & 0xFF));  // Low byte
  Serial.println("Data sent");
}

//Convertion of pressure to bars
float pressureInBars(uint16_t pressure) {
  float pressureInBars = (float)(pressure - 16384) / 32768 * 20;
  return pressureInBars;
}

//Conversion of temperature to Celcius
float temperatureInCelcius(uint16_t temperature) {
  float temperatureInCelcius = (float)(temperature - 16384) / 32768 * 20;
  return temperatureInCelcius;
}

void pressureStatus() {
  Serial.print("Pressure: ");
  Serial.println(pressure);
  Serial.print("Pressure in Bars: ");
  Serial.println(pressureInBars(pressure));
  Serial.print("Pressure High byte: ");
  Serial.println((byte)(pressure >> 8));  // High byte
  Serial.print("Pressure Low byte: ");
  Serial.println((byte)(pressure & 0xFF));  // Low byte
}

void temperatureStatus() {
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Temperature in Celcius: ");
  Serial.println(temperatureInCelcius(temperature));
  Serial.print("Temperature High byte: ");
  Serial.println((byte)(temperature >> 8));  // High byte
  Serial.print("Temperature Low byte: ");
  Serial.println((byte)(temperature & 0xFF));  // Low byte
}

void addressStatus() {
  Serial.print("\nI2C address: 0x");
  Serial.println(i2cAddress, HEX);
  Serial.print("I2C is ");
  isI2Cavailable ? Serial.println("ENABLED") : Serial.println("DISABLED");
}