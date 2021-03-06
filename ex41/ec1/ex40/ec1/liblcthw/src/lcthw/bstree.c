#include <lcthw/dbg.h>
#include <lcthw/bstree.h>
#include <stdlib.h>
#include <lcthw/bstrlib.h>

static int default_compare(void *a, void *b)
{
	return bstrcmp((bstring)a, (bstring)b);
}

BSTree *BSTree_create(BSTree_compare compare)
{
	BSTree *map = calloc(1, sizeof(BSTree));
	check_mem(map);

	map->compare = compare == NULL ? default_compare : compare;

	return map;

error:
	if(map) {
		BSTree_destroy(map);
	}
	return NULL;
}

static int BSTree_destroy_cb(BSTreeNode *node)
{
	free(node);
	return 0;
}

void BSTree_destroy(BSTree *map)
{
	if(map) {
		BSTree_traverse(map, BSTree_destroy_cb);
		free(map);
	}
}

static inline BSTreeNode *BSTreeNode_create(BSTreeNode *parent, void *key, void *data)
{
	BSTreeNode *node = calloc(1, sizeof(BSTreeNode));
	check_mem(node);

	node->key = key;
	node->data = data;
	node->parent = parent;
	return node;

error:
	return NULL;
}

static inline void BSTree_setnode(BSTree *map, BSTreeNode *node, void *key, void *data)
{
	while(node) {
		int cmp = map->compare(node->key, key);

		if(cmp < 0) {
			if(node->left) {
				node = node->left;
			} else {
				node->left = BSTreeNode_create(node, key, data);
				break;
			}
		} else if(cmp > 0) {
			if(node->right) {
				node = node->right;
			} else {
				node->right = BSTreeNode_create(node, key, data);
				break;
			}
		} else {
			node->data = data;
			break;
		}
	}
}

int BSTree_set(BSTree *map, void *key, void *data)
{
	if(map->root == NULL) {
		// first so just make it and get out
		map->root = BSTreeNode_create(NULL, key, data);
		check_mem(map->root);
	} else {
		BSTree_setnode(map, map->root, key, data);
	}

	return 0;
error:
	return -1;
}

static inline BSTreeNode *BSTree_getnode(BSTree *map, BSTreeNode *node, void *key)
{
	while(node) {
		int cmp = map->compare(node->key, key);

		if(cmp == 0) {
			return node;
		} else if(cmp < 0) {
			node = node->left;
		} else {
			node = node->right;
		}
	}

	return node;
}

void *BSTree_get(BSTree *map, void *key)
{
	if(map->root == NULL) {
		return NULL;
	} else {
		BSTreeNode *node = BSTree_getnode(map, map->root, key);
		return node == NULL ? NULL : node->data;
	}
}

static inline int BSTree_traverse_nodes(BSTreeNode *node, BSTree_traverse_cb traverse_cb)
{
	int rc = 0;

	if(node->left) {
		rc = BSTree_traverse_nodes(node->left, traverse_cb);
		if(rc != 0) return rc;
	}

	if(node->right) {
		rc = BSTree_traverse_nodes(node->right, traverse_cb);
		if(rc != 0) return rc;
	}

	return traverse_cb(node);
}

int BSTree_traverse(BSTree *map, BSTree_traverse_cb traverse_cb)
{
	if(map->root) {
		return BSTree_traverse_nodes(map->root, traverse_cb);
	}

	return 0;
}

static inline BSTreeNode *BSTree_find_min(BSTreeNode *node)
{
	while(node->left) {
		node = node->left;
	}

	return node;
}

static inline void BSTree_replace_node_in_parent(BSTree *map, BSTreeNode *node, BSTreeNode *new_value)
{
	if(node->parent) {
		if(node == node->parent->left) {
			node->parent->left = new_value;
		} else {
			node->parent->right = new_value;
		}
	} else {
		// this is the root so gotta change it
		map->root = new_value;
	}

	if(new_value) {
		new_value->parent = node->parent;
	}
}

static inline void BSTree_swap(BSTreeNode *a, BSTreeNode *b)
{
	void *temp = NULL;
	temp = b->key; b->key = a->key; a->key = temp;
	temp = b->data; b->data = a->data; a->data = temp;	
}

static inline BSTreeNode *BSTree_node_delete(BSTree *map, BSTreeNode *node, void *key)
{	
	BSTreeNode *delete = BSTree_getnode(map, node, key);

	if(delete) {
		if(delete->left && delete->right) {
			// swap this node for the smallest node that is bigger than us
			BSTreeNode *successor = BSTree_find_min(delete->right);
			BSTree_swap(successor, delete);

			// this leaves the old successor with possibly a right child
			// so replace it with the right child
			BSTree_replace_node_in_parent(map, successor, successor->right);

			// finally it's swapped, so return successor instead of node
			return successor;
		} else if(delete->left) {
			BSTree_replace_node_in_parent(map, delete, delete->left);
		} else if(delete->right) {
			BSTree_replace_node_in_parent(map, delete, delete->right);
		} else {
			BSTree_replace_node_in_parent(map, delete, NULL);
		}
	}

	return delete;
}

void *BSTree_delete(BSTree *map, void *key)
{
	void *data = NULL;
	
	if(map->root) {
		BSTreeNode *node = BSTree_node_delete(map, map->root, key);

		if(node) {
			data = node->data;
			free(node);
		}
	}

	return data;
}
