#include "BinaryDict.h"

BinaryDict::BinaryDict() : BST() {};

BinaryDict::BinaryDict(string rootElem) : BST(rootElem) {};

bool startsWith(string str, string prefix) {
	return str.compare(0, prefix.length(), prefix) == 0;
}

void BinaryDict::_getAutocomplete(Node<string>* curr, string val, string& currBest) {
	if (curr == nullptr) return;

	if (val < curr->getValue() && curr->getLeft() != nullptr) {
		_getAutocomplete(curr->getLeft(), val, currBest);
	} 
	else if (val > curr->getValue() && curr->getRight() != nullptr) {
		_getAutocomplete(curr->getRight(), val, currBest);
	}

	if ((currBest.length() > curr->getValue().length() || currBest == "") && startsWith(curr->getValue(), val)) {
		currBest = curr->getValue();
	}
}

string BinaryDict::getAutocomplete(string searchingFor) {
	string curr = "";
	_getAutocomplete(root, searchingFor, curr);

	return curr;
}

int BinaryDict::editDistance(string a, string b) {
	int n = a.length();
	int m = b.length();
	vector<vector<int>> dp(n + 1, vector<int>(m + 1, 0));

	for (int i = 0; i <= m; ++i) {
		dp[0][i] = i;
	}

	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= m; ++j) {
			dp[i][j] = min(min(dp[i - 1][j] + 1, dp[i][j - 1] + 1), dp[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1));
		}
	}

	return dp[n][m];
}

string BinaryDict::getSimilarWords(string searchingFor, vector<pair<int, string>>& closestMatches, int matchNum) {
	Node<string>* res = _getSimilarWords(root, searchingFor, closestMatches, matchNum);
	if (res == nullptr) {
		return string();
	}
	else {
		return res->getValue();
	}
}

Node<string>* BinaryDict::_getSimilarWords(Node<string>* curr, string val, vector<pair<int, string>>& closestMatches, int& matchNum) {
	if (curr == nullptr) {
		return nullptr;
	}

	Node<string>* retVal = nullptr;
	string currVal = curr->getValue();
	if (currVal == val) {
		return curr;
	}
	else if (currVal < val && curr->getRight() != nullptr) {
		retVal = _getSimilarWords(curr->getRight(), val, closestMatches, matchNum);
	}
	else if (currVal > val && curr->getLeft() != nullptr) {
		retVal = _getSimilarWords(curr->getLeft(), val, closestMatches, matchNum);
	}

	if (retVal == nullptr) {
		if (matchNum > 0) {
			closestMatches.push_back({ editDistance(val, curr->getValue()), curr->getValue()});
			matchNum--;
		}
		else {
			int diff = 0, biggestDiffIndex = -1, currDiff = editDistance(curr->getValue(), val);
			for (int i = 0; i < closestMatches.size(); i++) {
				if (closestMatches[i].first < diff && closestMatches[i].first < currDiff) {
					diff = closestMatches[i].first;
					biggestDiffIndex = i;
				}
			}

			if (biggestDiffIndex != -1) {
				closestMatches[biggestDiffIndex] = { currDiff, curr->getValue() };
			}
		}
	}
	return retVal;
}