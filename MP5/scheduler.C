/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  // assert(false);
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  // assert(false);
	Thread* next_thread = ready_queue.dequeue();
	if (next_thread == NULL) {
		Console.puts("Error getting next thread!!!\n");
		while (true);
	}
	else {
		Thread::dispatch_to (next_thread);
	}
}

void Scheduler::resume(Thread * _thread) {
  // assert(false);
	ready_queue.enqueue(_thread);
}

void Scheduler::add(Thread * _thread) {
  // assert(false);
	ready_queue.enqueue(_thread)
}

void Scheduler::terminate(Thread * _thread) {
  // assert(false);
	delete _thread;
}
