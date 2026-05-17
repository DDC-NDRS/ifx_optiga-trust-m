/**
 * SPDX-FileCopyrightText: 2024 Infineon Technologies AG
 * SPDX-License-Identifier: MIT
 *
 * \author Infineon Technologies AG
 *
 * \file    pal_logger.c
 *
 * \brief   This file provides the prototypes declarations for pal logger.
 *
 * \ingroup grPAL
 *
 * @{
 */
#include "pal/pal_logger.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(OPTIGA);

// Unused for this implementation, but needs to defined
pal_logger_t logger_console = {0};

pal_status_t pal_logger_init(void* ctx) {
    (void) ctx;

    return (PAL_STATUS_SUCCESS);
}

pal_status_t pal_logger_deinit(void* ctx) {
    (void) ctx;

    return (PAL_STATUS_SUCCESS);
}

pal_status_t pal_logger_write(void* ctx, uint8_t const* du, uint32_t len) {
    (void) ctx;

    if (du == NULL) {
        return (PAL_STATUS_FAILURE);
    }

    char* buf = (char*)k_malloc(len + 1);
    if (buf == NULL) {
        return (PAL_STATUS_FAILURE);
    }

    memcpy(buf, du, len);
    buf[len] = 0;

    LOG_RAW("%s", buf);

    k_free(buf);

    return (PAL_STATUS_SUCCESS);
}

// NOLINTBEGIN(readability-non-const-parameter)
pal_status_t pal_logger_read(void* ctx, uint8_t* du, uint32_t len) {
    (void) ctx;
    (void) du;
    (void) len;

    // Not supported
    return (PAL_STATUS_FAILURE);
}

// NOLINTEND(readability-non-const-parameter)
