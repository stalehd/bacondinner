/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include "sysinit/sysinit.h"
#include "os/os.h"
#include "console/console.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"

static const char *device_name = "the_bacon";

// Measure from 1m distance
const uint8_t measured_tx_power = 0;

static void ble_app_set_addr()
{
    ble_addr_t addr;
    int rc;

    rc = ble_hs_id_gen_rnd(1, &addr);
    assert(rc == 0);

    rc = ble_hs_id_set_rnd(addr.val);
    assert(rc == 0);
}

static void ble_app_advertise()
{
    console_printf("Advertising!\n");
    struct ble_gap_adv_params adv_params;

    uint8_t uuid128[16];
    int rc;

    /* Fill the UUID buffer with a string of 0x11 bytes. */
    memset(uuid128, 0x12, sizeof uuid128);

    /* Major version=2; minor version=10. */
    rc = ble_ibeacon_set_adv_data(uuid128, 2, 10, measured_tx_power);
    if (rc != 0)
    {
        console_printf("Unable to set adv data\n");
    }
    assert(rc == 0);

    /* Begin advertising. */
    adv_params = (struct ble_gap_adv_params){0};
    rc = ble_gap_adv_start(BLE_OWN_ADDR_RANDOM, NULL, BLE_HS_FOREVER,
                           &adv_params, NULL, NULL);
    if (rc != 0)
    {
        console_printf("Unable to advertise!\n");
    }
    assert(rc == 0);
    console_printf("Advertising done\n");
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
    int rc;

    sysinit();

    console_printf("Hello world, I'm the bacon!\n");
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    rc = ble_svc_gap_device_name_set(device_name);
    while (1)
    {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}
