/* SPDX-License-Identifier: GPL-2.0-or-later */
/* 
 * OP9Pro Extreme Performance KPM (Advanced Engine)
 */

#include <compiler.h>
#include <kpmodule.h>
#include <linux/printk.h>
#include <common.h>
#include <kputils.h>
#include <linux/string.h>
#include <hook.h>

KPM_NAME("perf-op9p");
KPM_VERSION("5.0.0");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("Salman");
KPM_DESCRIPTION("OP9P Advanced Gaming Engine");

void *thermal_zone_get_temp_ptr = 0;
void *cpufreq_driver_target_ptr = 0;
void *power_supply_set_property_ptr = 0;
void *vfs_fsync_ptr = 0;

typedef hook_err_t (*hook_wrap2_func_t)(void *func, void *before, void *after, void* udata);
typedef hook_err_t (*hook_wrap3_func_t)(void *func, void *before, void *after, void* udata);
typedef hook_err_t (*unhook_func_t)(void *func);

hook_wrap2_func_t p_hook_wrap2 = 0;
hook_wrap3_func_t p_hook_wrap3 = 0;
unhook_func_t p_unhook = 0;

static bool is_gaming_mode = false;

void after_thermal_zone_get_temp(hook_fargs2_t *args, void *udata)
{
    if (!is_gaming_mode) return; 
    int *temp = (int *)args->arg1;
    if (temp) *temp = 35000;
    args->ret = 0;
}

void before_cpufreq_driver_target(hook_fargs3_t *args, void *udata)
{
    if (!is_gaming_mode) return; 
    args->arg1 = 3000000;
}

union power_supply_propval {
    int intval;
    const char *strval;
};

void before_power_supply_set_property(hook_fargs3_t *args, void *udata)
{
    if (!is_gaming_mode) return;
    union power_supply_propval *val = (union power_supply_propval *)args->arg2;
    // Force Warp Charge limits (cap requests heavily up to 6500mA)
    if (val && val->intval > 1000000 && val->intval < 6500000) {
        val->intval = 6500000; 
    }
}

void before_vfs_fsync(hook_fargs2_t *args, void *udata)
{
    if (!is_gaming_mode) return;
    args->arg1 = 0; // Force datasync=0 to prevent blocking IO during gaming
}

static long kpm_init(const char *args, const char *event, void *__user reserved)
{
    pr_info("perf-op9p: loaded, kpver=0x%x\n", kpver);

    p_hook_wrap2 = (hook_wrap2_func_t)kallsyms_lookup_name("hook_wrap2");
    p_hook_wrap3 = (hook_wrap3_func_t)kallsyms_lookup_name("hook_wrap3");
    p_unhook = (unhook_func_t)kallsyms_lookup_name("unhook");

    if (!p_hook_wrap2 || !p_unhook) {
        pr_warn("perf-op9p: Dynamic hooks missing!\n");
        return 0;
    }

    thermal_zone_get_temp_ptr = (void *)kallsyms_lookup_name("thermal_zone_get_temp");
    if (thermal_zone_get_temp_ptr) {
        p_hook_wrap2(thermal_zone_get_temp_ptr, 0, after_thermal_zone_get_temp, 0);
    }

    if (p_hook_wrap3) {
        cpufreq_driver_target_ptr = (void *)kallsyms_lookup_name("cpufreq_driver_target");
        if (cpufreq_driver_target_ptr) {
            p_hook_wrap3(cpufreq_driver_target_ptr, before_cpufreq_driver_target, 0, 0);
        }

        power_supply_set_property_ptr = (void *)kallsyms_lookup_name("power_supply_set_property");
        if (power_supply_set_property_ptr) {
            p_hook_wrap3(power_supply_set_property_ptr, before_power_supply_set_property, 0, 0);
        }
    }

    vfs_fsync_ptr = (void *)kallsyms_lookup_name("vfs_fsync");
    if (vfs_fsync_ptr) {
        p_hook_wrap2(vfs_fsync_ptr, before_vfs_fsync, 0, 0);
    }

    is_gaming_mode = false;
    return 0;
}

static long kpm_ctl0(const char *args, char *__user out_msg, int outlen)
{
    if (!args) return 0;

    if (strcmp(args, "gaming") == 0) {
        is_gaming_mode = true;
        pr_info("perf-op9p: ---> ADVANCED GAMING ENGINE ACTIVATED! <---\n");
        if (out_msg && outlen > 0) compat_copy_to_user(out_msg, "Gaming Engine ON", 17);
    } 
    else if (strcmp(args, "normal") == 0) {
        is_gaming_mode = false;
        pr_info("perf-op9p: ---> NORMAL MODE RESTORED! <---\n");
        if (out_msg && outlen > 0) compat_copy_to_user(out_msg, "Normal Mode ON", 15);
    }
    return 0;
}

static long kpm_ctl1(void *a1, void *a2, void *a3) { return 0; }

static long kpm_exit(void *__user reserved)
{
    if (p_unhook) {
        if (thermal_zone_get_temp_ptr) p_unhook(thermal_zone_get_temp_ptr);
        if (cpufreq_driver_target_ptr) p_unhook(cpufreq_driver_target_ptr);
        if (power_supply_set_property_ptr) p_unhook(power_supply_set_property_ptr);
        if (vfs_fsync_ptr) p_unhook(vfs_fsync_ptr);
    }
    return 0;
}

KPM_INIT(kpm_init);
KPM_CTL0(kpm_ctl0);
KPM_CTL1(kpm_ctl1);
KPM_EXIT(kpm_exit);