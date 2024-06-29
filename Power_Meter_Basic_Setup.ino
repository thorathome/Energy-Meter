#define SKETCH_NAME "Power_Meter_Basic_Startup"
#define SERIAL_SPEED 230400

/* ATM90E32 Energy Monitor Demo Application

   The MIT License (MIT)

  Copyright (c) 2016 whatnick and Ryzee

  
  Modified for simplicity as a basic get-started sketch by thorathome

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


// Based on this:   https://github.com/CircuitSetup/Expandable-6-Channel-ESP32-Energy-Meter
//                  https://github.com/CircuitSetup/Expandable-6-Channel-ESP32-Energy-Meter/blob/master/Software/EmonESP/src/energy_meter.cpp
//                  https://github.com/CircuitSetup/Expandable-6-Channel-ESP32-Energy-Meter/blob/master/Software/EmonESP/src/energy_meter.h
// Info on ATM90E32 lib
//                  https://github.com/CircuitSetup/ATM90E32
// Calibration info from
//                  https://esphome.io/components/sensor/atm90e32.html
// Read info here on US 220 VAC power monitoring
//                  https://esphome.io/components/sensor/atm90e32.html#voltage


#include <SPI.h>
#include <ATM90E32.h> // Make sure you have the ATM90E32.h library from CircuitSetup's library, above



// The Expandable 6-Channel Energy Monitor has 6, count 'em 6 channels
#define NUM_CHANNELS 6



/***** CALIBRATION SETTINGS *****/

/* Line Frequency
 * 4485 for 60 Hz (North America)
 * 389 for 50 hz (rest of the world)
 */
//unsigned short lineFrequency = 4485;         
unsigned short lineFrequency = 4321;       // As per energy_meter.h


/*  Current Gain for the CT
 *  0 for 1x (CTs up to 60mA/720mV)        // This sketch sets pgaGain to Zero, adjusts currentGain per each sensor/clamp
 * 21 for 2x (CTs up to 30mA/360mV)
 * 42 for 4x (CTs up to 15mA/180mV)
 */        
unsigned short pgaGain = 0;                // As per energy_meter.h


/* Voltage Gain for the Board
 * Here are common voltage calibrations for the Expandable 6 Channel Energy Meter:
 * For meter <= v1.2:
 * 42080 - 9v AC Transformer - Jameco 112336
 * 32428 - 12v AC Transformer - Jameco 167151

 * For meter > v1.3:
 * 7305 - 9v AC Transformer - Jameco 157041
 */
 // Set for the entire board
unsigned short voltageGain = 7305;         // 9v AC Transformer - Jameco 157041 



 /* Current Gain for each CT clamp
  * Here are common current calibrations for the Expandable 6 Channel Energy Meter when gain_pga is set to 1X:
 *   (from https://esphome.io/components/sensor/atm90e32.html)
 *   20A/25mA SCT-006: 11131
 *   30A/1V SCT-013-030: 8650
 *   50A/1V SCT-013-050: 15420
 *   80A/26.6mA SCT-010: 41996 (note this will saturate at 2^16/10^3 amps)
 *   100A/50ma SCT-013-000: 27961
 *   120A/40mA: SCT-016: 41880
 */
//                                            80A    80A    80A    100A   100A   100A = my setup
unsigned short currentGain[NUM_CHANNELS] = { 41996, 41996, 41996, 27961, 27961, 27961 };
// These values can be tweaked for each amp clamp sensor


#if defined ESP32
  #warning "Yes, we found the ESP32"
  // Chip Select Pins on the ESP32 for the two ATM90E32 chips
  /*
  ESP32 Pins
    CLK - 18
    MISO - 19
    MOSI - 23
    CS1 - 5 (CT1-CT3 & Voltage 1)
    CS2 - 4 (CT4-CT6 & Voltage 2)
  */
  const int cs1pin = 5;  // CT 1-3 These are fixed for the main board. 
  const int cs2pin = 4;  // CT 4-6

#else
  #warning "No idea what this board is"
#endif



// Initialize the two ATM90E32 ICs on the 6-Channel Board
ATM90E32 sensorIC1{};
ATM90E32 sensorIC2{};



// Global variables to be read from ATM90E32 chips
float temp1, temp2, freq1, freq2, 
      voltageCT[NUM_CHANNELS], currentCT[NUM_CHANNELS],
      reactivePowerCT[NUM_CHANNELS], vaPowerCT[NUM_CHANNELS], powerFactorCT[NUM_CHANNELS];





// -------------------------------------------------------------------
// SETUP
// -------------------------------------------------------------------
void setup() 
{
  /* Initialize the serial port to host */
  Serial.begin ( SERIAL_SPEED );
  while ( ! Serial ) 
  {  /* wait for serial port to connect. Needed for native USB */ }

  

  Serial.println ( "\n\n**********************************************" ); 
  Serial.println ( "\nFiring up " + String ( SKETCH_NAME ) ); 


  // Initialise the two ATM90E32 ICs & Pass Chip Elect (CS) pin and calibrations to its library 
  Serial.println ( "Initialize the two ATM90E32 ICs" );

  sensorIC1.begin ( cs1pin, lineFrequency, pgaGain, voltageGain, currentGain[0], currentGain[1], currentGain[2] ); // Sensors 1-3 on IC1
  sensorIC2.begin ( cs2pin, lineFrequency, pgaGain, voltageGain, currentGain[3], currentGain[4], currentGain[5] ); // Sensors 4-6 on IC2

  Serial.println ( "   ATM90E32 initialization completed." ); 


  delay ( 1500 );
}





long loopCount = 0; // Simple counter to see progress in the Serial output window

// Repeatedly fetch values from the ATM90E32
void loop() 
{
  loopCount++; 
  Serial.println ( "\n\nReading " + String ( loopCount ) + " at " + String ( millis() ) + " ms." ); 

  // Get System Status readings. No doc found on what these mean. 
  unsigned short sys0_1 = sensorIC1.GetSysStatus0();   //EMMState0
  unsigned short sys1_1 = sensorIC1.GetSysStatus1();   //EMMState1
  unsigned short en0_1  = sensorIC1.GetMeterStatus0(); //EMMIntState0
  unsigned short en1_1  = sensorIC1.GetMeterStatus1(); //EMMInsState1

  unsigned short sys0_2 = sensorIC2.GetSysStatus0();
  unsigned short sys1_2 = sensorIC2.GetSysStatus1();
  unsigned short en0_2  = sensorIC2.GetMeterStatus0();
  unsigned short en1_2  = sensorIC2.GetMeterStatus1();

  Serial.println ( "Sys Status IC1:   S0:0x" + String ( sys0_1, HEX ) + " S1:0x" + String ( sys1_1, HEX ) );
  Serial.println ( "Meter Status IC1: E0:0x" + String ( en0_1, HEX  ) + " E1:0x" + String ( en1_1, HEX ) );

  Serial.println ( "Sys Status IC2:   S0:0x" + String ( sys0_2, HEX ) + " S1:0x" + String ( sys1_2, HEX ) );
  Serial.println ( "Meter Status IC2: E0:0x" + String ( en0_2, HEX  ) + " E1:0x" + String ( en1_2, HEX ) );
  delay(10);


  // Line frequency
  freq1 = sensorIC1.GetFrequency();
  freq2 = sensorIC2.GetFrequency();

  // Chip temperature
  temp1 = sensorIC1.GetTemperature();
  temp2 = sensorIC2.GetTemperature();

  Serial.println ( "\n          IC1          IC2" ); 

  Serial.print ( "Temp: " ); 
  printPad ( temp1 ); 
  Serial.print ( String ( temp1, 1 ) + " C    " ); 
  printPad ( temp2 ); 
  Serial.println ( String ( temp2, 1 ) + " C" );

  Serial.print ( "Freq: " ); 
  printPad ( freq1 ); 
  Serial.print ( String ( freq1, 1 ) + " Hz   " ); 
  printPad ( freq2 ); 
  Serial.println ( String ( freq2, 1 ) + " Hz" );



  // Look for errors from ICs
  if ( sys0_1 == 65535 || sys0_1 == 0 || sys0_2 == 65535 || sys0_2 == 0 )
  {
    /* Print error message if we can't talk to the master board */
    Serial.println ( "Error: Not receiving data from the energy meter - check your connections" );
    Serial.println ( "\n\n\n\n\n" ); 
  }

  else  // Board appears to be working properly.
  {     // Let's get some readings

    /* Get readings from each IC */
    // Line voltage
    voltageCT[0]       = sensorIC1.GetLineVoltageA();
    voltageCT[1]       = sensorIC1.GetLineVoltageB();
    voltageCT[2]       = sensorIC1.GetLineVoltageC();
    voltageCT[3]       = sensorIC2.GetLineVoltageA();
    voltageCT[4]       = sensorIC2.GetLineVoltageB();
    voltageCT[5]       = sensorIC2.GetLineVoltageC();

    // Current
    currentCT[0]       = sensorIC1.GetLineCurrentA();
    currentCT[1]       = sensorIC1.GetLineCurrentB();
    currentCT[2]       = sensorIC1.GetLineCurrentC();
    currentCT[3]       = sensorIC2.GetLineCurrentA();
    currentCT[4]       = sensorIC2.GetLineCurrentB();
    currentCT[5]       = sensorIC2.GetLineCurrentC();

    // Active Energy in Watt Hours
    reactivePowerCT[0] = sensorIC1.GetActivePowerA();
    reactivePowerCT[1] = sensorIC1.GetActivePowerB();
    reactivePowerCT[2] = sensorIC1.GetActivePowerC();
    reactivePowerCT[3] = sensorIC2.GetActivePowerA();
    reactivePowerCT[4] = sensorIC2.GetActivePowerB();
    reactivePowerCT[5] = sensorIC2.GetActivePowerC();
    // realTotalPower  = sensor_icX.GetTotalActivePower();  // Get TOTAL Active Power

    // Wattage at volts x amps
    vaPowerCT[0]       = sensorIC1.GetApparentPowerA();
    vaPowerCT[1]       = sensorIC1.GetApparentPowerB();
    vaPowerCT[2]       = sensorIC1.GetApparentPowerC();
    vaPowerCT[3]       = sensorIC2.GetApparentPowerA();
    vaPowerCT[4]       = sensorIC2.GetApparentPowerB();
    vaPowerCT[5]       = sensorIC2.GetApparentPowerC();

    // Plus or minus 1
    powerFactorCT[0]   = sensorIC1.GetPowerFactorA();
    powerFactorCT[1]   = sensorIC1.GetPowerFactorB();
    powerFactorCT[2]   = sensorIC1.GetPowerFactorC();
    powerFactorCT[3]   = sensorIC2.GetPowerFactorA();
    powerFactorCT[4]   = sensorIC2.GetPowerFactorB();
    powerFactorCT[5]   = sensorIC2.GetPowerFactorC();


    // Print out a little table for the 6 channels
    printoutTable(); 

  }

  delay ( 5000 ); // Pacing is everything

} // end loop





// Print out a formatted table using Serial.print
void printoutTable()
{
  Serial.println ( "\n                   volts        amps        Rpower      VAPower      PFactor"); 
  for ( int sensor = 0; sensor < NUM_CHANNELS; sensor++ )
  {
    Serial.print ( "  Sensor [" + String ( sensor + 1 ) + "] " ); 


    printPad ( voltageCT[sensor] );
    Serial.print ( String ( voltageCT[sensor],       1 ) + " VAC   "); 

    printPad ( currentCT[sensor] );
    Serial.print ( String ( currentCT[sensor],       1 ) + " A    " ); 

    printPad ( reactivePowerCT[sensor] );
    Serial.print ( String ( reactivePowerCT[sensor], 1 ) + " WH   " ); 

    printPad ( vaPowerCT[sensor] );
    Serial.print ( String ( vaPowerCT[sensor],       1 ) + " WH   " ); 

    printPad ( powerFactorCT[sensor] );
    Serial.print ( String ( powerFactorCT[sensor],   1 ) + " PF" ); 


    Serial.println(); 
    Serial.println(); 
  }
} // end printoutTable





// To keep Serial.print table formatted, adjust spacing
void printPad ( float value )
{
  if ( abs ( value ) < 1000.0 ) Serial.print ( " " ); // space instead of leading zeros
  if ( abs ( value ) <  100.0 ) Serial.print ( " " ); 
  if ( abs ( value ) <   10.0 ) Serial.print ( " " ); 
  if (       value   >=   0.0 ) Serial.print ( " " ); // Negative values throw in a - sign

} // end printPad

