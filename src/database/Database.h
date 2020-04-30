/****************************** Database **************************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _DATABASE_HANDLER_
#define _DATABASE_HANDLER_

#include <config/Config.h>
#include "EepromDatabase.h"


/**
 * define database tables max limit
 */
#define MAX_TABLES      20


class DatabaseTableAbstractLayer{

	public:

		friend class Database;

		/**
		 * DatabaseTableAbstractLayer constructor
		 */
		DatabaseTableAbstractLayer(){

			if( DatabaseTableAbstractLayer::_total_instances < MAX_TABLES ){
				DatabaseTableAbstractLayer::_instances[DatabaseTableAbstractLayer::_total_instances] = this;
				DatabaseTableAbstractLayer::_total_instances++;
			}
		}

		/**
		 * DatabaseTableAbstractLayer destructor
		 */
		~DatabaseTableAbstractLayer(){
		}

		/**
		 * override boot
		 */
		virtual void boot() = 0;
		/**
		 * override clear
		 */
		virtual void clear() = 0;

		/**
     * @array	DatabaseTableAbstractLayer* _instances
     */
		static DatabaseTableAbstractLayer* _instances[MAX_TABLES];
		/**
     * @var	int _total_instances
     */
		static int _total_instances;
};

/**
 * define table structure here
 */
struct struct_tables {
	uint16_t _table_address;
  uint16_t _table_size;
  void* _default_table;
	DatabaseTableAbstractLayer* _instance;
};


/**
 * Database class
 */
class Database {

	public:

		std::vector<struct_tables> _database_tables;


		Database(){
		}
		~Database(){
		}

    void init_database( uint16_t _size = DATABASE_MAX_SIZE );
		bool register_table( struct_tables& _table );
		struct_tables get_last_table( void );
		void clear_all( void );
};

extern Database __database;
#endif
