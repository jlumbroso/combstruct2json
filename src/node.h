#ifndef NODESTTYPE
#define NODESTTYPE
typedef long long int key_t;

typedef struct NodeST_s NodeST;

typedef enum {UNIT_N, ID_N, EXP_N, EXPLIST_N, STMT_N, STMTLIST_N, ERROR_N, GRAMMAR_N} NodeType;
#endif

#ifndef NODE_H
#define NODE_H
/********************************** Constructors **********************************/

/*
	Returns an empty NodeST.
*/
NodeST* newNodeST();

/********************************** Functions **********************************/

/*
	Adds the given node of the abstract syntax tree to the ST, returning a unique key.
*/
key_t addNode(void* component, NodeType type, NodeST* ST);

/*
	Removes the node corresponding to the given key from the ST, without freeing its component.
	Returns 1 if key corresponds to a node, and 0 if could not find a corresponding node.
*/
int removeNode(key_t key, NodeST* ST);

/*
	Frees and removes all the nodes (and its components) currently in the ST.
*/
int cleanup(NodeST* ST);

#endif
