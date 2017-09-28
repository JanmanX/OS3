#include <tree.h>
#include <libc.h>
#include <stdint.h>
#include <types.h>
#include <list.h>


tree_t* tree_create(void)
{
	tree_t* t = malloc(sizeof(tree_t));
	ASSERT(t != NULL, "Could not malloc tree");

	t->data = NULL;
	t->count = 0;
	t->children = list_create();
	t->parent = NULL;

	return t;
}


tree_t* tree_insert_child(tree_t* parent, void* data)
{
	/* Create the child */
	tree_t *c = malloc(sizeof(tree_t));
	ASSERT(c != NULL, "Could not allocate node");

	c->data = data;
	c->count = 0;
	c->children = list_create();

	/* Insert into tree */
	tree_insert_tree(parent, c);

	return c;
}


void tree_free(tree_t* tree)
{
	if(tree == NULL) {
		return;
	}

	/* For each child, call recursivelly */
	LIST_FOREACH_NODE(tree->children, child) {
		tree_free((tree_t*)child->value);
	}

	free(tree->children);
	free(tree);
}


void tree_insert_tree(tree_t* parent, tree_t* child)
{
	list_push(parent->children, child);
	parent->count += 1 + child->count;
	child->parent = parent;
}

void tree_remove_tree(tree_t* parent, tree_t* child)
{
	LIST_FOREACH_NODE(parent->children, n) {
		if(child == (tree_t*)n->value) {
			break;
		}
	}

	list_remove(parent->children, n);

	parent->count -= (1 + child->count);

	child->parent = NULL;
}



