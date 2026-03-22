/* SPDX-License-Identifier: GPL-2.0-or-later */
/* 
 * Copyright (C) 2024. All Rights Reserved.
 * Extreme Performance KPM - Thermal Bypass
 */

#include <log.h>
#include <compiler.h>
#include <kpmodule.h>
#include <hook.h>
#include <linux/printk.h>
#include <kputils.h>

KPM_NAME("kpm-performance-op9p");
KPM_VERSION("1.0.0");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("Salman");
KPM_DESCRIPTION("KernelPatch Module for Extreme Performance (Thermal Bypass)");

void *thermal_zone_get_temp_ptr = 0;

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

static long performance_init(const char *args, const char *event, void *__user reserved)
{
    logkd("kpm performance op9p init\n");

    // Locate the kernel function
    thermal_zone_get_temp_ptr = (void *)kallsyms_lookup_name("thermal_zone_get_temp");
    
    if (thermal_zone_get_temp_ptr) {
        // Wrap the function and hook the 'after' execution
        hook_err_t err = hook_wrap2(thermal_zone_get_temp_ptr, 0, after_thermal_zone_get_temp, 0);
        logkd("thermal_zone_get_temp hook status: %d\n", err);
        if (err == 0) {
            pr_info("[KPM] Thermal bypass successfully activated! Temp locked at 35C.\n");
        }
    } else {
        pr_warn("[KPM] Failed to find thermal_zone_get_temp!\n");
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
    logkd("kpm performance op9p exit\n");
    return 0;
}

KPM_INIT(performance_init);
KPM_CTL0(performance_control0);
KPM_EXIT(performance_exit);
