#include <Adafruit_GFX_AS.h>
#include <Adafruit_ILI9341_AS.h>
#include <MySignals.h>

Adafruit_ILI9341_AS tft = Adafruit_ILI9341_AS(TFT_CS, TFT_DC);


// For temprature
float tempRead;
float prevTempRead;
int timeTemp = 0;
int typeTemp = 1;

// For SPO2 and Pulse
int valuePulse;
int valueSPO2;
int statusSPO2;
uint8_t pulsioximeter_state = 0;
int timeSPO2 = 0;
int typeSPO2 = 2;
int timePuls = 0;
int typePuls = 1;
st


void setup(void) 
{
  Serial.begin(115200);
  
  MySignals.begin();

  // Setup for SPO2 and Pulse
  MySignals.initSensorUART();
  MySignals.enableSensorUART(PULSIOXIMETER);

  // Setup something for TFT Screen
  tft.init();
  tft.setRotation(3);
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_BLUE); 
  tft.drawRightString("Temp(C):",53,5,2);
  tft.drawRightString("SPO2:",53,28,2);
  tft.drawRightString("PR(bPm):",53,48,2);
}

void loop() 
{
  TempSensor();
  SPO2Sensor();

  // Alert
  if (timeTemp == 5) {
    // Send data to sim808 and send sms to alert for user
    
    Serial.println("CANH BAO NHIET DO CO THE DANG O MUC " + String(typeTemp));
  }
}
void TempSensor() {
  // Temprature
  tempRead = (float)MySignals.getTemperature();
  // > 39: Sot cao ----------------------- 4
  // 38.3 - 40: Sot ---------------------- 3
  // 37.5 - 38.3: Tang than nhiet -------- 2
  // 36.5 - 37.5: Binh thuong ------------ 1
  // <35: Ha than nhiet ------------------ 0
  
  tft.fillRect(60,5,45,18,ILI9341_RED);
  tft.setTextColor(ILI9341_WHITE);
  tft.drawFloat(tempRead, 2, 63, 5, 2);

  // Prepare to Alert --- <!>
  if (tempRead > 40) {
    if (typeTemp == 4) {
      timeTemp++;
    } else {
      typeTemp = 4;
      timeTemp = 1;
    }
  } else if (tempRead > 38.3 && tempRead < 40) {
    if (typeTemp == 3) {
      timeTemp++;
    } else {
      typeTemp = 3;
      timeTemp = 1;
    }
  } else if (tempRead > 37.5 && tempRead < 38.3) {
    if (typeTemp == 2) {
      timeTemp++;
    } else {
      typeTemp = 2;
      timeTemp = 1;
    }
  } else if (tempRead > 36.5 && tempRead < 37.5) {
    typeTemp = 1;
    timeTemp = 0;
  } else if (tempRead > 30 && tempRead < 36.5) {
    if (typeTemp == 0) {
      timeTemp++;
    } else {
      typeTemp = 0;
      timeTemp = 1;
    }
  }
  tft.drawFloat(typeTemp, 0,110,5,2);
  
  delay(2000);
  SPI.end();  
}
void SPO2Sensor(){
  // == SPO2
  // 97-99: OK ----------------------- 3
  // 94-96: MEDIUM (Need more oxi ---- 2
  // 92-93: LOW: (Suy ho hap) -------- 1
  // 90-92: Suy ho hap nang ---------- 0
  // <90: NEED HELP
  // == PULSE
  // >150: NEED HELP ----------------- 3
  // 100-150: ALERT ------------------ 2
  // 60-100: OK ---------------------- 1
  // <60: Nhip tim thap -------------- 0
  
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
      // tft.drawNumber(MySignals.pulsioximeterData.BPM, 60, 28, 2);
      // tft.drawNumber(MySignals.pulsioximeterData.O2, 60, 48, 2);
      statusSPO2 = 1;
     
    }
    else if (getPulsioximeterMicro_state == 2)
    {
      //Serial.println(F("Finger out or calculating"));
      // tft.drawString("none", 63, 30, 2);
      // tft.drawString("none", 63, 45, 2);
      statusSPO2 = 2;
    }
    else
    {
      MySignals.spo2_micro_detected = 0;
      //Serial.println(F("SPO2 Micro lost connection"));
      // tft.drawString("disc", 63, 30, 2);
      // tft.drawString("disc", 63, 45, 2);
      statusSPO2 = 0;
    }
  }


  if (MySignals.spo2_mini_detected == 1)
  {
    MySignals.enableSensorUART(PULSIOXIMETER);

    uint8_t getPulsioximeterMini_state = MySignals.getPulsioximeterMini();

    if (getPulsioximeterMini_state == 1)
    {
      // tft.drawNumber(MySignals.pulsioximeterData.BPM, 63, 30, 2);
      // tft.drawNumber(MySignals.pulsioximeterData.O2, 63, 45, 2);
      statusSPO2 = 1;
    }
    else if (getPulsioximeterMini_state == 2)
    {
      // Finger out or Caculating
      // tft.drawString("none", 63, 30, 2);
      // tft.drawString("none", 63, 45, 2);
      statusSPO2 = 2;
    }
    else if (getPulsioximeterMini_state == 0)
    {
      // Disconnected
      MySignals.spo2_mini_detected = 0;
      // tft.drawString("disc", 63, 30, 2);
      // tft.drawString("disc", 63, 45, 2);
      statusSPO2 = 0;
    }
  }
}
