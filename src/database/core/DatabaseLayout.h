/************************** Database Layout ***********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Compile time validation of what the tables ask the database to hold. Records are
placed by the layout engine, so tables can no longer collide with each other,
but they can still outgrow the store between them. Every table compiled into
this build is measured here and the total is checked against the store, so a
struct that no longer fits fails the build instead of failing on the device.

Sizes are read from the table structs themselves, so nothing here restates one.
A table left out of the list is simply not counted.

Author          : Suraj I.
created Date    : 22nd Aug 2026
******************************************************************************/

#ifndef _DATABASE_LAYOUT_H_
#define _DATABASE_LAYOUT_H_

#include <database/core/DatabaseTable.h>

/**
 * @brief Bytes a sealed record adds, zero where sealing is not compiled in.
 */
#ifdef ENABLE_DB_SEALING
#define DB_SEAL_BUDGET DB_SEAL_OVERHEAD
#else
#define DB_SEAL_BUDGET 0
#endif

/**
 * @brief Every table record compiled into this build, a sealed one counted with
 *        the nonce and tag it carries.
 */
constexpr uint16_t DB_RECORD_SIZES[] = {
#ifdef LOGIN_TABLE_ID
    sizeof(login_credential_table) + DB_SEAL_BUDGET,
#endif
#ifdef WIFI_TABLE_ID
    sizeof(wifi_config_table) + DB_SEAL_BUDGET,
#endif
#ifdef OTA_TABLE_ID
    sizeof(ota_config_table),
#endif
#ifdef GPIO_TABLE_ID
    sizeof(gpio_config_table),
#endif
#ifdef MQTT_GENERAL_TABLE_ID
    sizeof(mqtt_general_config_table) + DB_SEAL_BUDGET,
#endif
#ifdef MQTT_LWT_TABLE_ID
    sizeof(mqtt_lwt_config_table),
#endif
#ifdef MQTT_PUBSUB_TABLE_ID
    sizeof(mqtt_pubsub_config_table),
#endif
#ifdef EMAIL_TABLE_ID
    sizeof(email_config_table) + DB_SEAL_BUDGET,
#endif
#ifdef DEVICE_IOT_TABLE_ID
    sizeof(device_iot_config_table) + DB_SEAL_BUDGET,
#endif
    0,
};

constexpr size_t DB_RECORD_COUNT = (sizeof(DB_RECORD_SIZES) / sizeof(DB_RECORD_SIZES[0])) - 1;

/**
 * @brief Payload bytes every record asks for, summed from _i onward.
 */
constexpr uint32_t db_records_total(size_t _i)
{
    return _i >= DB_RECORD_COUNT ? 0 : (uint32_t)DB_RECORD_SIZES[_i] + db_records_total(_i + 1);
}

static_assert(DB_RECORD_COUNT <= MAX_TABLES,
              "more config tables than MAX_DB_TABLES allows, raise it in the device config");

#ifdef ENABLE_DB_SEALING
#define DB_EEPROM_BUDGET (DATABASE_MAX_SIZE - DB_KEY_REGION_BYTES)
#else
#define DB_EEPROM_BUDGET DATABASE_MAX_SIZE
#endif

static_assert(DB_PAYLOAD_START + db_records_total(0) <= DB_EEPROM_BUDGET,
              "the config tables no longer fit the eeprom, shrink a struct or raise DATABASE_MAX_SIZE");

#ifdef ENABLE_DB_SEALING
static_assert(DB_PAYLOAD_START + db_records_total(0) <= DB_CONTAINER_BYTES,
              "the config tables no longer fit the container, shrink a struct or raise DB_CONTAINER_BYTES");
#endif

#endif
