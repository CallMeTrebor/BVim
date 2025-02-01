#include <iostream>
#include <conio.h>
#include <fstream>
#include <algorithm>
#include <thread>
#include <cstdint>
#include "BinaryDict.h"
#include "Editor.h"

using namespace std;

const string DICT_FILE_NAME = "word.in";
const string ERRORS[] = { "Invalid param num" };

void initTree(string fileName, Editor* editor);
void insertionWrapper(BinaryDict* tree, string str);
void handleInputForEditor(Editor& editor);
int argCheck(int argc, const char* argv[]);
bool isStrNumber(const char* str);
bool isStrNumber(string& str);

int main(int argc, const char* argv[]) {
	int argRet = argCheck(argc, argv);
	if (argRet != 0) {
		cout << ERRORS[argRet - 1] << endl;
		cout << "USAGE: " << argv[0] << " FILENAME" << endl;
		return argRet;
	}

	Editor editor(nullptr, 5, argv[1]);
	thread(initTree, DICT_FILE_NAME, &editor).detach();
	handleInputForEditor(editor);

	system("cls");
	return 0;
}

bool isStrNumber(const char* str) {
	for (int i = 0; i < strlen(str); ++i) {
		if (str[i] < '0' || str[i] > '9') {
			return false;
		}
	}
	return true;
}

bool isStrNumber(string& str) {
	return isStrNumber(str.c_str());
}

int argCheck(int argc, const char* argv[]) {
	if (argc != 2) return 1;
	return 0;
}

void insertionWrapper(BinaryDict* tree, string str) {
	tree->insert(str);
}

void initTree(string fileName, Editor* editor) {
	ifstream file(fileName);

	BinaryDict* tree = new BinaryDict(string("pocket"));
	vector<thread> threads;
	string line;
	int lastDepth = -1;
	while (getline(file, line) && !editor->getQuitFlag()) {
		threads.emplace_back(thread(insertionWrapper, tree, string(line)));
		if(lastDepth != tree->getMaxDepth()){
			lastDepth = tree->getMaxDepth();
			editor->requestExclusiveAccess([&editor, &fileName, &tree]() { editor->updateFooter("Loading dictionary from file: " + fileName + " | current depth : " + to_string(tree->getMaxDepth())); });
		}
	}

	for (int i = 0; i < threads.size(); ++i) {
		threads[i].join();
	}

	editor->requestExclusiveAccess([&editor, &tree]() {editor->updateFooter("Dictionary loaded with max depth of : " + to_string(tree->getMaxDepth())); });
	editor->getDictPointer() = tree;
	editor->setDictStatus(true);
}

void handleInputForEditor(Editor& editor) {
	HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
	DWORD numEvents = 0, size = 256;
	PINPUT_RECORD record = new INPUT_RECORD[size];
	
	while (!editor.getQuitFlag()) {
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		uint32_t width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		uint32_t height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

		if (editor.getWidth() != width || editor.getHeight() != height) {
			editor.requestExclusiveAccess([&editor, width, height]() { editor.resize(width, height); });
		}

		ReadConsoleInput(input, record, size, &numEvents);

		for (int i = 0; i < numEvents; ++i) {
			if (record[i].EventType == KEY_EVENT && record[i].Event.KeyEvent.bKeyDown) {
				KEY_EVENT_RECORD keyEvent = record[i].Event.KeyEvent;

				editor.requestExclusiveAccess([&editor, keyEvent]() {editor.handleInput(keyEvent); });
			}
		};
	}

	delete[] record;
}