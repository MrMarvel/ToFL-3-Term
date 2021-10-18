#define _CRT_SECURE_NO_WARNINGS
#define NUM_OF_KWORDS 2

#include <iostream>
#include <string>
#include <regex>

using namespace std;

template<typename T, typename K>
struct Pair {
	T first;
	K second;
	Pair() {}
	Pair(T f, K s) {
		first = f;
		second = s;
	}
};

vector<Pair<string, string>> lex;

const char* keywords[NUM_OF_KWORDS] = { "for", "do" };
enum states { H, ID, NM, ASGN, DLM, ERR };
enum tok_names { KWORD, IDENT, NUM, OPER, DELIM };
struct token {
	enum tok_names token_name;
	char* token_value;
};
struct lexeme_table {
	struct token tok;
	struct lexeme_table* next;
};
struct lexeme_table* lt = NULL;
struct lexeme_table* lt_head = NULL;
int lexer(char* filename);
int is_num(string);
int is_kword(char* id);
int add_token(struct token* tok);
int lexer(const char* filename) {
	FILE* fd;
	int c, err_symbol;
	struct token tok;
	if ((fd = fopen(filename, "r")) == NULL) {
		printf("\nCannot open file %s.\n", filename);
		return -1;
	}
	enum states CS = H;
	c = fgetc(fd);
	while (!feof(fd)) {
		switch (CS) {
			case H:
			{
				while ((c == ' ') || (c == '\t') || (c == '\n')) {
					c = fgetc(fd);
				}
				if (((c >= 'A') && (c <= 'Z')) ||
					((c >= 'a') && (c <= 'z')) || (c == '_')) {
					CS = ID;
				} else if (((c >= '0') && (c <= '9')) || (c == '.')
					||
					(c == '+') || (c == '-')) {
					CS = NM;
				} else if (c == ':') {
					CS = ASGN;
				} else {
					CS = DLM;
				}
				break;
			}// case H
			case ASGN:
			{
				int colon = c;
				c = fgetc(fd);
				if (c == '=') {
					tok.token_name = OPER;
					if ((tok.token_value = (char*)malloc(sizeof(2))) == NULL) {
						printf("\nMemory allocation error in function \"lexer\"\n");
						return -1;
					}
					strcpy(tok.token_value, ":=");
					add_token(&tok);
					c = fgetc(fd);
					CS = H;
				} else {
					err_symbol = colon;
					CS = ERR;
				}
				break;
			}// case ASGN
			case DLM:
			{
				if ((c == '(') || (c == ')') || (c == ';')) {
					tok.token_name = DELIM;
					if ((tok.token_value =
						(char*)malloc(sizeof(1))) == NULL) {
						printf("\nMemory allocation error in function \"lexer\"\n");
						return -1;
					}
					sprintf(tok.token_value, "%c", c);
					add_token(&tok);
					c = fgetc(fd);
					CS = H;
				} else if ((c == '<') || (c == '>') || (c == '=')) {
					tok.token_name = OPER;
					if ((tok.token_value =
						(char*)malloc(sizeof(1))) == NULL) {
						printf("\nMemory allocation error in function \"lexer\"\n");
						return -1;
					}
					sprintf(tok.token_value, "%c", c);
					add_token(&tok);
					c = fgetc(fd);
					CS = H;
				} else {
					err_symbol = c;
					c = fgetc(fd);
					CS = ERR;
				}// if((c == '(') || (c == ')') || (c == ';'))
				break;
			}// case DLM
			case ERR:
			{
				printf("\nUnknown character: %c\n", err_symbol);
				return -1;
				CS = H;
				break;
			}
			case ID:
			{
				int size = 0;
				char buf[256];
				buf[size] = c;
				size++;
				c = fgetc(fd);
				while (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') &&
					(c <= 'z')) || ((c >= '0') && (c <= '9')) ||
					(c == '_')) {
					buf[size] = c;
					size++;
					c = fgetc(fd);
				}
				buf[size] = '\0';
				if (is_kword(buf)) {
					tok.token_name = KWORD;
				} else {
					tok.token_name = IDENT;
				}
				if ((tok.token_value = (char*)malloc(strlen(buf)))
					== NULL) {
					printf("\nMemory allocation error in function \"lexer\"\n");
					return -1;
				}
				strcpy(tok.token_value, buf);
				add_token(&tok);
				CS = H;
				break;
			} // case ID
			case NM:
			{
				char ch = c;
				string msg = "";
				while (ch >= '1' && ch <= '9' || ch == '.' || ch == 'E' || ch == 'e' || ch == '+' || ch == '-') {
					msg += ch;
					c = fgetc(fd);
					ch = c;
				}
				if (!is_num(msg)) {
					CS = ERR;
					err_symbol = ch;
					break;
				}
				tok.token_name = NUM;
				tok.token_value = const_cast<char*>(msg.c_str());
				add_token(&tok);
				CS = H;
				break;
			} // case NM
		} // switch
	} // while
} // int lexer(…) 

int is_num(string num) {
	bool o = true;
	smatch match;
	regex r(R"(^[-|+]?([0-9]*\.[0-9]*|[0-9]*\.|[0-9]+)([(e|E)[-|+][0-9]*)?[f|l|F|L]?$)");
	return regex_match(num, match, r);
}

int is_kword(char* id) {
	string ids = id;
	return ids == "for" | ids == "do";
}

int add_token(token* tok) {
	Pair<string, string> pair;
	pair.second = tok->token_value;
	switch (tok->token_name) {//KWORD, IDENT, NUM, OPER, DELIM
		case KWORD:
			pair.first = "KWORD";
			break;
		case IDENT:
			pair.first = "IDENT";
			break;
		case NUM:
			pair.first = "NUM";
			break;
		case OPER:
			pair.first = "OPER";
			break;
		case DELIM:
			pair.first = "DELIM";
			break;
	}
	lex.push_back(pair);
	return 0;
}



int main() {
	int state = lexer("input.txt");
	for (auto pair : lex) {
		cout.width(5);
		cout << pair.second << " ";
	}
	cout << endl;
	for (auto pair : lex) {
		cout.width(5);
		cout << pair.first << " ";
	}
	cout << endl;
	if (state < 0) printf("Lexical Analysis ERROR!\n");
	else printf("Lexical Analysis is successful.\n");
	system("pause");
	return 0;
}