#ifndef BINARY_DICT_H
#define BINARY_DICT_H

#include "BST.h"
#include "BST.cpp"
#include <string>
#include <vector>

using namespace std;

class BinaryDict : public BST<string> {
private:
	Node<string>* _getSimilarWords(Node<string>*, string, vector<pair<int, string>>&, int&);
	void _getAutocomplete(Node<string>*, string, string&);

public:
	BinaryDict();
	BinaryDict(string rootElem);
	string getSimilarWords(string, vector<pair<int, string>>&, int);
	string getAutocomplete(string);
	int editDistance(string a, string b);
};

#endif // !BINARY_DICT_H
