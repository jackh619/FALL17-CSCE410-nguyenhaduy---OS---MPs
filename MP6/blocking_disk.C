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

	Thread* current_thread = Thread::CurrentThread();
	
	// Register a new thread to the block_thread queue
	block_queue.enqueue(current_thread);
	
	// Issue read disk operation
	issue_operation(READ, _block_no);
	
	// If disk thread gains CPU access, check if the disk is ready
	// If the disk is not ready, yield the CPU to other threads
	// while (!is_ready()) {
	// 	// if not, gives up CPU and wait
	// 	if (block_queue.size != 0) {
	// 		SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
	// 		Thread* target = block_queue.dequeue();
	// 		SYSTEM_SCHEDULER->resume(target);
	// 		SYSTEM_SCHEDULER->yield();

	// 	}

	// }
	
	// if yes, apply I/O operation;
	int i;
	unsigned short tmpw;
	for (i = 0; i < 256; i++) {
		tmpw = Machine::inportw(0x1F0);
		_buf[i*2]   = (unsigned char)tmpw;
		_buf[i*2+1] = (unsigned char)(tmpw >> 8);
	}

	// after the I/O operation is done
	// dequeue the thread in the block queue
	// dequeue the disk in the disk queue
	block_queue.dequeue();

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {

	Thread* current_thread = Thread::CurrentThread();
	
	// Register a new thread to the block_thread queue
	block_queue.enqueue(current_thread);

	// Issue write disk operation
	issue_operation(WRITE, _block_no);
	
	// If disk thread gains CPU access, check if the disk is ready
	// If the disk is not ready, yield the CPU to other threads
	while (!is_ready()) {
		SYSTEM_SCHEDULER->add(current_thread);
		SYSTEM_SCHEDULER->yield();
	}
	
	// If the disk is ready, write data to port
	int i; 
	unsigned short tmpw;
	for (i = 0; i < 256; i++) {
		tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
		Machine::outportw(0x1F0, tmpw);
	}
	// after the I/O operation is done
	// dequeue the thread in the block queue
	// dequeue the disk in the disk queue
	block_queue.dequeue();

	// write_lock = false;   //free the lock
}


void BlockingDisk::interrupt_handler(REGS* _regs) {
	if (!Machine::interrupts_enabled()) {
		return;
	}

	// whenever the disk is ready to have I/O operation, this interrupt is trigerred
	Thread* thread_to_active = block_queue.front();
	
	if (thread_to_active == Thread::CurrentThread()) {
		// if this is the case, simply return and do no operations
		return;
	}
	else {
		// SYSTEM_SCHEDULER->preempt(thread_to_active);
		SYSTEM_SCHEDULER->add(Thread::CurrentThread());
		Thread::dispatch_to(thread_to_active);
		return;
	}
}