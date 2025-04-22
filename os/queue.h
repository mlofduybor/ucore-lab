#ifndef QUEUE_H
#define QUEUE_H
#define QUEUE_SIZE (1024)

// TODO: change the queue to a priority queue sorted by priority

struct queue_data {
	int index_pool;
	int priority;
};

struct queue {
	struct queue_data data[QUEUE_SIZE + 1];
	// int front;
	// int tail;
	int size ;
	// int empty;
};

void init_queue(struct queue *);
void push_queue(struct queue *, int ,int );
int pop_queue(struct queue *);

#endif // QUEUE_H
