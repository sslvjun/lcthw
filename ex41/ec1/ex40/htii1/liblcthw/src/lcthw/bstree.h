#ifndef _lcthw_BSTree_h
#define _lcthw_BSTree_h

#include <lcthw/hashmap.h>
#include <stdint.h>

extern char BSTREE_ERROR;
#define BSTREE_ERROR_POINTER ((void *)&BSTREE_ERROR)

typedef int (*BSTree_compare)(void *a, void *b);

typedef struct BSTreeNode {
	void *key;
	void *data;
	uint32_t hash;

	struct BSTreeNode *left;
	struct BSTreeNode *right;
	struct BSTreeNode *parent;
} BSTreeNode;

typedef struct BSTree {
	int count;
	Hashmap_hash hash_func;
	BSTree_compare compare;
	BSTreeNode *root;
} BSTree;

typedef int (*BSTree_traverse_cb)(BSTreeNode *node);

BSTree *BSTree_create(BSTree_compare compare, Hashmap_hash hash_func);
void BSTree_destroy(BSTree *map);

int BSTree_set(BSTree *map, void *key, void *data);
void *BSTree_get(BSTree *map, void *key);

int BSTree_traverse(BSTree *map, BSTree_traverse_cb traverse_cb);
int BSTree_traverse_reverse(BSTree *map, BSTree_traverse_cb traverse_cb);

void *BSTree_delete(BSTree *map, void *key);

#endif
