#pragma once

typedef struct list_node
{
        struct list_node *next;
        struct list_node *prev;
        void *value;
} list_node_t;

typedef struct list
{
        int count;
        list_node_t *first;
        list_node_t *last;
} list_t;

/* Creats new list on heap and returns it */
list_t *list_create(void);

/* Frees the list, as well as all nodes in the list
 * (does not free the values!) */
void list_destroy(list_t *list);

/* Frees all values in the list */
void list_clear(list_t *list);

#define list_count(A) ((A)->count)
#define list_first(A) ((A)->first != NULL ? (A)->first->value : NULL)
#define list_last(A) ((A)->last != NULL ? (A)->last->value : NULL)

/* Pushes an element to the end of the list */
list_node_t* list_push(list_t *list, void *value);

/* Removes and returns the last element of the list */
void *list_pop(list_t *list);

/* Removes an element from list */
void *list_remove(list_t *list, list_node_t *node);

/* Removes all occurences of an element from list, by value */
void *list_find(list_t *list, void *search);



#define LIST_FOREACH_VALUE(L,V) list_node_t *_node;\
			void* V = NULL;\
	for(_node = L->first, V = _node->value;\
	    _node != NULL;\
	    _node = _node->next, V = _node->value)

#define LIST_FOREACH_NODE(L,N) list_node_t* N;\
	for(N = L->first; N != NULL; N = N->next)



/*
 * Past me made this weird macro. Wonder why?
#define LIST_FOREACH(L,S,M,V) list_node_t *_node = NULL;\
                                                list_node_t *V = NULL;\
for(V = _node = L->S; _node != NULL; V = _node = _node->M)
*/

