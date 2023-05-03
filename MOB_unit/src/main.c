#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <nrf_modem_gnss.h>
#include <modem/lte_lc.h>

#include <zephyr/net/coap.h>

K_SEM_DEFINE(lte_connected, 0, 1);
K_SEM_DEFINE(gnss_fix_sem, 0, 1);

// LTE link controller library:
#include <modem/lte_lc.h>


//define thee coap message token:
static uint16_t next_token;



// LOGGING:
LOG_MODULE_REGISTER(MOB_unit, LOG_LEVEL_INF);

static void gnss_event_handler(int event)
{
    switch (event)
    {
    case NRF_MODEM_GNSS_EVT_PVT:
        k_sem_give(&gnss_fix_sem);
        break;
    default:
        break;
    }
}

void send_data_to_nrf_cloud(void)
{
    struct coap_packet request;
    static uint8_t payload[] = "Hello World!";

    next_token++;
}

static void lte_event_handler(const struct lte_lc_evt *const evt)
{
    switch (evt->type)
    {
    case LTE_LC_EVT_NW_REG_STATUS:
        if (evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_HOME &&
            evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_ROAMING)
        { // either conected to home or roaming to continue
            printk("Network registration status: Connected\n");
        }
        else
        {
            printk("Network registration status: Not connected\n");
        }
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


    default: // if not any of the above, do nothing  
        break;
    }
}


int main(void) // can also use void main(void)?
{
    

    while (1)
    {
        k_sem_take(&gnss_fix_sem, K_FOREVER);
        //retrieve the PVT data
        struct nrf_modem_gnss_pvt_data_frame pvt;
        int err = nrf_modem_gnss_read(&pvt, sizeof(pvt), NRF_MODEM_GNSS_DATA_PVT);
        if (err)
        {
            printk("Failed to read PVT data, error: %d\n", err);
            continue;
        }
        //convert latitude and longitude to string
        char location[32];
        snprintf(location, sizeof(location), "%f,%f", pvt.latitude, pvt.longitude);

        send_location_data_over_coap(location);

        k_sleep(K_SECONDS(1));  // sleep for 1 second
    }
    lte_lc_power_off();
    LOG_ERR("Error. Shutting down.");

}


