#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* LTE link controller library: */
#include <modem/lte_lc.h>


LOG_MODULE_REGISTER(MOB_unit, LOG_LEVEL_INF);



static void lte_event_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type) {
        case LTE_LC_EVT_NW_REG_STATUS:
            if (evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_HOME &&
                evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_ROAMING) { //either conected to home or roaming to continue
                printk("Network registration status: Connected\n");
            } else {
                printk("Network registration status: Not connected\n");
            }
            break;
        case LTE_LC_EVT_LTE_MODE_UPDATE:
            printk("LTE mode: %s\n",
                evt->lte_mode == LTE_LC_LTE_MODE_NONE ? "None" :
                evt->lte_mode == LTE_LC_LTE_MODE_LTEM ? "LTE-M" :
                "NB-IoT");
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
}



static void modem_configure(void)
{
    int err = lte_lc_init_and_connect_async(lte_event_handler);
    if (err) {
        printk("Modem could not be configured, error: %d\n", err);
        return;
    }
}




int main(void) //can also use void main(void)?
{

    modem_configure();

    
    LOG_INF("Connected to LTE network");

    while(1){


    }
}

