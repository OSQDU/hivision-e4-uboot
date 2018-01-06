/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/proc-armv/ptrace.h>
#include <div64.h>
#include <asm/arch/hardware.h>

//#define USE_PTS_AS_CLOCKSOURCE

#ifdef USE_PTS_AS_CLOCKSOURCE
#define TICKS_PER_USEC 						(CONFIG_FH81_PTS_CLOCK / 1000000)
#else
#define TICKS_PER_USEC 						(CONFIG_FH81_TIMER1_CLOCK / 1000000)
#endif

#define	REG_TIMER_BASE					(0xf0c00000)
#define	REG_TIMER1_LOADCNT				(REG_TIMER_BASE + 0x00)
#define	REG_TIMER1_CUR_VAL				(REG_TIMER_BASE + 0x04)
#define	REG_TIMER1_CTRL_REG				(REG_TIMER_BASE + 0x08)
#define	REG_TIMER1_EOI_REG				(REG_TIMER_BASE + 0x0c)
#define	REG_TIMER1_INTSTATUS			(REG_TIMER_BASE + 0x10)
#define	REG_TIMERS_RAWINTSTATUS			(REG_TIMER_BASE + 0xa8)
#define	REG_PAE_PTS_REG					(0xec100000 + 0x0040)
#define U32_COUNTER_MAX					(0xffffffff)


static ulong timer_load_val;
/* Internal tick units */
/* Last decremneter snapshot */
static unsigned long lastdec;
/* Monotonic incrementing timer */
static unsigned long long timestamp;

/* read the timer  */
static inline ulong read_timer(void)
{
#ifdef USE_PTS_AS_CLOCKSOURCE
	ulong ticks= GET_REG(REG_PAE_PTS_REG);
#else
	ulong ticks= ~GET_REG(REG_TIMER1_CUR_VAL);
#endif
	return ticks;
}

int timer_init(void)
{
#ifdef USE_PTS_AS_CLOCKSOURCE
	lastdec = GET_REG(REG_PAE_PTS_REG);
#else
	SET_REG(REG_TIMER1_CTRL_REG, 0x05);
	timer_load_val = TICKS_PER_USEC * 100000000; /* ticks in 100s*/
	/* load value for 10 ms timeout */
	SET_REG(REG_TIMER1_LOADCNT, timer_load_val);
	// reset
	SET_REG_M(REG_TIMER1_CTRL_REG, 0, 1);
	SET_REG_M(REG_TIMER1_CTRL_REG, 1, 1);
	lastdec = 0;
#endif
	return 0;
}

/*
 * timer without interrupts
 */
/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	ulong now = read_timer();
	if (now >= lastdec) {
		/* normal mode */
		timestamp += now - lastdec;
	} else {
		now = read_timer();
		if (now >= lastdec) {
			timestamp += now - lastdec;
		} else {
			//printf("4.prev: 0x%lu, now: 0x%lu\n", lastdec, now);
			/* we have an overflow ... */
			timestamp += now + U32_COUNTER_MAX - lastdec;
		}
	}
	lastdec = now;
	return timestamp / (TICKS_PER_USEC * 10);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	/* We overrun in 100s */
	return (ulong)(timer_load_val / 100);
}

void reset_timer_masked(void)
{
	/* reset time */
	lastdec = read_timer();
	timestamp = 0;
}

void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer_masked(void)
{
	unsigned long long res = get_ticks();
	do_div(res, 100);
	return res; //ms
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	timestamp = t * 100;///(timer_load_val / (100 * CONFIG_SYS_HZ));
}

void __udelay(unsigned long usec)
{
	unsigned long long tmp;
	ulong tmo;
	tmo = (usec + 9) / 10;
	tmp = get_ticks() + tmo; /* get current timestamp */

	while (get_ticks() < tmp)
		/* loop till event */
		/*NOP*/;
}

