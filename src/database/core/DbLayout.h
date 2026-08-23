/**************************** Database Layout Engine **************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Frames a superblock, a fixed directory and packed records over a flat store.
Tables are addressed by a stable id, never by a byte offset, so a table struct
can grow or a table can be added without any hand assigned address moving.

  region start   superblock       DB_SUPERBLOCK_BYTES
                 directory        DB_DIR_ENTRY_BYTES * MAX_TABLES
                 payload area     records packed in registration order

Every record carries its own length, version and crc, so a record written by an
older firmware still loads: the stored bytes land on a default constructed
object and any field added since keeps its default.

Author          : Suraj I.
created Date    : 22nd Aug 2026
******************************************************************************/

#ifndef _DATABASE_LAYOUT_ENGINE_H_
#define _DATABASE_LAYOUT_ENGINE_H_

#include <interface/pdi/modules/database/iDbStoreInterface.h>
#include <utility/Database.h>

/**
 * @brief On media format revision, bumped when the framing itself changes.
 */
#define DB_FORMAT_VERSION 1

/**
 * @brief First byte of the database region within the store.
 */
#define DB_REGION_START CONFIG_START

/**
 * @brief Serialized sizes of the framing structures.
 */
#define DB_SUPERBLOCK_BYTES 12
#define DB_DIR_ENTRY_BYTES 13

/**
 * @brief Window a record is checksummed through before it is handed over, so a
 *        corrupt record never lands on the caller's defaults.
 */
#define DB_CRC_WINDOW_BYTES 32

/**
 * @brief First byte of the directory and of the payload area.
 */
#define DB_DIR_START (DB_REGION_START + DB_SUPERBLOCK_BYTES)
#define DB_PAYLOAD_START (DB_DIR_START + (DB_DIR_ENTRY_BYTES * MAX_TABLES))

/**
 * @brief Set on a record whose payload is sealed.
 *
 * Sealing guards a container that can be copied off the device. Without one the
 * database lives only in the eeprom, which nothing in the shell can reach, so a
 * build with no storage service carries neither the ciphers nor their state.
 */
#define DB_ENTRY_FLAG_SEALED 0x01

#ifdef ENABLE_DB_SEALING
/**
 * @brief The cipher and mac states a seal works through, defined by the engine.
 *
 * Held only as a pointer here so the sealed path can pass one scratch down its
 * whole call chain and allocate it once per operation.
 */
struct db_seal_scratch_t;
#endif

/**
 * @struct db_dir_entry_t
 * @brief One record's placement and state, held in ram and mirrored on media.
 */
struct db_dir_entry_t
{
    db_dir_entry_t() : m_id(0), m_version(0), m_offset(0), m_cap(0), m_len(0), m_crc(0), m_flags(0) {}

    uint16_t m_id;      ///< Stable table id.
    uint16_t m_version; ///< Table version the payload was written by.
    uint16_t m_offset;  ///< Payload offset within the store.
    uint16_t m_cap;     ///< Bytes reserved for the payload.
    uint16_t m_len;     ///< Bytes actually written, 0 when never written.
    uint16_t m_crc;     ///< crc16 over the written bytes.
    uint8_t m_flags;    ///< Per record handling, see DB_ENTRY_FLAG_*.
};

/**
 * @class DbLayout
 * @brief Reads and writes records by table id over a store.
 */
class DbLayout
{

public:
    DbLayout();
    ~DbLayout();

    /**
     * @brief Bind a store and reconcile it against the registered tables.
     *
     * A store whose superblock does not verify is formatted fresh. A store
     * whose geometry no longer matches the registered tables is repacked,
     * carrying every payload across by id.
     *
     * @param _store Byte device holding the database.
     * @param _tables Registered table descriptors.
     * @return PDI_OK when the database is usable.
     */
    pdi_err_t mount(iDbStoreInterface *_store, pdiutil::vector<struct_tables> &_tables);

    /**
     * @brief Whether a store is bound and its superblock verified.
     */
    bool is_mounted() const { return m_mounted; }

    /**
     * @brief Whether mount() had to lay down a fresh database.
     */
    bool was_formatted() const { return m_formatted; }

    /**
     * @brief Copy a record's stored bytes over _buf.
     *
     * Fewer than _cap bytes are written when the record was stored by an older
     * and shorter version of the struct, leaving the tail of _buf untouched.
     *
     * @param _id Table id.
     * @param _buf Destination, default constructed by the caller.
     * @param _cap Size of the destination struct.
     * @param _len Receives the number of bytes copied.
     * @param _version Receives the version the payload was written by.
     */
    pdi_err_t read_record(uint16_t _id, uint8_t *_buf, uint16_t _cap, uint16_t &_len, uint16_t &_version);

    /**
     * @brief Store _len bytes as the record for _id.
     */
    pdi_err_t write_record(uint16_t _id, const uint8_t *_buf, uint16_t _len, uint16_t _version);

    /**
     * @brief Drop a record's contents so reads fall back to struct defaults.
     */
    pdi_err_t clear_record(uint16_t _id);

    /**
     * @brief Lay down a fresh superblock and directory for _tables.
     */
    pdi_err_t format(pdiutil::vector<struct_tables> &_tables);

#ifdef ENABLE_STORAGE_SERVICE
    /**
     * @brief Copy every record this layout knows about out of _source.
     *
     * A record _source does not hold, or holds empty, is cleared here, so the
     * copy leaves this layout saying exactly what _source says.
     *
     * @param _source Layout to take the records from.
     * @return PDI_OK when every record was carried across.
     */
    pdi_err_t copy_from(DbLayout &_source);
#endif

    /**
     * @brief Run a record past its checksum or tag without copying it out.
     *
     * Needs no buffer of its own, so a caller can check a record it has no room
     * to hold.
     *
     * @param _id Table id.
     * @return PDI_OK when the record verifies.
     */
    pdi_err_t verify_record(uint16_t _id);

    uint32_t firmware_version() const { return m_firmware; }
    uint8_t launch_year() const { return m_year; }
    void set_firmware_version(uint32_t _version);
    void set_launch_year(uint8_t _year);

    /**
     * @brief Payload bytes reserved by all records.
     */
    uint16_t used_bytes() const;

    /**
     * @brief Payload bytes still free in the store.
     */
    uint16_t free_bytes() const;

    uint8_t entry_count() const { return m_entries; }
    const db_dir_entry_t *entry_at(uint8_t _index) const;

private:
    db_dir_entry_t *find_entry(uint16_t _id);
    pdi_err_t verify_plain(db_dir_entry_t *_entry);
#ifdef ENABLE_DB_SEALING
    pdi_err_t seal_record(db_dir_entry_t *_entry, const uint8_t *_buf, uint16_t _len);
    pdi_err_t unseal_record(db_dir_entry_t *_entry, uint8_t *_buf, uint16_t _read);
    pdi_err_t verify_sealed(db_dir_entry_t *_entry, const uint8_t *_tag, db_seal_scratch_t *_scratch);
#endif
    pdi_err_t read_superblock();
    pdi_err_t write_superblock();
    pdi_err_t read_directory();
    pdi_err_t write_entry(uint8_t _index);
    pdi_err_t repack(pdiutil::vector<struct_tables> &_tables);
    bool geometry_matches(pdiutil::vector<struct_tables> &_tables);

    iDbStoreInterface *m_store;
    db_dir_entry_t m_directory[MAX_TABLES];
    uint32_t m_firmware;
    uint8_t m_entries;
    uint8_t m_year;
    bool m_mounted;
    bool m_formatted;
};

/**
 * @brief The database the device is running on.
 */
extern DbLayout __db_layout;


#endif
