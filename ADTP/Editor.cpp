#include "editor.h"
#include <cctype>
#include <fstream>

// makes the input string lowercase
string lowerCase(string input) {
	transform(input.begin(), input.end(), input.begin(), ::tolower);
	return input;
};

Editor::Editor(BinaryDict* dict, int suggestionNum, string fileName){
	this->dict = dict;
	this->consoleOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	this->suggestionNum = suggestionNum;

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(consoleOutHandle, &csbi);
	width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

	currSuggestions.resize(suggestionNum);
	currFile = fileName;
	readFromFile(fileName);
	lines.push_back("");
	update();
}

int Editor::clamp(int min, int max, int newVal) {
	if (newVal < min) {
		return min;
	}
	else if (newVal > max) {
		return max;
	}
	return newVal;
}

void Editor::moveCursor(int offsetX, int offsetY) {
	cursorY = clamp(0, lines.size() - 1, cursorY + offsetY);
	cursorX = clamp(0, lines[cursorY].length(), cursorX + offsetX);

	bool needRerender = false;
	if (cursorY > (height + this->offsetY - 2)) {
		this->offsetY++;
		needRerender = true;
	}
	else if (cursorY < (0 + this->offsetY)) {
		this->offsetY--;
		needRerender = true;
	}

	if (cursorX > (width + this->offsetX)) {
		this->offsetX++;
		needRerender = true;
	}
	else if (cursorX < (0 + this->offsetX)) {
		this->offsetX--;
		needRerender = true;
	}

	if (needRerender) {
		update();
	}

	setCursor(cursorX - this->offsetX, cursorY - this->offsetY);
}

void Editor::getSuggestionsForReplaceMode() {
	if (mode == REPLACE && dictLoaded) {
		int start = cursorX - 1, end = cursorX - 1;
		size_t lineLength = lines[cursorY].length();
		for (start; start >= 0 && isalnum(lines[cursorY][start]); --start);
		for (end; end < lineLength && isalnum(lines[cursorY][end]); ++end);

		start = clamp(0, lineLength, start);
		end = clamp(0, lineLength, end);

		string word;
		if (start == 0) {
			word = lines[cursorY].substr(start, end - start);
		}
		else {
			word = lines[cursorY].substr(start + 1, end - (start + 1));
		}

		trimString(word);
		if (word == "") return;
		word = lowerCase(word);
		vector<pair<int, string>> suggestions;
		string res = dict->getSimilarWords(word, suggestions, suggestionNum);

		string suggestionString;
		if (res == string()) {
			suggestionString = "Suggestions: ";
			for (int i = 0; i < suggestions.size(); ++i) {
				suggestionString += " | " + to_string(i + 1) + ": " + suggestions[i].second;
				currSuggestions[i] = suggestions[i].second;
			}
		}
		else {
			suggestionString = "Word found: " + res;
		}
		updateFooter(suggestionString);
	}
}

void Editor::handleDownArrow() {
	if(cursorY >= lines.size() - 1) {
		lines.push_back("");
	}
	moveCursor(0, 1);
	update();
}

void Editor::handleUpArrow() {
	if (cursorY > 0) {
		moveCursor(0, -1);
	}
}

void Editor::handleRightArrow() {
	if(cursorX < lines[cursorY].length()) {
		moveCursor(1, 0);
	}
}

void Editor::handleLeftArrow() {
	if (cursorX > 0) {
		moveCursor(-1, 0);
	}
}

void Editor::fireArrowEvent() {
	// all functions that listen to arrow key events go here
	getSuggestionsForReplaceMode();
}

void Editor::waitAndLockUpdate() {
	while (updating) {
		Sleep(0);
	}
	updating = true;
}

void Editor::unlockUpdate() {
	updating = false;
}

void Editor::setCursor(int x, int y, int offsetX, int offsetY) {
	COORD cursorPos = { x - offsetX, y - offsetY };
	SetConsoleCursorPosition(consoleOutHandle, cursorPos);
}

void Editor::update() {
	string state;
	int i = offsetY;
	for (i; i < lines.size() && i < offsetY + height - 1; ++i) {
		int fillerSize = width - lines[i].length();
		state += lines[i].substr(clamp(0, lines[i].length(), offsetX), width) + string(fillerSize > 0 ? fillerSize : 0, ' ') + "\n";
	}

	for(i; i < offsetY + height - 1; ++i) {
		state += "~" + string(width - 1, ' ') + "\n";
	}

	setCursor(0, 0);
	WriteConsoleA(consoleOutHandle, state.c_str(), state.length(), NULL, NULL);
	setCursor(cursorX, cursorY, offsetX, offsetY);

	i = offsetY;
	for (i; syntaxMode && i < lines.size() && i < offsetY + height - 1; ++i) {
		syntaxHighlight(i);
	}
}

// remove all spaces
string Editor::trimString(string toTrim) {
	string trimmed = "";
	for (char& c : toTrim) {
		if (isalnum(c)) {
			trimmed += c;
		}
	}
	return trimmed;
}

void Editor::insertMode(int input) {
	if (lines.size() <= cursorY) {
		lines.push_back("");
	}

	// if we press enter, we add a new line
	// we now need to rerender everything below the current line
	if (input == 13) {
		// split the line up into two at the cursorX, move the second half to the next line
		string newLine = lines[cursorY].substr(cursorX);

		// delete everything after the cursor
		lines[cursorY].erase(cursorX);

		// add the new line
		lines.insert(lines.begin() + cursorY + 1, newLine);

		// update each line below
		int32_t currX = cursorX, currY = cursorY;
		update();

		// update console cursor to next line relative to the current line
		cursorX = clamp(0, lines[currY + 1].length(), currX);
		cursorY = currY + 1;
		setCursor(cursorX, cursorY);
	}
	// if we are deleteing a char, we need to rerender the current line
	// also check if we are deleting a line, if so, we need to rerender everything below the current line
	else if (input == '\b') {
		if (cursorX == 0) {
			// delete the current line
			if (cursorY > 0) {
				int originalLineLength = lines[cursorY - 1].length();
				lines[cursorY - 1] += lines[cursorY];
				lines.erase(lines.begin() + cursorY);
				update();
				moveCursor(originalLineLength, -1);
			}
		}
		else {
			// delete a singular char
			lines[cursorY].erase(cursorX - 1, 1);

			update();

			// update console cursor
			moveCursor(-1, 0);
		}
	}
	else if (input == '\t') {
		lines[cursorY].insert(cursorX, 4, ' ');

		// update console cursor
		moveCursor(4, 0);

		update();
	}
	// if we add a char, we need to rerender the current line
	else if (isprint(input)) {
		lines[cursorY].insert(cursorX, 1, (char)input);

		// update console cursor
		moveCursor(1, 0);
		update();
	}
}

void Editor::replaceMode(int input) {
	if (input >= '1' && input <= suggestionNum + '0') {
		int start = cursorX - 1, end = cursorX - 1;
		size_t lineLength = lines[cursorY].length();
		for (start; start >= 0 && isalnum(lines[cursorY][start]); --start);
		for (end; end < lineLength && isalnum(lines[cursorY][end]); ++end);

		start++;
		start = clamp(0, lineLength, start);
		end = clamp(0, lineLength, end);

		string currWord = lines[cursorY].substr(start, end - start);
		string newWord = currSuggestions[input - 1 - '0'];
		lines[cursorY].erase(start, end - start);
		
		for (int i = 0; i < currWord.length() && i < newWord.length(); ++i) {
			if (isupper(currWord[i])) {
				newWord[i] = toupper(newWord[i]);
			}
		}

		lines[cursorY].insert(start, newWord);
		update();

		int xDiff = currWord.length() - newWord.length();
		moveCursor(xDiff, 0);
	}
}

void Editor::readFromFile(string fileName) {
	ifstream inFile(fileName);
	if (!inFile.is_open()) {
		ofstream tmp(fileName);
		tmp << "";
		tmp.close();

		inFile = ifstream(fileName);
	}

	lines.clear();
	string line;
	while (getline(inFile, line)) {
		// replace each tab with 4 spaces
		for (int i = 0; i < line.length(); ++i) {
			if (line[i] == '\t') {
				line.replace(i, 1, "    ");
			}
		}

		lines.push_back(line);
	}
	inFile.close();
}

void Editor::consoleMode(int input) {
	if (input == '\b') {
		if(consoleCommand.length() == 0) return;
		consoleCommand.pop_back();
		updateFooter(consoleCommand, CONSOLE);
	}
	else if (input == 13) {
		string command = consoleCommand.substr(0, consoleCommand.find_first_of(' '));
		string args = consoleCommand.substr(consoleCommand.find_first_of(' ') + 1);
		if (args == command) args = "";
		if (command[0] == ':') command = command.substr(1); // trim leading ':' if found

		if (command[0] == '!') {
			string command = "start cmd /c \"" + consoleCommand.substr(1, consoleCommand.length() - 1) + " & pause\"";
			int returnVal = system(command.c_str());
			updateFooter("Command returned: " + to_string(returnVal), CONSOLE);
		}
		else if (command.compare("q") == 0) {
			quit();
		}
		else if (command.compare("w") == 0) {
			writeToFile(currFile);
			updateFooter("Wrote to file " + currFile, CONSOLE);
		}
		else if (command.compare("wq") == 0) {
			writeToFile(currFile);
			quit();
		}
		else if (command.compare("stx") == 0) {
			updateFooter(string("Switched syntax mode ") + (syntaxMode ? "OFF" : "ON"), CONSOLE);
			switchSyntaxMode(args);
		}
		else {
			updateFooter("Command not found: \" " + consoleCommand + " \"", -1);
		}

		consoleCommand = "";
	}
	else if(isprint(input)){
		consoleCommand += (char) input;
		updateFooter(consoleCommand, CONSOLE);
	}
}

void loadDictFromFileSync(string fileName, BinaryDict* &dict) {
	ifstream file(fileName);
	if (!file.is_open()) throw "Error opening file";

	// let's delete the old dict
	if (dict != nullptr) {
		delete dict;
	}
	dict = new BinaryDict;

	string line;
	while (getline(file, line)) {
		dict->insert(line);
	}
}

void Editor::switchSyntaxMode(string syntaxFile) {
	if (!dictLoaded) {
		updateFooter("Main dictionary not loaded, please wait", -1);
		return;
	}

	if ((!syntaxMode && !syntaxTreeLoaded) || syntaxFile != "") {
		try {
			loadDictFromFileSync(syntaxFile, syntaxTree);
			syntaxTreeLoaded = true;
		}
		catch (const char* e) {
			updateFooter(e, -1);
			return;
		}
	}

	if (syntaxFile != "") {
		if (!syntaxMode) {
			swap(dict, syntaxTree);
		}

		syntaxMode = true;
	}
	else {
		syntaxMode = !syntaxMode;
		swap(dict, syntaxTree);
	}
	update();
}

bool Editor::getQuitFlag(){
	return quitFlag;
}

bool Editor::quit() {
	updateFooter("Bye!", CONSOLE);
	quitFlag = true;
	return true;
}

bool Editor::writeToFile(string filename) {
	ofstream file(filename);
	if (!file.is_open()) {
		return false;
	}

	for(string& line : lines) {
		file << line << endl;
	}
	file.close();
	return true;
}

void Editor::normalMode(int input) {
	switch (input) {
	case 'r':
		switchMode(REPLACE);
		getSuggestionsForReplaceMode();
		break;
	case 'i':
		switchMode(INSERT);
		break;
	case ':':
		switchMode(CONSOLE);
		break;
	}
}

void Editor::getAutocompleteOption() {
	if(!dictLoaded) return;

	// get currword pos
	uint32_t currX = cursorX;
	for(; currX > 0 && isalnum(lines[cursorY][currX - 1]); --currX);

	autoComplete = dict->getAutocomplete(lowerCase(lines[cursorY].substr(currX, cursorX - currX)));

	// print suggestions to footer
	updateFooter(autoComplete);
}

void Editor::syntaxHighlight(int lineIndex) {
	if (!syntaxMode) return;
	highlighting = true;

	string currWord = "";
	for (int i = offsetX; i <= lines[lineIndex].length(); ++i) {
		if (i < lines[lineIndex].length() && isalnum(lines[lineIndex][i])) {
			currWord += lines[lineIndex][i];
		}
		else {
			if (dict->searchFor(currWord) == nullptr) {
				currWord = "";
				continue;
			}
			
			int initialX = cursorX;
			setCursor(i - currWord.length(), lineIndex - offsetY);
			SetConsoleTextAttribute(consoleOutHandle, FOREGROUND_GREEN | FOREGROUND_BLUE);
			WriteConsoleA(consoleOutHandle, currWord.c_str(), currWord.length(), NULL, NULL);
			SetConsoleTextAttribute(consoleOutHandle, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
			setCursor(cursorX, cursorY, offsetX, offsetY);

			currWord = "";
		}
	}

	highlighting = false;
}

void Editor::handleInput(KEY_EVENT_RECORD input) {
	// in every case if control is pressed we will enter autocomplete mode
	if (input.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
		if (waitingForAutoComplete) {
			uint32_t currX = cursorX;
			for (; currX > 0 && isalnum(lines[cursorY][currX - 1]); --currX);
			lines[cursorY].erase(currX, cursorX - currX);
			lines[cursorY].insert(currX, autoComplete);

			waitingForAutoComplete = false;
			update();
			moveCursor(autoComplete.length() - (cursorX - currX), 0);
		}

		getAutocompleteOption();
		waitingForAutoComplete = true;
		return;
	}
	else {
		autoComplete = "";
		waitingForAutoComplete = false;
	}

	if (input.wVirtualKeyCode == VK_UP) {
		handleUpArrow();
	}
	else if (input.wVirtualKeyCode == VK_DOWN) {
		handleDownArrow();
	}
	else if (input.wVirtualKeyCode == VK_LEFT) {
		handleLeftArrow();
	}
	else if (input.wVirtualKeyCode == VK_RIGHT) {
		handleRightArrow();
	}
	else {
		int inputChar = input.uChar.AsciiChar;

		if (inputChar == 27) {
			if(mode == CONSOLE) consoleCommand = "";
			switchMode(NORMAL);
			return;
		}

		switch (mode) {
		case INSERT:
			insertMode(inputChar);
			break;
		case NORMAL:
			normalMode(inputChar);
			break;
		case REPLACE:
			replaceMode(inputChar);
			break;
		case CONSOLE:
			consoleMode(inputChar);
			break;
		}
		return;
	}

	// if any of the above if statements were triggered (excluding the else) then next line will run
	fireArrowEvent();
}

void Editor::switchMode(int newMode) {
	if (newMode <= modeNum) {
		mode = newMode;
		updateFooter("Entered " + modeMap[newMode] + " mode", mode);
	}
	else {
		updateFooter("Attempt to switch mode unsuccessfull", -1);
	}
}

void Editor::updateFooter(string newFooter, int writeMode) {
	if (mode == CONSOLE && writeMode != CONSOLE && writeMode != -1) return;
	footer = newFooter;
	setCursor(0, height - 1);

	// Hide cursor
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(consoleOutHandle, &cursorInfo);
	//cursorInfo.bVisible = FALSE; // Set cursor visibility to false
	SetConsoleCursorInfo(consoleOutHandle, &cursorInfo);

	// error mode
	if (writeMode == -1) {
		SetConsoleTextAttribute(consoleOutHandle, BACKGROUND_RED);
	}
	else {
		SetConsoleTextAttribute(consoleOutHandle, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
	}


	// clear row
	string footerClear(width - modeMap[mode].length() - newFooter.length(), ' ');
	WriteConsoleA(consoleOutHandle, ("\r" + newFooter + footerClear).c_str(), width - modeMap[mode].length() + 1, NULL, NULL);

	// now let's write the current mode
	// move cursor to the left - (the length of the mode string size)
	// write the mode string with a green background
	SetConsoleTextAttribute(consoleOutHandle, BACKGROUND_GREEN);
	setCursor(width - modeMap[mode].length(), height - 1);
	WriteConsoleA(consoleOutHandle, modeMap[mode].c_str(), modeMap[mode].length(), NULL, NULL);

	// revert colors
	SetConsoleTextAttribute(consoleOutHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	setCursor(cursorX, cursorY);

	// Restore cursor visibility
	cursorInfo.bVisible = TRUE;
	SetConsoleCursorInfo(consoleOutHandle, &cursorInfo);
}

string& Editor::operator[](int index) {
	return lines[index];
}

string& Editor::getFooter() {
	return footer;
}

BinaryDict*& Editor::getDictPointer() {
	return dict;
}

void Editor::setDictStatus(bool status) {
	dictLoaded = status;
}

void Editor::resize(uint32_t newWidth, uint32_t newHeight) {
	width = newWidth;
	height = newHeight;
	update();
	updateFooter("Resized to " + to_string(width) + "x" + to_string(height));
}

uint32_t Editor::getWidth() {
	return width;
}

uint32_t Editor::getHeight() {
	return height;
}