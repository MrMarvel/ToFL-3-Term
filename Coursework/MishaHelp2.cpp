#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <stack>

using namespace std;

class compiler {
private:
	ifstream programm;
	fstream lex_out;
	char CH = ' ';
	string buff = "", LEX = "";
	int lex_num = 0;
	bool first = 0;

	stack <int> TI_STACK;
	stack <string> TYPE_STACK;

	//Описание переменной
	struct descr_id {
		bool isdescr = 0;
		string type = "";
		string addres = "";
	};

	//Описание числа
	struct descr_num {
		string type = "";
		string addres = "";
	};

	/*
	TW - таблица служебных слов М-языка, #1
	TL – таблица ограничителей М-языка, #2
	TN – таблица чисел, используемых в программе #3
	TI - таблица идентификаторов программы, #4
	TN_SEM – дополненная таблиа TN, используемая для семантического анализа
	TI_SEM - дополненная таблиа TI, используемая для семантического анализа
	table - Таблица для доступа к остальным таблицам
	table_sem - все возможные типы выражений
	*/

	unordered_map <string, int> TW = { {"readln", 1}, {"writeln", 2}, {"if", 3}, {"else", 4}, {"for", 5}, {"to", 6}, {"next", 7},
									   {"while", 8}, {"true", 9}, {"false", 10}, {"begin", 11}, {"end", 12}, {"step", 13} };
	unordered_map <string, int> TL = { {"{", 1}, {"}", 2}, {"%", 3}, {"!", 4}, {"$", 5}, {",", 6},  {";", 7},  {":", 8},  {"(", 9},
									   {")", 10},  {"+", 11},  {"-", 12},  {"||", 13},  {"*", 14},  {"/", 15}, {"&&", 16}, {"!=", 17},
									   {":=", 18},  {"==", 19},  {"<", 20},  {"<=", 21},  {">", 22},  {">=", 23}, {"/*", 24}, {"*/", 25}, };
	unordered_map <string, int> TN = {};
	unordered_map <string, int> TI = {};
	unordered_map <string, descr_id*> TI_SEM = {};
	unordered_map <string, descr_num*> TN_SEM = {};
	unordered_map <string, string> table_sem = { {"%+%","%"}, {"%-%","%"}, {"%/%","!"}, {"%*%","%"},
												 {"%+!","!"}, {"%-!","!"}, {"%/!","!"}, {"%*!","!"},
												 {"!+%","!"}, {"!-%","!"}, {"!/%","!"}, {"!*%","!"},
												 {"!+!","!"}, {"!-!","!"}, {"!/!","!"}, {"!*!","!"},
												 {"%>%","$"}, {"%<%","$"}, {"%>=%","$"}, {"%<=%","$"},
												 {"%>!","$"}, {"%<!","$"}, {"%>=!","$"}, {"%<=!","$"},
												 {"!>%","$"}, {"!<%","$"}, {"!>=%","$"}, {"!<=%","$"},
												 {"!>!","$"}, {"!<!","$"}, {"!>=!","$"}, {"!<=!","$"},
												 {"%==%","$"}, {"!==!","$"}, {"!==%","$"}, {"%==!","$"},
												 {"%!=%","$"}, {"%!=!","$"}, {"!!=%","$"}, {"!!=!","$"},
												 {"$!=$","$"}, {"%==%","$"}, {"!==!","$"}, {"!==%","$"},
												 {"%==!","$"}, {"$==$","$"}, {"$||$","$"}, {"$&&$","$"} };

	unordered_map < string, unordered_map <string, int>> table = { {"TW", {TW}}, {"TL", {TL}}, {"TI", {TI}}, {"TN", {TN}} };

	//Все возможные состояния
	enum states {
		H, I, N2, N8, N10, N16, C1, C2, M5, B1, B2,
		C3, M1, M2, M3, P1, P2, B, O, D, HX, E11,
		E12, E13, E22, ZN, E21, OG, V, ER, M4
	};

	/*
	CS - текущее состояние буфера
	Н – начало;
	I – идентификатор;
	N2 – двоичное число;
	N8 – восьмеричное число;
	N10 –десятичное число;
	N16 – шестнадцатеричное число;
	С1, C2, C3 – комментарий;
	M1 – меньше, меньше или равно;
	M2 – больше, больше или равно;
	M3 – не равно;
	M4 – присваивание;
	M5 – равно;
	B1 – логическое И;
	B2 – логическое ИЛИ;
	P1 – точка;
	P2 – дробная часть числа;
	B – символ «B» или «b»;
	O – символ «O» или «o»;
	D – символ «D» или «d»;
	HX – символ «H» или «h»;
	E11 - символ «E» или «e»;
	E12,E13, E22 – порядок числа в экспоненциальной форме;
	ZN, E21 – знак порядка числа в экспоненциальной форме;
	ОG – ограничитель;
	V – выход;
	ER –ошибка;
	*/

	//Считывание символа из входного файла
	int get_char_lex() {
		programm.get(CH);
		if (CH != programm.eof()) return 1;
		else return 0;
	}

	//Проверка на букву
	bool alpha() { return isalpha(CH); }

	//Проверка на цифру
	bool digit() { return isdigit(CH); }

	//Обнуление буффера
	void null() { buff = ""; }

	//Добавление символа в буффер
	void add() { buff += CH; }

	//Проверка лексемы на существование
	int check_lexem(string t) {
		if (table[t].find(buff) == table[t].end()) lex_num = 0;
		else lex_num = table[t][buff];
		return lex_num;
	}

	//Добавление лексемы в таблицу
	int put(string t, string type = "") {
		if (t == "TN") TN_SEM[buff] = new descr_num({ type, "" });
		if (t == "TI") TI_SEM[buff] = new descr_id({ 0, "", "" });
		if (table[t].find(buff) == table[t].end()) table[t][buff] = table[t].size() + 1;
		lex_num = table[t][buff];
		return lex_num;
	}

	//Вывод ЛА в файл
	void out(int n, int k) {
		lex_out << "(" << n << ", " << k << ")\n";
	}

	//Проверка на букву в 16ричном формате
	bool check_hex() { return isxdigit(CH); }

	//Проверка на букву в 16ричном формате или H|h
	bool check_16_num() { return (isxdigit(CH) || CH == 'H' || CH == 'h'); }

	//Перевод в 10 СС
	int translate(int base) {
		int i = 0, rem = 0, number = 0, result = 0;
		stringstream stream;

		switch (base) {
		case 2:
			stream << buff;
			stream >> number;
			while (number != 0) {
				rem = number % 10;
				number /= 10;
				result += rem * pow(2, i);
				++i;
			}
			return result;
			break;
		case 8:
			stream << buff;
			stream >> number;
			while (number != 0) {
				rem = number % 10;
				number /= 10;
				result += rem * pow(8, i);
				++i;
			}
			return result;
			break;
		case 16:
			stream << buff;
			stream >> std::hex >> result;
			return result;
			break;
		}
	}

	//Перевод числа в десятичный формат и запись в строку
	double convert() {
		double num;
		stringstream stream;
		stream << buff;
		stream >> num;
		char a[100];
		sprintf_s(a, "Число = %.8f", num);
		stream << a;
		stream >> num;
		return num;
	}

	//Считываение лексемы из файла
	void get_lexem() {
		int table_num, lex_num;
		char a;
		a = lex_out.get();
		if (first) {
			a = lex_out.get();
			a = lex_out.get();
			a = lex_out.get();
		}
		lex_out >> table_num;
		a = lex_out.get();
		a = lex_out.get();
		lex_out >> lex_num;
		switch (table_num) {
		case 1:
			for (auto it : table["TW"])
				if (it.second == lex_num) { LEX = it.first; break; }
			break;
		case 2:
			for (auto it : table["TL"])
				if (it.second == lex_num) { LEX = it.first; break; }
			break;
		case 3:
			for (auto it : table["TN"])
				if (it.second == lex_num) { LEX = it.first; break; }
			break;
		case 4:
			for (auto it : table["TI"])
				if (it.second == lex_num) { LEX = it.first; break; }
			break;
		}
		first = 1;
	}

	//Сравнение с лексемой
	bool equal_lex(string lex) { return LEX == lex; }

	//Проверка на существование идентификатора
	bool is_ID() {
		if (table["TI"].find(LEX) == table["TI"].end()) return 0;
		else return 1;
	}

	//Проверка на существование числа
	bool is_NUM() {
		if (table["TN"].find(LEX) == table["TN"].end()) return 0;
		else return 1;
	}

	//Обработка ошибок
	void err_proc(int num, string err = "") {
		switch (num) {
		case 1:
			cout << "ОШИБКА 1: Повторное объявление идентификатора " << err;
			break;
		case 2:
			cout << "ОШИБКА 2: Несоответствие типов выражения " << err;
			break;
		case 3:
			cout << "ОШИБКА 3: Тип переменной не соответствует типу выражения " << err;
			break;
		case 4:
			cout << "ОШИБКА 4: Отсутствие {";
			break;
		case 5:
			cout << "ОШИБКА 5: Отсутствие }";
			break;
		case 6:
			cout << "ОШИБКА 6: Неизвестный оператор " << LEX;
			break;
		case 7:
			cout << "ОШИБКА 7: Отсутствие ;";
			break;
		case 8:
			cout << "ОШИБКА 8: Неизвестный тип " << LEX;
			break;
		case 9:
			cout << "ОШИБКА 9: " << LEX << " не является идентификатором";
			break;
		case 10:
			cout << "ОШИБКА 10: Отсутствие )";
			break;
		case 11:
			cout << "ОШИБКА 11: Неизвестный тип выражения";
			break;
		case 12:
			cout << "ОШИБКА 12: Отсутствие \"end\"";
			break;
		case 13:
			cout << "ОШИБКА 13: Некорекктный оператор присваивания " << LEX;
			break;
		case 14:
			cout << "ОШИБКА 14: Отсутствие (";
			break;
		case 15:
			cout << "ОШИБКА 15: Отсутствие \"to\"";
			break;
		case 16:
			cout << "ОШИБКА 16: Отсутствие \"next\"";
			break;
		case 17:
			cout << "ОШИБКА 17: Некорекктное выражение";
		}
		exit(0);
	}

	//ТОЛЬКО для объявления переменных
	void to_stack() {
		TI_STACK.push(table["TI"][LEX]);
	}

	//Для вставки нуля в стек
	void to_stack(int num) {
		TI_STACK.push(0);
	}

	//ТОЛЬКО для проверки правильности и соответствия типов
	void to_stack(string type) {
		TYPE_STACK.push(type);
	}

	//Описание переменных из стека
	void set_descr(string type) {
		int curr = TI_STACK.top();
		string lex;
		TI_STACK.pop();
		while (curr != 0) {
			for (auto it : table["TI"])
				if (it.second == curr) { lex = it.first; break; }
			if (TI_SEM[lex]->isdescr) err_proc(1, lex);
			else TI_SEM[lex] = new descr_id({ 1, type, "0x000000" });
			curr = TI_STACK.top();
			TI_STACK.pop();
		}
	}

	//Проверка идентификатора на существование (если не объявили будет ошибка)
	void checkid() {
		if (!TI_SEM[LEX]->isdescr) err_proc(1, LEX);
		else to_stack(TI_SEM[LEX]->type);
	}

	//Метод проверки правильности и соответствия типов выражения
	void check_expr() {
		string types = "";
		for (int i = 0; i < 3; i++) {
			types += TYPE_STACK.top();
			TYPE_STACK.pop();
		}
		if (table_sem.find(types) == table_sem.end()) err_proc(2, types);
		else to_stack(table_sem[types]);
	}

	//Метод проверки правильности и соответствия типов выражения с отрицанием
	void check_not() {
		string type = "";
		type = TYPE_STACK.top();
		TYPE_STACK.pop();
		if (type != "$") err_proc(2);
		else to_stack("$");
	}

	//Проверка между переменной и выражением на соответствие типов перед присваиванием
	void eqtype() {
		string type1, type2;
		type1 = TYPE_STACK.top();
		TYPE_STACK.pop();
		type2 = TYPE_STACK.top();
		TYPE_STACK.pop();
		if (type1 != type2) err_proc(3, type1 + " := " + type2);
	}

	//Метод проверки соответствия выражения типу bool
	void eqbool() {
		string type1;
		type1 = TYPE_STACK.top();
		TYPE_STACK.pop();
		if (type1 != "$") err_proc(2, type1);
	}


	void PR() {
		get_lexem();
		if (equal_lex("{")) {
			get_lexem();
			BODY();
			if (equal_lex("}") == 0) err_proc(4);
		} else err_proc(5);
	}

	void BODY() {

		if (ID1(0)) {
			if (equal_lex(":")) {
				get_lexem();
				DESCR();
			} else {
				while (!TI_STACK.empty()) TI_STACK.pop();
				OPER();
			}
		} else if (equal_lex("if") || equal_lex("begin") || equal_lex("while") || equal_lex("for") || equal_lex("readln") || equal_lex("writeln")) OPER();
		else err_proc(6);

		while (equal_lex(";")) {
			get_lexem();
			if (ID1(0)) {
				if (equal_lex(":")) {
					get_lexem();
					DESCR();
				} else {
					while (!TI_STACK.empty()) TI_STACK.pop();
					OPER();
				}
			} else if (equal_lex("if") || equal_lex("begin") || equal_lex("while") || equal_lex("for") || equal_lex("readln") || equal_lex("writeln")) OPER();
			else if (equal_lex("}")) return;
			else err_proc(6);
		}
		err_proc(7);
	}

	void DESCR() {
		if (equal_lex("%")) { set_descr("%"); } else if (equal_lex("!")) { set_descr("!"); } else if (equal_lex("$")) { set_descr("$"); } else err_proc(8);
		get_lexem();
	}

	bool ID1(bool descr = 1) {
		to_stack(0);
		if (is_ID()) {
			to_stack();
			to_stack(TI_SEM[LEX]->type);
			get_lexem();
			if (!equal_lex(":=")) TYPE_STACK.pop();
			while (equal_lex(",")) {
				get_lexem();
				if (is_ID()) {
					to_stack();
					get_lexem();
				} else err_proc(9);
			}
			if (descr)
				while (!TI_STACK.empty()) TI_STACK.pop();
		} else {
			TI_STACK.pop();
			return 0;
		}
		return 1;
	}

	void FACT() {
		if (is_ID()) { checkid(); get_lexem(); } else if (is_NUM()) { to_stack(TN_SEM[LEX]->type); get_lexem(); } else if (equal_lex("true")) { to_stack("$"); get_lexem(); } else if (equal_lex("false")) { to_stack("$"); get_lexem(); } else if (equal_lex("!")) {
			get_lexem();
			FACT();
			check_not();
		} else if (equal_lex("(")) {
			get_lexem();
			COMPARE();
			if (equal_lex(")")) get_lexem();
			else err_proc(10);
		} else err_proc(11);
	}

	void MULT() {
		FACT();
		if (equal_lex("/") || equal_lex("*") || equal_lex("&&")) {
			to_stack(LEX);
			get_lexem();
			FACT();
			check_expr();
		}
	}

	void ADD() {
		MULT();
		if (equal_lex("+") || equal_lex("-") || equal_lex("||")) {
			to_stack(LEX);
			get_lexem();
			MULT();
			check_expr();
		}
	}

	bool COMPARE() {
		ADD();
		if (equal_lex("!=") || equal_lex("==") || equal_lex(">") || equal_lex(">=") || equal_lex("<") || equal_lex("<=") || equal_lex("&&") || equal_lex("||")) {
			to_stack(LEX);
			get_lexem();
			ADD();
			check_expr();
		}
		return 1;
	}

	void OPER() {

		if (equal_lex("begin")) {
			OPER1();

			if (!equal_lex("end")) err_proc(12);
			else get_lexem();

			if (is_ID()) to_stack(TI_SEM[LEX]->type);
			else if (is_NUM()) to_stack(TN_SEM[LEX]->type);
		}

		else if (ID1()) {
			if (equal_lex(":=")) {
				get_lexem();
				COMPARE();
				eqtype();
			} else err_proc(13);
		}

		else if (equal_lex("readln")) {
			get_lexem();
			if (!ID1()) err_proc(9);
		}

		else if (equal_lex("writeln")) {
			EXPR();
		}

		else if (equal_lex("while")) {
			get_lexem();
			if (equal_lex("(")) {
				get_lexem();
				COMPARE();
				eqbool();
			} else err_proc(14);

			if (!equal_lex(")")) err_proc(10);
			else get_lexem();

			OPER();
		}

		else if (equal_lex("if")) {
			get_lexem();
			if (equal_lex("(")) {
				get_lexem();
				COMPARE();
				eqbool();
			} else err_proc(14);

			if (!equal_lex(")")) err_proc(10);
			else get_lexem();

			OPER();
			if (equal_lex("else")) {
				get_lexem();
				OPER();
			} else return;
		}

		else if (equal_lex("for")) {
			get_lexem();
			if (ID1()) {
				if (TYPE_STACK.top() != "%") err_proc(3, TYPE_STACK.top());

				if (equal_lex(":=")) {
					get_lexem();
					COMPARE();
					eqtype();
				} else err_proc(16);

				if (equal_lex("to")) {
					get_lexem();
					COMPARE();
					if (TYPE_STACK.top() != "%") err_proc(3, TYPE_STACK.top());
					TYPE_STACK.pop();
				} else err_proc(15);

				if (equal_lex("step")) {
					get_lexem();
					COMPARE();
					if (TYPE_STACK.top() != "%") err_proc(3, TYPE_STACK.top());
					TYPE_STACK.pop();
				}

				if (equal_lex("if") || equal_lex("begin") || equal_lex("while") || equal_lex("for") || equal_lex("readln") || equal_lex("writeln")) {
					OPER();
					if (!equal_lex("next")) err_proc(16);
					else get_lexem();
				} else err_proc(6, LEX);
			}
		}

		else if (equal_lex(":=")) {
			get_lexem();
			COMPARE();
			eqtype();
		} else err_proc(6, LEX);
	}

	void EXPR() {
		get_lexem();
		if (COMPARE()) {
			TYPE_STACK.pop();
			while (equal_lex(",")) {
				get_lexem();
				if (COMPARE()) {
					TYPE_STACK.pop();
					get_lexem();
				} else err_proc(17);
			}
		} else err_proc(17);
	}

	void OPER1() {
		get_lexem();
		OPER();
		while (equal_lex(";")) {
			get_lexem();
			OPER();
		}
	}

public:

	bool lexer() {
		programm.open("programm.txt");
		lex_out.open("out.txt", ios::out);
		states STATE;
		get_char_lex();
		STATE = H;
		int flag = 0;
		do {
			switch (STATE) {
			case H:
				while (CH == ' ' || CH == '\n' && !programm.eof()) get_char_lex();
				if (programm.eof()) STATE = ER;
				if (alpha()) {
					null();
					add();
					get_char_lex();
					STATE = I;
				} else if (CH == '0' || CH == '1') { null(); STATE = N2; add(); get_char_lex(); } else if (CH >= '2' && CH <= '7') { null(); STATE = N8; add(); get_char_lex(); } else if (CH >= '8' && CH <= '9') { null(); STATE = N10; add(); get_char_lex(); } else if (CH == '.') { null(); add(); get_char_lex(); STATE = P1; } else if (CH == '/') { get_char_lex(); STATE = C1; } else if (CH == '<') { get_char_lex(); STATE = M1; } else if (CH == '>') { get_char_lex(); STATE = M2; } else if (CH == '!') { get_char_lex(); STATE = M3; } else if (CH == ':') { get_char_lex(); STATE = M4; } else if (CH == '=') { get_char_lex(); STATE = M5; } else if (CH == '|') { get_char_lex(); STATE = B1; } else if (CH == '&') { get_char_lex(); STATE = B2; } else if (CH == '}') { out(2, 2); STATE = V; } else STATE = OG;
				break;
			case I:
				while (alpha() || digit()) { add(); get_char_lex(); }
				check_lexem("TW");
				if (lex_num != 0) { out(1, lex_num); STATE = H; } else { put("TI"); out(4, lex_num); STATE = H; }
				break;
			case N2:
				while (CH == '0' || CH == '1') { add(); get_char_lex(); }
				if (CH >= '2' && CH <= '7') STATE = N8;
				else if (CH == '8' || CH == '9') STATE = N10;
				else if (CH == 'A' || CH == 'a' || CH == 'C' || CH == 'c' || CH == 'F' || CH == 'f') STATE = N16;
				else if (CH == 'E' || CH == 'e') { add(); get_char_lex(); STATE = E11; } else if (CH == 'D' || CH == 'd') { add(); get_char_lex(); STATE = D; } else if (CH == 'O' || CH == 'o') STATE = O;
				else if (CH == 'H' || CH == 'h') { get_char_lex(); STATE = HX; } else if (CH == '.') { add(); get_char_lex(); STATE = P1; } else if (CH == 'B' || CH == 'b') { add(); get_char_lex(); STATE = B; } else if (alpha()) STATE = ER;
				else STATE = N10;
				break;
			case N8:
				while (CH >= '2' && CH <= '7') { add(); get_char_lex(); }
				if (CH == '8' || CH == '9') STATE = N10;
				else if (CH == 'A' || CH == 'a' || CH == 'B' || CH == 'b' || CH == 'C' || CH == 'c' || CH == 'F' || CH == 'f') STATE = N16;
				else if (CH == 'E' || CH == 'e') { add(); get_char_lex(); STATE = E11; } else if (CH == 'D' || CH == 'd') { add(); get_char_lex(); STATE = D; } else if (CH == 'H' || CH == 'h') { get_char_lex(); STATE = HX; } else if (CH == '.') { add(); get_char_lex(); STATE = P1; } else if (CH == 'O' || CH == 'o') { get_char_lex(); STATE = O; } else if (alpha()) STATE = ER;
				else STATE = N10;
				break;
			case N10:
				while (CH == '8' || CH == '9') { add(); get_char_lex(); }
				if (CH == 'A' || CH == 'a' || CH == 'B' || CH == 'b' || CH == 'C' || CH == 'c' || CH == 'F' || CH == 'f') STATE = N16;
				else if (CH == 'E' || CH == 'e') { add(); get_char_lex(); STATE = E11; } else if (CH == 'H' || CH == 'h') { get_char_lex(); STATE = HX; } else if (CH == '.') { add(); get_char_lex(); STATE = P1; } else if (CH == 'D' || CH == 'd') { add(); get_char_lex(); STATE = D; } else if (alpha()) STATE = ER;
				else { put("TN", "%"); out(3, lex_num); STATE = H; }
				break;
			case N16:
				while (check_hex()) { add(); get_char_lex(); }
				if (CH == 'H' || CH == 'h') { get_char_lex(); STATE = HX; } else STATE = ER;
				break;
			case B:
				if (check_hex()) STATE = N16;
				else if (CH == 'H' || CH == 'h') { get_char_lex(); STATE = HX; } else if (alpha()) STATE = ER;
				else { translate(2); put("TN", "%"); out(3, lex_num); STATE = H; }
				break;
			case O:
				if (alpha() || digit()) STATE = ER;
				else { translate(8); put("TN", "%"); out(3, lex_num); STATE = H; }
				break;
			case D:
				if (CH == 'H' || CH == 'h') { get_char_lex(); STATE = HX; } else if (check_hex()) STATE = N16;
				else if (alpha()) STATE = ER;
				else { put("TN", "%"); out(3, lex_num); STATE = H; }
				break;
			case HX:
				if (alpha() || digit())STATE = ER;
				else { translate(16); put("TN", "%"); out(3, lex_num); STATE = H; }
				break;
			case E11:
				if (digit()) { add(); get_char_lex(); STATE = E12; } else if (CH == '+' || CH == '-') { add(); get_char_lex(); STATE = ZN; } else if (CH == 'H' || CH == 'h') { get_char_lex(); STATE = HX; } else if (check_hex()) { add(); get_char_lex(); STATE = N16; } else STATE = ER;
				break;
			case ZN:
				if (digit()) { add(); get_char_lex(); STATE = E13; } else STATE = ER;
				break;
			case E12:
				while (digit()) { add(); get_char_lex(); }
				if (check_hex()) STATE = N16;
				else if (CH == 'H' || CH == 'h') { get_char_lex(); STATE = HX; } else if (alpha()) STATE = ER;
				else { convert(); put("TN", "!"); out(3, lex_num); STATE = H; }
				break;
			case E13:
				while (digit()) { add(); get_char_lex(); }
				if (alpha() || CH == '.') STATE = ER;
				else { convert(); put("TN", "!"); out(3, lex_num); STATE = H; }
				break;
			case P1:
				if (digit()) STATE = P2; else STATE = ER;
				break;
			case P2:
				while (digit()) { add(); get_char_lex(); }
				if (CH == 'E' || CH == 'e') { add(); get_char_lex(); STATE = E21; } else if (alpha() || CH == '.') STATE = ER;
				else { convert(); put("TN", "!"); out(3, lex_num); STATE = H; }
				break;
			case E21:
				if (CH == '+' || CH == '-') { add(); get_char_lex(); STATE = ZN; } else if (digit()) STATE = E22;
				else STATE = ER;
				break;
			case E22:
				while (digit()) { add(); get_char_lex(); }
				if (alpha() || CH == '.') STATE = ER;
				else { convert(); put("TN", "!"); out(3, lex_num); STATE = H; }
				break;
			case C1:
				if (CH == '*') { get_char_lex(); STATE = C2; } else { out(2, 15); STATE = H; }
				break;
			case C2:
				flag = 1;
				while (CH != '*' && flag && CH != '}') flag = get_char_lex();
				if (CH == '}' || !flag) STATE = ER;
				else { get_char_lex(); STATE = C3; }
				break;
			case C3:
				if (CH == '/') { get_char_lex(); STATE = H; } else STATE = C2;
				break;
			case M1:
				if (CH == '=') { get_char_lex(); out(2, 21); STATE = H; } else { out(2, 20); STATE = H; }
				break;
			case M2:
				if (CH == '=') { get_char_lex(); out(2, 23); STATE = H; } else { out(2, 22); STATE = H; }
				break;
			case M3:
				if (CH == '=') { get_char_lex(); out(2, 17); STATE = H; } else { out(2, 4); STATE = H; }
				break;
			case M4:
				if (CH == '=') { get_char_lex(); out(2, 18); STATE = H; } else { out(2, 8); STATE = H; }
				break;
			case M5:
				if (CH == '=') { get_char_lex(); out(2, 19); STATE = H; } else { STATE = ER; }
				break;
			case B1:
				if (CH == '|') { get_char_lex(); out(2, 13); STATE = H; } else { STATE = ER; }
				break;
			case B2:
				if (CH == '&') { get_char_lex(); out(2, 16); STATE = H; } else { STATE = ER; }
				break;
			case OG:
				null();
				add();
				check_lexem("TL");
				if (lex_num != 0) {
					get_char_lex();
					out(2, lex_num);
					STATE = H;
				} else STATE = ER;
				break;
			}
		} while (STATE != V && STATE != ER);
		lex_out.close();
		programm.close();
		return STATE;
	}

	void syntax_sem() {
		lex_out.open("out.txt", ios::in | ios::binary);
		PR();
	}
};

int main() {
	setlocale(LC_ALL, "RUS");
	compiler l;
	l.lexer();
	l.syntax_sem();
}
