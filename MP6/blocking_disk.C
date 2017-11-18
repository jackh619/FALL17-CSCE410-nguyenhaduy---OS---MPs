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
#include "scheduler.H"
#include "machine.H"

/*--------------------------------------------------------------------------*/
/* EXTERNS */
/*--------------------------------------------------------------------------*/

extern Scheduler* SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/
// bool SimpleDisk::write_lock = false;


BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
  	write_lock = false;
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  	// register the thread that is to block as well as the corresponding disk number
	block_queue.enqueue(Thread::CurrentThread());
	
	// tell the disk the thread is about to apply I/O operation
	issue_operation(READ, _block_no);
	
	// any time the thread gains CPU, it checks if the disk is ready
	while (!is_ready()) {
		// if not, gives up CPU and wait
		SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
		SYSTEM_SCHEDULER->yield();
	}
	
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
  while (write_lock) {}   //wait other threads to finish writing
	write_lock = true;   //set the lock
	
	// Register a new thread to the block_thread queue
	block_queue.enqueue(Thread::CurrentThread());


	// tell the disk the thread is about to apply I/O operation
	issue_operation(WRITE, _block_no);
	
	// If disk thread gains CPU access, check if the disk is ready
	// If the disk is not ready, yield the CPU to other threads
	while (!is_ready()) {
		SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
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

	write_lock = false;   //free the lock
}
