// Joystick.c - Joystick ADC implementation

#include <HAL/Joystick.h>

void initADC() {
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_SYSOSC, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
}

void startADC() {
   ADC14_enableConversion();
   ADC14_toggleConversionTrigger();
}

void initJoyStick() {
    // X axis
    ADC14_configureConversionMemory(ADC_MEM0,
                                  ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                  ADC_INPUT_A15,
                                  ADC_NONDIFFERENTIAL_INPUTS);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
                                               GPIO_PIN0,
                                               GPIO_TERTIARY_MODULE_FUNCTION);

    // Y axis
    ADC14_configureConversionMemory(ADC_MEM1,
                                      ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                      ADC_INPUT_A9,
                                      ADC_NONDIFFERENTIAL_INPUTS);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
                                               GPIO_PIN4,
                                               GPIO_TERTIARY_MODULE_FUNCTION);
}

void getSampleJoyStick(unsigned *X, unsigned *Y) {
    *X = ADC14_getResult(ADC_MEM0);
    *Y = ADC14_getResult(ADC_MEM1);
}

Joystick Joystick_construct() {
  Joystick Joystick;
  initADC();
  initJoyStick();
  startADC();
  return Joystick;
}

void Joystick_refresh(Joystick* joystick_p) {
    joystick_p->x = ADC14_getResult(ADC_MEM0);
    joystick_p->y = ADC14_getResult(ADC_MEM1);
}
