//Select Environment: env:V450_5k(ATTINYCellModule)
//Also Try in Desktop

#define RX_BUFFER_SIZE 64

#include "IOlib.h"

#include "Ebms_tinyAVR2.h"

#if !defined(EBMSBAUD)
#error Expected EBMSBAUD define
#endif

#if !defined(SAMPLEAVERAGING)
#error Expected SAMPLEAVERAGING define
#endif

// Our project code includes
#include "defines.h"
#include "settings.h"
#include <FastPID.h>
#include <SerialEncoder.h>
#include "packet_processor.h"

uint8_t SerialPacketReceiveBuffer[8 + sizeof(PacketStruct)];

SerialEncoder myPacketSerial;

// Default values which get overwritten by EEPROM on power up
CellModuleConfig myConfig;

PacketProcessor PP(&myConfig);

volatile bool wdt_triggered = false;

// Interrupt counter
volatile uint8_t InterruptCounter = 0;
volatile uint16_t PulsePeriod = 0;
volatile uint16_t OnPulseCount = 0;

void DefaultConfig()
{
myConfig.Calibration = 1.0;


#if defined(EBMSMODULEVERSION) && (EBMSMODULEVERSION == 420 && !defined(SWAPR19R20))
  // Keep temperature low for modules with R19 and R20 not swapped
  myConfig.BypassTemperatureSetPoint = 45;
#else
  // Stop running bypass if temperature over 65 degrees C
  myConfig.BypassTemperatureSetPoint = 65;
#endif

  // Start bypass at 4.1V
  myConfig.BypassThresholdmV = 4100;

  //#if defined(EBMSMODULEVERSION) && (EBMSMODULEVERSION == 430 || EBMSMODULEVERSION == 420 || EBMSMODULEVERSION == 421)
  // Murata Electronics NCP18WB473J03RB = 47K ±5% 4050K ±2% 100mW 0603 NTC Thermistors RoHS
  // myConfig.Internal_BCoefficient = 4050;
  //#else
  // 4150 = B constant (25-50℃)
  // myConfig.Internal_BCoefficient = 4150;
  //#endif

  // 4150 = B constant (25-50℃)
  // myConfig.External_BCoefficient = 4150;

  // Resistance @ 25℃ = 47k, B Constant 4150, 0.20mA max current
  // Using https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html
}

void watchdog()
{
  // This is the watchdog timer - something went wrong and no serial activity received in over 8 seconds
  wdt_triggered = true;
  PP.IncrementWatchdogCounter();
}

void onPacketReceived()
{
  EBMSHAL::EnableSerial0TX();

  // A data packet has just arrived, process it and forward the results to the
  // next module
  if (PP.onPacketReceived((PacketStruct *)SerialPacketReceiveBuffer))
  {
    // Only light Notification if packet is good
    EBMSHAL::NotificationLedOn();
  }

  // Send the packet (fixed length!) (even if it was invalid so controller can
  // count crc errors)
  myPacketSerial.sendBuffer(SerialPacketReceiveBuffer);

  // PLATFORM ATTINYCORE version is 1.3.2 which is old, and SERIAL.FLUSH simply
  // clears the buffer (bad)
  // Therefore we use 1.4.1 which has the correct code to wait until the buffer
  // is empty.
  EBMSHAL::FlushSerial0();

  EBMSHAL::NotificationLedOff();
}

// Kp: Determines how aggressively the PID reacts to the current amount of error
// (Proportional)
// Ki: Determines how aggressively the PID reacts to error over time (Integral)
// Kd: Determines how aggressively the PID reacts to the change in error
// (Derivative)

// 6Hz rate - number of times we call this code in Loop
// Kp, Ki, Kd, Hz, output_bits, output_signed);
// Settings for V4.00 boards with 2R2 resistors = (4.0, 0.5, 0.2, 6, 8, false);
FastPID myPID(5.0, 1.0, 0.1, 3, 8, false);

void ValidateConfiguration()
{
  if (myConfig.Calibration < 0.8 || myConfig.Calibration > 10.0)
  {
    myConfig.Calibration = 1.0;
  }

  if (myConfig.BypassTemperatureSetPoint > EBMS_MODULE_SafetyTemperatureCutoff)
  {
    myConfig.BypassTemperatureSetPoint = EBMS_MODULE_SafetyTemperatureCutoff - 10;
  }

#if defined(EBMSMODULEVERSION) && (EBMSMODULEVERSION == 420 && !defined(SWAPR19R20))
  // Keep temperature low for modules with R19 and R20 not swapped
  if (myConfig.BypassTemperatureSetPoint > 45)
  {
    myConfig.BypassTemperatureSetPoint = 45;
  }
#endif
}

void StopBalance()
{
  PP.WeAreInBypass = false;
  PP.bypassCountDown = 0;
  PP.bypassHasJustFinished = 0;
  PP.PWMSetPoint = 0;
  PP.SettingsHaveChanged = false;

  OnPulseCount = 0;
  PulsePeriod = 0;

  EBMSHAL::StopTimer1();
  EBMSHAL::DumpLoadOff();
}

void setup()
{
  // Must be first line of code
  wdt_disable();
  wdt_reset();

  // Hold flag to cater for tinyAVR2 chips
  bool just_powered_up = true;

#if defined(__AVR_ATtinyx24__)
  // Did we have a watchdog reboot?
  // The megaTinycore copies the RSTCTRL.RSTFR register into GPIOR0 on reset (watchdog, power up etc)
  // RSTFR is cleared before our code runs, so this is the only way to identify a watchdog reset
  // when using megaTinycore and tinyAVR chips.
  if ((GPIOR0 & RSTCTRL_WDRF_bm) == RSTCTRL_WDRF_bm)
  {
    watchdog();
    // Its not a power on reset, its a watchdog reset
    just_powered_up = false;
  }


/*
//Brown Out Detection
  if ((GPIOR0 & RSTCTRL_BORF_bm) == RSTCTRL_BORF_bm)
  {
    EBMSHAL::FlashNotificationLed(6, 250);
  }
*/
#endif

  EBMSHAL::SetPrescaler();

  // 8 second between watchdogs
  EBMSHAL::SetWatchdog8sec();

  // Setup IO ports
  EBMSHAL::ConfigurePorts();

  if (just_powered_up)
  {
    // 4 flashes
    EBMSHAL::PowerOn_Notification_led();
  }

  // Check if setup routine needs to be run
  if (!Settings::ReadConfigFromEEPROM((uint8_t *)&myConfig, sizeof(myConfig), EEPROM_CONFIG_ADDRESS))
  {
    DefaultConfig();
    // No need to save here as the default config will load every time if the CRC is wrong
  }

  ValidateConfiguration();

#if defined(EBMSMODULEVERSION) && EBMSMODULEVERSION < 440
  EBMSHAL::double_tap_blue_led();
#endif

  // The PID can vary between 0 and 255 (1 byte)
  myPID.setOutputRange(0, 255);

  StopBalance();

  // Set up data handler
  Serial.begin(EBMSBAUD, SERIAL_8N1);

  myPacketSerial.begin(&Serial, &onPacketReceived, sizeof(PacketStruct), SerialPacketReceiveBuffer, sizeof(SerialPacketReceiveBuffer));
}
void BalanceTimer()
{
  // when v=1, the duration between on and off is 2.019ms (1.0095ms per
  // interrupt) - on for 2.019ms off for 255.7ms, 0.7844% duty
  // when v=128 (50%), the duration between on and off is 130.8ms (1.0218ms per
  // interrupt) - on for 130.8ms, off for 127.4ms 50.67% duty
  // when v=192 (75%), the duration between on and off is 195.6ms (1.0187ms per
  // interrupt) - on for 62.63ms, off for 30.83ms, 75.74% duty
  InterruptCounter++;
  PulsePeriod++;
  // Reset at top
  if (InterruptCounter == 255)
  {
    InterruptCounter = 0;
  }
  // Switch the load on if the counter is below the SETPOINT
  if (InterruptCounter <= PP.PWMSetPoint)
  {
    // Enable the pin
    EBMSHAL::DumpLoadOn();

    // Count the number of "on" periods, so we can calculate the amount of
    // energy consumed
    // over time
    OnPulseCount++;
  }
  else
  {
    // Off
    EBMSHAL::DumpLoadOff();
  }

  if (PulsePeriod == 1000 && OnPulseCount != 0)
  {
    // Floats are not good on ATTINY/8bit controllers, need to look at moving to
    // fixed decimal/integer calculations

    // CellVoltage is in millivolts, so we get milli-amp current reading out.
    // For example 4000mV / 4.4R = 909.0909mA
    float CurrentmA = ((float)PP.CellVoltage() / (float)LOAD_RESISTANCE);

    // Scale down to the number of "ON" pulses
    // Assuming 100% on, 909.0909mA * (1000/1000) * 0.0002777 = 0.2525 milli amp
    // hours (or 0.000252 amp hours)
    float milliAmpHours =
        (CurrentmA * ((float)OnPulseCount / (float)1000.0)) * (1.0 / 3600.0);

    // 0.2525 * 3600 / 1000 = 0.909Ah

    // Keep running total
    PP.MilliAmpHourBalanceCounter += milliAmpHours;

    OnPulseCount = 0;
    PulsePeriod = 0;
  }
}

#if defined(__AVR_ATtinyx24__)
// We don't use ADC interrupts on ATtinyX24 as there is no benefit
ISR(TCA0_OVF_vect)
{
  // This ISR is called every 1ms when balancing is in operation

  //EBMSHAL::SpareToggle();
  BalanceTimer();
  //EBMSHAL::SpareOff();
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}
#endif

inline void identifyModule()
{
  if (PP.identifyModule > 0)
  {
#if defined(EBMSMODULEVERSION) && EBMSMODULEVERSION < 440
    EBMSHAL::BlueLedOn();
#else
    EBMSHAL::NotificationLedOn();
#endif
    PP.identifyModule--;

    if (PP.identifyModule == 0)
    {
#if defined(EBMSMODULEVERSION) && EBMSMODULEVERSION < 440
      EBMSHAL::BlueLedOff();
#else
      EBMSHAL::NotificationLedOff();
#endif
    }
  }
}

/*
void loop()
{
  // This loop runs around 3 times per second when the module is in bypass
  wdt_reset();

  Serial.println("Hello world!");
}
*/

void loop()
{
  // This loop runs around 3 times per second when the module is in bypass
  wdt_reset();
  identifyModule();

  if (PP.SettingsHaveChanged)
  {
    // The configuration has just been modified so stop balancing if we are and
    // reset our status
    StopBalance();
  }

  if (wdt_triggered)
  {
#if defined(EBMSMODULEVERSION) && EBMSMODULEVERSION < 440
    // Flash blue LED twice after a watchdog wake up
    EBMSHAL::double_tap_blue_led();
#else
    // Flash Notification LED twice after a watchdog wake up
    EBMSHAL::double_tap_Notification_led();
#endif

    // If we have just woken up, we shouldn't be in balance safety check that we are not
    StopBalance();
  }

  // DEBUG
  //EBMSHAL::NotificationLedOn();

  // We always take a voltage and temperature reading on every loop cycle to check if we need to go into bypass
  // this is also triggered by the watchdog should comms fail or the module is running standalone
  EBMSHAL::TemperatureVoltageOn();

  // Internal temperature
  PP.TakeAnAnalogueReading(ADC_INTERNAL_TEMP);

  // Only take these readings when we are NOT in bypass....
  // this causes the voltage and temperature to "freeze" during bypass cycles
  if (PP.bypassCountDown == 0)
  {
    // Just for debug purposes, shows when voltage is read
    //#if defined(EBMSMODULEVERSION) && EBMSMODULEVERSION < 430
    //     EBMSHAL::BlueLedOn();
    //#endif

    // External temperature
    PP.TakeAnAnalogueReading(ADC_EXTERNAL_TEMP);

    EBMSHAL::ReferenceVoltageOn();

#if (SAMPLEAVERAGING == 1)
    // Sample averaging not enabled
    PP.TakeAnAnalogueReading(ADC_CELL_VOLTAGE);
#else
    // Take several samples and average the result
    for (size_t i = 0; i < SAMPLEAVERAGING; i++)
    {
      PP.TakeAnAnalogueReading(ADC_CELL_VOLTAGE);
    }
#endif
  }

  // Switch reference off if we are not in bypass (otherwise leave on)
  EBMSHAL::ReferenceVoltageOff();
  EBMSHAL::TemperatureVoltageOff();

  // DEBUG
  //EBMSHAL::NotificationLedOff();

  if (wdt_triggered == true && Serial.available() == 0)
  {
    // If the WDT trigered, and NO serial is available, don't do anything here
    // this avoids waiting 200ms for data that is never going to arrive.
  }
  else
  {
    // Loop here processing any packets then go back to sleep

    // NOTE this loop size is dependant on the size of the packet buffer (40 bytes)
    //      too small a loop will prevent anything being processed as we go back to Sleep
    //      before packet is received correctly
    // PacketProcessed = false;
    for (size_t i = 0; i < 200; i++)
    {
      // Call update to receive, decode and process incoming packets.
      myPacketSerial.checkInputStream();

      // Allow data to be received in buffer (delay must be AFTER) checkInputStream
      delay(1);
    }
  }

  // We should probably check for invalid InternalTemperature ranges here and throw error (shorted or unconnecter thermistor for example)
  int16_t internal_temperature = PP.InternalTemperature();

  if (internal_temperature > EBMS_MODULE_SafetyTemperatureCutoff || internal_temperature > (myConfig.BypassTemperatureSetPoint + 10))
  {
    // Force shut down if temperature is too high although this does run the risk that the voltage on the cell will go high
    // but the BMS controller should shut off the charger in this situation
    myPID.clear();
    StopBalance();
  }

  // Only enter bypass if the board temperature is below safety
  if (PP.BypassCheck() && internal_temperature < EBMS_MODULE_SafetyTemperatureCutoff)
  {
    // Our cell voltage is OVER the voltage setpoint limit, start draining cell using bypass resistor

    if (!PP.WeAreInBypass)
    {
      // We have just entered the bypass code
      PP.WeAreInBypass = true;

      // This controls how many cycles of loop() we make before re-checking the situation
      // about every 30 seconds
      PP.bypassCountDown = 50;
      PP.bypassHasJustFinished = 0;

      // Start PWM
      EBMSHAL::StartTimer1();
      PP.PWMSetPoint = 0;
    }
  }

  if (PP.bypassCountDown > 0)
  {
    if (internal_temperature < (myConfig.BypassTemperatureSetPoint - 6))
    {
      // Full power if we are no where near the setpoint (more than 6 degrees C away)
      PP.PWMSetPoint = 0xFF;
    }
    else
    {
      // Compare the real temperature against max setpoint, we want the PID to keep at this temperature
      PP.PWMSetPoint = myPID.step(myConfig.BypassTemperatureSetPoint, internal_temperature);

      if (myPID.err())
      {
        // Clear the error and stop balancing
        myPID.clear();
        StopBalance();
      }
    }

    PP.bypassCountDown--;

    if (PP.bypassCountDown == 0)
    {
      // Switch everything off for this cycle
      StopBalance();
      // On the next iteration of loop, don't sleep so we are forced to take another
      // cell voltage reading without the bypass being enabled, and we can then
      // evaluate if we need to stay in bypass mode, we do this a few times
      // as the cell has a tendancy to float back up in voltage once load resistor is removed
      PP.bypassHasJustFinished = 150;
    }
  }

  if (PP.bypassHasJustFinished > 0)
  {
    PP.bypassHasJustFinished--;
  }

  // Clear watchdog assertion if its set
  wdt_triggered = false;

  // Determine if we should sleep or repeat loop (if balancing)
  if (!PP.WeAreInBypass && PP.bypassHasJustFinished == 0 && Serial.available() == 0)
  {
    // Go to SLEEP, we are not in bypass and no serial data waiting...
    // Reset PID to defaults, in case we want to start balancing
    myPID.clear();

    // Program stops here until woken by watchdog or Serial port ISR
    EBMSHAL::Sleep();
    // We are awake again....
  }
}
