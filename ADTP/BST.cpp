#include "BST.h"

template <typename T>
void Node<T>::_delete(Node<T>* curr) {
	if(curr == nullptr) return;
	_delete(curr->getLeft());
	_delete(curr->getRight());
}

template <typename T>
Node<T>::~Node() {
	_delete(this);
}

template <typename T>
Node<T>::Node(T val) {
	this->val = val;
	this->left = this->right = nullptr;
}

template <typename T>
Node<T>::Node(Node& node) {
	this->val = node.val;
	this->left = node.left;
	this->right = node.right;
}

template <typename T>
Node<T>::Node(){}

template <typename T>
T& Node<T>::getValue() {
	return this->val;
}

template <typename T>
T Node<T>::setValue(T& val) {
	this->val = val;
	return this->val;
}

template <typename T>
Node<T>*& Node<T>::getLeft() {
	return this->left;
}

template <typename T>
Node<T>*& Node<T>::getRight() {
	return this->right;
}

template <typename T>
Node<T>* Node<T>::setLeft(Node<T>* left) {
	this->left = left;
	return this->left;
}

template <typename T>
Node<T>* Node<T>::setRight(Node<T>* right) {
	this->right = right;
	return this->right;
}

template <typename T>
BST<T>::BST(T initVal) {
	this->root = new Node<T>(initVal);
}

template <typename T>
Node<T>* BST<T>::_insert(T& val, Node<T>* curr, int& currDepth) {
	currDepth++;
	if (val > curr->getValue()) {
		if (curr->getRight() == nullptr) {
			curr->setRight(new Node<T>(val));
			if (maxDepth < currDepth) {
				maxDepth = currDepth;
			}
			return curr;
		}
		else {
			curr->setRight(_insert(val, curr->getRight(), currDepth));
		}
	}
	else {
		if (curr->getLeft() == nullptr) {
			curr->setLeft(new Node<T>(val));
			if (maxDepth < currDepth) {
				maxDepth = currDepth;
			}
			return curr;
		}
		else {
			curr->setLeft(_insert(val, curr->getLeft(), currDepth));
		}
	}
	return curr;
}

template <typename T>
BST<T>::BST() {
	root = nullptr;
}

template <typename T>
int BST<T>::getMaxDepth() {
	return maxDepth;
}

template<typename T>
void BST<T>::printInOrder(){
	_printInOrder(root);
}

template <typename T>
void BST<T>::insert(T val) {
	if (root == nullptr) {
		root = new Node<T>(val);
		maxDepth = 1;
		return;
	}

	int dpth = 1;
	_insert(val, root, dpth);
}

template <typename T>
Node<T>* BST<T>::_searchFor(Node<T>* curr, T val) {
	if (curr == nullptr) {
		return nullptr;
	}

	T currVal = curr->getValue();
	if (currVal == val) {
		return curr;
	}
	else if (currVal < val) {
		if (curr->getRight() == nullptr) return nullptr; // did not find the value
		return _searchFor(curr->getRight(), val);
	}
	else {
		if (curr->getLeft() == nullptr) return nullptr; // did not find the value
		return _searchFor(curr->getLeft(), val);
	}
}

template<typename T>
T* BST<T>::searchFor(T searchingFor) {
	Node<T>* res = _searchFor(root, searchingFor);
	if(res == nullptr) return nullptr;
	return &res->getValue();
}

template<typename T>
Node<T>*& BST<T>::mostRightElement(Node<T>*& curr) {
	if (curr->getRight() == nullptr) {
		return curr;
	}
	else {
		return mostRightElement(curr->getRight());
	}
}

template<typename T>
void BST<T>::_printInOrder(Node<T>* root) {
	if (root == nullptr) return;
	_printInOrder(root->getLeft());
	std::cout << root->getValue() << " ";
	_printInOrder(root->getRight());
}

template<typename T>
void BST<T>::printPreOrder() {
	_printPreOrder(root);
}

template<typename T>
void BST<T>::_printPreOrder(Node<T>* root) {
	if (root == nullptr) return;
	std::cout << root->getValue() << " ";
	_printPreOrder(root->getLeft());
	_printPreOrder(root->getRight());
}

template<typename T>
void BST<T>::printPostOrder() {
	_printPostOrder(root);
}

template<typename T>
void BST<T>::_printPostOrder(Node<T>* root) {
	if (root == nullptr) return;
	_printPostOrder(root->getLeft());
	_printPostOrder(root->getRight());
	std::cout << root->getValue() << " ";
}

template<typename T>
Node<T>* BST<T>::_remove(Node<T>* curr, T& val) {
	if (curr->getValue() == val) {
		if (curr->getLeft() == nullptr && curr->getRight() == nullptr) {
			delete curr;
			return nullptr;
		}
		else if (curr->getLeft() == nullptr) {
			Node<T>* tmp = curr->getRight();
			curr->setRight(nullptr);
			delete curr;
			return tmp;
		}
		else if(curr->getRight() == nullptr) {
			Node<T>* tmp = curr->getLeft();
			curr->setLeft(nullptr);
			delete curr;
			return tmp;
		}
		else {
			Node<T>*& mostRightElementOfLeftSubtree = mostRightElement(curr->getLeft());
			curr->setValue(mostRightElementOfLeftSubtree->getValue());
			delete mostRightElementOfLeftSubtree;
			mostRightElementOfLeftSubtree = nullptr;
			return curr;
		}
	}

	if (val < curr->getValue()) {
		curr->setLeft(_remove(curr->getLeft(), val));
		return curr;
	}
	else {
		curr->setRight(_remove(curr->getRight(), val));
		return curr;
	}
}

template<typename T>
void BST<T>::remove(T toRemove) {
	_remove(root, toRemove);
}

template<typename T>
BST<T>::~BST() {
	if (root != nullptr) {
		delete root;
	}
}