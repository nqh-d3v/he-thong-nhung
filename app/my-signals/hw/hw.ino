#include <Adafruit_GFX_AS.h>
#include <Adafruit_ILI9341_AS.h>
#include <MySignals.h>
#include <Wire.h>

Adafruit_ILI9341_AS tft = Adafruit_ILI9341_AS(TFT_CS, TFT_DC);


// For temprature
float tempRead;
int timeTemp = 0;
int typeTemp = 1;

// For SPO2 and Pulse
int valuePulse;
int valueSPO2;
int statusSPO2;
uint8_t pulsioximeter_state = 0;
int timeSPO2 = 0;
int typeSPO2 = 2;
int timePulse = 0;
int typePulse = 1;



void setup(void) 
{
  Serial.begin(115200);
  
  MySignals.begin();

  // Setup for SPO2 and Pulse
  MySignals.initSensorUART();
  MySignals.enableSensorUART(PULSIOXIMETER);

  // Setup something for TFT Screen
  tft.init();
  tft.setRotation(1);
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_BLUE); 
  tft.drawRightString("Temp(C):",53,5,2);
  tft.drawRightString("SPO2:",53,28,2);
  tft.drawRightString("PR(bPm):",53,48,2);
}

void loop() 
{
  tempRead = (float)MySignals.getTemperature();
   
  //Serial.println(tempRead);
  //delay(2000);
  TempSensor();
  SPO2Sensor();

  // Alert
  if (timeTemp == 10 && typeTemp > 1) {
    timeTemp = 0;
    Serial.println("Temperature in level " + String(typeTemp));
    Serial.println("Sent temp");
    Wire.beginTransmission( 9 );
    Wire.write(typeTemp);
    Wire.endTransmission();
  }
  // 1 - 6 <=> [1 - 6] : Temp
  // 7 - 11 <=> [1 - 5] : SPO2
  // 12 - 15 <=> [1 - 4] : Pulse
  if (timeSPO2 == 10 && typeSPO2 > 0) {
    timeSPO2 = 0;
    Serial.println("SPO2 in level " + String(typeSPO2 - 6));
    Wire.beginTransmission( 9 );
    Wire.write(typeSPO2);
    Wire.endTransmission();
  }
  if (timePulse == 10 && typeTemp > 0) {
    timePulse = 0;
    Serial.println("PRbPm in level " + String(typePulse - 11));
    Wire.beginTransmission( 9 );
    Wire.write(typePulse);
    Wire.endTransmission();
  }
}
void TempSensor() {
  // Temprature
  tempRead = (float)MySignals.getTemperature();
  // > 39: Sot cao ----------------------- 6
  // 38.3 - 40: Sot ---------------------- 5
  // 37.5 - 38.3: Tang than nhiet -------- 4
  // 36.5 - 37.5: Binh thuong ------------ 3 <=
  // <35: Ha than nhiet ------------------ 2
  // <30: Khong dc su dung --------------- 1
  
  tft.fillRect(60,5,45,18,ILI9341_RED);
  tft.setTextColor(ILI9341_WHITE);
  tft.drawFloat(tempRead, 2, 63, 5, 2);

  // Prepare to Alert --- <!>
  if (tempRead > 40) {
    if (typeTemp == 6) {
      timeTemp++;
    } else {
      typeTemp = 6;
      timeTemp = 1;
    }
  } else if (tempRead > 38.3 && tempRead < 40) {
    if (typeTemp == 5) {
      timeTemp++;
    } else {
      typeTemp = 5;
      timeTemp = 1;
    }
  } else if (tempRead > 37.5 && tempRead < 38.3) {
    if (typeTemp == 4) {
      timeTemp++;
    } else {
      typeTemp = 4;
      timeTemp = 1;
    }
  } else if (tempRead > 36.5 && tempRead < 37.5) {
    typeTemp = 3;
    timeTemp = 0;
  } else if (tempRead > 22.5 && tempRead < 36.5) {
    if (typeTemp == 2) {
      timeTemp++;
    } else {
      typeTemp = 2;
      timeTemp = 1;
    }
  } else {
    typeTemp = 1;
    timeTemp = 0;
    tft.fillRect(148,3,10,20,ILI9341_RED);
  }

  
  Serial.println(typeTemp);
  tft.fillRect(108,3,20,20,ILI9341_BLUE);
  tft.drawFloat(typeTemp, 0,110,5,2);
  
  delay(2000);
  SPI.end();  
}
void SPO2Sensor(){
  
  tft.fillRect(60,28,45,18,ILI9341_RED);
  tft.fillRect(60,48,45,18,ILI9341_RED);
  if (MySignals.spo2_micro_detected == 0 && MySignals.spo2_mini_detected == 0)
  {
      uint8_t statusPulsioximeter = MySignals.getStatusPulsioximeterGeneral();
  
      if (statusPulsioximeter == 1)
      {
          MySignals.spo2_mini_detected = 0;
          MySignals.spo2_micro_detected = 1;
          
          // Micro Detected
          tft.drawString("Micro Detected",110,28,2);
      }
      else if (statusPulsioximeter == 2)
      {
          MySignals.spo2_mini_detected = 1;
          MySignals.spo2_micro_detected = 0;
    
          tft.drawString("Mini Detected",110,48,2);
      }
      else
      {
          MySignals.spo2_micro_detected = 0;
          MySignals.spo2_mini_detected = 0;
      }
  }


  if (MySignals.spo2_micro_detected == 1)
  {
    MySignals.enableSensorUART(PULSIOXIMETER_MICRO);
    delay(10);
    uint8_t getPulsioximeterMicro_state = MySignals.getPulsioximeterMicro();

    if (getPulsioximeterMicro_state == 1)
    {
      valuePulse = MySignals.pulsioximeterData.BPM;
      valueSPO2 = MySignals.pulsioximeterData.O2;
      tft.drawNumber(valuePulse, 60, 28, 2);
      tft.drawNumber(valueSPO2, 60, 48, 2);
      statusSPO2 = 1;
     
    }
    else if (getPulsioximeterMicro_state == 2)
    {
      //Serial.println(F("Finger out or calculating"));
      tft.drawString("none", 63, 30, 2);
      tft.drawString("none", 63, 45, 2);
      statusSPO2 = 2;
    }
    else
    {
      MySignals.spo2_micro_detected = 0;
      //Serial.println(F("SPO2 Micro lost connection"));
      tft.drawString("disc", 63, 30, 2);
      tft.drawString("disc", 63, 45, 2);
      statusSPO2 = 0;
    }
  }


  if (MySignals.spo2_mini_detected == 1)
  {
    MySignals.enableSensorUART(PULSIOXIMETER);

    uint8_t getPulsioximeterMini_state = MySignals.getPulsioximeterMini();

    if (getPulsioximeterMini_state == 1)
    {
      valuePulse = MySignals.pulsioximeterData.BPM;
      valueSPO2 = MySignals.pulsioximeterData.O2;
      tft.drawNumber(valuePulse, 63, 30, 2);
      tft.drawNumber(valueSPO2, 63, 45, 2);
      statusSPO2 = 1;
    }
    else if (getPulsioximeterMini_state == 2)
    {
      // Finger out or Caculating
      tft.drawString("none", 63, 30, 2);
      tft.drawString("none", 63, 45, 2);
      statusSPO2 = 2;
    }
    else if (getPulsioximeterMini_state == 0)
    {
      // Disconnected
      MySignals.spo2_mini_detected = 0;
      tft.drawString("disc", 63, 30, 2);
      tft.drawString("disc", 63, 45, 2);
      statusSPO2 = 0;
    }
  }

  // ----- PREPARE TO ALERT ------
  // == SPO2 - value
  // 97-99: OK ----------------------- 5 - 11
  // 94-96: MEDIUM (Need more oxi ---- 4 - 10
  // 92-93: LOW: (Suy ho hap) -------- 3 - 9
  // 90-92: Suy ho hap nang ---------- 2 - 8
  // <90: NEED HELP ------------------ 1 - 7
  if (valueSPO2 < 90) {
    if (typeSPO2 == 7) {
      timeSPO2++;
    } else {
      typeSPO2 = 7;
      timeSPO2 = 1;
    }
  } else if (valueSPO2 < 92) {
    if (typeSPO2 == 8) {
      timeSPO2++;
    } else {
      typeSPO2 = 8;
      timeSPO2 = 1;
    }
  } else if (valueSPO2 < 93) {
    if (typeSPO2 == 9) {
      timeSPO2++;
    } else {
      typeSPO2 = 9;
      timeSPO2 = 1;
    }
  } else if (valueSPO2 < 96) {
    if (typeSPO2 == 10) {
      timeSPO2++;
    } else {
      typeSPO2 = 10;
      timeSPO2 = 1;
    }
  } else if (valueSPO2 <= 100) {
      typeSPO2 = 11;
      timeSPO2 = 0;
  } else {
    valueSPO2 = 100;
  }
  // == PULSE
  // >150: NEED HELP ----------------- 4 - 15
  // 100-150: ALERT ------------------ 3 - 14
  // 60-100: OK ---------------------- 2 - 13
  // <60: Nhip tim thap -------------- 1 - 12
  if (valuePulse < 60) {
    if (typePulse == 12) {
      timePulse++;
    } else {
      typePulse = 12;
      timePulse = 1;
    }
  } else if (valuePulse < 100) {
      typePulse = 13;
      timePulse = 0;
  } else if (valuePulse < 150) {
    if (typePulse == 14) {
      timePulse++;
    } else {
      typePulse = 14;
      timePulse = 1;
    }
  } else {
    if (typePulse == 15) {
      timePulse++;
    } else {
      typePulse = 15;
      timePulse = 1;
    }
  }
}
