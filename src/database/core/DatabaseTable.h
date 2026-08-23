/************************** Database Factory **********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _DATABASE_FACTORY_
#define _DATABASE_FACTORY_

#include <config/Config.h>
#include <database/core/DbLayout.h>
#include <utility/Database.h>
#include <interface/pdi.h>
#include <interface/pdi/impl/log/LogMacros.h>

/**
 * DatabaseTable class
 *
 * Binds a table struct to a stable id. Where the record is kept is decided by
 * the layout engine, so a struct can gain fields at its end without any table
 * having to move. A record written by an older and shorter version of the
 * struct still loads, the fields added since simply keep their defaults.
 *
 * A table declared secret has its record sealed on the way to the medium, so a
 * copy of the database taken off the device carries ciphertext for it.
 */
template <uint16_t table_id, class Table, uint16_t table_version = 1, bool table_secret = false>
class DatabaseTable : public DatabaseTableAbstractLayer
{

public:
	/**
	 * DatabaseTable constructor
	 */
	DatabaseTable() : m_registered(false)
	{
	}

	/**
	 * DatabaseTable destructor
	 */
	~DatabaseTable()
	{
	}

    /**
     * @purpose register table to database.
     */
    bool boot()
    {
        struct_tables _t;
        _t.m_table_id = table_id;
        _t.m_table_size = sizeof(Table);
        _t.m_table_version = table_version;
        _t.m_table_secret = table_secret;
        _t.m_instance = this;

        this->m_registered = __database.register_table(_t);

        if (!this->m_registered)
        {
            SysLogE("DB table %u of %u bytes was not registered\n", (unsigned)table_id, (unsigned)sizeof(Table));
        }

        return this->m_registered;
    }

    /**
     * @purpose get/fetch table from database.
     *
     * _table is expected to arrive default constructed, anything the stored
     * record does not cover keeps the default it came in with.
     */
    bool get(Table *_table)
    {
        if (!this->m_registered)
        {
            return false;
        }

        uint16_t _len = 0;
        uint16_t _version = 0;
        pdi_err_t _err = __db_layout.read_record(table_id, (uint8_t *)_table, sizeof(Table), _len, _version);

        if (PDI_OK != _err)
        {
            SysLogE("DB table %u read failed (%d), using defaults\n", (unsigned)table_id, (int)_err);
            return false;
        }

        if (0 != _len && _version != table_version)
        {
            return this->migrate(_version, _table);
        }

        return true;
    }

    /**
     * @purpose set table in database.
     */
    bool set(Table *_table)
    {
        if (!this->m_registered)
        {
            return false;
        }

        return PDI_OK == __db_layout.write_record(table_id, (const uint8_t *)_table, sizeof(Table), table_version);
    }

    /**
     * @purpose clear table in database.
     *
     * Drops the record so the next read falls back to the struct defaults,
     * which costs no payload writes at all.
     */
    bool clear()
    {
        if (!this->m_registered)
        {
            return false;
        }

        return PDI_OK == __db_layout.clear_record(table_id);
    }

protected:
	/**
	 * carry a payload written by an older version of this table forward.
	 *
	 * Fields appended to the end of a struct need nothing here, the short read
	 * already left them at their defaults. Override only when a field changed
	 * meaning or moved.
	 *
	 * @param   uint16_t  _from_version
	 * @param   Table*    _table
	 * @return  bool	  true when the payload is usable
	 */
	virtual bool migrate(uint16_t _from_version, Table *_table)
	{
		return true;
	}

private:
	/**
	 * @var bool m_registered
	 * @brief Whether the registry accepted this table at boot.
	 */
	bool m_registered;
};

#endif
