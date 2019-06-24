/******************************* Utility **************************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef __EWINGS_UTILITY_H__
#define __EWINGS_UTILITY_H__

/**
 * max callbacks
 */
#define MAX_PERIODIC_CALLBACKS	20
#define MAX_FACTORY_RESET_CALLBACKS	MAX_PERIODIC_CALLBACKS

#include <Esp.h>
#include <database/StoreStruct.h>
#include "DataTypeConversions.h"
#include "StringOperations.h"
#include "queue/queue.h"
#include "Log.h"

/**
 * callback type
 */
typedef std::function<void(int)> CallBackIntArgFn;
typedef std::function<void(void)> CallBackVoidArgFn;

/**
 * This template clone program memory object to data memory.
 *
 * @param	cost T* sce
 * @param	T&	dest
 */
template <typename T> void PROGMEM_readAnything (const T * sce, T& dest)
{
	memcpy_P (&dest, sce, sizeof (T));
}

/**
 * This template returns static copy of program memory object.
 *
 * @param		cost T* sce
 * @return	T
 */
template <typename T> T PROGMEM_getAnything (const T * sce)
{
	static T temp;
	memcpy_P (&temp, sce, sizeof (T));
	return temp;
}

/**
 * Template to clear type of object/struct.
 *
 * @param	cost Struct* _object
 */
template <typename Struct> void _ClearObject (const Struct * _object) {
  for (unsigned int i = 0; i < sizeof((*_object)); i++)
    *((char*) & (*_object) + i) = 0;
}

#ifdef ENABLE_GPIO_CONFIG
bool is_exceptional_gpio_pin( uint8_t _pin );
#endif

// int getPinMode(uint8_t pin){
//
//   if (pin >= NUM_DIGITAL_PINS) return (-1);
//
//   uint8_t bit = digitalPinToBitMask(pin);
//   uint8_t port = digitalPinToPort(pin);
//   volatile uint8_t *reg = portModeRegister(port);
//   if (*reg & bit) return (OUTPUT);
//
//   volatile uint8_t *out = portOutputRegister(port);
//   return ((*out & bit) ? INPUT_PULLUP : INPUT);
// }

/**
 * periodic callback struct for scheduler
 */
struct periodic_cb_ {
	CallBackVoidArgFn _cb;
	uint32_t _duration;
	unsigned long _last_millis;
	int _max_attempts;
	int _event_id;
};

struct factory_reset_cb_ {
	CallBackVoidArgFn _cb;
};

/**
 * PeriodicCallBack class
 */
class PeriodicCallBack{

	public:

		/**
		 * PeriodicCallBack constructor
		 */
		PeriodicCallBack(){
		}

		/**
		 * PeriodicCallBack constructor
		 *
		 * @param	CallBackVoidArgFn	_fn
		 * @param	uint32_t	_duration
		 * @param	unsigned long|0	_last_millis
		 */
		PeriodicCallBack( CallBackVoidArgFn _fn, uint32_t _duration, unsigned long _last_millis=0 ){

			this->register_periodic_callback( _fn, _duration, _last_millis );
		}

		/**
		 * set callback function which will execute once after given duration.
		 * it will return callback id for reference.
		 *
		 * @param		CallBackVoidArgFn	_fn
		 * @param		uint32_t	_duration
		 * @return	int
		 */
		int setTimeout( CallBackVoidArgFn _fn, uint32_t _duration ){
			return this->register_periodic_callback( _fn, _duration, millis(), 1 );
		}

		/**
		 * set callback function which will execute after every given duration.
		 * it will return callback id for reference.
		 *
		 * @param		CallBackVoidArgFn	_fn
		 * @param		uint32_t	_duration
		 * @return	int
		 */
		int setInterval( CallBackVoidArgFn _fn, uint32_t _duration ){
			return this->register_periodic_callback( _fn, _duration, millis() );
		}

		/**
		 * this function updates already registered callback task.
		 * it will return updated callback id for reference.
		 *
		 * @param		int	_registered_id
		 * @param		CallBackVoidArgFn	_fn
		 * @param		uint32_t	_duration
		 * @param		unsigned long|0	_last_millis
		 * @param		int|-1	_max_attampts
		 * @return	int
		 */
		int updateInterval( int _registered_id, CallBackVoidArgFn _fn, uint32_t _duration, unsigned long _last_millis=0, int _max_attampts=-1 ){

			int _registered_index = this->is_registered_callback( _registered_id );
			if( _registered_index > -1 ){

				this->_periods[_registered_index]._cb = _fn;
				this->_periods[_registered_index]._duration = _duration;
				this->_periods[_registered_index]._last_millis = _last_millis;
				this->_periods[_registered_index]._max_attempts = _max_attampts;
				return _registered_id;
			}else{

				return this->register_periodic_callback( _fn, _duration,_last_millis, _max_attampts );
			}
		}

		/**
		 * this function clears the callback task of given ref. callback id
		 * it return success/failure of operation.
		 *
		 * @param		int	_id
		 * @return	bool
		 */
		bool clearTimeout( int _id ){
			return this->remove_callback( _id );
		}

		/**
		 * this function clears the callback task of given ref. callback id
		 * it return success/failure of operation.
		 *
		 * @param		int	_id
		 * @return	bool
		 */
		bool clearInterval( int _id ){
			return this->remove_callback( _id );
		}

		/**
		 * this function registers callback task of given duration with unique id.
		 * it will return callback task id for reference.
		 *
		 * @param		CallBackVoidArgFn	_fn
		 * @param		uint32_t	_duration
		 * @param		unsigned long|0	_last_millis
		 * @param		int|-1	_max_attampts
		 * @return	int
		 */
		int register_periodic_callback( CallBackVoidArgFn _fn, uint32_t _duration, unsigned long _last_millis=0, int _max_attampts=-1 ){

			if( this->_periods.size() < MAX_PERIODIC_CALLBACKS ){

				periodic_cb_ _new_period;
				_new_period._cb = _fn;
				_new_period._duration = _duration;
				_new_period._last_millis = _last_millis;
				_new_period._max_attempts = _max_attampts;
				_new_period._event_id = this->get_unique_id_for_cb();
				this->_periods.push_back(_new_period);
				return _new_period._event_id;
			}
			return -1;
		}

		/**
		 * this function handles/executes callback task on their given time periods.
		 * this function must be called every millisecond in loop for proper task handles.
		 *
		 * @param	unsigned long _millis
		 */
		void handle_periodic_callbacks( unsigned long _millis ){

			// #ifdef EW_SERIAL_LOG
			// Log(F("in handle periodic callbacks : current millis = "));Logln(_millis);
			// Logln(F("id\tduration\tattempts\tlast_millis"));
			// #endif
			for ( uint16_t i = 0; i < this->_periods.size(); i++) {

				if( _millis < this->_periods[i]._last_millis ){
		      this->_periods[i]._last_millis = _millis;
		    }

		    if( ( _millis - this->_periods[i]._last_millis ) > this->_periods[i]._duration ){

					if( this->_periods[i]._cb ) this->_periods[i]._cb();
		      this->_periods[i]._last_millis = _millis;
					this->_periods[i]._max_attempts = this->_periods[i]._max_attempts > 0 ? this->_periods[i]._max_attempts-1:this->_periods[i]._max_attempts;
		    }

				// #ifdef EW_SERIAL_LOG
		    // Log(this->_periods[i]._event_id);Log("\t");
				// Log(this->_periods[i]._duration);Log("\t");
				// Log(this->_periods[i]._max_attempts);Log("\t");
				// Log(this->_periods[i]._last_millis);Logln();
		    // #endif

			}
			this->remove_expired_callbacks();
		}

		/**
		 * this function removes expired callback task from lists
		 */
		void remove_expired_callbacks(){

			for ( uint16_t i = 0; i < this->_periods.size(); i++) {

				if( this->_periods[i]._max_attempts == 0 ){
					this->_periods.erase( this->_periods.begin() + i );
		    }
			}
		}

		/**
		 * this function checks whether callback task is registered for given id.
		 * it returns id on found registered else it returns -1
		 *
		 * @param		int	_id
		 * @return	int
		 */
		int is_registered_callback( int _id ){

			for ( uint16_t i = 0; i < this->_periods.size(); i++) {

				if( this->_periods[i]._event_id == _id ){
					return i;
		    }
			}return -1;
		}

		/**
		 * this function remove callback task by given id.
		 * it returns true on removed successfully else it returns false on not found.
		 *
		 * @param		int	_id
		 * @return	bool
		 */
		bool remove_callback( int _id ){

			bool _removed = false;
			for ( uint16_t i = 0; i < this->_periods.size(); i++) {

				if( this->_periods[i]._event_id == _id ){
					this->_periods.erase( this->_periods.begin() + i );
					_removed = true;
		    }
			}return _removed;
		}

		/**
		 * this function returns available unique id required for callback task.
		 *
		 * @return	int
		 */
		int get_unique_id_for_cb( ){

			for ( int _id = 1; _id < MAX_PERIODIC_CALLBACKS; _id++) {

				bool _id_used = false;
				for ( int i = 0; i < this->_periods.size(); i++) {

					if( this->_periods[i]._event_id == _id ){
						_id_used = true;
			    }
				}
				if( !_id_used ) return _id;
			} return -1;
		}

	protected:

		/**
		 * @var	std::vector<periodic_cb_> vector
		 */
		std::vector<periodic_cb_> _periods;
};


/**
 * DeviceFactoryReset class
 */
class DeviceFactoryReset{

	public:

		/**
		 * DeviceFactoryReset constructor
		 */
		DeviceFactoryReset(){
		}

		/**
		 * this function perform the factory reset operation.
		 * it also performs the essential functionality registered as to be done
		 * before device reset
		 */
		void factory_reset(){

			for ( uint16_t i = 0; i < this->_factory_reset_cbs.size(); i++ ) {

				if( this->_factory_reset_cbs[i]._cb ) this->_factory_reset_cbs[i]._cb();
			}
			this->restart_device();
		}

		/**
		 * this function register the callback tasks which needs to be done before factory reset.
		 *
		 * @param		CallBackVoidArgFn	_fn
		 * @return	bool
		 */
		bool run_while_factory_reset( CallBackVoidArgFn _fn ){

			if( this->_factory_reset_cbs.size() < MAX_FACTORY_RESET_CALLBACKS ){
				factory_reset_cb_ _new_factory_reset_cb;
				_new_factory_reset_cb._cb = _fn;
				this->_factory_reset_cbs.push_back(_new_factory_reset_cb);
				return true;
			}return false;
		}

		/**
		 * reset the device.
		 */
		void reset_device( int _timeout=0 ){
		  delay( _timeout );
		  ESP.reset();
		}

		/**
		 * restart the device
		 */
		void restart_device( int _timeout=0 ){
		  delay( _timeout );
		  ESP.restart();
		}

	protected:

		/**
		 * @var	std::vector<factory_reset_cb_>	_factory_reset_cbs
		 */
		std::vector<factory_reset_cb_> _factory_reset_cbs;
};

#endif
