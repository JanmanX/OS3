#include "test.h"
#include <stdint.h>
#include <types.h>
#include <list.h>
#include <tree.h>


uint64_t tree_test(void)
{
	uint64_t errors = 0x00;
	char *data = "DATA";

	/* Test creation */
	tree_t* t = tree_create();
	TEST_NOT_EQUAL(t, NULL);

	/* Test insertion */
	tree_insert_child(t, (void*)NULL);
	TEST_EQUAL(t->count, 1);
	tree_insert_child(t, (void*)data);
	TEST_EQUAL(t->count, 2);

	/* Test tree insertion */
	tree_t* t2 = tree_create();
	tree_insert_child(t2, (void*)data);
	tree_insert_child(t2, (void*)data);
	tree_insert_tree(t, t2);
	TEST_EQUAL(t->count, 5);

	/* Test tree removal */
	list_node_t* search = list_find(t->children, t2);
	tree_remove_tree(t, search->value);
	TEST_EQUAL(t->count, 2);
	LIST_FOREACH_VALUE(t->children, val) {
		TEST_NOT_EQUAL(val, t2);
	}


	return errors;
}
