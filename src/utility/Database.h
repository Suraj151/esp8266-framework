/****************************** Database **************************************
This file is part of the PDI stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

The Database utility provides an abstraction layer for managing database tables
within the PDI stack. It allows for the registration, initialization, and 
management of database tables, including clearing and retrieving table data.

Author          : Suraj I.
Created Date    : 1st June 2019
******************************************************************************/

#ifndef _DATABASE_HANDLER_
#define _DATABASE_HANDLER_

#include <utility/Utility.h>

/**
 * @brief Defines the maximum number of database tables.
 */
#ifdef MAX_DB_TABLES
#define MAX_TABLES MAX_DB_TABLES
#else
#define MAX_TABLES 20
#endif

/**
 * @brief Id reserved for "no table", so a real table id is never zero.
 */
#define DB_TABLE_ID_NONE 0

/**
 * @class DatabaseTableAbstractLayer
 * @brief Abstract layer for database tables.
 *
 * This class provides an interface for managing individual database tables.
 * It allows for the creation, initialization, and clearing of tables. The
 * class also tracks the total number of table instances and ensures that the
 * number of tables does not exceed the defined maximum.
 */
class DatabaseTableAbstractLayer
{

public:
    friend class Database;

    /**
     * @brief Constructor for the DatabaseTableAbstractLayer class.
     *
     * Registers the table instance and increments the total instance count,
     * ensuring the number of instances does not exceed the maximum limit.
     */
    DatabaseTableAbstractLayer()
    {
        if (DatabaseTableAbstractLayer::m_total_instances < MAX_TABLES)
        {
            DatabaseTableAbstractLayer::m_instances[DatabaseTableAbstractLayer::m_total_instances] = this;
            DatabaseTableAbstractLayer::m_total_instances++;
        }
    }

    /**
     * @brief Destructor for the DatabaseTableAbstractLayer class.
     */
    ~DatabaseTableAbstractLayer()
    {
    }

    /**
     * @brief Initializes the table.
     *
     * This method must be overridden by derived classes to provide specific
     * initialization logic for the table.
     *
     * @return True when the table reached the database, false otherwise.
     */
    virtual bool boot() = 0;

    /**
     * @brief Clears the table data.
     *
     * This method must be overridden by derived classes to provide specific
     * logic for clearing the table data.
     *
     * @return True when the defaults reached storage, false otherwise.
     */
    virtual bool clear() = 0;

    /**
     * @var DatabaseTableAbstractLayer* m_instances[MAX_TABLES]
     * @brief Array of pointers to table instances.
     */
    static DatabaseTableAbstractLayer *m_instances[MAX_TABLES];

    /**
     * @var int m_total_instances
     * @brief Total number of table instances.
     */
    static int m_total_instances;
};

/**
 * @struct struct_tables
 * @brief Describes one registered database table.
 *
 * A table is identified by an id that never changes and never gets reused, so
 * where its record lives is left entirely to the layout engine.
 */
struct struct_tables
{
    uint16_t m_table_id;      ///< Stable identity of the table.
    uint16_t m_table_size;    ///< Size of the table struct in bytes.
    uint16_t m_table_version; ///< Layout version of the table struct.
    bool m_table_secret;      ///< Whether the record holds a credential.
    DatabaseTableAbstractLayer *m_instance; ///< Pointer to the table instance.
};

/**
 * @class Database
 * @brief Registry of the tables compiled into this build.
 *
 * The registry holds what each table is and how big it is. Placing those
 * records on a medium belongs to the layout engine, which reads this registry
 * when it mounts a store.
 */
class Database
{

public:
    /**
     * @var pdiutil::vector<struct_tables> m_database_tables
     * @brief Vector of registered database tables.
     */
    pdiutil::vector<struct_tables> m_database_tables;

    /**
     * @brief Constructor for the Database class.
     */
    Database();

    /**
     * @brief Destructor for the Database class.
     */
    ~Database();

    /**
     * @brief Boots every table instance so each one registers itself.
     * @return Number of table instances that failed to register.
     */
    uint8_t init_database(void);

    /**
     * @brief Registers a new table in the registry.
     * @param _table The table descriptor to register.
     * @return True if the table was successfully registered, false otherwise.
     */
    bool register_table(struct_tables &_table);

    /**
     * @brief Clears all tables in the database.
     * @return True when every registered table reached its defaults.
     */
    bool clear_all(void);
};

/**
 * @brief Global instance of the Database class.
 *
 * This instance is used to manage database operations throughout the PDI stack.
 */
extern Database __database;

#endif
