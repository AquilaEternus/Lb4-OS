/*
 * elevator clook
 * Modified from noop-iosched.c to implement C-LOOK algorithm.
 * Modified by: Jose Hernandez
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct clook_data {
	struct list_head queue;
};

static void clook_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

/* Added a printk statement after a request had been sent to the dispatch
 * queue to track the direction (READ or WRITE) and the location of the
 * request.
 */
static int clook_dispatch(struct request_queue *q, int force)
{
	struct clook_data *nd = q->elevator->elevator_data;
	char direction;
	if (!list_empty(&nd->queue)) {
		struct request *rq;
		rq = list_entry(nd->queue.next, struct request, queuelist);
		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
		direction = (rq_data_dir(rq) == READ) ? 'R' : 'W';
		printk("[CLOOK] dsp %c %lu\n", direction, blk_rq_pos(rq));
		return 1;
	}
	return 0;
}

/* Traverses through the queuelist and places the request passed
 * to rq parameter before the request with a larger location in memory. This 
 * sorts the lists of requests in ascending order and implements the C-LOOK 
 * algorithm. After the request has been added, a printk statement states 
 * the direction of the request(READ or WRITE) and the location of 
 * the request block.
 */
static void clook_add_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;
	/*Pointer to the current rq in the list*/
	struct list_head *curr = NULL;
	char direction;
	/*Traverses the linked list to find where rq can be placed based on
	its location.*/
	list_for_each(curr, &nd->queue){
		struct request *currReq = list_entry(curr, struct request, queuelist);
		if(rq_end_sector(rq) < rq_end_sector(currReq))
			break;
		
	}
	list_add_tail(&rq->queuelist, curr);
	direction = (rq_data_dir(rq) == READ) ? 'R' : 'W';
	printk("[CLOOK] add %c %lu\n", direction, blk_rq_pos(rq));
}

static struct request *
clook_former_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
clook_latter_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static int clook_init_queue(struct request_queue *q)
{
	struct clook_data *nd;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd)
		return -ENOMEM;

	INIT_LIST_HEAD(&nd->queue);
	q->elevator->elevator_data = nd;
	return 0;
}

static void clook_exit_queue(struct elevator_queue *e)
{
	struct clook_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_clook = {
	.ops = {
		.elevator_merge_req_fn		= clook_merged_requests,
		.elevator_dispatch_fn		= clook_dispatch,
		.elevator_add_req_fn		= clook_add_request,
		.elevator_former_req_fn		= clook_former_request,
		.elevator_latter_req_fn		= clook_latter_request,
		.elevator_init_fn		= clook_init_queue,
		.elevator_exit_fn		= clook_exit_queue,
	},
	.elevator_name = "clook",
	.elevator_owner = THIS_MODULE,
};

static int __init clook_init(void)
{
	return elv_register(&elevator_clook);
}

static void __exit clook_exit(void)
{
	elv_unregister(&elevator_clook);
}

module_init(clook_init);
module_exit(clook_exit);


MODULE_AUTHOR("Jose Hernandez");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CLOOK IO scheduler");
