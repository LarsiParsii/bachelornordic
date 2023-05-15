#include "lte.h"
#include <zephyr/kernel.h>
#include <modem/lte_lc.h>
#include <stdio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LTE, LOG_LEVEL_INF);

/* SEMAPHORES */
K_SEM_DEFINE(lte_connected, 0, 1);

/* FUNCTION DEFINITIONS */
static void lte_handler(const struct lte_lc_evt *const evt)
{
	switch (evt->type)
	{
	case LTE_LC_EVT_NW_REG_STATUS:
		if ((evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_HOME) &&
			(evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_ROAMING))
		{
			break;
		}

		LOG_INF("Network registration status: %s\n",
				evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ? "Connected - home network" : "Connected - roaming\n");
		k_sem_give(&lte_connected);
		break;
	case LTE_LC_EVT_PSM_UPDATE:
		LOG_INF("PSM parameter update: TAU: %d, Active time: %d\n",
				evt->psm_cfg.tau, evt->psm_cfg.active_time);
		break;
	case LTE_LC_EVT_EDRX_UPDATE:
	{
		char log_buf[60];
		ssize_t len;

		len = snprintf(log_buf, sizeof(log_buf),
					   "eDRX parameter update: eDRX: %f, PTW: %f\n",
					   evt->edrx_cfg.edrx, evt->edrx_cfg.ptw);
		if (len > 0)
		{
			LOG_INF("%s\n", log_buf);
		}
		break;
	}
	case LTE_LC_EVT_RRC_UPDATE:
		LOG_INF("RRC mode: %s\n",
				evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED ? "Connected" : "Idle\n");
		break;
	case LTE_LC_EVT_CELL_UPDATE:
		LOG_INF("LTE cell changed: Cell ID: %d, Tracking area: %d\n",
				evt->cell.id, evt->cell.tac);
		break;
	default:
		break;
	}
}

void modem_configure(void)
{
	int err;

	err = lte_lc_init_and_connect_async(lte_handler);
	if (err)
	{
		LOG_ERR("Modem could not be configured, error: %d", err);
		return;
	}
	/* Decativate LTE and enable GNSS. */
	if (lte_lc_func_mode_set(LTE_LC_FUNC_MODE_DEACTIVATE_LTE) != 0)
	{
		LOG_ERR("Failed to decativate LTE and enable GNSS functional mode");
		return;
	}
}