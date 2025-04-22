#include "queue.h"
#include "defs.h"

// void init_queue(struct queue *q)
// {
// 	q->front = q->tail = 0;
// 	q->empty = 1;
// }

// void push_queue(struct queue *q, int value)
// {
// 	if (!q->empty && q->front == q->tail) {
// 		panic("queue shouldn't be overflow");
// 	}

// 	q->empty = 0;
// 	q->data[q->tail] = value;
// 	q->tail = (q->tail + 1) % NPROC;
// }

// int pop_queue(struct queue *q)
// {
// 	if (q->empty)
// 		return -1;
// 	int value = q->data[q->front];
// 	q->front = (q->front + 1) % NPROC;
// 	if (q->front == q->tail)
// 		q->empty = 1;
// 	return value;
// }


void p_down(struct queue *q, int u) {
	int t = u;
	if (u * 2 <= q->size && q->data[u*2].priority < q->data[t].priority) t = u*2;
	if (u*2 + 1 <= q->size && q->data[u*2 + 1].priority < q->data[t].priority) t = u*2+1;
	if (t != u) {
		struct queue_data tmp = q->data[u];
		q->data[u] = q->data[t];
		q->data[t] = tmp;
		p_down(q, t);
	}
}



void p_up(struct queue *q, int u) {
	while(u/2 && q->data[u/2].priority > q->data[u].priority) {
		struct queue_data tmp = q->data[u];
		q->data[u] = q->data[u/2];
		q->data[u/2] = tmp;
		u = u/2;
	}
}


void init_queue(struct queue *q) {
	q->size = 0;
}


void push_queue(struct queue *q, int index_pool, int priority) {
	//if (q->size == 0) panic("queue shouldn't be overflow");
	struct queue_data tmp;
	tmp.index_pool = index_pool;
	tmp.priority = priority;
	q->data[++q->size] = tmp;
	p_up(q, q->size);
	// for (int i = 1;i<=q->size;i++) printf("queue: %d %d ", q->data[i].index_pool, q->data[i].priority);
	// printf("\n");
}

int pop_queue(struct queue *q) {
	if (q->size == 0)
		return -1;
	struct queue_data tmp = q->data[1];
	q->data[1] = q->data[q->size];
	q->size --;
	p_down(q, 1);
	return tmp.index_pool;
}
