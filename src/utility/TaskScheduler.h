/**************************** TaskScheduler ***********************************
This file is part of the Ewings Esp8266 Stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef __TASK_SCHEDULER_H__
#define __TASK_SCHEDULER_H__


#include <config/Config.h>


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

		int setTimeout( CallBackVoidArgFn _task_fn, uint32_t _duration );
		int setInterval( CallBackVoidArgFn _task_fn, uint32_t _duration );
		int updateInterval( int _task_id, CallBackVoidArgFn _task_fn, uint32_t _duration, unsigned long _last_millis=0, int _max_attampts=-1 );
		bool clearTimeout( int _id );
		bool clearInterval( int _id );
		int register_task( CallBackVoidArgFn _task_fn, uint32_t _duration, unsigned long _last_millis=0, int _max_attampts=-1 );
		void handle_tasks( unsigned long _millis=millis() );
		void remove_expired_tasks( void );
		int is_registered_task( int _id );
		bool remove_task( int _id );
		int get_unique_task_id( void );

	protected:

		/**
		 * @var	std::vector<task_t> vector
		 */
		std::vector<task_t> _tasks;
};

extern TaskScheduler __task_scheduler;

#endif
