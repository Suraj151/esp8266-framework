/**************************** TaskScheduler ***********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef __EWINGS_TASK_SCHEDULER_H__
#define __EWINGS_TASK_SCHEDULER_H__

/**
 * max callbacks
 */
#define MAX_PERIODIC_CALLBACKS	20
#define MAX_FACTORY_RESET_CALLBACKS	MAX_PERIODIC_CALLBACKS

#include <Esp.h>
#include <config/Config.h>

/**
 * callback type
 */
typedef std::function<void(int)> CallBackIntArgFn;
typedef std::function<void(void)> CallBackVoidArgFn;


/**
 * task struct type for scheduler
 */
struct task_t {
	int _task_id;
	int _max_attempts;
	uint32_t _duration;
	unsigned long _last_millis;
	CallBackVoidArgFn _task;
};

struct factory_reset_cb_ {
	CallBackVoidArgFn _cb;
};

/**
 * TaskScheduler class
 */
class TaskScheduler{

	public:

		/**
		 * TaskScheduler constructor
		 */
		TaskScheduler(){
		}

		/**
		 * TaskScheduler constructor
		 *
		 * @param	CallBackVoidArgFn	_task_fn
		 * @param	uint32_t	_duration
		 * @param	unsigned long|0	_last_millis
		 */
		TaskScheduler( CallBackVoidArgFn _task_fn, uint32_t _duration, unsigned long _last_millis=0 ){

			this->register_task( _task_fn, _duration, _last_millis );
		}

		/**
		 * set callback function which will execute once after given duration.
		 * it will return callback task id for reference.
		 *
		 * @param		CallBackVoidArgFn	_task_fn
		 * @param		uint32_t	_duration
		 * @return	int
		 */
		int setTimeout( CallBackVoidArgFn _task_fn, uint32_t _duration ){
			return this->register_task( _task_fn, _duration, millis(), 1 );
		}

		/**
		 * set callback function which will execute after every given duration.
		 * it will return callback task id for reference.
		 *
		 * @param		CallBackVoidArgFn	_task_fn
		 * @param		uint32_t	_duration
		 * @return	int
		 */
		int setInterval( CallBackVoidArgFn _task_fn, uint32_t _duration ){
			return this->register_task( _task_fn, _duration, millis() );
		}

		/**
		 * this function updates already registered callback task.
		 * it will return updated callback task id for reference.
		 *
		 * @param		int	_task_id
		 * @param		CallBackVoidArgFn	_task_fn
		 * @param		uint32_t	_duration
		 * @param		unsigned long|0	_last_millis
		 * @param		int|-1	_max_attampts
		 * @return	int
		 */
		int updateInterval( int _task_id, CallBackVoidArgFn _task_fn, uint32_t _duration, unsigned long _last_millis=0, int _max_attampts=-1 ){

			int _registered_index = this->is_registered_task( _task_id );
			if( _registered_index > -1 ){

				this->_periods[_registered_index]._task = _task_fn;
				this->_periods[_registered_index]._duration = _duration;
				this->_periods[_registered_index]._last_millis = _last_millis;
				this->_periods[_registered_index]._max_attempts = _max_attampts;
				return _task_id;
			}else{

				return this->register_task( _task_fn, _duration,_last_millis, _max_attampts );
			}
		}

		/**
		 * this function clears the callback task of given ref. callback task id
		 * it return success/failure of operation.
		 *
		 * @param		int	_id
		 * @return	bool
		 */
		bool clearTimeout( int _id ){
			return this->remove_task( _id );
		}

		/**
		 * this function clears the callback task of given ref. callback task id
		 * it return success/failure of operation.
		 *
		 * @param		int	_id
		 * @return	bool
		 */
		bool clearInterval( int _id ){
			return this->remove_task( _id );
		}

		/**
		 * this function registers callback task of given duration with unique id.
		 * it will return callback task id for reference.
		 *
		 * @param		CallBackVoidArgFn	_task_fn
		 * @param		uint32_t	_duration
		 * @param		unsigned long|0	_last_millis
		 * @param		int|-1	_max_attampts
		 * @return	int
		 */
		int register_task( CallBackVoidArgFn _task_fn, uint32_t _duration, unsigned long _last_millis=0, int _max_attampts=-1 ){

			if( this->_periods.size() < MAX_PERIODIC_CALLBACKS ){

				task_t _new_task;
				_new_task._task = _task_fn;
				_new_task._duration = _duration;
				_new_task._last_millis = _last_millis;
				_new_task._max_attempts = _max_attampts;
				_new_task._task_id = this->get_unique_task_id();
				this->_periods.push_back(_new_task);
				return _new_task._task_id;
			}
			return -1;
		}

		/**
		 * this function handles/executes callback task on their given time periods.
		 * this function must be called every millisecond in loop for proper task handles.
		 *
		 * @param	unsigned long _millis
		 */
		void handle_tasks( unsigned long _millis=millis() ){

			// #ifdef EW_SERIAL_LOG
			// Log(F("in handle task callbacks : current millis = "));Logln(_millis);
			// Logln(F("id\tduration\tattempts\tlast_millis"));
			// #endif
			for ( uint16_t i = 0; i < this->_periods.size(); i++) {

				if( _millis < this->_periods[i]._last_millis ){
		      this->_periods[i]._last_millis = _millis;
		    }

		    if( ( _millis - this->_periods[i]._last_millis ) > this->_periods[i]._duration ){

					if( this->_periods[i]._task ) this->_periods[i]._task();
		      this->_periods[i]._last_millis = _millis;
					this->_periods[i]._max_attempts = this->_periods[i]._max_attempts > 0 ? this->_periods[i]._max_attempts-1:this->_periods[i]._max_attempts;
		    }

				// #ifdef EW_SERIAL_LOG
		    // Log(this->_periods[i]._task_id);Log("\t");
				// Log(this->_periods[i]._duration);Log("\t");
				// Log(this->_periods[i]._max_attempts);Log("\t");
				// Log(this->_periods[i]._last_millis);Logln();
		    // #endif

			}
			this->remove_expired_tasks();
		}

		/**
		 * this function removes expired callback task from lists
		 */
		void remove_expired_tasks(){

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
		int is_registered_task( int _id ){

			for ( uint16_t i = 0; i < this->_periods.size(); i++) {

				if( this->_periods[i]._task_id == _id ){
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
		bool remove_task( int _id ){

			bool _removed = false;
			for ( uint16_t i = 0; i < this->_periods.size(); i++) {

				if( this->_periods[i]._task_id == _id ){
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
		int get_unique_task_id( ){

			for ( int _id = 1; _id < MAX_PERIODIC_CALLBACKS; _id++) {

				bool _id_used = false;
				for ( int i = 0; i < this->_periods.size(); i++) {

					if( this->_periods[i]._task_id == _id ){
						_id_used = true;
			    }
				}
				if( !_id_used ) return _id;
			} return -1;
		}

	protected:

		/**
		 * @var	std::vector<task_t> vector
		 */
		std::vector<task_t> _periods;
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
			this->reset_device();
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
			ESP.eraseConfig();
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
