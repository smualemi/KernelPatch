/* SPDX-License-Identifier: GPL-2.0-or-later */
/* 
 * Copyright (C) 2024. All Rights Reserved.
 * Extreme Performance KPM - Thermal Bypass & CPU Boost
 */

#include <log.h>
#include <compiler.h>
#include <kpmodule.h>
#include <hook.h>
#include <linux/printk.h>
#include <kputils.h>

KPM_NAME("kpm-performance-op9p");
KPM_VERSION("2.0.0");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("Salman");
KPM_DESCRIPTION("KernelPatch Module for Extreme Performance (Thermal Bypass & CPU Boost)");

void *thermal_zone_get_temp_ptr = 0;
void *cpufreq_driver_target_ptr = 0;

/* Hook the after execution of thermal_zone_get_temp to always return 35 degrees */
void after_thermal_zone_get_temp(hook_fargs2_t *args, void *udata)
{
    int *temp = (int *)args->arg1;
    if (temp) {
        *temp = 35000; // 35000 millidegrees = 35°C
    }
    
    // Setting return value to 0 (Success)
    args->ret = 0;
}

/* Hook cpufreq_driver_target to force maximum requested frequency */
void before_cpufreq_driver_target(hook_fargs3_t *args, void *udata)
{
    // The second argument is target_freq. By setting it to a massively high value, 
    // the cpufreq driver will automatically clamp it to the highest supported frequency.
    args->arg1 = (uint64_t)3000000; // Request 3,000,000 KHz (3 GHz)
}

static long performance_init(const char *args, const char *event, void *__user reserved)
{
    logkd("kpm performance op9p init\n");

    // 1. Thermal Bypass Hook
    thermal_zone_get_temp_ptr = (void *)kallsyms_lookup_name("thermal_zone_get_temp");
    
    if (thermal_zone_get_temp_ptr) {
        hook_err_t err = hook_wrap2(thermal_zone_get_temp_ptr, 0, after_thermal_zone_get_temp, 0);
        if (err == 0) {
            pr_info("[KPM] Thermal bypass successfully activated! Temp locked at 35C.\n");
        }
    } else {
        pr_warn("[KPM] Failed to find thermal_zone_get_temp!\n");
    }

    // 2. CPU Boost Hook
    cpufreq_driver_target_ptr = (void *)kallsyms_lookup_name("cpufreq_driver_target");
    if (cpufreq_driver_target_ptr) {
        hook_err_t err = hook_wrap3(cpufreq_driver_target_ptr, before_cpufreq_driver_target, 0, 0);
        if (err == 0) {
            pr_info("[KPM] CPU Boost successfully activated! Forcing Max Freq.\n");
        }
    } else {
        pr_warn("[KPM] Failed to find cpufreq_driver_target!\n");
    }

    return 0;
}

static long performance_control0(const char *args, char *__user out_msg, int outlen)
{
    pr_info("kpm performance control, args: %s\n", args);
    return 0;
}

static long performance_exit(void *__user reserved)
{
    if (thermal_zone_get_temp_ptr) {
        unhook(thermal_zone_get_temp_ptr);
        pr_info("[KPM] Thermal bypass deactivated!\n");
    }
    if (cpufreq_driver_target_ptr) {
        unhook(cpufreq_driver_target_ptr);
        pr_info("[KPM] CPU Boost deactivated!\n");
    }
    logkd("kpm performance op9p exit\n");
    return 0;
}

KPM_INIT(performance_init);
KPM_CTL0(performance_control0);
KPM_EXIT(performance_exit);