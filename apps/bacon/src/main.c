#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "host/ble_hs.h"
#include "host/ble_ibeacon.h"
#include "controller/ble_phy.h"

const uint8_t tx_power = 0;

#define PHY_TX_POWER 30

#define MAJOR_VERSION 2
#define MINOR_VERSION 11

static void ble_app_set_addr(void)
{
    ble_addr_t addr;
    int rc;

    rc = ble_hs_id_gen_rnd(1, &addr);
    assert(rc == 0);

    rc = ble_hs_id_set_rnd(addr.val);
    assert(rc == 0);
}

static int on_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_ADV_COMPLETE:
        console_printf("GAP Advertising complete\n");
        return 0;

    case BLE_GAP_EVENT_CONNECT:
        console_printf("GAP connect\n");
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        console_printf("GAP disconnect\n");
        return 0;

    default:
        console_printf("GAP event %d\n", event->type);
        return 0;
    }
}

static void ble_app_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    uint8_t uuid128[16] = {0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x9A, 0xAB, 0xBC, 0xCD, 0xDE, MAJOR_VERSION, MINOR_VERSION};
    int rc;

    /* Arbitrarily set the UUID to a string of 0x11 bytes. */
    //    memset(uuid128, 0x11, sizeof uuid128);

    /* Major version=2; minor version=10. */
    rc = ble_ibeacon_set_adv_data(uuid128, MAJOR_VERSION, MINOR_VERSION, tx_power);
    assert(rc == 0);

    /* Begin advertising. */
    adv_params = (struct ble_gap_adv_params){0};
    rc = ble_gap_adv_start(BLE_OWN_ADDR_RANDOM, NULL, BLE_HS_FOREVER,
                           &adv_params, on_gap_event, NULL);
    assert(rc == 0);
}

static void ble_app_on_sync(void)
{
    /* Generate a non-resolvable private address. */
    ble_app_set_addr();

    /* Advertise indefinitely. */
    ble_app_advertise();
}

int main(int argc, char **argv)
{
    sysinit();

    ble_hs_cfg.sync_cb = ble_app_on_sync;
    console_printf("PHY tx power = %d\n", ble_phy_txpwr_get());
    int rc = ble_phy_txpwr_set(PHY_TX_POWER);
    if (rc != 0)
    {
        console_printf("Got error setting tx power: %d\n", rc);
    }
    else
    {
        console_printf("TX power set to %d\n", PHY_TX_POWER);
    }
    /* As the last thing, process events from default event queue. */
    while (1)
    {
        os_eventq_run(os_eventq_dflt_get());
    }
}