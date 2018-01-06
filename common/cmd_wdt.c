#include <common.h>
#include <command.h>
#include <asm/io.h>
#ifdef CONFIG_CMD_WDT

#define WDT_THRESHOLD    5000
extern void hw_watchdog_init(unsigned int timeout);

int do_wdt(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
        unsigned long user_time_ms;
        char *endp;
        user_time_ms = simple_strtoul(argv[1], &endp, 0);
        if(user_time_ms < WDT_THRESHOLD)
            user_time_ms = WDT_THRESHOLD;

        hw_watchdog_init(user_time_ms);
        return 0;
}

U_BOOT_CMD(
                wdt, 5, 1, do_wdt,
                "WDT utility commands",
                "wdt time    - `time' means that: if the software DO NOT kick the WDT in 'time'(unit:ms), the WDT will reset the SOC.\n"
                "if the WDT is not enable,when you call the cmd, the WDT will work immediately.\n"
);

#endif


