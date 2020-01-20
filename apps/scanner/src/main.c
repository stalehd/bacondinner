#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_hs_adv.h"

static void ble_app_set_addr(void)
{
    ble_addr_t addr;
    int rc;

    rc = ble_hs_id_gen_rnd(1, &addr);
    assert(rc == 0);

    rc = ble_hs_id_set_rnd(addr.val);
    assert(rc == 0);
}

static uint64_t devices[10];
static uint8_t didx = 0;

static bool add_addr(ble_addr_t addr)
{
    uint64_t *tmp = (uint64_t *)&addr;
    uint64_t val = *tmp;
    for (int i = 0; i < 10; i++)
    {
        if (devices[i] == val)
        {
            return false;
        }
    }
    if (didx >= 10)
    {
        didx = 0;
    }
    devices[didx++] = val;
    return true;
}
static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    if (event->type == BLE_GAP_EVENT_DISC)
    {
        if (add_addr(event->disc.addr))
        {
            console_printf("Found BLE device: %02x%02x%02x %02x%02x%02x  RSSI: %d data=%d\n",
                           event->disc.addr.val[0], event->disc.addr.val[1], event->disc.addr.val[2],
                           event->disc.addr.val[3], event->disc.addr.val[4], event->disc.addr.val[5],
                           event->disc.rssi, event->disc.length_data);
        }
        /*
                02 01 06 1A FF 4C 00 02 15: iBeacon prefix (fixed except for 3rd byte - flags)
                B9 40 7F 30 F5 F8 46 6E AF F9 25 55 6B 57 FE 6D: proximity UUID (here: Estimote’s fixed UUID)
                00 49: major
                00 0A: minor
                C5: 2’s complement of measured TX power
            */
        if (event->disc.addr.val[0] == 0x4c && event->disc.addr.val[1] == 0x00)
        {
            // Apple identifier
            if (event->disc.addr.val[2] == 0x02 && event->disc.addr.val[3] == 0x15)
            {
                // iBeacon identifier
                console_printf("Found iBeacon rssi=%d\n", event->disc.rssi);
            }
        }
        return 0;
    }
    if (event->type == BLE_GAP_EVENT_EXT_DISC)
    {

        if (event->ext_disc.length_data > 25)
        {
            if (event->ext_disc.data[4] == 0xff && event->ext_disc.data[7] == 0x02 && event->ext_disc.data[5] == 0x4c && event->ext_disc.data[6] == 0x00)
            {
                // 02 01 06 1a ff 4c 00 02 15
                // 01 12 23 34 45 56 67 78 89 9a ab bc cd de ef f0 16
                // 00 02 00 0a 00
                console_printf("iBeacon major: %02x%02x minor: %02x%02x RSSI: %d (tx power=%02x)\n",
                               event->ext_disc.data[25], event->ext_disc.data[26],
                               event->ext_disc.data[27], event->ext_disc.data[28],
                               event->ext_disc.rssi,
                               event->ext_disc.data[29]);
            }
        }

        return 0;
    }
    console_printf("GAP event %d\n", event->type);
    return 0;
}

static void ble_app_discover(void)
{
    int rc;
    struct ble_gap_disc_params disc_params = {
        .itvl = 0,
        .window = 0,
        .filter_policy = 0,
        .limited = 0,
        .passive = 1,
        .filter_duplicates = 0,

    };
    /* struct ble_gap_ext_disc_params disc_params = {
        .itvl = 0,
        .window = 0,
        .passive = 0,
    };*/

    uint8_t own_addr_type;
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0)
    {
        console_printf("Error inferring auto address");
    }
    /* rc = ble_gap_ext_disc(own_addr_type, 0, 0, 0, 0, 0,
                          NULL, NULL, gap_event_cb, NULL);*/
    rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params, gap_event_cb, NULL);
    if (rc != 0)
    {
        console_printf("Error starting discovery: %d\n", rc);
    }
}

static void ble_app_on_sync(void)
{
    /* Generate a non-resolvable private address. */
    ble_app_set_addr();

    /* scan for devices */
    ble_app_discover();

    console_printf("Sync completed\n");
}

int main(int argc, char **argv)
{
    /* Initialize all packages. */
    sysinit();

    ble_hs_cfg.sync_cb = ble_app_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    console_printf("Starting scanner...\n");
    /* As the last thing, process events from default event queue. */
    while (1)
    {
        os_eventq_run(os_eventq_dflt_get());
    }

    return 0;
}
