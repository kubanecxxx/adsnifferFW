/**
 * @file shell_user.c
 * @author kubanec
 * @date 9.8.2012
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "ch.h"
#include "hal.h"
//#include "test.h"

#include "shell.h"
#include "chprintf.h"
#include "stdlib.h"
#include "string.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

#define TEST_WA_SIZE    THD_WA_SIZE(1024)

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[])
{
	size_t n, size;

	(void) argv;
	if (argc > 0)
	{
		chprintf(chp, "Usage: mem\r\n");
		return;
	}
	n = chHeapStatus(NULL, &size);
	chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
	chprintf(chp, "heap fragments   : %u\r\n", n);
	chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[])
{
	static const char *states[] =
	{ THD_STATE_NAMES};
	Thread *tp;

	(void) argv;
	if (argc > 0)
	{
		chprintf(chp, "Usage: threads\r\n");
		return;
	}
	chprintf(chp, "   name    stack prio refs     state time  \r\n");
	tp = chRegFirstThread();
	do
	{
		chprintf(chp, "%15s  %.8lx  %4lu  %4lu  %9s  %lu \r\n", tp->p_name,
				(uint32_t) tp->p_ctx.r13, (uint32_t) tp->p_prio,
				(uint32_t) (tp->p_refs - 1), states[tp->p_state],
				(uint32_t) tp->p_time);
		tp = chRegNextThread(tp);
	}while (tp != NULL );
}

extern BaseSequentialStream * stream;
extern BinarySemaphore semaphore;
extern uint16_t speed;

static void cmd_read(BaseSequentialStream *chp, int argc, char *argv[])
{
	stream = chp;
	chBSemSignal(&semaphore);
	palSetPad(GPIOD,14);
}

static void cmd_stop(BaseSequentialStream *chp, int argc, char *argv[])
{
	stream = NULL;
	chBSemWait(&semaphore);
	palClearPad(GPIOD,14);
}

static void cmd_speed(BaseSequentialStream *chp, int argc, char *argv[])
{
	uint16_t temp;

	if (argc != 1)
		return;

	temp = atoi(argv[0]);
	if (temp < 2)
		return;

	speed = temp;
}

const ShellCommand commands[] =
{
{ "mem", cmd_mem },
{ "threads", cmd_threads },
{ "start", cmd_read },
{ "stop", cmd_stop },
{ "speed" , cmd_speed},
{ NULL, NULL } };
