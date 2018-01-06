/*
 * fh_rtc.c
 *
 *  Created on: Aug 25, 2016
 *      Author: duobao
 */

#include "fh_rtc.h"


//define here

#define RTC_REG_BASE                  (0xf1500000)
#define SEC_BIT_START		0
#define SEC_VAL_MASK		0x3f
#define MIN_BIT_START		6
#define MIN_VAL_MASK		0xfc0
#define HOUR_BIT_START		12
#define HOUR_VAL_MASK		0x1f000
#define DAY_BIT_START		17
#define DAY_VAL_MASK		0xfffe0000


#define DAY_A_WEEK		7
#define DAY_A_MONTH		30
#define MONTH_A_YEAR	12


#define BASIC_YEAR		1970
#define BASIC_MONTH		1
#define BASIC_DAY		1



#define LEAPS_THRU_END_OF(y) ((y)/4 - (y)/100 + (y)/400)


//struct here
struct fh_rtc_controller {
	// vadd
	void * regs; /* vaddr of the control registers */
	u32 pregs; /* paddr of the control registers */

	u32 base_year;
	u32 base_month;
	u32 base_day;

};


enum {
	TIME_FUNC = 0,
	ALARM_FUNC,
};

enum {

	ISR_SEC_POS = 0,
	ISR_MIN_POS = 1,
	ISR_HOUR_POS = 2,
	ISR_DAY_POS = 3,
	ISR_ALARM_POS = 4,
	ISR_MASK_OFFSET = 27,
};



//func here
u32 rtc_month_days(unsigned int month, unsigned int year);

//g_val
static struct fh_rtc_controller g_rtc_control = {
		.regs = (void *) RTC_REG_BASE,
		.pregs = RTC_REG_BASE,
		.base_year = BASIC_YEAR,
		.base_month = BASIC_MONTH,
		.base_day = BASIC_DAY,
};

static const u32 rtc_ydays[2][13] = {
/* Normal years */
{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
/* Leap years */
{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 } };

static const u32 rtc_days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};





//#define FH_RTC_DEBUG_PRI

#ifdef FH_RTC_DEBUG_PRI
#define RTC_PRINT_DBG(fmt, args...) \
	printf("[FH_RTC_DEBUG]: "); \
	printf(fmt, ## args)
#else
#define RTC_PRINT_DBG(fmt, args...)  do { } while (0)
#endif



static void fh_rtc_dump_regs(struct fh_rtc_controller *rtc)
{
	RTC_PRINT_DBG("--------------------------------\n");
	RTC_PRINT_DBG("cnt:0x%x\n", fh_rtc_get_time(rtc->pregs));
	RTC_PRINT_DBG("offset:0x%x\n", fh_rtc_get_offset(rtc->pregs));
	RTC_PRINT_DBG("fail:0x%x\n", fh_rtc_get_power_fail(rtc->pregs));
	RTC_PRINT_DBG("alarm_cnt:0x%x\n", fh_rtc_get_alarm_time(rtc->pregs));
	RTC_PRINT_DBG("int stat:0x%x\n", fh_rtc_get_int_status(rtc->pregs));
	RTC_PRINT_DBG("int en:0x%x\n", fh_rtc_get_enabled_interrupt(rtc->pregs));
	RTC_PRINT_DBG("sync:0x%x\n", fh_rtc_get_sync(rtc->pregs));
	RTC_PRINT_DBG("debug:0x%x\n", fh_rtc_get_debug(rtc->pregs));
	RTC_PRINT_DBG("--------------------------------\n");
}

static void fh_rtc_dump_current_data(struct rtc_time *rtc_tm)
{

	RTC_PRINT_DBG("--------------------------------\n");
	RTC_PRINT_DBG("year:\t%d\n", rtc_tm->tm_year);
	RTC_PRINT_DBG("month:\t%d\n", rtc_tm->tm_mon);
	RTC_PRINT_DBG("day:\t%d\n", rtc_tm->tm_mday);
	RTC_PRINT_DBG("hour:\t%d\n", rtc_tm->tm_hour);
	RTC_PRINT_DBG("min:\t%d\n", rtc_tm->tm_min);
	RTC_PRINT_DBG("sec:\t%d\n", rtc_tm->tm_sec);
	RTC_PRINT_DBG("--------------------------------\n");

}

static u32 is_leap_year(unsigned int year)
{
	return (!(year % 4) && (year % 100)) || !(year % 400);
}




/*************
 *
 *
 * HW PROCESS
 *
 *
 ************/

u32 rtc_year_days(unsigned int day, unsigned int month, unsigned int year)
{
	u32 temp;
	temp = is_leap_year(year);

	//return rtc_ydays[is_leap_year(year)][month] + day-1;
	temp = rtc_ydays[temp][month] + day - 1;
	return temp;
}
void rtc_time_to_tm(unsigned long time, struct rtc_time *tm)
{
	unsigned int month, year;
	int days;

	days = time / 86400;
	time -= (unsigned int) days * 86400;

	/* day of the week, 1970-01-01 was a Thursday */
	tm->tm_wday = (days + 4) % 7;

	year = 1970 + days / 365;
	days -= (year - 1970) * 365
		+ LEAPS_THRU_END_OF(year - 1)
		- LEAPS_THRU_END_OF(1970 - 1);
	if (days < 0) {
		year -= 1;
		days += 365 + is_leap_year(year);
	}
	tm->tm_year = year - 1900;
	tm->tm_yday = days + 1;

	for (month = 0; month < 11; month++) {
		int newdays;

		newdays = days - rtc_month_days(month, year);
		if (newdays < 0)
			break;
		days = newdays;
	}
	tm->tm_mon = month;
	tm->tm_mday = days + 1;

	tm->tm_hour = time / 3600;
	time -= tm->tm_hour * 3600;
	tm->tm_min = time / 60;
	tm->tm_sec = time - tm->tm_min * 60;
}

#if(0)
//fixme ...add sync to rtc core...
static u32 _read_hw_time_data(struct fh_rtc_controller *fh_rtc)
{
	return rtc_readl(fh_rtc, RTC_COUNTER);
}

//fixme ...add sync to rtc core...
static void _set_hw_time_data(struct fh_rtc_controller *fh_rtc, u32 value)
{

	rtc_writel(fh_rtc, RTC_COUNTER, value);
}

//fixme ...add sync to rtc core...
static u32 _read_hw_alarm_data(struct fh_rtc_controller *fh_rtc)
{
	return rtc_readl(fh_rtc, ALARM_COUNTER);
}

//fixme ...add sync to rtc core...
static void _set_hw_alarm_data(struct fh_rtc_controller *fh_rtc, u32 value)
{
	rtc_writel(fh_rtc, ALARM_COUNTER, value);
}
#endif


u32 rtc_month_days(unsigned int month, unsigned int year)
{
	return rtc_days_in_month[month] + (is_leap_year(year) && month == 1);
}

static u32 fh_rtc_get_hw_sec_data(struct fh_rtc_controller *fh_rtc,
                u32 func_switch)
{

	u32 ret_sec;
	u32 raw_value;
	u32 sec_value;
	u32 min_value;
	u32 hour_value;
	u32 day_value;

	if (func_switch == TIME_FUNC)
		//raw_value = _read_hw_time_data(fh_rtc);
		raw_value = fh_rtc_get_time(fh_rtc->pregs);
	else
		raw_value = fh_rtc_get_alarm_time(fh_rtc->pregs);

	sec_value = FH_GET_RTC_SEC(raw_value);
	min_value = FH_GET_RTC_MIN(raw_value);
	hour_value = FH_GET_RTC_HOUR(raw_value);
	day_value = FH_GET_RTC_DAY(raw_value);



	RTC_PRINT_DBG("hw read day value is %d\n",day_value);

	ret_sec = (day_value * 86400) + (hour_value * 3600) + (min_value * 60)
	                + sec_value;
	return ret_sec;

}

static void fh_rtc_set_hw_sec_data(struct fh_rtc_controller *fh_rtc,
                struct rtc_time *rtc_tm, u32 func_switch)
{
	u32 raw_value;

	u32 sec_value;
	u32 min_value;
	u32 hour_value;
	u32 day_value;

	fh_rtc_dump_current_data(rtc_tm);
	day_value = rtc_year_days(rtc_tm->tm_mday, rtc_tm->tm_mon,
	                rtc_tm->tm_year);

	hour_value = rtc_tm->tm_hour;
	min_value = rtc_tm->tm_min;
	sec_value = rtc_tm->tm_sec;

	raw_value = (day_value << DAY_BIT_START)
	                | (hour_value << HOUR_BIT_START)
	                | (min_value << MIN_BIT_START)
	                | (sec_value << MIN_BIT_START);

	RTC_PRINT_DBG("set time data is %x\n", raw_value);
	if (func_switch == TIME_FUNC)
		//_set_hw_time_data(fh_rtc, raw_value);
		fh_rtc_set_time(fh_rtc->pregs,raw_value);
	else
		//_set_hw_alarm_data(fh_rtc, raw_value);
		fh_rtc_set_alarm_time(fh_rtc->pregs,raw_value);

}

// return hw total sec value..
static u32 fh_rtc_get_hw_time_sec_data(struct fh_rtc_controller *fh_rtc)
{

	return fh_rtc_get_hw_sec_data(fh_rtc, TIME_FUNC);
}

static void fh_rtc_set_hw_time_data(struct fh_rtc_controller *fh_rtc,
                struct rtc_time *rtc_tm)
{

	fh_rtc_set_hw_sec_data(fh_rtc, rtc_tm, TIME_FUNC);
}







//uboot app call func..

int rtc_get(struct rtc_time *tmp)
{
	RTC_PRINT_DBG("%s\n", __func__);

	//first get hw data ...if year is 0...set to 1970...
	u32 temp_sec;
	temp_sec = fh_rtc_get_hw_time_sec_data(&g_rtc_control);

	rtc_time_to_tm(temp_sec, tmp);
	fh_rtc_dump_current_data(tmp);

	return 0;
}

int rtc_set(struct rtc_time *tmp)
{

	RTC_PRINT_DBG("%s\n", __func__);


	fh_rtc_set_hw_time_data(&g_rtc_control,tmp);
	fh_rtc_dump_current_data(tmp);

	return 0;
}

void rtc_reset(void)
{
	struct rtc_time tmp = {0};
	//set hw cnt back to 0;
	RTC_PRINT_DBG("%s\n", __func__);

	tmp.tm_year = g_rtc_control.base_year;
	tmp.tm_mon = g_rtc_control.base_month;
	tmp.tm_mday= g_rtc_control.base_day;
	fh_rtc_set_hw_time_data(&g_rtc_control,&tmp);

	fh_rtc_dump_regs(&g_rtc_control);
	return;

}
