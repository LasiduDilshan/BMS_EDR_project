#include "IOlib.h"

#if defined(__AVR_ATtinyx24__)

#include "Ebms_tinyAVR2.h"


#ifdef MILLIS_USE_TIMERA0
#error "This sketch takes over TCA0 - please use a different timer for millis"
#endif
#ifndef MILLIS_USE_TIMERB0
  #error "This sketch is written for use with TCB0 as the millis timing source"
#endif


void EBMSHAL::FlashNotificationLed(size_t times, uint32_t milliseconds)  //General function for LED turn on
{
  for (size_t i = 0; i < times; i++)
  {
    NotificationLedOn();
    delay(milliseconds);
    NotificationLedOff();
    delay(milliseconds);
  }
}

void EBMSHAL::PowerOn_Notification_led()  //Notification for power on
{
  FlashNotificationLed(4, 150);
}

void EBMSHAL::double_tap_Notification_led()  //Notification for double-tap action or user interaction
{
  FlashNotificationLed(2, 50);
}

void EBMSHAL::ConfigurePorts()
{
  takeOverTCA0();

  PORTA.DIRSET = PIN1_bm | PIN6_bm | PIN2_bm;    //PA1, PA6 and PA2 are outputs

  PORTB.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm;   //PB0, PB1 and PB2 are outputs

  PORTB.DIRCLR = PIN3_bm;  //PB3 input

  PORTA.DIRCLR = PIN3_bm | PIN7_bm;  //PA3, PA7 are inputs

  //Control register configuration for each pin
  PORTA.PIN0CTRL = PORT_ISC_enum::PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN1CTRL = PORT_ISC_enum::PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN2CTRL = PORT_ISC_enum::PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN3CTRL = PORT_ISC_enum::PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN4CTRL = PORT_ISC_enum::PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN5CTRL = PORT_ISC_enum::PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN6CTRL = PORT_ISC_enum::PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PIN7CTRL = PORT_ISC_enum::PORT_ISC_INPUT_DISABLE_gc;

  PORTB.PIN0CTRL = PORT_ISC_enum::PORT_ISC_INPUT_DISABLE_gc;
  PORTB.PIN1CTRL = PORT_ISC_enum::PORT_ISC_INPUT_DISABLE_gc;

  // step 1: Enable ADC
  ADC0.CTRLA = ADC_ENABLE_bm;
  // PRESC[3:0]
  // DIV16 = 5Mhz/2 = 2500000hz
  // Max. conversion rate up to 375 ksps at 12-bit resolution
  ADC0.CTRLB = ADC_PRESC_enum::ADC_PRESC_DIV2_gc;
  // SAMPDUR[7:0]
  ADC0.CTRLE = 128;   //128 means the ADC will hold the input signal for 128 clock cycles during the sampling phase
  // WINSRC / WINCM[2:0]
  ADC0.CTRLD = 0;
  ADC0.PGACTRL = 0;

  // Set pins to initial state
  DumpLoadOff();
  ReferenceVoltageOff();
  TemperatureVoltageOff();
  NotificationLedOff();

  // More power saving changes
  // EnableSerial0();
}

uint16_t EBMSHAL::BeginADCReading(uint8_t mode)
{
  uint16_t value = 0;

  // RUNSTDBY / LOWLAT / ENABLE
  // Enable ADC
  // Step 1: Enable the ADC
  ADC0.CTRLA = ADC_ENABLE_bm;

  // TIMEBASE[4:0] / REFSEL[2:0]
  // Step 2: Configure the ADC to use the VDD
  ADC0.CTRLC = TIMEBASE_1US | ADC_REFSEL_enum::ADC_REFSEL_VDD_gc; // FOR READING VREF

  // Take multiple samples (over sample)
  // Step 3: Set the ADC to start conversion immediately
  ADC0.COMMAND = ADC_MODE_BURST_SCALING_gc | ADC_START_IMMEDIATE_gc;

  // Step 4: Wait for the conversion to complete
  while (!(ADC0.INTFLAGS & ADC_RESRDY_bm))
    ;
  // Step 5: Read the conversion result
  value = (uint16_t)ADC0.RESULT;

  // Switch off ADC
  // Step 6: Disable the ADC
  ADC0.CTRLA &= ~ADC_ENABLE_bm;

  return value;
}

#endif