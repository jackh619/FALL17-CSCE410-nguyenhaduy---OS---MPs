/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "machine.H"

/*--------------------------------------------------------------------------*/
/* EXTERNS */
/*--------------------------------------------------------------------------*/

extern Scheduler* SYSTEM_SCHEDULER;

Queue BlockingDisk::block_queue;
bool BlockingDisk::write_lock = false;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/
// bool SimpleDisk::write_lock = false;


BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {

	
	
	// Issue read disk operation
	issue_operation(READ, _block_no);
	
	bool cont = true;

	// Register a new thread to the block_thread queue
	Thread* current_thread = Thread::CurrentThread();				
	block_queue.enqueue(current_thread);

	// If disk thread gains CPU access, check if the disk is ready
	// If the disk is not ready, yield the CPU to other threads
	while (!is_ready()) {
		current_thread = Thread::CurrentThread();

		if (block_queue.front() != NULL) {		
			if (current_thread->get_thread_id() == block_queue.front()->get_thread_id()) {
				cont = false;
			}
			else {
				cont = true;
				SYSTEM_SCHEDULER->resume(current_thread);
				SYSTEM_SCHEDULER->yield();	
			}
		}		
		Console::puts("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	}
	
	block_queue.dequeue();
	
	// if yes, apply I/O operation;
	int i;
	unsigned short tmpw;
	for (i = 0; i < 256; i++) {
		tmpw = Machine::inportw(0x1F0);
		_buf[i*2]   = (unsigned char)tmpw;
		_buf[i*2+1] = (unsigned char)(tmpw >> 8);
	}
}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {

	// Issue write disk operation
	issue_operation(WRITE, _block_no);
		// Console::puts("#################################################################\n");
	
	bool cont = true;

	// Register a new thread to the block_thread queue
	Thread* current_thread = Thread::CurrentThread();				
	block_queue.enqueue(current_thread);

	// If disk thread gains CPU access, check if the disk is ready
	// If the disk is not ready, yield the CPU to other threads
	while (!is_ready()) {
		current_thread = Thread::CurrentThread();	

		if (block_queue.front() != NULL) {		
			if (current_thread->get_thread_id() == block_queue.front()->get_thread_id()) {
				cont = false;
			}
			else {
				cont = true;
				SYSTEM_SCHEDULER->resume(current_thread);
				SYSTEM_SCHEDULER->yield();	
			}
		}		

		Console::puts("#################################################################\n");

	}

	block_queue.dequeue();

	
	// If the disk is ready, write data to port
	int i; 
	unsigned short tmpw;
	for (i = 0; i < 256; i++) {
		tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
		Machine::outportw(0x1F0, tmpw);
	}
}


// void BlockingDisk::interrupt_handler(REGS* _regs) {
// 	if (!Machine::interrupts_enabled()) {
// 		return;
// 	}

// 	// whenever the disk is ready to have I/O operation, this interrupt is trigerred
// 	Thread* thread_to_active = block_queue.front();
	
// 	if (thread_to_active == Thread::CurrentThread()) {
// 		// if this is the case, simply return and do no operations
// 		return;
// 	}
// 	else {
// 		// SYSTEM_SCHEDULER->preempt(thread_to_active);
// 		SYSTEM_SCHEDULER->add(Thread::CurrentThread());
// 		Thread::dispatch_to(thread_to_active);
// 		return;
// 	}
// }