#ifndef BST_H
#define BST_H

#include <iostream>

// helper class for the BST
template<typename T>
class Node {
	T val;
	Node<T>* left = nullptr, * right = nullptr;

private:
	void _delete(Node<T>*);

public:
	~Node();
	Node();
	Node(T);
	Node(Node&);
	T& getValue();
	T setValue(T&);
	Node<T>*& getLeft();
	Node<T>*& getRight();
	Node<T>* setLeft(Node<T>*);
	Node<T>* setRight(Node<T>*);
};

template<typename T>
class BST {
protected:
	Node<T>* root = nullptr;

	// tracks the tree's "depth"
	int maxDepth = 0;

private:
	// helper recursive functions
	Node<T>* _searchFor(Node<T>*, T);
	Node<T>* _insert(T&, Node<T>*, int&);
	Node<T>* _remove(Node<T>*, T&);
	Node<T>*& mostRightElement(Node<T>*& curr);

	void _printInOrder(Node<T>*);
	void _printPreOrder(Node<T>*);
	void _printPostOrder(Node<T>*);
	
public:
	~BST();
	BST();

	// construct with a given root
	BST(T);

	// returns a pointer to the element with the key given as parameter,
	// or nullpointer if not in tree
	T* searchFor(T);
	void insert(T);
	void remove(T);
	int getMaxDepth();

	void printInOrder();
	void printPreOrder();
	void printPostOrder();
};

#endif // !BST_H