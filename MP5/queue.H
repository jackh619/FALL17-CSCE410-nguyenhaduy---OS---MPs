/* 
    Author: R. Bettati, Joshua Capehart
            Department of Computer Science
            Texas A&M University
			
	    A queue.

*/
#ifndef QUEUE_H
#define QUEUE_H

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "utils.H"
#include "thread.H"

/*--------------------------------------------------------------------------*/
/* Queue */
/*--------------------------------------------------------------------------*/

class Node {

private:
  Thread* thread;
  Node* next_node;
  Node* previous_node;

public:
  Node () {
    thread = NULL;
    next_node = NULL;
    previous_node = NULL;
  }

  Node (Thread* _thread) {
    thread = _thread;
    next_node = NULL;
    previous_node = NULL;
  }

  Node (Node& _node) {
    thread = _node.thread;
    next_node = _node.next_node;
    previous_node = _node.previous_node;
  }
};


class Queue {

private:
  int size;
  Node* head;
  Node* tail;

public:
  Queue () {
    size = 0;
    head = new Node();
    tail = head;
  }

  int get_size() {
    return size;
  }

  bool is_empty() {
    if (size) {
      return false;
    }
    return true;
  }

  void enqueue (Thread* _thread) {
    Node new_node = new Node (_thread);
    tail->next_node = new_node;
    tail = new_node;
    ++size;
    return;
  }

  Thread* dequeue () {
    if (size == 0) {
      return NULL;
    }
    return head;
  }
}
	

#endif
