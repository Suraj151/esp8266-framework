/**************************** TaskScheduler ***********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#include "TaskScheduler.h"


/**
 * set callback function which will execute once after given duration.
 * it will return callback task id for reference.
 *
 * @param		CallBackVoidArgFn	_task_fn
 * @param		uint32_t	_duration
 * @return	int
 */
int TaskScheduler::setTimeout( CallBackVoidArgFn _task_fn, uint32_t _duration ){
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
int TaskScheduler::setInterval( CallBackVoidArgFn _task_fn, uint32_t _duration ){
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
 * @param		int|-1	_max_attempts
 * @return	int
 */
int TaskScheduler::updateInterval( int _task_id, CallBackVoidArgFn _task_fn, uint32_t _duration, unsigned long _last_millis, int _max_attempts ){

	int _registered_index = this->is_registered_task( _task_id );
	if( _registered_index > -1 ){

		this->_tasks[_registered_index]._task = _task_fn;
		this->_tasks[_registered_index]._duration = _duration;
		this->_tasks[_registered_index]._last_millis = _last_millis;
		this->_tasks[_registered_index]._max_attempts = _max_attempts;
		return _task_id;
	}else{

		return this->register_task( _task_fn, _duration,_last_millis, _max_attempts );
	}
}

/**
 * this function clears the callback task of given ref. callback task id
 * it return success/failure of operation.
 *
 * @param		int	_id
 * @return	bool
 */
bool TaskScheduler::clearTimeout( int _id ){
	return this->remove_task( _id );
}

/**
 * this function clears the callback task of given ref. callback task id
 * it return success/failure of operation.
 *
 * @param		int	_id
 * @return	bool
 */
bool TaskScheduler::clearInterval( int _id ){
	return this->remove_task( _id );
}

/**
 * this function registers callback task of given duration with unique id.
 * it will return callback task id for reference.
 *
 * @param		CallBackVoidArgFn	_task_fn
 * @param		uint32_t	_duration
 * @param		unsigned long|0	_last_millis
 * @param		int|-1	_max_attempts
 * @return	int
 */
int TaskScheduler::register_task( CallBackVoidArgFn _task_fn, uint32_t _duration, unsigned long _last_millis, int _max_attempts ){

	if( this->_tasks.size() < MAX_SCHEDULABLE_TASKS ){

		task_t _new_task;
		_new_task._task = _task_fn;
		_new_task._duration = _duration;
		_new_task._last_millis = _last_millis;
		_new_task._max_attempts = _max_attempts;
		_new_task._task_id = this->get_unique_task_id();
		this->_tasks.push_back(_new_task);
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
void TaskScheduler::handle_tasks( unsigned long _millis ){

	// #ifdef EW_SERIAL_LOG
	// Log(F("in handle task callbacks : current millis = "));Logln(_millis);
	// Logln(F("id\tduration\tattempts\tlast_millis"));
	// #endif
	for ( uint16_t i = 0; i < this->_tasks.size(); i++) {

		if( _millis < this->_tasks[i]._last_millis ){
      this->_tasks[i]._last_millis = _millis;
    }

    if( ( _millis - this->_tasks[i]._last_millis ) > this->_tasks[i]._duration ){

			if( this->_tasks[i]._task ) this->_tasks[i]._task();
      this->_tasks[i]._last_millis = _millis;
			this->_tasks[i]._max_attempts = this->_tasks[i]._max_attempts > 0 ? this->_tasks[i]._max_attempts-1:this->_tasks[i]._max_attempts;
    }

		// #ifdef EW_SERIAL_LOG
    // Log(this->_tasks[i]._task_id);Log("\t");
		// Log(this->_tasks[i]._duration);Log("\t");
		// Log(this->_tasks[i]._max_attempts);Log("\t");
		// Log(this->_tasks[i]._last_millis);Logln();
    // #endif

	}
	this->remove_expired_tasks();
}

/**
 * this function removes expired callback task from lists
 */
void TaskScheduler::remove_expired_tasks(){

	for ( uint16_t i = 0; i < this->_tasks.size(); i++) {

		if( this->_tasks[i]._max_attempts == 0 ){
			this->_tasks.erase( this->_tasks.begin() + i );
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
int TaskScheduler::is_registered_task( int _id ){

	for ( uint16_t i = 0; i < this->_tasks.size(); i++) {

		if( this->_tasks[i]._task_id == _id ){
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
bool TaskScheduler::remove_task( int _id ){

	bool _removed = false;
	for ( uint16_t i = 0; i < this->_tasks.size(); i++) {

		if( this->_tasks[i]._task_id == _id ){
			this->_tasks.erase( this->_tasks.begin() + i );
			_removed = true;
    }
	}return _removed;
}

/**
 * this function returns available unique id required for callback task.
 *
 * @return	int
 */
int TaskScheduler::get_unique_task_id( ){

	for ( int _id = 1; _id < MAX_SCHEDULABLE_TASKS; _id++) {

		bool _id_used = false;
		for ( int i = 0; i < this->_tasks.size(); i++) {

			if( this->_tasks[i]._task_id == _id ){
				_id_used = true;
	    }
		}
		if( !_id_used ) return _id;
	} return -1;
}

TaskScheduler __task_scheduler;
