#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

enum states { K, END, ER };

void lexer(string);
void lexer(const string filename) {
	ifstream ifs = ifstream(filename);
	if (!ifs.is_open()) {
		printf("Не получается открыть файл ");
		cout << filename;
		printf("\n");
		return;
	}
	enum states CS = K;
	char ch;
	bool isEndFile = false;
	while (!isEndFile) {
		ch = ifs.get();
		switch (CS) {
			case K:
			{
				if (!ifs.good()) {
					CS = ER;
					break;
				}
				if (isalnum(ch)) {//CH | DIG
					CS = K;
					break;
				}
				if (ch == ';') {
					CS = END;
					break;
				}
				CS = ER;
				break;
			}
			case ER:
			{
				ifs.clear();
				ifs.ignore(32767, ';');
				printf("Слово НЕ принадлежит заданному формальному языку\n");
				CS = K;
				if (ifs.eof()) isEndFile = true;
				ifs.putback(ch);
				break;
			}
			case END:
			{
				printf("Слово принадлежит заданному формальному языку\n");
				CS = K;
				if (ifs.eof()) isEndFile = true;
				ifs.putback(ch);
				break;
			}
		}
	}
}




int main() {
	setlocale(LC_ALL, "RUS");
	lexer("input.txt");
	system("pause");
	return 0;
}