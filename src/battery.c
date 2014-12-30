#include "battery.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "adc_f373.h"
#include <stdio.h>

// ... ADC registers don't match from F303 to F373!
#define ADC_BASE (PERIPH_BASE_APB2 + 0x2400)
#define ADC ADC_BASE

#define ADC_SR(base) MMIO32(base + 0x00)
#define ADC_SR_EOC (1<<1)

#define ADC_CR1(base) MMIO32(base + 0x04)
#define ADC_CR1_SCAN (1<<8)

#define RCC_APB2_ADC_CLOCK (1<<9)

#define HERE do { printf("%s:%d\n", __FILE__, __LINE__); for(int i=0; i<800000; i++) __asm__("nop"); } while(0)

// pin 14 as battery (PA4 / ADC_IN4)
void battery_setup(void) {

printf("note: ADC_BASE = %x\n", ADC_BASE);
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2_ADC_CLOCK);
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO4);

    // adc_off()
    ADC_CR2(ADC) &= ~ADC_CR2_ADON;

    // adc_set_clk_prescale(ADC_CCR_CKMODE_DIV2);
    RCC_CFGR = (RCC_CFGR & MASK_ADCPRE);

    // adc_set_single_conversion_mode(ADC);
    ADC_CR2(ADC) &= ~ADC_CR2_CONT;

    // adc_disable_external_trigger_regular(ADC);
    ADC_CR2(ADC) |= ~ADC_CR2_EXTSEL(7);

    //adc_set_right_aligned(ADC);
    ADC_CR2(ADC) &= ~ADC_CR2_ALIGN;

    // adc_set_sample_time_on_all_channels(ADC, ADC_SMPR1_SMP_61DOT5CYC);
    // actually, just channel 4...
    ADC_SMPR1(ADC) = (ADC_SMPR1(ADC) & ~ADC_SMPR1_SMP4(7)) | ~ADC_SMPR1_SMP4(6);

    // select sequence containing only ADC4 
    ADC_SQR1(ADC) = ADC_SQR1_L(0);
    ADC_SQR3(ADC) = ADC_SQR3_SQ1(4);

    // adc_power_on(ADC);
    ADC_CR2(ADC) |= ADC_CR2_ADON;

    for (int i = 0; i < 800000; i++)
            __asm__("nop");
}

int battery_get_mv(void) {
// printf("%d\n", __LINE__); for (int i = 0; i < 800000; i++) __asm__("nop");
    ADC_CR2(ADC) |= ADC_CR2_SWSTART;

    while (!(ADC_SR(ADC) & ADC_SR_EOC)) ;

    return ADC_DR(ADC);
}

