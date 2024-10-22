#include "EmbeddedFiles_Defines.h"

#ifndef EBMS_DEFINES_H // include guard
#define EBMS_DEFINES_H


//This is where the data begins in EEPROM
#define EEPROM_CONFIG_ADDRESS 0


// Only the lowest 4 bits can be used!
enum COMMAND: uint8_t
{
    ResetBadPacketCounter = 0,
    ReadVoltageAndStatus=1,
    Identify=2,
    ReadTemperature=3,
    ReadBadPacketCounter=4,
    ReadSettings=5,
    WriteSettings=6,
    ReadBalancePowerPWM=7,
    Timing=8,
    ReadBalanceCurrentCounter=9,
    ReadPacketReceivedCounter=10,
    ResetBalanceCurrentCounter=11
};


//Default values
struct CellModuleConfig {
  //uint8_t mybank;
  uint8_t BypassTemperatureSetPoint;
  uint16_t BypassThresholdmV;

  // Resistance of bypass load
  //float LoadResistance;
  //Voltage Calibration
  float Calibration;
  //Reference voltage (millivolt) normally 2.00mV
  //float mVPerADC;
  //Internal Thermistor settings
  //uint16_t Internal_BCoefficient;
  //External Thermistor settings
  //uint16_t External_BCoefficient;
} __attribute__((packed));

#endif
