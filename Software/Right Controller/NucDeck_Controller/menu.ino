void drawInterfaceSlider(int xPos, int yPos, int length, int min, int max, int value, uint32_t colour){ //Used to draw sliders in RGB menu
  int height = 14;
  int valueAdjusted = map(value, min, max, 0, length);
  tft.drawRect(xPos, yPos, length, height, TFT_WHITE);
  tft.fillRect(xPos + 1,yPos + 1,length -2,height - 2,TFT_BLACK);
  tft.fillRect(xPos + 1,yPos + 1,valueAdjusted,height - 2,colour);
}

void rgbConfig(){ //RGB Menu
  if(interfaceTimer + interfaceDelay < millis()){
    tft.setTextSize(2);
    tft.drawString("RGB", 7 , 2, 2);
    if(menuSelected == 0){
      tft.setTextColor(TFT_RED, TFT_BLACK);
    }
    tft.setTextSize(1);
    tft.drawString(rgbMode[rgbModeSelect] + "    ", 8, 35, 2);
    
    //Speed
    if(menuSelected == 1){
      tft.setTextColor(TFT_RED, TFT_BLACK);
    } else {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString("Speed: ", 8, 51, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    drawInterfaceSlider(50, 51, 25, 100, 0, LED_Speed, TFT_WHITE);

    //Hue
    if(menuSelected == 2){
      tft.setTextColor(TFT_RED, TFT_BLACK);
    } else {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString("Hue: ", 8, 77, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString((String)ledHue, 55, 77, 2);

    //Sat
    if(menuSelected == 3){
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
    } else {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString("Sat: ", 8, 93, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString((String)ledSat, 55, 93, 2);

    //Brightness
    if(menuSelected == 4){
      tft.setTextColor(TFT_BLUE, TFT_BLACK);
    } else {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    tft.drawString("Bright: ", 8, 109, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString((String)ledBrightness, 55, 109, 2);
    

    if(lastButtonState[16] == 1){
        if(menuSelected > 0){
          menuSelected--;
        }
        delay(50);
    }
    if(lastButtonState[15] == 1){
      if(menuSelected < 4){
        menuSelected++;
      }
      delay(50);
    }
    if(lastButtonState[14] == 1){ //Dpad Left
      if(menuSelected == 0){
        if(rgbModeSelect == 0){ 
          rgbModeSelect = 1;
        } else {
          rgbModeSelect = 0;
        }
        delay(50);
      } else if(menuSelected == 1){
        if(LED_Speed < 100){
          LED_Speed++;
        }
      } else if(menuSelected == 2){
        if(ledHue > 0){
          ledHue--;
        }
      } else if(menuSelected == 3){
        if(ledSat > 0){
          ledSat--;
        }
      } else if(menuSelected == 4){
        if(ledBrightness > 0){
          ledBrightness--;
        }
      }
      
    }
    if(lastButtonState[13] == 1){ //Dpad Right
      if(menuSelected == 0){
        if(rgbModeSelect == 0){ 
          rgbModeSelect = 1;
        } else {
          rgbModeSelect = 0;
        }
        delay(50);
      } else if(menuSelected == 1){
        if(LED_Speed > 0){
          LED_Speed--;
        }
      } else if(menuSelected == 2){
        if(ledHue < 255){
          ledHue++;
        }
      } else if(menuSelected == 3){
        if(ledSat < 255){
          ledSat++;
        }
      } else if(menuSelected == 4){
        if(ledBrightness < 255){
          ledBrightness++;
        }
      }
      
    }
    if(lastButtonState[4] == 1){ //Exit
        tft.fillScreen(TFT_BLACK);
        menuSelected = 0;
        rgbMenu = false;
        delay(150);
    }

    interfaceTimer = millis();
  }   
}

void batteryInfo(){ //Battery Info Menu
  String temp = (String)(getInstantaneousVoltage() * 4);
  tft.setTextSize(2);
  tft.drawString("Stats", 1 , 2, 2);
  tft.setTextSize(1);

  tft.drawString("Volts: ", 1 , 35, 2);
  tft.drawString(temp, 37, 35, 2);

  int tempSOC = getSOC();
  temp = (String)tempSOC;
  tft.drawString("SOC: ", 1 , 53, 2);
  tft.drawString(temp, 33, 53, 2);

  float tempCurrent = getAverageCurrent();
  if(tempCurrent > 0){
    temp = "+" + (String)tempCurrent;
  } else {
    temp = (String)tempCurrent;
  }
  
  tft.drawString("Amps: ", 1 , 71, 2);
  tft.drawString(temp, 38, 71, 2);

  if(charging){
    tft.drawString("Charging   ", 1 , 89, 2);
  } else if(onAC){
    tft.drawString("AC         ", 1 , 89, 2);
  } else {
    tft.drawString("Discharging", 1 , 89, 2);
  }

  if(lastButtonState[4] == 1){ //Exit
      tft.fillScreen(TFT_BLACK);
      menuSelected = 0;
      batteryMenu = false;
      delay(150);
  }
}

void startupScreen(){
  tft.fillScreen(TFT_BLACK);
  tft.drawXBitmap(0, 0, splashIMG, splashWidth, splashHeight, TFT_WHITE);
}

void menuMode(){ //Main Menu
  if(interfaceTimer + interfaceDelay < millis()){
    bool closeMenu = false;
    if(!menuOpen){
      delay(150);
      tft.fillScreen(TFT_BLACK);
      menuOpen = true;
    } else {
      if(lastButtonState[4] == 1){
        closeMenu = true;
      }
    }
    tft.setTextSize(2);
    tft.drawString("Menu", 7 , 2, 2);
  
    if(lastButtonState[16] == 1){
      if(menuSelected > 0){
        menuSelected--;
      }
    }
    if(lastButtonState[15] == 1){
      if(menuSelected < menuOptions){
        menuSelected++;
      }
    }
    if(lastButtonState[0] == 1){ //Select Option
      if(menuSelected == 0){//Gyro Aim
        if(aimAssist){
          aimAssist = false;
        } else {
          aimAssist = true;
        }
        closeMenu = true;
      } else if(menuSelected == 1){//Calibration
        tft.fillScreen(TFT_BLACK);
        closeMenu = true;
        menuSelected = 0;
        calibrationMode = true;
      } else if(menuSelected == 3){//RGB Config
        tft.fillScreen(TFT_BLACK);
        closeMenu = true;
        menuSelected = 0;
        rgbMenu = true;
      } else if(menuSelected == 2){ //Battery Info
        tft.fillScreen(TFT_BLACK);
        closeMenu = true;
        menuSelected = 0;
        batteryMenu = true;
      } else if(menuSelected == 5){ // Force Close
        tft.fillScreen(TFT_BLACK);
        closeMenu = true;
        menuSelected = 0;
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_F4);
        delay(50);
        Keyboard.releaseAll();
      } else if(menuSelected == 6){ // Shutdown Test
        tft.fillScreen(TFT_BLACK);
        closeMenu = true;
        menuSelected = 0;
        Keyboard.press(KEY_ESC);
        delay(50);
        Keyboard.releaseAll();
      }
    }
    tft.setTextSize(1);
    int startY = 35;
    int startX = 8;
    for(int i = 0; i < menuOptions; i++){
      if(i == menuSelected){
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString(mainMenu[i], startX, startY, 2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
      } else {
        tft.drawString(mainMenu[i], startX, startY, 2);
      }
      
      startY += 16;
    }
  
    if(closeMenu){
      menuOpen = false;
      tft.fillScreen(TFT_BLACK);
      lastMode = 2;
      closeMenu = false;
      menuSelected = 0;
      lastBatteryDisplay = 0;
    }
    interfaceTimer = millis();
  }
}

void mainScreen(){ //Main Screen Display
  int xStart;
  if(lastBatteryDisplay != batteryPercent | lastChargingDisplay != charging){
    if(lastBatteryDisplay > batteryPercent | lastChargingDisplay != charging){
      tft.fillScreen(TFT_BLACK);
      lastMode = 2;
    }
    int startX = 7;
    int startY = 0;
    tft.drawRect(startX, startY, (tft.width() - 20), 30, TFT_WHITE);
    tft.fillRect(startX + 60,startY + 10,6,10,TFT_WHITE);
    int progressBar = map(batteryPercent, 0, 100, 0,(tft.width() - 22));
    if(batteryPercent < 15){
      tft.fillRect(startX + 1,startY + 1,progressBar,28,TFT_RED);
    } else if(batteryPercent < 35){
      tft.fillRect(startX + 1,startY + 1,progressBar,28,TFT_YELLOW);
    } else {
      tft.fillRect(startX + 1,startY + 1,progressBar,28,TFT_GREEN);
    }
    
    lastBatteryDisplay = batteryPercent;
    String percent = ((String)batteryPercent + "%"); //"100%";
    if(batteryPercent < 10){
      xStart = 30;
    } else if(batteryPercent == 100){
       xStart = 20;
    } else {
      xStart = 25;
    }
    tft.drawString(percent, xStart, 7, 2);
  }
  //Display Battery State
  if(onAC){
    tft.drawString("On A/C", 13, 35, 2);
  } else if(charging){
    tft.drawString("Charging", 12, 35, 2);
  } else {
    tft.drawString("Discharging", 8, 35, 2);
    tft.setTextSize(2);
    tft.drawString(TTE, 14, 51, 2);
    tft.setTextSize(1);
  }
  lastChargingDisplay = charging;

  //Display Aim Assist State
  /*tft.drawString("Aim", 30, 60, 2);         //Uncomment this section to display aim assist state. Not yet implemented.
  if(aimAssist){
    tft.drawString("Assist: ON", 10, 75, 2);
  } else {
    tft.drawString("Assist: OFF", 10, 75, 2);
  }*/
}

void joystickCalibration(){ //Joystick Calibration Sequence
  if(calibrationStep == 1){
    if(!calibrationDisplayUpdate){
      delay(150);
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(1);
      tft.drawString("Center", 20 , 12, 2);
      tft.drawString("Sticks", 20 , 26, 2);
      tft.drawString("Press A to", 6 , 76, 2);
      tft.drawString("Continue", 13, 92, 2);
      calibrationDisplayUpdate = true;
    }
    buttonRead();
    if(lastButtonState[0] == 1){
      midLeftX = i2c_Request(controllerAddr, xAxis);
      midLeftY = i2c_Request(controllerAddr, yAxis);
      midRightX = analogRead(27);
      midRightY = analogRead(26);
      delay(50);
      minLeftX = midLeftX;
      minLeftY = midLeftY;
      maxLeftX = 0;
      maxLeftY = 0;
      minRightX = midRightX;
      minRightY = midRightY;
      maxRightX = 0;
      maxRightY = 0;
      calibrationStep = 2;
      calibrationDisplayUpdate = false;
      //Serial.write(calibrationStepTwo);
      delay(150);
    }
  } else if(calibrationStep == 2){
    if(!calibrationDisplayUpdate){
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(1);
      tft.drawString("Move Sticks", 1 , 12, 2);
      tft.drawString("to Extents", 3 , 26, 2);
      tft.drawString("Press A to", 6 , 76, 2);
      tft.drawString("Complete", 13, 92, 2);
      calibrationDisplayUpdate = true;
    }
    int var = i2c_Request(controllerAddr, xAxis);
    if(var != 0){
      if(var > maxLeftX) maxLeftX = var;
      if(var < minLeftX) minLeftX = var;
    }
    var = i2c_Request(controllerAddr, yAxis);
    if(var != 0){
      if(var > maxLeftY) maxLeftY = var;
      if(var < minLeftY) minLeftY = var;
    }
    var = analogRead(27);
    if(var != 0){
      if(var > maxRightX) maxRightX = var;
      if(var < minRightX) minRightX = var;
    }
    var = analogRead(26);
    if(var != 0){
      if(var > maxRightY) maxRightY = var;
      if(var < minRightY) minRightY = var;
    }
    buttonRead();
    if(lastButtonState[0] == 1){
      calibrationStep = 3;
      calibrationDisplayUpdate = false;
      delay(150);
    }
  } else if(calibrationStep == 3){
    if(!calibrationDisplayUpdate){
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(1);
      tft.drawString("Move L2 & R2", 1 , 7, 2);
      tft.drawString("to Extents", 3 , 26, 2);
      tft.drawString("Press A to", 6 , 76, 2);
      tft.drawString("Complete", 13, 92, 2);
      calibrationDisplayUpdate = true;
      minL2 = 1023;
      maxL2 = 0;
      minR2 = 1023;
      maxR2 = 0;
    }
    int var = i2c_Request(controllerAddr, zAxis); //L2
    if(var != 0){
      if(var > maxL2) maxL2 = var;
      if(var < minL2) minL2 = var;
    }
    var = analogRead(28); //R2TestTestTestTest
    if(var != 0){
      if(var > maxR2) maxR2 = var;
      if(var < minR2) minR2 = var;
    }
    if(lastButtonState[0] == 1){ //Complete Calibration
      delay(150);
      writeCalibrationData(); //Update EEPROM
      rebuildLUTs();
      delay(50);
      tft.fillScreen(TFT_BLACK);
      calibrationStep = 1;
      calibrationMode = false;
      calibrationDisplayUpdate = false;
    }
  }
}