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

#include <MySignals.h>
#include "Wire.h"
#include "SPI.h" 

extern volatile unsigned long timer0_overflow_count;
float fanalog0;
int analog0;
unsigned long time;

byte serialByte;

void setup() 
{
  Serial.begin(9600);
  MySignals.begin();
  Serial.println("Starting...");
}

void loop() 
{ 
  while (Serial.available()>0)
  {  
    serialByte=Serial.read();
    if (serialByte=='C')
    {        
      while(1){
        fanalog0=MySignals.getECG(DATA);  
        // Use the timer0 => 1 tick every 4 us
        time=(timer0_overflow_count << 8) + TCNT0;        
        // Microseconds conversion.
        time=(time*4);   
        //Print in a file for simulation
        Serial.print(time);
        Serial.print(";");
        Serial.println(fanalog0,5);

        if (Serial.available()>0)
        {
          serialByte=Serial.read();
          if (serialByte=='F')  break;
        }
      }
    }
  }
}


