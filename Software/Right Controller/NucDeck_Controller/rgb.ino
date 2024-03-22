void rgbHalo(){ //LED Halo effect for joystick surrounds
  if(LED_Timer + (LED_Speed * 100) < millis()){
    ledSequenceNo++;
    if(ledSequenceNo > 2){
      ledSequenceNo = 0;
    }
    LED_Timer = millis();
  }
  if(LED_Fadein + ((LED_Speed * 100) / ledBrightness) < millis()){//Fade In
    if(LED_Values[ledSequenceNo] < ledBrightness){
      LED_Values[ledSequenceNo]++;
      LED_Values[ledSequenceNo + 3]++;
    }
    LED_Fadein = millis();
  }
  if(LED_Fadeout + LED_OutSpeed < millis()){//Fade Out
    for(int i = 0; i < 3; i++){
      if(ledSequenceNo != i){
        if(LED_Values[i] > 0){
          LED_Values[i] -= 2;
          LED_Values[i + 3] -= 2;
        }
      }
      //CRGB newcolor = CHSV(ledHue, ledSat, LED_Values[i]);
      leds[i] = CHSV(ledHue, ledSat, LED_Values[i]);//CRGB(LED_Values[i][0],LED_Values[i][1],LED_Values[i][2]);
      leds[i + 3] = CHSV(ledHue, ledSat, LED_Values[i]);
    }
    LED_Fadeout = millis();
  }
  FastLED.show();
}

void rgbBreath(){ //Breathing effect for joystick surrounds
  if(LED_Timer + LED_Speed < millis()){
    if(ledSequenceNo == 1){ //Increase
      if(currentBrightness < ledBrightness){
        currentBrightness++;
      } else {
        ledSequenceNo = 0;
      }
    } else { //Decrease
      if(currentBrightness > 0){
        currentBrightness--;
      } else {
        ledSequenceNo = 1;
      }
    }
    
    CRGB newcolor = CHSV(ledHue, ledSat, currentBrightness);
    for(int i = 0; i < 6; i++){
      leds[i] = newcolor;
    }
    FastLED.show();
    LED_Timer = millis();
  }
}