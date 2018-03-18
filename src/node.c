#include <stdlib.h>
#include "node.h"
#include "absyn.h"

/********************************** Structures **********************************/

/*
	Represents a node of the abstract syntax tree, as an element of the ST.
*/
typedef struct Node_s
{
	struct Node_s* next;
	void* component;
	NodeType type;
	key_t key; 
} Node;

/*
	Stores nodes that have not been deallocated.
*/
struct NodeST_s
{
	Node* first;
	key_t nextKey;
	int size;
};

/********************************** Constructors **********************************/

/*
	Returns an empty NodeST.
*/
NodeST* newNodeST()
{
	NodeST* ST = malloc(sizeof(NodeST));
	ST->first = NULL;
	ST->nextKey = 1;
	ST->size = 0;
	return ST;
}

/********************************** Functions **********************************/

/*
	Adds the given node of the abstract syntax tree to the ST, returning a unique key.
*/
key_t addNode(void* component, NodeType type, NodeST* ST) 
{
	Node* node  = malloc(sizeof(Node));
	node->component = component;
	node->type = type;
	node->key = ST->nextKey;
	ST->nextKey++;
	node->next = ST->first;
	ST->first = node;
	ST->size++;
	return node->key;
}

/*
	Removes the node corresponding to the given key from the ST, without freeing its component.
	This is called by freeNode(). Returns 1 if key corresponds to a node, and 0 if could not find
	a corresponding node.
*/
int removeNode(key_t key, NodeST* ST)
{
	// ST is empty
	if (ST->size == 0) {
		return 0;
	}

	// first element matches
	if (ST->first->key == key) {
		if (ST->size == 1) {
			free(ST->first);
			ST->first = NULL;
			ST->size = 0;
			return 1;
		} else {
			Node* next = ST->first->next;
			free(ST->first);
			ST->first = next;
			ST->size--;
			return 1;
		}
	}

	// first element doesn't match
	Node* previous = ST->first;
	Node* current = previous->next;
	while (current != NULL) {
		if (current->key == key) {
			previous->next = current->next;
			free(current);
			ST->size--;
			return 1;
		}
		previous = current;
		current = previous->next;
	}

	return 0;
}

/*
	Frees and removes all the nodes currently in the ST.
*/
int cleanup(NodeST* ST)
{
	int size = ST->size;

	Node* current = ST->first;
	Node* next = current;
	while (current != NULL) {
		next = current->next;
		freeNode(current->component, current->type); // will efficiently free the component and remove from ST
		current = next;
	}

	return size;
}
