/* SPDX-License-Identifier: GPL-2.0-or-later */
/* 
 * OP9Pro Extreme Performance KPM
 */

#include <compiler.h>
#include <kpmodule.h>
#include <linux/printk.h>
#include <common.h>
#include <kputils.h>
#include <linux/string.h>
#include <hook.h>

KPM_NAME("perf-op9p");
KPM_VERSION("2.1.0");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("Salman");
KPM_DESCRIPTION("OP9P Thermal & CPU Boost");

void *thermal_zone_get_temp_ptr = 0;
void *cpufreq_driver_target_ptr = 0;

void after_thermal_zone_get_temp(hook_fargs2_t *args, void *udata)
{
    int *temp = (int *)args->arg1;
    if (temp) {
        *temp = 35000;
    }
    args->ret = 0;
}

void before_cpufreq_driver_target(hook_fargs3_t *args, void *udata)
{
    args->arg1 = 3000000;
}

static long kpm_init(const char *args, const char *event, void *__user reserved)
{
    pr_info("perf-op9p: loaded, kpver=0x%x\n", kpver);

    thermal_zone_get_temp_ptr = (void *)kallsyms_lookup_name("thermal_zone_get_temp");
    if (thermal_zone_get_temp_ptr) {
        hook_err_t err = hook_wrap2(thermal_zone_get_temp_ptr, 0, after_thermal_zone_get_temp, 0);
        if (err == 0) pr_info("perf-op9p: Thermal bypass activated.\n");
    }

    cpufreq_driver_target_ptr = (void *)kallsyms_lookup_name("cpufreq_driver_target");
    if (cpufreq_driver_target_ptr) {
        hook_err_t err = hook_wrap3(cpufreq_driver_target_ptr, before_cpufreq_driver_target, 0, 0);
        if (err == 0) pr_info("perf-op9p: CPU Boost activated.\n");
    }

    return 0;
}

static long kpm_ctl0(const char *args, char *__user out_msg, int outlen)
{
    pr_info("perf-op9p: ctl0 args=%s\n", args ? args : "(null)");
    return 0;
}

static long kpm_ctl1(void *a1, void *a2, void *a3) { return 0; }

static long kpm_exit(void *__user reserved)
{
    if (thermal_zone_get_temp_ptr) {
        unhook(thermal_zone_get_temp_ptr);
    }
    if (cpufreq_driver_target_ptr) {
        unhook(cpufreq_driver_target_ptr);
    }
    pr_info("perf-op9p: unloaded\n");
    return 0;
}

KPM_INIT(kpm_init);
KPM_CTL0(kpm_ctl0);
KPM_CTL1(kpm_ctl1);
KPM_EXIT(kpm_exit);
