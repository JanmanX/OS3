#include "test.h"
#include <stdint.h>
#include <types.h>
#include <list.h>
#include <tree.h>

uint64_t list_test(void)
{
	uint64_t errors = 0x00;
	char *data = "DATA";
	char *data2= "DATA2";

	/* Test creation */
	list_t* list = list_create();
	TEST_NOT_EQUAL(list, NULL);
	TEST_EQUAL(list->count, 0x00);

	/* Test insertion */
	list_node_t* node1 = list_push(list, data);
	TEST_EQUAL(list->count, 0x01);
	TEST_EQUAL(list->first, node1);
	TEST_EQUAL(list->last, node1);

	list_node_t* node2 = list_push(list, data2);
	TEST_EQUAL(list->last, node2);
	TEST_NOT_EQUAL(list->first, node2);


	/* Test removal */
	list_remove(list, node1);
	TEST_EQUAL(list->count, 0x01);
	TEST_EQUAL(list->first, node2);
	TEST_EQUAL(list->last, node2);

	list_pop(list);
	TEST_EQUAL(list->count, 0x00);
	TEST_EQUAL(list->first, NULL);
	TEST_EQUAL(list->last, NULL);

	if(errors == 0) {
		LOG("list_tests passed");
	}
	return errors;
}
