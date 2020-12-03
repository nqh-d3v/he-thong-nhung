/*

    Copyright (C) 2017 Libelium Comunicaciones Distribuidas S.L.
   http://www.libelium.com

    By using it you accept the MySignals Terms and Conditions.
    You can find them at: http://libelium.com/legal

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Version:           2.0
    Design:            David Gascon
    Implementation:    Luis Martin / Victor Boria
*/

#include <Adafruit_GFX_AS.h>
#include <Adafruit_ILI9341_AS.h>
#include <MySignals.h>
#include <MySignals_BLE.h>


// Write here the MAC address of BLE device to find
char MAC_SCALE[14] = "8CDE5260BCEF";

//Scale User profile
uint8_t user_gender = 1;   //1-Male    0-Female
uint8_t user_height = 175; //in cm (0-255)
uint8_t user_age = 29;     //(0-255)



uint8_t available_scale = 0;
uint8_t connected_scale = 0;
uint8_t connection_handle_scale = 0;


#define SCALE_HANDLE 83
#define SCALE_PROFILE_HANDLE 85

char buffer_tft[30];

Adafruit_ILI9341_AS tft = Adafruit_ILI9341_AS(TFT_CS, TFT_DC);

//!Struct to store data of the glucometer.
struct scaleDataVector
{
  uint16_t weight;
  uint16_t bodyfat;
  uint16_t musclemass;
  uint16_t water;
  uint16_t calories;
  uint8_t visceralfat;
  uint8_t bonemass;
};

//!Vector to store the glucometer measures and dates.
scaleDataVector scaleData;

void setup()
{

  MySignals.begin();

  tft.init();
  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

  //TFT message: Welcome to MySignals
  strcpy_P((char*)buffer_tft, (char*)pgm_read_word(&(table_MISC[0])));
  tft.drawString(buffer_tft, 0, 0, 2);

  Serial.begin(115200);

  MySignals.initSensorUART();
  MySignals.enableSensorUART(BLE);

   //Enable BLE module power -> bit6: 1
  bitSet(MySignals.expanderState, EXP_BLE_POWER);
  MySignals.expanderWrite(MySignals.expanderState);

  //Enable BLE UART flow control -> bit5: 0
  bitClear(MySignals.expanderState, EXP_BLE_FLOW_CONTROL);
  MySignals.expanderWrite(MySignals.expanderState);


  //Disable BLE module power -> bit6: 0
  bitClear(MySignals.expanderState, EXP_BLE_POWER);
  MySignals.expanderWrite(MySignals.expanderState);

  delay(500);

  //Enable BLE module power -> bit6: 1
  bitSet(MySignals.expanderState, EXP_BLE_POWER);
  MySignals.expanderWrite(MySignals.expanderState);
  delay(1000);
  
  MySignals_BLE.initialize_BLE_values();


  if (MySignals_BLE.initModule() == 1)
  {

    if (MySignals_BLE.sayHello() == 1)
    {
      //TFT message: "BLE init ok";
      strcpy_P((char*)buffer_tft, (char*)pgm_read_word(&(table_MISC[1])));
      tft.drawString(buffer_tft, 0, 15, 2);
    }
    else
    {
      //TFT message:"BLE init fail"
      strcpy_P((char*)buffer_tft, (char*)pgm_read_word(&(table_MISC[2])));
      tft.drawString(buffer_tft, 0, 15, 2);


      while (1)
      {
      };
    }
  }
  else
  {
    //TFT message: "BLE init fail"
    strcpy_P((char*)buffer_tft, (char*)pgm_read_word(&(table_MISC[2])));
    tft.drawString(buffer_tft, 0, 15, 2);

    while (1)
    {
    };
  }


}

void loop()
{
  available_scale = MySignals_BLE.scanDevice(MAC_SCALE, 1000, TX_POWER_MAX);

  tft.drawString("Scale available:", 0, 30, 2);
  tft.drawNumber(available_scale, 110, 30, 2);



  if (available_scale == 1)
  {

    if (MySignals_BLE.connectDirect(MAC_SCALE) == 1)
    {
      tft.drawString("Connected    ", 0, 45, 2);
      connected_scale = 1;
      delay(500);

      uint8_t attributeData[2] =
      {
        0x01 , 0x00
      };
      if (MySignals_BLE.attributeWrite(MySignals_BLE.connection_handle, SCALE_HANDLE, attributeData, 2) == 0)
      {

        tft.drawString("Subscribed", 0, 60, 2);

        delay(4000);

        uint8_t scale_user_profile[8] =
        {
          0xfe,  // 0  BYTE 1
          0x00,  // 1  BYTE 2 -> user group
          0x00,  // 2  BYTE 3 -> gender
          0x00,  // 3  BYTE 4 -> level
          0x00,  // 4  BYTE 5 -> height
          0x00,  // 5  BYTE 6 -> age
          0x00,  // 6  BYTE 7 -> unit
          0x00   // 7  BYTE 8 -> xor
        };

        scale_user_profile[1] = 1;              // User group
        scale_user_profile[2] = user_gender;    // gender: 1=male, 0=female
        scale_user_profile[3] = 0;              // level 0=normal
        scale_user_profile[4] = user_height;    // height
        scale_user_profile[5] = user_age;       // age
        scale_user_profile[6] = 1;              // unit KG


        uint8_t xor_result = scale_user_profile[1] xor scale_user_profile[2];
        xor_result = xor_result xor scale_user_profile[3];
        xor_result = xor_result xor scale_user_profile[4];
        xor_result = xor_result xor scale_user_profile[5];
        scale_user_profile[7] = xor_result xor scale_user_profile[6];

        if (MySignals_BLE.attributeWrite(MySignals_BLE.connection_handle, SCALE_PROFILE_HANDLE, scale_user_profile, 8) == 0)
        {
          delay(500);



          unsigned long previous = millis();
          do
          {
            if (MySignals_BLE.waitEvent(1000) == BLE_EVENT_ATTCLIENT_ATTRIBUTE_VALUE)
            {

              uint8_t scale_weight_high = MySignals_BLE.event[13];
              uint8_t scale_weight_low = MySignals_BLE.event[14];

              uint8_t scale_bodyfat_high = MySignals_BLE.event[15];
              uint8_t scale_bodyfat_low = MySignals_BLE.event[16];



              uint8_t scale_musclemass_high = MySignals_BLE.event[18];
              uint8_t scale_musclemass_low = MySignals_BLE.event[19];

              scaleData.visceralfat = MySignals_BLE.event[20];

              uint8_t scale_water_high = MySignals_BLE.event[21];
              uint8_t scale_water_low = MySignals_BLE.event[22];

              uint8_t scale_calories_high = MySignals_BLE.event[23];
              uint8_t scale_calories_low = MySignals_BLE.event[24];


              scaleData.weight = (scale_weight_high * 256) + scale_weight_low;
              scaleData.bodyfat = (scale_bodyfat_high * 256) + scale_bodyfat_low;
              scaleData.musclemass = (scale_musclemass_high * 256) + scale_musclemass_low;
              scaleData.water = (scale_water_high * 256) + scale_water_low;
              scaleData.calories = (scale_calories_high * 256) + scale_calories_low;

              scaleData.bonemass = MySignals_BLE.event[17] * 1000 / scaleData.weight;


              tft.drawString("Weight:", 0, 75, 2);
              tft.drawFloat((scaleData.weight / 10.0), 1, 100, 75, 2);

              tft.drawString("Body fat:", 0, 90, 2);
              tft.drawFloat((scaleData.bodyfat / 10.0), 1, 100, 90, 2);

              tft.drawString("Muscle mass:", 0, 105, 2);
              tft.drawFloat((scaleData.musclemass / 10.0), 1, 100, 105, 2);

              tft.drawString("Visceral fat:", 0, 120, 2);
              tft.drawNumber(scaleData.visceralfat, 100, 120, 2);

              tft.drawString("Water:", 0, 135, 2);
              tft.drawFloat((scaleData.water / 10.0), 1, 100, 135, 2);

              tft.drawString("Calories:", 0, 150, 2);
              tft.drawNumber(scaleData.calories, 100, 150, 2);

              tft.drawString("Bone mass:", 0, 165, 2);
              tft.drawFloat((scaleData.bonemass / 10.0), 1, 100, 165, 2);

              delay(1000);

              uint8_t shutdown_command[8] =
              {
                0xfd, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x35
              };

              MySignals_BLE.attributeWrite(MySignals_BLE.connection_handle, SCALE_PROFILE_HANDLE, shutdown_command, 8);

              MySignals_BLE.disconnect(MySignals_BLE.connection_handle);

              delay(200);

              connected_scale = 0;
            }
          }
          while ((connected_scale == 1) && ((millis() - previous) < 20000));

          connected_scale = 0;

        }
        else
        {
          tft.drawString("Error subscribing", 0, 60, 2);
        }
      }
      else
      {
        tft.drawString("Not Connected", 0, 45, 2);
      }
    }
    else
    {
      connected_scale = 0;
      tft.drawString("Not Connected", 0, 45, 2);
    }
  }
  else if (available_scale == 0)
  {
    //Do nothing
  }
  else
  {
    MySignals_BLE.hardwareReset();
    MySignals_BLE.initialize_BLE_values();
    delay(100);

  }
  delay(1000);
}


