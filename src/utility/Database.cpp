/****************************** Database **************************************
This file is part of the PDI stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
Created Date    : 1st June 2019
******************************************************************************/

#include "Database.h"

// Initialize static members of DatabaseTableAbstractLayer
int DatabaseTableAbstractLayer::m_total_instances = 0;
DatabaseTableAbstractLayer *DatabaseTableAbstractLayer::m_instances[MAX_TABLES] = {nullptr};

/**
 * @brief Constructor for the Database class.
 */
Database::Database()
{
}

/**
 * @brief Destructor for the Database class.
 *
 * Cleans up resources used by the database object.
 */
Database::~Database()
{
}

/**
 * @brief Boots every table instance so each one registers itself.
 *
 * Instances add themselves to a static list as they are constructed, this walks
 * that list once the registry is ready to receive them.
 *
 * @return Number of table instances that failed to register.
 */
uint8_t Database::init_database()
{
    uint8_t _failed = 0;

    this->m_database_tables.reserve(MAX_TABLES);

    for (size_t i = 0; i < DatabaseTableAbstractLayer::m_total_instances; i++)
    {
        if (!DatabaseTableAbstractLayer::m_instances[i]->boot())
        {
            _failed++;
        }
    }

    return _failed;
}

/**
 * @brief Clears all tables in the database.
 *
 * This method iterates through all registered database tables and calls their
 * `clear` method to reset their data.
 *
 * @return True when every registered table reached its defaults.
 */
bool Database::clear_all()
{
    bool _status = true;

    for (uint8_t i = 0; i < this->m_database_tables.size(); i++)
    {
        if (nullptr != this->m_database_tables[i].m_instance)
        {
            _status = this->m_database_tables[i].m_instance->clear() && _status;
        }
    }

    return _status;
}

/**
 * @brief Registers a table in the registry.
 *
 * A table is rejected when it carries no id, when the registry is full, or when
 * its id is already held by a different table, so two tables can never share an
 * identity. Registering the same instance again is how a service reinitialises
 * and keeps its entry, so it succeeds and refreshes the descriptor in place.
 *
 * @param _table The table descriptor to register.
 * @return True if the table is registered, false otherwise.
 */
bool Database::register_table(struct_tables &_table)
{
    if (DB_TABLE_ID_NONE == _table.m_table_id || 0 == _table.m_table_size)
    {
        return false;
    }

    for (uint8_t i = 0; i < this->m_database_tables.size(); i++)
    {
        if (this->m_database_tables[i].m_table_id == _table.m_table_id)
        {
            if (nullptr == _table.m_instance ||
                this->m_database_tables[i].m_instance != _table.m_instance)
            {
                return false;
            }

            this->m_database_tables[i] = _table;
            return true;
        }
    }

    if (this->m_database_tables.size() >= MAX_TABLES)
    {
        return false;
    }

    this->m_database_tables.push_back(_table);
    return true;
}

/**
 * @brief Global instance of the Database class.
 *
 * This instance is used to manage database operations throughout the PDI stack.
 */
Database __database;
