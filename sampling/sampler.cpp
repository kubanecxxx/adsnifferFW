/**
 * @file sampler.cpp
 * @author kubanec
 * @date 19.6.2013
 *
 */

#include "sampler.h"
#include "chprintf.h"
#include "hal.h"
#include <string.h>

BaseSequentialStream * stream = NULL;
BSEMAPHORE_DECL(semaphore,TRUE);
uint16_t speed = 50;


static void adccb(ADCDriver *adcp, adcsample_t *buffer, size_t n);
/* Total number of channels to be sampled by a single ADC operation.*/
#define ADC_GRP1_NUM_CHANNELS  7

/* Depth of the conversion buffer, channels are sampled four times each.*/
#define ADC_GRP1_BUF_DEPTH      2

/*
 * ADC samples buffer.
 */
static adcsample_t samples[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 4 samples of 2 channels, SW triggered.
 * Channels:    IN11   (48 cycles sample time)
 *              Sensor (192 cycles sample time)
 */
static const ADCConversionGroup adcgrpcfg =
{ FALSE, ADC_GRP1_NUM_CHANNELS, adccb, NULL,
/* HW dependent part.*/
0, ADC_CR2_SWSTART,
		ADC_SMPR1_SMP_AN11(ADC_SAMPLE_56) | ADC_SMPR1_SMP_AN12(ADC_SAMPLE_56)
				| ADC_SMPR1_SMP_AN15(ADC_SAMPLE_56),
		ADC_SMPR2_SMP_AN1(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN2(ADC_SAMPLE_56)
				| ADC_SMPR2_SMP_AN8(ADC_SAMPLE_56)
				| ADC_SMPR2_SMP_AN9(ADC_SAMPLE_56), ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
		ADC_SQR2_SQ7_N(ADC_CHANNEL_IN15), ADC_SQR3_SQ1_N(ADC_CHANNEL_IN1)
				| ADC_SQR3_SQ2_N(ADC_CHANNEL_IN2)
				| ADC_SQR3_SQ3_N(ADC_CHANNEL_IN8)
				| ADC_SQR3_SQ4_N(ADC_CHANNEL_IN9)
				| ADC_SQR3_SQ5_N(ADC_CHANNEL_IN11)
				| ADC_SQR3_SQ6_N(ADC_CHANNEL_IN12) };

static adcsample_t Samples[ADC_GRP1_NUM_CHANNELS];

static void cb(void * a)
{
	chSysLockFromIsr();
	adcStartConversionI(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);
	chSysUnlockFromIsr()
}

static VirtualTimer vt;

void adccb(ADCDriver *adcp, adcsample_t *buffer, size_t n)
{
	if (!adcp->state == ADC_COMPLETE)
		return;

	memcpy(&Samples, buffer, n * sizeof(adcsample_t) * ADC_GRP1_NUM_CHANNELS);

	if (chVTIsArmedI(&vt))
		return;

#if 0
	chSysLockFromIsr();
	chVTSetI(&vt, MS2ST(10), cb, NULL);
	chSysUnlockFromIsr()
#endif
}

sampler::sampler() :
	EnhancedThread("sample thread", NORMALPRIO)
{
	/*
	 * Initializes the ADC driver 1 and enable the thermal sensor.
	 * The pin PC1 on the port GPIOC is programmed as analog input.
	 */
	adcStart(&ADCD1, NULL);
	palSetGroupMode(GPIOC, 0b100110,0, PAL_MODE_INPUT_ANALOG);
	palSetGroupMode(GPIOA,0b110,0,PAL_MODE_INPUT_ANALOG);
	palSetGroupMode(GPIOB,0b11,0,PAL_MODE_INPUT_ANALOG);
	adcStartConversion(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);
}

msg_t sampler::Main(void)
{
	uint16_t digital;
	while (TRUE)
	{

		if (stream)
		{
			for (int i = 0; i < ADC_GRP1_NUM_CHANNELS; i++)
			{
				chprintf(stream,"%d ",Samples[i]);
			}

			digital = palReadGroup(GPIOE,0b11111,2);
			digital |= palReadGroup(GPIOC,0b111,13) << 5;

			chprintf(stream,"%d",digital);

			chprintf(stream, "\n\r");
			adcStartConversion(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);
			palTogglePad(GPIOD,13);
		}

		chThdSleepMilliseconds(speed);
	}
}
