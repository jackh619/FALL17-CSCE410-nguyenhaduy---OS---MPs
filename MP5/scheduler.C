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

Node::Node () {
    thread = NULL;
    next_node = NULL;
    previous_node = NULL;
}

Node::Node (Thread* _thread) {
    thread = _thread;
    next_node = NULL;
    previous_node = NULL;
}

Node::Node (Node& _node) {
    thread = _node.thread;
    next_node = _node.next_node;
    previous_node = _node.previous_node;
}

Queue::Queue () {
    size = 0;
    head = NULL;
    tail = NULL;
}

int Queue::get_size() {
    return size;
}

bool Queue::is_empty() {
    if (size) {
      	return false;
    }
    return true;
}

void Queue::enqueue (Thread* _thread) {
    Node* new_node = new Node (_thread);
    new_node->next_node = NULL;
    ++size;
    if (head == NULL) {
      	head = new_node;
    	Console::puts("Create head of queue!\n");
    }
    else {
      	tail->next_node = new_node;      	
    }    
    tail = new_node;
    // return;
}

Thread* Queue::dequeue () {
    // if (size == 0) {
    //   return NULL;
    // }
    if (head == NULL)
    	Console::puts("Cannot dequeue an empty queue\n");
    else {
    	Node* temp = head;
    	head = head->next_node;
    	--size;    	
    	return temp->thread;
    }    
    // return NULL;
}

Thread* Queue::front () {
    if (size == 0) {
      	return NULL;
    }
    return head->thread;
}

Scheduler::Scheduler() {
  	// assert(false);
  	Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  // assert(false);
	if (Machine::interrupts_enabled())
		Machine::disable_interrupts();

	Thread* next_thread = ready_queue.dequeue();
	if (next_thread == NULL) {
		Console::puts("Error getting next thread!!!\n");
		while (true);
	}
	else {
    	Console::puts("Yeild to next thread in the queue!\n");
		Thread::dispatch_to (next_thread);
	}
	
	if (!Machine::interrupts_enabled())
		Machine::enable_interrupts();
	return;
}

void Scheduler::resume(Thread * _thread) {
  // assert(false);
	if (Machine::interrupts_enabled())
		Machine::disable_interrupts();

    Console::puts("Resume to a thread in the queue!\n");
	ready_queue.enqueue(_thread);

	if (!Machine::interrupts_enabled())
		Machine::enable_interrupts();
	return;
}

void Scheduler::add(Thread * _thread) {
  // assert(false);
	if (Machine::interrupts_enabled())
		Machine::disable_interrupts();

    Console::puts("Add new thread to end of queue!\n");
	ready_queue.enqueue(_thread);

	if (!Machine::interrupts_enabled())
		Machine::enable_interrupts();
	return;
}

void Scheduler::terminate(Thread * _thread) {
  // assert(false);
	if (Machine::interrupts_enabled())
		Machine::disable_interrupts();

	Console::puts("Terminated a thread!!!\n");
	Node* temp_node = ready_queue.head;

	while (temp_node != NULL) {
		if (temp_node->thread == _thread) {
			temp_node->previous_node->next_node = temp_node->next_node;
			temp_node->next_node->previous_node = temp_node->previous_node;
			--ready_queue.size;
			delete _thread; 
			// break;
		}
		temp_node = temp_node->next_node;
	}

	if (!Machine::interrupts_enabled())
		Machine::enable_interrupts();

	return;
}
