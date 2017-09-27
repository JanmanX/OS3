#include "list.h"
#include <libc.h>
#include <mem/mem.h>

list *list_create(void)
{
        list_t *list = malloc(sizeof(list_t));
        ASSERT(list != NULL, "Failed to malloc memory");
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
        LIST_FOREACH_NODE(list, first, next, cur)
        {
                free(cur->value);
                cur->value = NULL;
        }
}

void list_push(list_t *list, void *value)
{
        list_node_t *node = malloc(sizeof(list_node_t));
	ASSERT(node != NULL, "Malloc returned NULL");

        node->value = value;
        if(list->last == NULL)
        {
                list->first = node;
                list->last = node;
        }
        else
        {
                list->last->next = node;
                node->prev = list->last;
                list->last = node;
        }

        list->count++;
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

        if(node == list->first && node == list->last)
        {/* If only element in list (both first and last element) */
                list->first = NULL;
                list->last = NULL;
        }
        else if(node == list->first)
        {
                list->first = list->first->next;
                list->first->prev = NULL;
        }
        else if(node == list->last)
        {
                list->last = list->last->prev;
                list->last->next = NULL;
        }
        else {/* Else if somewhere in the middle of the list */
		node->prev->next = node->next;
		node->next->prev = node->prev;
        }

        list->count--;
        result = node->value;
        free(node);

	return result;
}
