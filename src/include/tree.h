#pragma once
#include <stdint.h>
#include <list.h>


typedef struct tree {
	void* data;		/* Pointer to the data */
	size_t count;		/* Number of children */

	struct tree* parent;	/* Parent tree node */
	list_t* children;	/* List of children */

} tree_t;


/* Creates a new tree on the heap */
tree_t* tree_create(void);

/* Frees the tree, and all its children. Does not free the values */
void tree_free(tree_t* tree);

/* Creates a new node with <data> and adds it as child of <parent> */
tree_t* tree_insert_child(tree_t* parent, void* data);

/* Adds a subtree under <parent> */
void tree_insert_tree(tree_t* parent, tree_t* child);

/* Remove child from parent :( */
void tree_remove_tree(tree_t* parent, tree_t* child);


