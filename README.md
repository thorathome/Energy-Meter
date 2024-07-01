# CircuitSetup 6-Channel Energy-Meter

Software for CircuitSetup's 6-Channel EnergyMeter

    This is a Get Started Arduino sketch for an ESP32 
    mounted on a CircuitSetup 6 Channel Energy Meter Main Board
    https://circuitsetup.us/product/expandable-6-channel-esp32-energy-meter/?v=7516fd43adaa

    This sketch reads the 6 channels monitored by the Energy Meter,
    calculates Watt-Hours for each channel, and 
    Serial.prints out the values for each channel in a formatted table. 

    It does nothing more, nothinbg less.  

    You have my permission to use this code free of charge. 
    https://github.com/thorathome/Energy-Meter/blob/main/Energy_Meter_Basic_Setup

    July 2024


    This sketch is based on multiple documents and code sources throughout CircuitSetup's realm
    https://github.com/CircuitSetup
    https://github.com/CircuitSetup/Expandable-6-Channel-ESP32-Energy-Meter

    I found these two sketches to be valuable
    https://github.com/CircuitSetup/Expandable-6-Channel-ESP32-Energy-Meter/blob/master/Software/EmonESP/src/energy_meter.cpp
    https://github.com/CircuitSetup/Expandable-6-Channel-ESP32-Energy-Meter/blob/master/Software/EmonESP/src/energy_meter.h

    Info on the customized ATM90E32 lib
    Just create a library with these four files in it as the "custon" ATM90E32 library needed for this device
    https://github.com/CircuitSetup/ATM90E32

    Configuration info
    https://esphome.io/components/sensor/atm90e32.html

    Read info here on US 220 VAC power monitoring
    https://esphome.io/components/sensor/atm90e32.html#voltage

    Read here for better understanding of VApower, RealPower and Power Factor
    ALL of which are calculated by this marvelous board.  
    Short article
    https://www.apc.com/us/en/solutions/industry-insights/watts-vs-va-whats-difference-anyway.jsp

