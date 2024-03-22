#include <Wire.h>

byte i2cAddr = 0x01;    //Left Controller i2c address
byte buttons = 0xF1;    //Register for buttons
byte xAxis = 0xF2;      //Register for xAxis
byte yAxis = 0xF3;      //Register for yAxis
byte zAxis = 0xF4;      //Register for zAxis
byte outputMode = 0x00; //Set to match a register for output
byte outputArray[2];    //Output data stored here

//Buttons
byte buttonPins[10] = {0,1,2,3,4,5,6,7,8,9}; //Up, Down, Left, Right, Start, Select, L1, L3, Vol+, Vol-
byte buttonCount = 10;
byte lastButtonState[10]; //Button state storage

//Analog Axis
byte xAxisPin = A0;
byte yAxisPin = A1;
byte zAxisPin = A2;
int xAxisValue;
int yAxisValue;
int zAxisValue;

void setup() {
  Wire.begin(i2cAddr);          //Begin i2c communication
  Wire.onRequest(requestEvent); //Call requestEvent() function when a request is received.
  Wire.onReceive(receiveEvent); //Call rreceiveEvent() function when a data is received.

  for(int i = 0; i < buttonCount; i++){ //Setup controller input pins
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(xAxisPin, INPUT);
  pinMode(yAxisPin, INPUT);
  pinMode(zAxisPin, INPUT);
}

void loop() {
  buttonRead();
  axisRead();
}

void buttonRead(){ //Read button inputs and set state arrays.
  for (int i = 0; i < buttonCount; i++){ 
    int input = !digitalRead(buttonPins[i]);
    if (input != lastButtonState[i]){
      lastButtonState[i] = input;
    }
  }
}

void axisRead(){  //Read analog axis and store values
  xAxisValue = analogRead(xAxisPin);
  yAxisValue = analogRead(yAxisPin);
  zAxisValue = analogRead(zAxisPin);
}

void buttonOutput(){  //Package current state of buttons into output array as bytes. Each byte of the output represents a button.
  unsigned int output = 0;
  for(int i = 0; i < buttonCount; i++){
    output = output << 1;
    if(lastButtonState[i] == 1){
      output += 1;
    }
  }
  outputArray[0] = lowByte(output);
  outputArray[1] = highByte(output);
}

void requestEvent(){ //Send the requested data
  if(outputMode == buttons){
    buttonOutput();
    Wire.write(outputArray,2);
  } else if(outputMode == xAxis){
    outputArray[0] = lowByte(xAxisValue);
    outputArray[1] = highByte(xAxisValue);
    Wire.write(outputArray,2);
  } else if(outputMode == yAxis){
    outputArray[0] = lowByte(yAxisValue);
    outputArray[1] = highByte(yAxisValue);
    Wire.write(outputArray,2);
  } else if(outputMode == zAxis){
    outputArray[0] = lowByte(zAxisValue);
    outputArray[1] = highByte(zAxisValue);
    Wire.write(outputArray,2);
  }
}

void receiveEvent(int value){ //Sets which output type is being requested
  while(Wire.available()){
    outputMode = Wire.read();
  }
}
