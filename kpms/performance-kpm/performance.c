/* SPDX-License-Identifier: GPL-2.0-or-later */
/* 
 * OP9Pro Extreme Performance KPM (Controlled Edition)
 */

#include <compiler.h>
#include <kpmodule.h>
#include <linux/printk.h>
#include <common.h>
#include <kputils.h>
#include <linux/string.h>
#include <hook.h>

KPM_NAME("perf-op9p");
KPM_VERSION("4.0.0");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("Salman");
KPM_DESCRIPTION("OP9P Thermal & CPU Boost (Controlled)");

void *thermal_zone_get_temp_ptr = 0;
void *cpufreq_driver_target_ptr = 0;

typedef hook_err_t (*hook_wrap2_func_t)(void *func, void *before, void *after, void* udata);
typedef hook_err_t (*hook_wrap3_func_t)(void *func, void *before, void *after, void* udata);
typedef hook_err_t (*unhook_func_t)(void *func);

hook_wrap2_func_t p_hook_wrap2 = 0;
hook_wrap3_func_t p_hook_wrap3 = 0;
unhook_func_t p_unhook = 0;

// Global flag to control the performance mode safely
static bool is_gaming_mode = false;

void after_thermal_zone_get_temp(hook_fargs2_t *args, void *udata)
{
    if (!is_gaming_mode) return; // Pass-through normally if not gaming

    int *temp = (int *)args->arg1;
    if (temp) {
        *temp = 35000;
    }
    args->ret = 0;
}

void before_cpufreq_driver_target(hook_fargs3_t *args, void *udata)
{
    if (!is_gaming_mode) return; // Pass-through normally if not gaming

    args->arg1 = 3000000;
}

static long kpm_init(const char *args, const char *event, void *__user reserved)
{
    pr_info("perf-op9p: loaded, kpver=0x%x\n", kpver);
    pr_info("perf-op9p: Booted in NORMAL mode (100%% Safe). Use APatch Control to enable GAMING mode.\n");

    p_hook_wrap2 = (hook_wrap2_func_t)kallsyms_lookup_name("hook_wrap2");
    p_hook_wrap3 = (hook_wrap3_func_t)kallsyms_lookup_name("hook_wrap3");
    p_unhook = (unhook_func_t)kallsyms_lookup_name("unhook");

    if (!p_hook_wrap2 || !p_unhook) {
        pr_warn("perf-op9p: Failed to find hook_wrap2 or unhook. Hooks will be disabled.\n");
        return 0;
    }

    thermal_zone_get_temp_ptr = (void *)kallsyms_lookup_name("thermal_zone_get_temp");
    if (thermal_zone_get_temp_ptr) {
        hook_err_t err = p_hook_wrap2(thermal_zone_get_temp_ptr, 0, after_thermal_zone_get_temp, 0);
        if (err == 0) pr_info("perf-op9p: Thermal bypass hook placed (Currently Sleeping).\n");
    }

    if (p_hook_wrap3) {
        cpufreq_driver_target_ptr = (void *)kallsyms_lookup_name("cpufreq_driver_target");
        if (cpufreq_driver_target_ptr) {
            hook_err_t err = p_hook_wrap3(cpufreq_driver_target_ptr, before_cpufreq_driver_target, 0, 0);
            if (err == 0) pr_info("perf-op9p: CPU Boost hook placed (Currently Sleeping).\n");
        }
    }

    // Default to off at boot
    is_gaming_mode = false;
    return 0;
}

static long kpm_ctl0(const char *args, char *__user out_msg, int outlen)
{
    if (!args) return 0;

    if (strcmp(args, "gaming") == 0) {
        is_gaming_mode = true;
        pr_info("perf-op9p: ---> GAMING MODE ACTIVATED! <---\n");
        if (out_msg && outlen > 0) {
            compat_copy_to_user(out_msg, "Gaming Mode ON!", 16);
        }
    } 
    else if (strcmp(args, "normal") == 0) {
        is_gaming_mode = false;
        pr_info("perf-op9p: ---> NORMAL MODE RESTORED! <---\n");
        if (out_msg && outlen > 0) {
            compat_copy_to_user(out_msg, "Normal Mode ON!", 16);
        }
    }
    else {
        pr_info("perf-op9p: Unknown command received: %s\n", args);
    }
    
    return 0;
}

static long kpm_ctl1(void *a1, void *a2, void *a3) { return 0; }

static long kpm_exit(void *__user reserved)
{
    if (p_unhook) {
        if (thermal_zone_get_temp_ptr) p_unhook(thermal_zone_get_temp_ptr);
        if (cpufreq_driver_target_ptr) p_unhook(cpufreq_driver_target_ptr);
    }
    pr_info("perf-op9p: unloaded\n");
    return 0;
}

KPM_INIT(kpm_init);
KPM_CTL0(kpm_ctl0);
KPM_CTL1(kpm_ctl1);
KPM_EXIT(kpm_exit);