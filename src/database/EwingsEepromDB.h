#ifndef _EWINGS_EEPROM_DATABASE_H_
#define _EWINGS_EEPROM_DATABASE_H_

#define EEPROM_MAX      4096

#include <EEPROM.h>
#include <utility/Utility.h>
#include <utility/Log.h>

#include "StoreStruct.h"

struct struct_tables {
	uint16_t _table_address;
  uint16_t _table_size;
  int* _table;
};

class EwingsEepromDB{

  public:

    uint16_t __size;


    EwingsEepromDB( uint16_t _size = EEPROM_MAX ){
      this->init_database(_size);
    }

    ~EwingsEepromDB(){
    }

    void init_database(  uint16_t _size = EEPROM_MAX ){
      this->resize_database(_size);
    }

    void resize_database(  uint16_t _size ){
      EEPROM.begin(_size);
      this->__size = _size;
    }

    void clear_eeprom(){
      for (int i = 0; i < this->__size; i++) {
        EEPROM.write(i, 0);
      }
      EEPROM.end();
    }

    template <typename Struct> bool get_table( Struct * _object, uint16_t _address ) {

      Struct _t;
      for ( uint8_t i = 0; i < this->_tables.size(); i++) {

    		if( this->_tables[i]._table_address == _address ){
          this->loadConfig( _object, _address );
          return true;
        }
			}
      return false;
    }

    template <typename Struct> bool set_table( Struct * _object, uint16_t _address ) {

      Struct _t;
      for ( uint8_t i = 0; i < this->_tables.size(); i++) {

    		if( this->_tables[i]._table_address == _address ){
          this->saveConfig( _object, _address );
          return true;
        }
			}
      return false;
    }

    template <typename Struct> bool register_table( const Struct * _object, uint16_t _address, bool _register_with_object_copy=false ) {

      struct_tables _t, _last = get_last_table_struct();
      _t._table_address = _address;
      _t._table_size = sizeof((*_object));
      _t._table = (int*)_object;

      if( _address < EEPROM_MAX &&
        ( _last._table_address + _last._table_size + 2 ) < _address &&
        ( _address + _t._table_size + 2 ) < EEPROM_MAX
      ){
        this->_tables.push_back(_t);
        if( _register_with_object_copy ) this->clear_table_to_defaults( _object, _address );
				return true;
      }return false;
    }

    template <typename Struct> void clear_table_to_defaults( const Struct * _object, uint16_t _address ) {
      Struct _t;
      PROGMEM_readAnything ( _object, _t );
      this->saveConfig( &_t, _address );
      _ClearObject( &_t );
    }

    template <typename Struct> void loadConfig ( Struct * _object, uint16_t configStart ) {
      if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
          EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
          EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2])
        for (unsigned int i = 0; i < sizeof((*_object)); i++)
          *((char*) & (*_object) + i) = EEPROM.read(configStart + i);
    }

    template <typename Struct> void saveConfig ( Struct * _object, uint16_t configStart ) {
      bool _data_written = false;
      for (unsigned int i = 0; i < sizeof((*_object)); i++) {
        if ((char)EEPROM.read(configStart + i) != *((char*) & (*_object) + i)) {
          _data_written = true;
          EEPROM.write(configStart + i, *((char*) & (*_object) + i));
        }
      }
      if( _data_written ){
        EEPROM.commit();
      }
    }

    struct_tables get_last_table_struct(){

      struct_tables _last;
      uint8_t _lats_add = 0;
      for ( uint8_t i = 1, _lats_add=0; i < this->_tables.size(); i++) {

				if( this->_tables[_lats_add]._table_address < this->_tables[i]._table_address ){
		      _lats_add = i;
        }
			}
      if( _lats_add != 0 ) return this->_tables[_lats_add]; return _last;
    }

  protected:

    std::vector<struct_tables> _tables;

};

// extern EwingsEepromDB EwEepromDB;

#endif
