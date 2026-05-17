/**
 * SPDX-FileCopyrightText: 2024 Infineon Technologies AG
 * SPDX-License-Identifier: MIT
 *
 * \author Infineon Technologies AG
 *
 * \file pal_os_datastore.c
 *
 * \brief   This file implements the platform abstraction layer APIs for data
 * store.
 *
 * \ingroup  grPAL
 *
 * @{
 */

#include "pal/pal_os_datastore.h"

/// @cond hidden

/// Size of length field
#define LENGTH_SIZE (0x02)

/// Size of data store buffer to hold the shielded connection manage context
/// information (2 bytes length field + 64(0x40) bytes context)
#define MANAGE_CONTEXT_BUFFER_SIZE (0x42)

// Internal buffer to store the shielded connection manage context information
// (length field + Data)
uint8_t data_store_manage_context_buffer[LENGTH_SIZE + MANAGE_CONTEXT_BUFFER_SIZE];

// Internal buffer to store the optiga application context data during
// hibernate(length field + Data)
uint8_t data_store_app_context_buffer[LENGTH_SIZE + APP_CONTEXT_SIZE];

// Internal buffer to store the generated platform binding shared secret on Host
// (length field + shared secret)
uint8_t optiga_platform_binding_shared_secret[LENGTH_SIZE + OPTIGA_SHARED_SECRET_MAX_LENGTH] = {
    // Length of the shared secret, followed after the length information
    0x00, 0x40,

    // Shared secret. Buffer is defined to the maximum supported length [64 bytes].
    // But the actual size used is to be specified in the length field.
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40
};

pal_status_t pal_os_datastore_write(uint16_t id, uint8_t const* buf, uint16_t len) {
    pal_status_t status = PAL_STATUS_FAILURE;

    if (buf == NULL) {
        return (status);
    }

    switch (id) {
        case OPTIGA_PLATFORM_BINDING_SHARED_SECRET_ID : {
            // OPTIGA_LIB_PORTING_REQUIRED
            // This has to be enhanced by user only, in case of updating
            // the platform binding shared secret during the runtime into NVM.
            // In current implementation, platform binding shared secret is stored in RAM.
            if (len <= OPTIGA_SHARED_SECRET_MAX_LENGTH) {
                optiga_platform_binding_shared_secret[0] = (uint8_t)(len >> 8);
                optiga_platform_binding_shared_secret[1] = (uint8_t)(len);

                if ((len + LENGTH_SIZE) <= sizeof(optiga_platform_binding_shared_secret)) {
                    // flawfinder: ignore
                    memcpy(&optiga_platform_binding_shared_secret[LENGTH_SIZE], buf, len);
                    status = PAL_STATUS_SUCCESS;
                }
            }
            break;
        }

        case OPTIGA_COMMS_MANAGE_CONTEXT_ID : {
            // OPTIGA_LIB_PORTING_REQUIRED
            // This has to be enhanced by user only, in case of storing
            // the manage context information in non-volatile memory
            // to reuse for later during hard reset scenarios where the RAM gets flushed out.
            if (len <= MANAGE_CONTEXT_BUFFER_SIZE) {
                data_store_manage_context_buffer[0] = (uint8_t)(len >> 8);
                data_store_manage_context_buffer[1] = (uint8_t)(len);

                if ((len + LENGTH_SIZE) <= sizeof(data_store_manage_context_buffer)) {
                    // flawfinder: ignore
                    memcpy(&data_store_manage_context_buffer[LENGTH_SIZE], buf, len);
                    status = PAL_STATUS_SUCCESS;
                }
            }
            break;
        }

        case OPTIGA_HIBERNATE_CONTEXT_ID : {
            // OPTIGA_LIB_PORTING_REQUIRED
            // This has to be enhanced by user only, in case of storing
            // the application context information in non-volatile memory
            // to reuse for later during hard reset scenarios where the
            // RAM gets flushed out.
            if (len <= APP_CONTEXT_SIZE) {
                data_store_app_context_buffer[0] = (uint8_t)(len >> 8);
                data_store_app_context_buffer[1] = (uint8_t)(len);

                if ((len + LENGTH_SIZE) <= sizeof(data_store_app_context_buffer)) {
                    // flawfinder: ignore
                    memcpy(&data_store_app_context_buffer[LENGTH_SIZE], buf, len);
                    status = PAL_STATUS_SUCCESS;
                }
            }
            break;
        }

        default : {
            break;
        }
    }

    return (status);
}

pal_status_t pal_os_datastore_read(uint16_t id, uint8_t* buf, uint16_t* p_len) {
    pal_status_t status = PAL_STATUS_FAILURE;
    uint16_t dat_len;

    if ((buf == NULL) || (p_len == NULL)) {
        return (status);
    }

    switch (id) {
        case OPTIGA_PLATFORM_BINDING_SHARED_SECRET_ID : {
            // OPTIGA_LIB_PORTING_REQUIRED
            // This has to be enhanced by user only,
            // if the platform binding shared secret is stored in non-volatile
            // memory with a specific location and not as a context segment
            // else updating the share secret content is good enough.

            dat_len  = (uint16_t)(optiga_platform_binding_shared_secret[0] << 8);
            dat_len |= (uint16_t)(optiga_platform_binding_shared_secret[1]);
            if (dat_len <= OPTIGA_SHARED_SECRET_MAX_LENGTH && dat_len <= *p_len &&
                (dat_len + LENGTH_SIZE) <= sizeof(optiga_platform_binding_shared_secret)) {
                // flawfinder: ignore
                memcpy(buf, &optiga_platform_binding_shared_secret[LENGTH_SIZE], dat_len);
                *p_len = dat_len;
                status = PAL_STATUS_SUCCESS;
            }
            break;
        }

        case OPTIGA_COMMS_MANAGE_CONTEXT_ID : {
            // OPTIGA_LIB_PORTING_REQUIRED
            // This has to be enhanced by user only,
            // if manage context information is stored in NVM during the hibernate,
            // else this is not required to be enhanced.
            dat_len = (uint16_t)(data_store_manage_context_buffer[0] << 8);
            dat_len |= (uint16_t)(data_store_manage_context_buffer[1]);
            if (dat_len <= MANAGE_CONTEXT_BUFFER_SIZE && dat_len <= *p_len &&
                (dat_len + LENGTH_SIZE) <= sizeof(data_store_manage_context_buffer)) {
                // flawfinder: ignore
                memcpy(buf, &data_store_manage_context_buffer[LENGTH_SIZE], dat_len);
                *p_len = dat_len;
                status = PAL_STATUS_SUCCESS;
            }
            break;
        }

        case OPTIGA_HIBERNATE_CONTEXT_ID : {
            // OPTIGA_LIB_PORTING_REQUIRED
            // This has to be enhanced by user only,
            // if application context information is stored in NVM during the
            // hibernate, else this is not required to be enhanced.
            dat_len = (uint16_t)(data_store_app_context_buffer[0] << 8);
            dat_len |= (uint16_t)(data_store_app_context_buffer[1]);
            if (dat_len <= APP_CONTEXT_SIZE && dat_len <= *p_len &&
                (dat_len + LENGTH_SIZE) <= sizeof(data_store_app_context_buffer)) {
                // flawfinder: ignore
                memcpy(buf, &data_store_app_context_buffer[LENGTH_SIZE], dat_len);
                *p_len = dat_len;
                status = PAL_STATUS_SUCCESS;
            }
            break;
        }

        default : {
            *p_len = 0;
            break;
        }
    }

    return (status);
}

/// @endcond
/**
 * @}
 */
