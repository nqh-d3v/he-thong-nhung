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

    Version:           0.1
    Design:            David Gascon
    Implementation:    Luis Martin / Victor Boria
*/

#include <Adafruit_GFX_AS.h>
#include <Adafruit_ILI9341_AS.h>
#include <MySignals.h>

#define graphic_low_extrem 230
#define graphic_high_extrem 30
#define graphic_left_extrem 0
#define graphic_right_extrem 320


Adafruit_ILI9341_AS tft = Adafruit_ILI9341_AS(TFT_CS, TFT_DC);


//! It stores the current value of x position in the LCD.
uint16_t graphic_x;
//! It stores the current value of the MySignals. 
uint16_t valRead;
//! It stores the previous value of the MySignals. 
uint16_t graphic_prevRead;


void setup(void) 
{
  Serial.begin(115200);
  
  MySignals.begin();
  MySignals.initBodyPosition();
   
  tft.init();   
  tft.setRotation(2);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
 

}

void loop() 
{
  tft.drawString("Body position:", 0, 0, 2);
  uint8_t position = MySignals.getBodyPosition(); 
  
   if (position == 1)
  {
    tft.drawString("Prone position", 0, 30, 2);
  }
  else if (position == 2)
  {
    tft.drawString("Left lateral   ", 0, 30, 2);
  }
  else if (position == 3)
  {
    tft.drawString("Rigth lateral  ", 0, 30, 2);
  }
  else if (position == 4)
  {
    tft.drawString("Supine position ", 0, 30, 2);
  }
  else if (position == 5)
  {
   tft.drawString("Stand or sit     ", 0, 30, 2);
  }
  else
  {
   tft.drawString("non-defined       ", 0, 30, 2);
  }
  
   
  SPI.end();
}



  
