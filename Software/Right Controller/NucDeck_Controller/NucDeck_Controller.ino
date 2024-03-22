/*
 * NUCDeck Controller - V1.0
 * Daniel McKenzie - 21st March 2024
 * 
 */
#include <Wire.h>
#include <FastLED.h>
#include <EEPROM.h>
#include <Joystick.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <TFT_eSPI.h>
#include "images.h" //XBM Images

//I2C Controller Addresses
byte controllerAddr = 0x01;         //i2c Address of other half of controller
byte buttons = 0xF1;                //Register for buttons
byte xAxis = 0xF2;                  //Register for X Axis
byte yAxis = 0xF3;                  //Register for Y Axis
byte zAxis = 0xF4;                  //Register for Z Axis
const byte CONTROLLER_I2C_SDA = 20; //SDA pin for other controller half
const byte CONTROLLER_I2C_SCL = 21; //SCL pin for other controller half
int i2cButtonCount = 10;            //Number of buttons on i2c

//MPU6050
const int MPU6050 = 0x68;
const byte MPU6050_SDA = 14;
const byte MPU6050_SCL = 15;
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float GyroLastX, GyroLastY, GyroLastZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;
int c = 0;
int xOffset = 0;
int yOffset = 0;
unsigned long gyroTimer;

//Default Joystick calibration settings and EEPROM storage Address
int minLeftX = 129; //EEPROM Adr = 1
int maxLeftX = 590; //EEPROM Adr = 3
int midLeftX = 362; //EEPROM Adr = 5

int minLeftY = 144; //EEPROM Adr = 7
int maxLeftY = 600; //EEPROM Adr = 9
int midLeftY = 372; //EEPROM Adr = 11

int minRightY = 225; //EEPROM Adr = 13
int maxRightY = 810; //EEPROM Adr = 15
int midRightY = 482; //EEPROM Adr = 17

int minRightX = 180; //EEPROM Adr = 19
int maxRightX = 760; //EEPROM Adr = 21
int midRightX = 442; //EEPROM Adr = 23

int minL2 = 465;  //EEPROM Adr = 25
int maxL2 = 750;  //EEPROM Adr = 27
int minR2 = 670;  //EEPROM Adr = 29
int maxR2 = 1023; //EEPROM Adr = 31

//Joystick LookUp Tables, compiled at startup or after joystick calibration.
byte leftXLUT[1024];
byte leftYLUT[1024];
byte rightXLUT[1024];
byte rightYLUT[1024];

//Joystick Settings
const bool invertLeftY = true;        //------------------------------------------
const bool invertLeftX = false;       //Change these settings for Inverted mounting 
const bool invertRightY = false;      //of joysticks.
const bool invertRightX = false;      //------------------------------------------
const int deadBandLeft = 25;          //
const int deadBandRight = 25;         //Joystick deadband settings. Deadband is the same for both axis on each joystick.
const bool useDeadband = true;        //
int joyDeadZone = 25;                 //Secondary Joystick Deadband. Can't remember why I added this, may be able to replace with deadBandLeft or deadBandRight.
const int earlyLeftX = 30;            //--------------------------------------------------
const int earlyLeftY = 30;            //Distance from end of travel to achieve full axis movement. 
const int earlyRightY = 30;           //This helps square out each axis response to allow full movement speed with direction input.
const int earlyRightX = 30;           //--------------------------------------------------
const int triggerDeadband = 40;       //Trigger Deadband
const int mouseSensitivity = 20;      //Sensitivity Setting for mouse movements
const int mouseRightClick = 40;       //Deadband for mouse right click

//LED Setup
#define NUM_LEDS 6  //Number of LED's
#define DATA_PIN 7  //IO pin of LED's
CRGB leds[NUM_LEDS];  //Setup LED's
int LED_Values[6] = {0,0,0,0,0,0}; //Array to store brightness values of LED's
int ledHue = 150; //LED Hue
int ledSat = 255; //LED Saturation
int ledBrightness = 255; //LED Brightness
int currentBrightness;  //Current LED Brightness
String rgbMode[2] = {"Breath","Halo"}; //LED pattern. Used in RGB Menu
int rgbModeSelect = 0;  //LED mode selected in menu
unsigned long LED_Timer;  //LED effect timer
unsigned long LED_Fadeout;  //LED fade out timer
unsigned long LED_Fadein; //LED fade in timer
int LED_Speed = 20; //LED Effect Speed
int LED_OutSpeed = 20;  //LED fade out speed
int ledSequenceNo = 0;  //Used to store which LED we are up to in the animation sequence

//Controller Button setup
int buttonCount = 7;                   //Number of buttons connected to Pico
byte buttonPins[7] = {0,1,8,13,4,6,9}; //A,B,X,Y,Menu, R1, R3
byte modeSwitch = 5;                   //Digital IO of controller mode switch
int lastMode = 2;                      //Last controller mode displayed. Used for display updates. 0 = Controller Mode, 1 = Keyboard Mode, 2 = Initialised

//Menu Variables
unsigned long interfaceTimer;     //Timer for menu interface
int interfaceDelay = 120;         //frequency to check for button presses in menu operation
bool menuOpen = false;            //Set to true to open Main menu
bool rgbMenu = false;             //Set to true to open RGB menu
bool batteryMenu = false;         //Set to true to open Battery menu
boolean calibrationMode = false;  //Set to true to open Calibration menu
int menuSelected = 0;             //which menu item is highlighted
int menuOptions = 7;              //Number of Items in Menu List
String mainMenu[7] = {"Aim Assist","Calibrate", "Battery", "RGB", "Settings", "ALT F4", "ESC"}; //Main Menu titles
bool aimAssist = true;            //If aim assist is enabled. NOT YET IMPLEMENTED
bool shutdownMode = false;        //Set to true if shutdown is initiated
int calibrationStep = 1;          //Stage of calibration process
bool calibrationDisplayUpdate = false;  //Used for display updates during calibration process        

//Button state arrays
byte lastButtonState[17]; //Empty State array for buttons last sent state.
byte currentButtonState[17]; //Empty State array for buttons.

TFT_eSPI tft = TFT_eSPI();  //Setup controller display.
/*The rest of display settings are configured in User_Setup.h from the TFT_eSPI library. Your display will not function until this file is set up
correctly. The following settings are what is required:

#define USER_SETUP_INFO "User_Setup"
#define ST7735_DRIVER
#define TFT_RGB_ORDER TFT_BGR
#define TFT_WIDTH  80
#define TFT_HEIGHT 160
#define ST7735_GREENTAB160x80
#define TFT_INVERSION_ON
#define TFT_CS   10
#define TFT_DC   16
#define TFT_RST  17
#define TFT_MOSI 19
#define TFT_SCLK 18
#define LOAD_GLCD
#define LOAD_FONT2
#define SMOOTH_FONT
#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
*/

//Battery
int batteryPercent = 100;           //System state of charge. Used to for battery display
bool charging = false;              //Set to true when charger is detected and battery is < 99.95%
bool lastChargingDisplay = false;   //Used for display update of charging status
bool onAC = false;                  //Set to true when charger is detected and battery is > 99.95%
long lastBatteryUpdate = 0;         //Battery display timer
int batteryUpdateInterval = 5000;   //Battery display update interval
String TTE;                         //String for storing time till empty. Used in battery display
float lastSOC;                      //SOC when last checked. Used for display update.
float currentSOC;                   //Current State of charge.
int lastBatteryDisplay;             //Last battery percentage displayed. Used for display update.

void setup() {
  //Left Controller I2C Setup
  Wire.setSDA(CONTROLLER_I2C_SDA);
  Wire.setSCL(CONTROLLER_I2C_SCL);
  Wire.begin();

  //MPU6050 Setup
  Wire1.setSDA(MPU6050_SDA);
  Wire1.setSCL(MPU6050_SCL);
  Wire1.begin();
  mpu6050_Setup();

  //Controller Output Setup
  Joystick.begin();
  Joystick.useManualSend(true);
  Joystick.use8bit(true);

  //Mouse and Keyboard Output Setup
  Mouse.begin();
  Keyboard.begin(KeyboardLayout_en_US);

  //Initialize eeprom
  EEPROM.begin(512);

  //LCD Setup
  tft.init();
  tft.fillScreen(TFT_BLACK);
  
  //LED Setup
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);

  //Pico Buttons Setup
  for(int i = 0; i < buttonCount; i++){ //Set all button pins as input pullup. Change to INPUT if using external resistors.
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(modeSwitch, INPUT_PULLUP);  //Turn on pullup for mode switch
  pinMode(28, INPUT); //Set input for right trigger
  pinMode(27, INPUT); //Set input for right joystick X
  pinMode(26, INPUT); //Set input for right joystick Y

  //PC Power Switch
  pinMode(3, OUTPUT); //Setup power switch pin as an output
  digitalWrite(3, LOW);  //Set output low so power doesn't switch off immediately 

  Serial.begin(9600); //Begin serial connection. Uncomment following two lines for debugging to ensure serial has connected before output begins.
  //while (!Serial)
     //delay(10);
  max17261Init(); //Set up fuel gauge
  calculate_IMU_error();  //Setup MPU6050
  readCalibrationData();  //Read joystick calibration from EEPROM
  rebuildLUTs();          //Build Lookup Tables with data from EEPROM
  startupScreen();        //Display welcome screen
  delay(5000);            //Wait 5s
}

void loop() {
  if(rgbModeSelect == 0){
    rgbBreath();
  } else {
    rgbHalo();
  }
  buttonRead();
  mediaControls();
  if(!menuOpen && !calibrationMode && !rgbMenu && !batteryMenu){
    mainScreen();
    if(lastButtonState[4] == 1){
      menuMode();
    }
    if(lastMode == 0){
      gamepadMode();
    } else {
      keyboardMode();
    }
    modeToggle();
  }
  if(menuOpen){
    menuMode();
  }
  if(calibrationMode){
    joystickCalibration();
  }
  if(rgbMenu){
    rgbConfig();
  }
  if(batteryMenu){
    batteryInfo();
  }
  currentSOC = getSOC();
  batteryPercent = currentSOC;

  if(lastSOC < currentSOC){
    charging = true;
    lastSOC = currentSOC;
  }
  if(lastSOC > currentSOC){
    charging = false;
    lastSOC = currentSOC;
  }
  if(currentSOC > 99.95){
    onAC = true;
  }

  if(lastBatteryUpdate + batteryUpdateInterval < millis() | lastBatteryUpdate == 0){
    float timeToEmpty = getTimeToEmpty();
    int hours = timeToEmpty;
    int minutes = ((timeToEmpty - hours) * 60);
    if(minutes < 10){
      TTE = ((String)hours + ":0" + (String)minutes);
    } else {
      TTE = ((String)hours + ":" + (String)minutes);
    }
    lastBatteryUpdate = millis();
  }
}

void rebuildLUTs(){ //Rebuild all joystick LUT's at once.
  joystickBuildLUT(leftXLUT, minLeftX, midLeftX, maxLeftX, earlyLeftX, deadBandLeft, invertLeftX);
  joystickBuildLUT(leftYLUT, minLeftY, midLeftY, maxLeftY, earlyLeftY, deadBandLeft, invertLeftY);
  joystickBuildLUT(rightXLUT, minRightX, midRightX, maxRightX, earlyRightX, deadBandRight, invertRightX);
  joystickBuildLUT(rightYLUT, minRightY, midRightY, maxRightY, earlyRightY, deadBandRight, invertRightY);
}

 /*This function builds a lookup table for the given axis using the provided min, max and middle values for each axis. Early stop, deadband and inversion 
  * are all configurable via the joystick settings variables. Maximum, minimum and mid point are all set by the calibration sequence.
  */
void joystickBuildLUT(byte output[1024], int minIn, int midIn, int maxIn, int earlyStop, int deadBand, bool invert){
  unsigned int temp;
  
  for(int i = 0; i < 1024; i++){
    if(i < midIn){
      if(i > minIn + earlyStop){
        temp = map(i, minIn, midIn - deadBand, 0, 127);
      } else {
        temp = 0;
      }
    } else {
      if(i < maxIn - earlyStop){
        temp = map(i, midIn + deadBand, maxIn, 127, 254);
      } else {
        temp = 254;
      }
    }
    if(i < midIn + deadBand && i > midIn - deadBand){
      temp = 127;
    }
    if(invert){
      temp = map(temp, 0, 254, 254, 0);
    }
    output[i] = temp;
  }
}

void buttonRead(){ //Read button inputs and set state arrays.
  for (int i = 0; i < buttonCount; i++){
    int input = !digitalRead(buttonPins[i]);
    if (input != lastButtonState[i]){
      lastButtonState[i] = input;
    }
  }
  unsigned int buttonReq = i2c_Request(controllerAddr, buttons);
  //Serial.println(buttonReq);
  for (int i = 0; i < i2cButtonCount; i++){
    if(buttonReq & 0b00000001 == 1){
      lastButtonState[i+buttonCount] = 1; //Pressed
    } else {
      lastButtonState[i+buttonCount] = 0; //Un-Pressed
    }
    buttonReq = buttonReq >> 1;
  }
}

/*int joystickScale(int val, int min, int max, int mid, bool invert){
  if(!invert){
    if(val < (mid - joyDeadZone) || val > (mid + joyDeadZone)){
      val = map(val, min - 10, max + 10, -32767, 32767);
    } else {
      val = 0;
    }
  } else {
    if(val < (mid - joyDeadZone) || val > (mid + joyDeadZone)){
      val = map(val, min - 10, max + 10, 32767, -32767);
    } else {
      val = 0;
    }
  }
  return val;
}*/

void gamepadMode(){ //Controller Gamepad mode
  joystickInput();
  //Buttons
  for(int i = 0; i < buttonCount + i2cButtonCount; i++){
    if(lastButtonState[i] != currentButtonState[i]){
      Joystick.button(i + 1, lastButtonState[i]);
      currentButtonState[i] = lastButtonState[i];
    }
  }
  Joystick.send_now();
  //delay(1);
}

void zeroGamepad(){ //Called to Zero all gamepad outputs when switching to keyboard mode
  for(int i = 0; i < buttonCount + i2cButtonCount; i++){
    Joystick.button(i + 1, 0);
  }
  Joystick.sliderLeft(127);
  Joystick.sliderRight(127);
  Joystick.Y(127);
  Joystick.X(127);
  Joystick.Z(0);
  Joystick.Zrotate(0);
  Joystick.send_now();
  //delay(1);
}

int i2c_Request(int adr, int i2c_Reg){ //Controller i2c request
  Wire.beginTransmission(adr);
  Wire.write(i2c_Reg);
  Wire.endTransmission();
  Wire.requestFrom(adr, 2);
  while (Wire.available()) {
    int readingLow = Wire.read();
    int readingHigh = Wire.read();
    return(word(readingHigh, readingLow));
  }
  return 0;
}

void mediaControls(){ //Volume button controls
  if(lastButtonState[7] == 1){
    Keyboard.consumerPress(KEY_VOLUME_INCREMENT);
    delay(50);
  } else if(lastButtonState[8] == 1){
    Keyboard.consumerPress(KEY_VOLUME_DECREMENT);
    delay(50);
  } else {
    if(!shutdownMode){
      Keyboard.consumerRelease();
    }
  }
}

void keyboardMode(){  //Controller Keyboard/mouse mode
  //Set all gamepad buttons/axis to 0.
  zeroGamepad();
  //Left Click
  if(lastButtonState[5] == 1){
    Mouse.press(MOUSE_LEFT);
  } else {
    Mouse.release(MOUSE_LEFT);
  }
  //Right Click
  if(triggerScale(analogRead(28), minR2, maxR2) > mouseRightClick){
    Mouse.press(MOUSE_RIGHT);
  } else {
    Mouse.release(MOUSE_RIGHT);
  }
  
  
  int mouseMoveX;
  int mouseMoveY;
  int var = i2c_Request(controllerAddr, xAxis);

  if(var != 0){
    mouseMoveX = map((leftXLUT[var]), 0, 254, -mouseSensitivity, mouseSensitivity);
  } else {
    mouseMoveX = 0;
  }
  var = i2c_Request(controllerAddr, yAxis);
  if(var != 0){
    mouseMoveY = map((leftYLUT[var]), 0, 254, -mouseSensitivity, mouseSensitivity);
  } else {
    mouseMoveY = 0;
  }

  Mouse.move(mouseMoveX,mouseMoveY);  //Moves mouse relative position. X,Y

  if(lastButtonState[11] == 1){
    Keyboard.press(KEY_ESC);  //Escape
    delay(50);
  } else if(lastButtonState[0] == 1){ 
    Keyboard.press(KEY_RETURN);  //Enter
    delay(50);
  } else if(lastButtonState[16] == 1){
    Keyboard.press(KEY_UP_ARROW);
    delay(50);
  } else if(lastButtonState[15] == 1){
    Keyboard.press(KEY_DOWN_ARROW);
    delay(50);
  } else if(lastButtonState[14] == 1){
    Keyboard.press(KEY_LEFT_ARROW);
    delay(50);
  } else if(lastButtonState[13] == 1){
    Keyboard.press(KEY_RIGHT_ARROW);
    delay(50);
  } else {
    Keyboard.releaseAll();
  }
}

void modeToggle(){  //Update display for controller mode
  int mode = digitalRead(modeSwitch);
  if(mode != lastMode){
    tft.fillScreen(TFT_BLACK);
    lastBatteryDisplay = 0;
    tft.setTextSize(1);
    if(mode){   
      tft.drawString("Keyboard", 11, 130, 2);
      tft.drawString("Mode", 25, 145, 2);
      tft.drawXBitmap(18, 100, keyboardIMG, keyboardWidth, keyboardHeight, TFT_WHITE);
    } else {
      tft.drawString("Controller", 10, 130, 2);
      tft.drawString("Mode", 25, 145, 2);
      tft.drawXBitmap(18, 100, gamepadIMG, gamepadWidth, gamepadHeight, TFT_WHITE);
    }
    lastMode = mode;
  }
}

void eepromLoad(){ //Loads stored settings from EEPROM
  if(readIntFromEEPROM(1) != -1){ //Check Joystick Calibration in EEPROM is not Empty
    readCalibrationData(); //Load joystick calibration from EEPROM
  }
}

void writeIntIntoEEPROM(int address, int number){ //Splits Int into BYTES for EEPROM
  byte byte1 = number >> 8;
  byte byte2 = number & 0xFF;
  EEPROM.write(address, byte1);
  EEPROM.write(address + 1, byte2);
}

int readIntFromEEPROM(int address){ //Converts BYTES to INT from EEPROM
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

void readCalibrationData(){ //Read calibration data from EEPROM
//Left X
  minLeftX = readIntFromEEPROM(1);
  maxLeftX = readIntFromEEPROM(3);
  midLeftX = readIntFromEEPROM(5);
//Left Y
  minLeftY = readIntFromEEPROM(7);
  maxLeftY = readIntFromEEPROM(9);
  midLeftY = readIntFromEEPROM(11);
//Right Y
  minRightY = readIntFromEEPROM(13);
  maxRightY = readIntFromEEPROM(15);
  midRightY = readIntFromEEPROM(17);
// Right X
  minRightX = readIntFromEEPROM(19);
  maxRightX = readIntFromEEPROM(21);
  midRightX = readIntFromEEPROM(23);
// L2
  minL2 = readIntFromEEPROM(25);
  maxL2 = readIntFromEEPROM(27);
// R2
  minR2 = readIntFromEEPROM(29);
  maxR2 = readIntFromEEPROM(31);
}

void writeCalibrationData(){ //Store calibration data in EEPROM
//Left X
  writeIntIntoEEPROM(1, minLeftX);
  writeIntIntoEEPROM(3, maxLeftX);
  writeIntIntoEEPROM(5, midLeftX);
//Left Y
  writeIntIntoEEPROM(7, minLeftY);
  writeIntIntoEEPROM(9, maxLeftY);
  writeIntIntoEEPROM(11, midLeftY);
//Right Y
  writeIntIntoEEPROM(13, minRightY);
  writeIntIntoEEPROM(15, maxRightY);
  writeIntIntoEEPROM(17, midRightY);
// Right X
  writeIntIntoEEPROM(19, minRightX);
  writeIntIntoEEPROM(21, maxRightX);
  writeIntIntoEEPROM(23, midRightX);
// L2
  writeIntIntoEEPROM(25, minL2);
  writeIntIntoEEPROM(27, maxL2);
// R2
  writeIntIntoEEPROM(29, minR2);
  writeIntIntoEEPROM(31, maxR2);
//Commit Changes
  EEPROM.commit();
}

void joystickInput(){ //Read joysticks and compare to LUT's . Read triggers and scale output
  int var = analogRead(26);
  if(aimAssist){
    Joystick.sliderLeft((rightYLUT[var] + yOffset) -127);
  } else {
    Joystick.sliderLeft((rightYLUT[var]) -127);
  }
  
  var = analogRead(27);
  if(aimAssist){
    Joystick.sliderRight((rightXLUT[var] + xOffset) -127);
  } else {
    Joystick.sliderRight((rightXLUT[var]) -127);
  }
  
  
  var = i2c_Request(controllerAddr, yAxis);
  if(var != 0){
    Joystick.Y((leftYLUT[var]) -127);
  }
  
  var = i2c_Request(controllerAddr, xAxis);
  if(var != 0){
    Joystick.X((leftXLUT[var]) -127);
  }

  var = i2c_Request(controllerAddr, zAxis);
  if(var != 0){
    var = triggerScale(var, minL2, maxL2);
    Joystick.Z(var -127);
  }

  var = analogRead(28);
  var = triggerScale(var, minR2, maxR2);
  Joystick.Zrotate(var -127);
}

int triggerScale(int value, int min, int max){  //Trigger scaling
  if(value < min + triggerDeadband){
    value = min;
  }
  if(value > max){
    value = max;
  }
  
  return map(value, min, max, 0, 254);
}

void read_MPU6050(){  //Read data from MPU6050. Incomplete. Motion control not yet implemented
  if(gyroTimer + 20 < millis()){
    if(yOffset > 0){
      yOffset--;
    }
    if(yOffset < 0){
      yOffset++;
    }
    if(xOffset > 0){
      xOffset--;
    }
    if(xOffset < 0){
      xOffset++;
    }
    gyroTimer = millis();
  }
  Wire1.beginTransmission(MPU6050);
  Wire1.write(0x43); // Gyro data first register address 0x43
  Wire1.endTransmission(false);
  Wire1.requestFrom(MPU6050, 6, true); // Read 4 registers total, each axis value is stored in 2 registers
  GyroX = (Wire1.read() << 8 | Wire1.read()) / 131.0; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GyroY = (Wire1.read() << 8 | Wire1.read()) / 131.0;
  GyroZ = (Wire1.read() << 8 | Wire1.read()) / 131.0;
  // Correct the outputs with the calculated error values
  GyroX = GyroX + 0.56; // GyroErrorX ~(-0.56)
  GyroY = GyroY - 2; // GyroErrorY ~(2)
  GyroZ = GyroZ + 0.79; // GyroErrorZ ~ (-0.8)

  int var = GyroX - GyroLastX;
  
  if(var > 2 || var < -2){ //deadzone
    if(var < 20 && var > -20){
      yOffset = var;
    }
    if(var > 20){
      yOffset = 20;
    }
    if(var < -20){
      yOffset = -20;
    }
  }
  GyroLastX = GyroX;

  var = GyroY - GyroLastY;
  //xOffset = 0;
  if(var > 2 || var < -2){ //deadzone
    if(var < 20 && var > -20){
      xOffset = var;
    }
    if(var > 20){
      xOffset = 20;
    }
    if(var < -20){
      xOffset = -20;
    }
  }
  GyroLastY = GyroY;

}

void mpu6050_Setup(){
  Wire1.begin();                      // Initialize comunication
  Wire1.beginTransmission(MPU6050);       // Start communication with MPU6050 // MPU=0x68
  Wire1.write(0x6B);                  // Talk to the register 6B
  Wire1.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire1.endTransmission(true);
}

void calculate_IMU_error() {
  while (c < 200) {
    Wire1.beginTransmission(MPU6050);
    Wire1.write(0x3B);
    Wire1.endTransmission(false);
    Wire1.requestFrom(MPU6050, 6, true);
    AccX = (Wire1.read() << 8 | Wire1.read()) / 16384.0 ;
    AccY = (Wire1.read() << 8 | Wire1.read()) / 16384.0 ;
    AccZ = (Wire1.read() << 8 | Wire1.read()) / 16384.0 ;
    //Serial.println(AccX);
    // Sum all readings
    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
    c++;
  }
  //Divide the sum by 200 to get the error value
  AccErrorX = AccErrorX / 200;
  AccErrorY = AccErrorY / 200;
  c = 0;
  // Read gyro values 200 times
  while (c < 200) {
    Wire1.beginTransmission(MPU6050);
    Wire1.write(0x43);
    Wire1.endTransmission(false);
    Wire1.requestFrom(MPU6050, 6, true);
    GyroX = Wire1.read() << 8 | Wire1.read();
    GyroY = Wire1.read() << 8 | Wire1.read();
    GyroZ = Wire1.read() << 8 | Wire1.read();
    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    c++;
  }
  //Divide the sum by 200 to get the error value
  GyroErrorX = GyroErrorX / 200;
  GyroErrorY = GyroErrorY / 200;
  GyroErrorZ = GyroErrorZ / 200;
  // Print the error values on the Serial Monitor
  /*Serial.print("AccErrorX: ");
  Serial.println(AccErrorX);
  Serial.print("AccErrorY: ");
  Serial.println(AccErrorY);
  Serial.print("GyroErrorX: ");
  Serial.println(GyroErrorX);
  Serial.print("GyroErrorY: ");
  Serial.println(GyroErrorY);
  Serial.print("GyroErrorZ: ");
  Serial.println(GyroErrorZ);*/
}
