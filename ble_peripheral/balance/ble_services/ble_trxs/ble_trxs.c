/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the license.txt file.
 */

#include "ble_trxs.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "app_error.h"
//#include "app_trace.h"

/**@brief Function for handling the Connect event.
 *
 * @param[in] p_trx      Inertial Measurement Unit Service structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
static void on_connect(ble_trx_t * p_trx, ble_evt_t * p_ble_evt)
{
    p_trx->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in] p_trx      Inertial Measurement Unit Service structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
static void on_disconnect(ble_trx_t * p_trx, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_trx->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Function for handling the Write event.
 *
 * @param[in] p_trx      Inertial Measurement Unit Service structure.
 * @param[in] p_ble_evt  Event received from the BLE stack.
 */
static void on_write(ble_trx_t * p_trx, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if ((p_evt_write->handle == p_trx->cmd_char_handles.value_handle) &&
            (p_evt_write->len <= IMU_CMD_CHAR_SIZE) &&
            (p_trx->cmd_write_handler != NULL))
    {
        p_trx->cmd_write_handler(p_trx, p_evt_write->data);
    }
}

void ble_trx_on_ble_evt(ble_trx_t * p_trx, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_trx, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_trx, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_trx, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for adding the quaternion characteristic.
 *
 * @param[in] p_trx      Inertial Measurement Unit Service structure.
 *
 * @retval NRF_SUCCESS on success, else an error value from the SoftDevice
 */

static uint32_t trx_char_add(ble_trx_t * p_trx)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_trx->uuid_type;
    ble_uuid.uuid = IMU_UUID_TRX_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = IMU_TRX_CHAR_SIZE;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = IMU_TRX_CHAR_SIZE;
    attr_char_value.p_value   = NULL;

    return sd_ble_gatts_characteristic_add(p_trx->service_handle,
            &char_md,
            &attr_char_value,
            &p_trx->trx_char_handles);
}

static uint32_t cmd_char_add(ble_trx_t * p_trx)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_trx->uuid_type;
    ble_uuid.uuid = IMU_UUID_CMD_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = IMU_CMD_CHAR_SIZE;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = IMU_CMD_CHAR_SIZE;
    attr_char_value.p_value      = NULL;

    return sd_ble_gatts_characteristic_add(p_trx->service_handle,
            &char_md,
            &attr_char_value,
            &p_trx->cmd_char_handles);
}

uint32_t ble_trx_init(ble_trx_t * p_trx, const ble_trx_init_t * p_trx_init)
{
    if (p_trx == NULL || p_trx_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure.
    p_trx->conn_handle       = BLE_CONN_HANDLE_INVALID;
    p_trx->cmd_write_handler = p_trx_init->cmd_write_handler;

    // Add service.
    ble_uuid128_t base_uuid = {IMU_UUID_BASE};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_trx->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_trx->uuid_type;
    ble_uuid.uuid = IMU_UUID_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
            &ble_uuid,
            &p_trx->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Adding Movement Characteristics.
    err_code = trx_char_add(p_trx);
    if (err_code != NRF_SUCCESS){
        return err_code;
    }

    // Adding Command Characteristics.
    err_code = cmd_char_add(p_trx);
    if (err_code != NRF_SUCCESS){
        return err_code;
    }

    return NRF_SUCCESS;
}


uint32_t ble_trx_update(ble_trx_t * p_trx, uint8_t *data)
{
    if (BLE_CONN_HANDLE_INVALID == p_trx->conn_handle)
        return NRF_ERROR_INVALID_STATE;

    ble_gatts_hvx_params_t params;

    memset(&params, 0, sizeof(ble_gatts_hvx_params_t));

    params.handle = p_trx->trx_char_handles.value_handle;
    params.type   = BLE_GATT_HVX_NOTIFICATION;
    params.p_data = data;
    uint16_t size  = IMU_TRX_CHAR_SIZE;
    params.p_len  = &size;

    uint32_t err_code = 0;
    err_code = sd_ble_gatts_hvx(p_trx->conn_handle, &params);

    return err_code;
}

