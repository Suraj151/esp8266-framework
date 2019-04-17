#ifndef __EWINGS_UTILITY_H__
#define __EWINGS_UTILITY_H__

#define MAX_PERIODIC_CALLBACKS	20
#define MAX_FACTORY_RESET_CALLBACKS	MAX_PERIODIC_CALLBACKS

#include <Esp.h>
#include <database/StoreStruct.h>
#include "DataTypeConversions.h"
#include "StringOperations.h"
#include "queue/queue.h"
#include "Log.h"

typedef std::function<void(int)> CallBackIntArgFn;
typedef std::function<void(void)> CallBackVoidArgFn;

template <typename T> void PROGMEM_readAnything (const T * sce, T& dest)
{
	memcpy_P (&dest, sce, sizeof (T));
}

template <typename T> T PROGMEM_getAnything (const T * sce)
{
	static T temp;
	memcpy_P (&temp, sce, sizeof (T));
	return temp;
}

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

class PeriodicCallBack{

	public:

		PeriodicCallBack(){
		}

		PeriodicCallBack( CallBackVoidArgFn _fn, uint32_t _duration, unsigned long _last_millis=0 ){

			this->register_periodic_callback( _fn, _duration, _last_millis );
		}

		int setTimeout( CallBackVoidArgFn _fn, uint32_t _duration ){
			return this->register_periodic_callback( _fn, _duration, millis(), 1 );
		}

		int setInterval( CallBackVoidArgFn _fn, uint32_t _duration ){
			return this->register_periodic_callback( _fn, _duration, millis() );
		}

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

		bool clearTimeout( int _id ){
			return this->remove_callback( _id );
		}

		bool clearInterval( int _id ){
			return this->remove_callback( _id );
		}

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

		void remove_expired_callbacks(){

			for ( uint16_t i = 0; i < this->_periods.size(); i++) {

				if( this->_periods[i]._max_attempts == 0 ){
					this->_periods.erase( this->_periods.begin() + i );
		    }
			}
		}

		int is_registered_callback( int _id ){

			for ( uint16_t i = 0; i < this->_periods.size(); i++) {

				if( this->_periods[i]._event_id == _id ){
					return i;
		    }
			}return -1;
		}

		bool remove_callback( int _id ){

			bool _removed = false;
			for ( uint16_t i = 0; i < this->_periods.size(); i++) {

				if( this->_periods[i]._event_id == _id ){
					this->_periods.erase( this->_periods.begin() + i );
					_removed = true;
		    }
			}return _removed;
		}

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

		std::vector<periodic_cb_> _periods;
};


class DeviceFactoryReset{

	public:

		DeviceFactoryReset(){
		}

		void factory_reset(){

			for ( uint16_t i = 0; i < this->_factory_reset_cbs.size(); i++ ) {

				if( this->_factory_reset_cbs[i]._cb ) this->_factory_reset_cbs[i]._cb();
			}
			this->restart_device();
		}

		bool run_while_factory_reset( CallBackVoidArgFn _fn ){

			if( this->_factory_reset_cbs.size() < MAX_FACTORY_RESET_CALLBACKS ){
				factory_reset_cb_ _new_factory_reset_cb;
				_new_factory_reset_cb._cb = _fn;
				this->_factory_reset_cbs.push_back(_new_factory_reset_cb);
				return true;
			}return false;
		}

		void reset_device( int _timeout=0 ){
		  delay( _timeout );
		  ESP.reset();
		}

		void restart_device( int _timeout=0 ){
		  delay( _timeout );
		  ESP.restart();
		}

	protected:

		std::vector<factory_reset_cb_> _factory_reset_cbs;
};

#endif
