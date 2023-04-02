#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* LTE link controller library: */
#include <modem/lte_lc.h>


static void lte_event_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type) {
        case LTE_LC_EVT_NW_REG_STATUS:
            if (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ||
                evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING) {
                printk("Network registration status: Connected\n");
            } else {
                printk("Network registration status: Not connected\n");
            }
            break;

        case LTE_LC_EVT_PSM_UPDATE:
            printk("PSM parameters updated: TAU: %d, Active time: %d\n",
                   evt->psm_cfg.tau, evt->psm_cfg.active_time);
            break;
        case LTE_LC_EVT_RRC_UPDATE:
            printk("RRC mode: %s\n",
                   evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED ? "Connected" :
                   "Idle");
            break;

        case LTE_LC_EVT_CELL_UPDATE:
            printk("Cell update: TAC: %u, Cell ID: %u\n",
                   evt->cell.tac, evt->cell.id);
            break;
        case LTE_LC_EVT_LTE_MODE_UPDATE:
            printk("LTE mode: %s\n",
                evt->lte_mode == LTE_LC_LTE_MODE_NONE ? "None" :
                evt->lte_mode == LTE_LC_LTE_MODE_LTEM ? "LTE-M" :
                "NB-IoT");
            break;
        case LTE_LC_EVT_TAU_PRE_WARNING:
            printk("TAU pre-warning: %llu seconds until TAU\n", evt->time);
            break;
        case LTE_LC_EVT_MODEM_SLEEP_EXIT_PRE_WARNING:
            printk("Modem sleep exit pre-warning: %llu seconds until modem exits sleep\n",
                evt->modem_sleep.time);
            break;
        case LTE_LC_EVT_MODEM_SLEEP_EXIT:
            printk("Modem sleep exit\n");
            break;

        case LTE_LC_EVT_MODEM_SLEEP_ENTER:
            printk("Modem sleep enter: Duration %llu seconds\n",
               evt->modem_sleep.time);
            break;


    default:
        break;
}