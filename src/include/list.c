#include "list.h"
#include <libc.h>
#include <mem/mem.h>

list_t* list_create(void)
{
        list_t *list = malloc(sizeof(list_t));
        ASSERT(list != NULL, "Failed to malloc memory");

	list->first = NULL;
	list->last = NULL;
	list->count = 0x00;

        return list;
}

void list_destroy(list_t *list)
{
        ASSERT(list != NULL, "list is NULL");

        LIST_FOREACH_NODE(list, cur)
        {
                if(cur->prev)
                {
                        free(cur->prev);
                }
        }

        free(list->last);
        free(list);
}

void list_clear(list_t *list)
{
        LIST_FOREACH_NODE(list, cur)
        {
                free(cur->value);
                cur->value = NULL;
        }
}

list_node_t* list_push(list_t *list, void *value)
{

        list_node_t *node = malloc(sizeof(list_node_t));
	ASSERT(node != NULL, "Malloc returned NULL");

        node->value = value;
	node->next = NULL; /* It is pushed, so its always last element */
	list->last = node;

	if(list->first == NULL) {
		list->first = node;
		node->prev = NULL;
	} else {
		/* Iterate to the last element from the start */
		list_node_t* iter = list->first;
		while(iter->next != NULL) {
			iter = iter->next;
		}
		iter->next = node;
		node->prev = iter;
	}

	list->count++;

	return node;
}

void *list_pop(list_t *list)
{
	ASSERT(list != NULL, "list is NULL");

	list_node_t *node = list->last;
	return node != NULL ? list_remove(list, node) : NULL;
}


void *list_remove(list_t *list, list_node_t *node)
{
	ASSERT(list != NULL, "list is NULL");

	void *result = NULL;

	LIST_FOREACH_NODE(list, cur) {
		if(cur != node) {
			continue;
		}

		/* Element has been found. Remove it from list appropriatelly */


		if(node == list->first) {
			list->first = node->next;
		}

		if(node == list->last) {
			list->last = node->prev;
		}

		if(node->prev != NULL) {
			node->prev->next = node->next;
		}

		if(node->next != NULL) {
			node->next->prev = node->prev;
		}

		list->count--;
		result = node->value;
		free(node);

		return result;


	}

	LOG("ITEM NOT FOUND");
	return NULL;
}

void *list_find(list_t *list, void *search)
{
	LIST_FOREACH_NODE(list, cur) {
		if(cur->value == search)  {
			return cur;
		}
	}

	return NULL;
}


