/**
 * SPDX-FileCopyrightText: 2024 Infineon Technologies AG
 * SPDX-License-Identifier: MIT
 *
 * \author Infineon Technologies AG
 *
 * \file pal_i2c.c
 *
 * \brief   This file implements the platform abstraction layer(pal) APIs for
 * I2C.
 *
 * \ingroup  grPAL
 *
 * @{
 */

#include "pal/pal_i2c.h"

#include <zephyr/drivers/i2c.h>

#include "pal/pal.h"

#define PAL_I2C_MAX_WAITING_TIME_MS (5000U)
#define I2C_NODE DT_ALIAS(optiga_i2c)

#if DT_NODE_HAS_PROP(I2C_NODE, clock_frequency)
#define PAL_I2C_MASTER_MAX_BITRATE DT_PROP(I2C_NODE, clock_frequency)
#else
#define PAL_I2C_MASTER_MAX_BITRATE (100000U)
#endif

K_SEM_DEFINE(i2c_semaphore, 1, 1);

static pal_i2c_t const* m_pal_i2c_cur_ctx;

// NOLINTBEGIN(readability-function-cognitive-complexity)
static pal_status_t pal_i2c_acquire(void) {
    int ret;

    ret = k_sem_take(&i2c_semaphore, K_MSEC(PAL_I2C_MAX_WAITING_TIME_MS));
    if (ret == 0) {
        return (PAL_STATUS_SUCCESS);
    }

    return (PAL_STATUS_FAILURE);
}

// NOLINTEND(readability-function-cognitive-complexity)
static void pal_i2c_release(void) {
    k_sem_give(&i2c_semaphore);
}

void invoke_upper_layer_callback(pal_i2c_t const* ctx, optiga_lib_status_t event) {
    upper_layer_callback_t up_hndl;

    up_hndl = (upper_layer_callback_t)ctx->upper_layer_event_handler;
    up_hndl(ctx->p_upper_layer_ctx, event);

    pal_i2c_release();                      // Release I2C Bus
}

// The next 5 functions are required only in case you have interrupt based i2c
// implementation
void i2c_master_end_of_transmit_callback(void) {
    invoke_upper_layer_callback(m_pal_i2c_cur_ctx, PAL_I2C_EVENT_SUCCESS);
}

void i2c_master_end_of_receive_callback(void) {
    invoke_upper_layer_callback(m_pal_i2c_cur_ctx, PAL_I2C_EVENT_SUCCESS);
}

void i2c_master_error_detected_callback(void) {
    invoke_upper_layer_callback(m_pal_i2c_cur_ctx, PAL_I2C_EVENT_ERROR);
}

void i2c_master_nack_received_callback(void) {
    i2c_master_error_detected_callback();
}

void i2c_master_arbitration_lost_callback(void) {
    i2c_master_error_detected_callback();
}

pal_status_t pal_i2c_init(pal_i2c_t const* ctx) {
    bool is_ready;

    if ((ctx == NULL) || (ctx->p_i2c_hw_config == NULL)) {
        return (PAL_STATUS_FAILURE);
    }

    const struct device* const i2c_dev = ctx->p_i2c_hw_config;

    is_ready = device_is_ready(i2c_dev);
    if (is_ready == false) {
        return (PAL_STATUS_I2C_BUSY);
    }

    return (PAL_STATUS_SUCCESS);
}

pal_status_t pal_i2c_deinit(pal_i2c_t const* ctx) {
    (void) ctx;

    return (PAL_STATUS_SUCCESS);
}

pal_status_t pal_i2c_write(pal_i2c_t const* ctx, uint8_t* du, uint16_t len) {
    pal_status_t status = PAL_STATUS_FAILURE;
    upper_layer_callback_t up_hndl;
    int ret;

    if ((ctx == NULL) || (ctx->p_i2c_hw_config == NULL) ||
        (du  == NULL) || (len == 0)) {
        return (PAL_STATUS_FAILURE);
    }

    const struct device* const i2c_dev = ctx->p_i2c_hw_config;

    // Acquire the I2C bus before read/write
    if (pal_i2c_acquire() == PAL_STATUS_SUCCESS) {
        m_pal_i2c_cur_ctx = ctx;

        // Invoke the low level i2c master driver API to write to the bus
        ret = i2c_write(i2c_dev, du, len, ctx->slave_address);
        if (ret == 0) {
            /**
             * Infineon I2C Protocol is a polling based protocol, if
             * foo_i2c_write will fail it will be reported to the upper layers
             * by calling
             * (p_i2c_context->upper_layer_event_handler))(p_i2c_context->p_upper_layer_ctx
             * , PAL_I2C_EVENT_ERROR); If the function foo_i2c_write() will
             * succedd then two options are possible
             * 1. if foo_i2c_write() is interrupt based, then you need to
             * configure interrupts in the function pal_i2c_init() so that on a
             * succesfull transmit interrupt the callback
             * i2c_master_end_of_transmit_callback(), in case of successfull
             * receive i2c_master_end_of_receive_callback() callback in case of
             * not acknowedged, arbitration lost, generic error
             * i2c_master_nack_received_callback() or
             *    i2c_master_arbitration_lost_callback()
             * 2. If foo_i2c_write() is a blocking function which will return
             * either ok or failure after transmitting data you can handle this
             * case directly here and call
             *    invoke_upper_layer_callback(gp_pal_i2c_current_ctx,
             * PAL_I2C_EVENT_SUCCESS);
             *
             */
            invoke_upper_layer_callback(m_pal_i2c_cur_ctx, PAL_I2C_EVENT_SUCCESS);

            status = PAL_STATUS_SUCCESS;
        }
        else {
            // If I2C Master fails to invoke the write operation, invoke upper
            // layer event handler with error.
            up_hndl = (upper_layer_callback_t)ctx->upper_layer_event_handler;
            up_hndl(ctx->p_upper_layer_ctx, PAL_I2C_EVENT_ERROR);

            pal_i2c_release();              // Release I2C Bus
        }
    }
    else {
        status = PAL_STATUS_I2C_BUSY;

        up_hndl = (upper_layer_callback_t)ctx->upper_layer_event_handler;
        up_hndl(ctx->p_upper_layer_ctx, PAL_I2C_EVENT_BUSY);
    }

    return (status);
}

pal_status_t pal_i2c_read(pal_i2c_t const* ctx, uint8_t* du, uint16_t len) {
    pal_status_t status = PAL_STATUS_FAILURE;
    upper_layer_callback_t up_hndl;
    int ret;

    if ((ctx == NULL) || (ctx->p_i2c_hw_config == NULL) ||
        (du  == NULL) || (len == 0)) {
        return (PAL_STATUS_FAILURE);
    }

    const struct device* const i2c_dev = ctx->p_i2c_hw_config;
    // Acquire the I2C bus before read/write
    if (PAL_STATUS_SUCCESS == pal_i2c_acquire()) {
        m_pal_i2c_cur_ctx = ctx;

        // Invoke the low level i2c master driver API to read from the bus
        ret = i2c_read(i2c_dev, du, len, ctx->slave_address);
        if (ret == 0) {
            /**
             * Similar to the foo_i2c_write() case you can directly call
             * invoke_upper_layer_callback(gp_pal_i2c_current_ctx,
             * PAL_I2C_EVENT_SUCCESS); if you have blocking (non-interrupt) i2c
             * calls
             */
            invoke_upper_layer_callback(m_pal_i2c_cur_ctx, PAL_I2C_EVENT_SUCCESS);

            status = PAL_STATUS_SUCCESS;
        }
        else {
            // If I2C Master fails to invoke the read operation, invoke upper
            // layer event handler with error.
            up_hndl = (upper_layer_callback_t)ctx->upper_layer_event_handler;
            up_hndl(ctx->p_upper_layer_ctx, PAL_I2C_EVENT_ERROR);

            pal_i2c_release();              // Release I2C Bus
        }
    }
    else {
        status = PAL_STATUS_I2C_BUSY;

        up_hndl = (upper_layer_callback_t)ctx->upper_layer_event_handler;
        up_hndl(ctx->p_upper_layer_ctx, PAL_I2C_EVENT_BUSY);
    }

    return (status);
}

pal_status_t pal_i2c_set_bitrate(pal_i2c_t const* ctx, uint16_t bitrate) {
    pal_status_t status = PAL_STATUS_FAILURE;
    uint32_t i2c_speed;
    int ret;

    if ((ctx == NULL) || (ctx->p_i2c_hw_config == NULL)) {
        return (PAL_STATUS_FAILURE);
    }

    optiga_lib_status_t event = PAL_I2C_EVENT_ERROR;
    const struct device* const i2c_dev = ctx->p_i2c_hw_config;

    // Acquire the I2C bus before setting the bitrate
    if (pal_i2c_acquire() == PAL_STATUS_SUCCESS) {
        // If the user provided bitrate is greater than the I2C master hardware
        // maximum supported value, set the I2C master to its maximum supported value.
        if (bitrate > (PAL_I2C_MASTER_MAX_BITRATE / 1000)) {
            bitrate = (PAL_I2C_MASTER_MAX_BITRATE / 1000);
        }

        if (bitrate >= 1000U) {
            i2c_speed = I2C_SPEED_FAST_PLUS;
        }
        else if (bitrate >= 400U) {
            i2c_speed = I2C_SPEED_FAST;
        }
        else {
            i2c_speed = I2C_SPEED_STANDARD;
        }

        ret = i2c_configure(i2c_dev, I2C_MODE_CONTROLLER | I2C_SPEED_SET(i2c_speed));
        if (ret == 0) {
            status = PAL_STATUS_SUCCESS;
            event  = PAL_I2C_EVENT_SUCCESS;
        }
        else {
            status = PAL_STATUS_FAILURE;
        }
    }
    else {
        status = PAL_STATUS_I2C_BUSY;
        event  = PAL_I2C_EVENT_BUSY;
    }

    if (ctx->upper_layer_event_handler != NULL) {
        callback_handler_t cbk;

        cbk = (callback_handler_t)ctx->upper_layer_event_handler;
        cbk(ctx->p_upper_layer_ctx, event);
    }

    // Release I2C Bus if its acquired
    if (status != PAL_STATUS_I2C_BUSY) {
        pal_i2c_release();
    }

    return (status);
}
