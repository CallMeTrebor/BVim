#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <Windows.h>
#include <conio.h>
#include <functional>
#include <map>
#include "BinaryDict.h"

enum Modes {
	NORMAL = 0,
	INSERT = 1,
	REPLACE = 2,
	CONSOLE = 3,
};

using namespace std;
class Editor {
	vector<string> lines;
	vector<string> currSuggestions;
	string currFile = "";
	string footer;
	string consoleCommand = "";
	string autoComplete = "";

	uint32_t width, height;
	int32_t cursorX = 0, cursorY = 0;
	int32_t offsetX = 0, offsetY = 0;

	BinaryDict* dict = nullptr;
	BinaryDict* syntaxTree = nullptr;

	HANDLE consoleOutHandle;

	int mode = NORMAL;
	int suggestionNum = 5;
	map<int, string> modeMap{ { NORMAL, "NORMAL" }, { INSERT, "INSERT" },
		{ REPLACE, "REPLACE" }, { CONSOLE, "CONSOLE" }};
	const int modeNum = 3;

	bool quitFlag = false;
	bool dictLoaded = false;
	bool syntaxTreeLoaded = false;
	bool numbered = false;
	bool waitingForAutoComplete = false;
	bool syntaxMode = false;
	bool highlighting = false;
	atomic<bool> updating = false;
private:
	bool writeToFile(string filename);
	bool quit();
	void waitAndLockUpdate();
	void unlockUpdate();
	void insertMode(int input);
	void normalMode(int input);
	void replaceMode(int input);
	void consoleMode(int input);
	void getAutocompleteOption();
	void setCursor(int x, int y, int offsetX = 0, int offsetY = 0);
	void moveCursor(int offsetX, int offsetY);
	void getSuggestionsForReplaceMode();
	void readFromFile(string filename);
	void fireArrowEvent();
	void switchSyntaxMode(string syntaxFile = "");
	void syntaxHighlight(int lineIndex);
	int clamp(int min, int max, int newVal);
	string trimString(string toTrim);
public:
	Editor(BinaryDict *dict, int suggestionNum, string fileName);
	bool getQuitFlag();
	void handleInput(KEY_EVENT_RECORD input);
	void update();
	void handleUpArrow();
	void handleDownArrow();
	void handleLeftArrow();
	void handleRightArrow();
	void updateFooter(string newFooter, int writeMode = 0);
	void setDictStatus(bool status);
	void switchMode(int newMode);
	void resize(uint32_t newWidth, uint32_t newHeight);
	string& operator[](int index);
	string& getFooter();
	BinaryDict*& getDictPointer();
	uint32_t getWidth();
	uint32_t getHeight();

	template<typename BreakingFunc, typename... Args>
	void requestExclusiveAccess(BreakingFunc f, Args... args) {
		waitAndLockUpdate();
		f(args...);
		unlockUpdate();
	}
};
#endif // !EDITOR_H