// ReSharper disable CppSmartPointerVsMakeFunction
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <stack>

using namespace std;

class compiler {
private:
	ifstream cIn;
	ofstream lOut;
	ifstream lIn;
	char CH = ' ';
	string BUFFER, LEX;
	int lexNum = 0;
	bool first = false;
	int lexLine = 1;

	//Стек идентификаторов для ???
	stack <int> TI_STACK;
	//Стек типов для ???
	stack <string> TYPE_STACK;

	//Описание переменной
	struct Var {
		bool isDeclared = false;
		bool isAssigned = false;
		string type;
		string address;
	public:
		Var() = default;
		explicit Var(string type) : isDeclared(true), type(std::move(type)) {}
		Var(string type, string address): isDeclared(true), isAssigned(true), type(std::move(type)), address(std::move(address)) {}
		void declare(const string& type) {
			this->type = type;
			isDeclared = true;
		}
		void assign(const string& address) {
			if (!isDeclared) throw exception("Tried assign value to undeclared variable!");
			this->address = address;
			isAssigned = true;
		} 
	};

	//Описание числа
	struct Num {
		string type;
		string address;
	};

	unordered_map <string, int> TW = {
		{"readln", 1}, {"writeln", 2}, {"if", 3}, {"else", 4}, {"for", 5}, {"to", 6}, {"while", 7},
		{"begin", 8}, {"end", 9}, {"next", 10},{"true", 11}, {"false", 12},  {"step", 13},
		{"int", 14}, {"float", 15}, {"bool", 16}
	};
	unordered_map <string, int> TL = {
		{"{", 1}, {"}", 2}, {"&&", 3}, {"!", 4}, {"||", 5}, {",", 6},  {";", 7},  {"[", 8}, {"]", 9},
		{"(", 10}, {")", 11},  {"+", 12},  {"-", 13},  {"*", 14},  {"/", 15}, {"==", 16}, {"!=", 17},
		{">", 18}, {"<", 19}, {"<=", 20}, {">=", 21}, {"/*", 22}, {"*/", 23}, {":=", 24}, {"–", 13}
	};
	unordered_map <string, int> TN = {};
	unordered_map <string, int> TI = {};
	unordered_map <string, shared_ptr<Var>> TI_SEM = {};
	unordered_map <string, shared_ptr<Num>> TN_SEM = {};
	unordered_map <string, string> table_sem = {
		{"int+int","int"}, {"int-int","int"}, {"int/int","int"}, {"int*int","int"},
		{"int+float","float"}, {"int-float","float"}, {"int/float","float"}, {"int*float","float"},
		{"float+int","float"}, {"float-int","float"}, {"float/int","float"}, {"float*int","float"},
		{"float+float","float"}, {"float-float","float"}, {"float/float","float"}, {"float*float","float"},
		{"int>int","bool"}, {"int<int","bool"}, {"int>=int","bool"}, {"int<=int","bool"},
		{"int>float","bool"}, {"int<float","bool"}, {"int>=float","bool"}, {"int<=float","bool"},
		{"float>int","bool"}, {"float<int","bool"}, {"float>=int","bool"}, {"float<=int","bool"},
		{"float>float","bool"}, {"float<float","bool"}, {"float>=float","bool"}, {"float<=float","bool"},
		{"int==int","bool"}, {"float==float","bool"}, {"float==int","bool"}, {"int==float","bool"},
		{"int!=int","bool"}, {"int!=float","bool"}, {"float!=int","bool"}, {"float!=float","bool"},
		{"bool!=bool","bool"}, {"int==int","bool"}, {"float==float","bool"}, {"float==int","bool"},
		{"int==float","bool"}, {"bool==bool","bool"}, {"bool||bool","bool"}, {"bool&&bool","bool"}
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


	unordered_map < string, unordered_map <string, int>> table = { {"TW", {TW}}, {"TL", {TL}}, {"TI", {TI}}, {"TN", {TN}} };

	enum states_ {
		H, I, N2, N8, N10, N16,
		C1, // комментарий/деление
		C2, // комментарий
		B1, // "&&"
		B2, // "||"
		C3, // комментарий
		M1, // "<="
		M2, // ">="
		M3, // "!="
		M4, // ":="
		M5, // "=="
		P1, // "."
		P2,
		B, // "b"
		O, D, HX, E11,
		E12, E13, E22, ZN, E21,
		OG, // ЗАТЫЧКА ДЛЯ ИСКЛЮЧЕНИЙ И СКОБОК
		V, // ВЫХОД
		ER, // ОШИБКА
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

	int gc() {
		if (cIn.eof()) return 0;
		cIn.get(CH);
		return 1;
	}

	bool let() const { return isalpha(CH); }

	bool digit() const { return isdigit(CH); }

	void nill() { BUFFER = ""; }

	void add() { BUFFER += CH; }

	int look(const string& t) {
		if (table[t].find(BUFFER) == table[t].end()) lexNum = 0;
		else lexNum = table[t][BUFFER];
		return lexNum;
	}

	int put(const string& t, const string& type = "") {
		if (t == "TN") TN_SEM[BUFFER] = shared_ptr<Num>(new Num({ type, "" }));
		if (t == "TI") TI_SEM[BUFFER] = shared_ptr<Var>(new Var());
		if (table[t].find(BUFFER) == table[t].end()) table[t][BUFFER] = table[t].size() + 1;
		lexNum = table[t][BUFFER];
		return lexNum;
	}

	void out(const int n, const int k) {
		lOut << "(" << n << ", " << k << ")";
	}

	bool check_hex() const { return isxdigit(CH); }

	bool AFH() const { return (isxdigit(CH) || CH == 'H' || CH == 'h'); }

	int translate(const int base) const {
		int i = 0, rem = 0, number = 0, result = 0;
		stringstream stream;

		switch (base) {
		case 2:
			stream << BUFFER;
			stream >> number;
			while (number != 0) {
				rem = number % 10;
				number /= 10;
				result += rem * pow(2, i);
				++i;
			}
			return result;
		case 8:
			stream << BUFFER;
			stream >> number;
			while (number != 0) {
				rem = number % 10;
				number /= 10;
				result += rem * pow(8, i);
				++i;
			}
			return result;
		case 16:
			stream << BUFFER;
			stream >> std::hex >> result;
			return result;
		}
		return 0;
	}

	double convert() const {
		double num;
		stringstream stream;
		stream << BUFFER;
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
		char a = lIn.get();
		if (first) {
			a = lIn.get();
			while (a == '\r' || a == '\n') {
				if (a == '\n') lexLine++;
				a = lIn.get();
			}
		}
		lIn >> table_num;
		lIn.get();
		lIn.get();
		lIn >> lex_num;
		switch (table_num) {
		case 1:
			for (const auto& it : table["TW"])
				if (it.second == lex_num) { LEX = it.first; break; }
			break;
		case 2:
			for (const auto& it : table["TL"])
				if (it.second == lex_num) { LEX = it.first; break; }
			break;
		case 3:
			for (const auto& it : table["TN"])
				if (it.second == lex_num) { LEX = it.first; break; }
			break;
		case 4:
			for (const auto& it : table["TI"])
				if (it.second == lex_num) { LEX = it.first; break; }
			break;
		default:
			LEX = "";
		}
		first = true;
	}

	//Сравнение с лексемой
	bool equal_lex(const string& lex) const { return LEX == lex; }

	bool contain_lex(vector<string> arr) const {
		return find(arr.begin(), arr.end(), LEX) != arr.end();
	}

	//Проверка на существование идентификатора
	bool is_ID() {
		if (table["TI"].find(LEX) == table["TI"].end()) return false;
		else return true;
	}

	bool is_type(string type = "") const {
		if (type.empty()) type = LEX;
		return type == "int" || type == "float" || type == "bool";
	}

	//Проверка на существование числа
	bool is_NUM() {
		if (table["TN"].find(LEX) == table["TN"].end()) return false;
		else return true;
	}

	//Обработка ошибок
	void err_proc(const int num, const string& err = "") const {
		cout << "ОШИБКА " << num << ": ";
		switch (num) {
			case 1:
				cout << "Повторное объявление идентификатора " << err;
				break;
			case 2:
				cout << "Несоответствие типов выражения " << err;
				break;
			case 3:
				cout << "Тип переменной не соответствует типу выражения " << err;
				break;
			case 4:
				cout << "Отсутствие \"{\"";
				break;
			case 5:
				cout << "Отсутствие \"}\"";
				break;
			case 6:
				cout << "Неизвестный оператор \"" << LEX << "\"";
				break;
			case 7:
				cout << "Отсутствие \";\"";
				break;
			case 8:
				cout << "Неизвестный тип " << LEX;
				break;
			case 9:
				cout << LEX << " не является идентификатором";
				break;
			case 10:
				cout << "Отсутствие )";
				break;
			case 11:
				cout << "Неизвестный тип выражения";
				break;
			case 12:
				cout << "Отсутствие \"end\"";
				break;
			case 13:
				cout << "Некорректный оператор присваивания " << LEX;
				break;
			case 14:
				cout << "Отсутствие \"(\"";
				break;
			case 15:
				cout << "Отсутствие \"to\"";
				break;
			case 16:
				cout << "Отсутствие \"next\"";
				break;
			case 17:
				cout << "Некорректное выражение";
				break;
			case 18:
				cout << "Отсутствие \":\"";
				break;
			case 19:
				cout << "Отсутствие идентификатора после типа";
				break;
			case 20:
				cout << "Не объявленная переменная";
				break;
			case 21:
				cout << "Не найдено step после [";
				break;
			case 22:
				cout << "Не найдено ]";
				break;
			case 23:
				cout << "Не объявленный идентификатор \"" << LEX << "\"";
				break;
		}
		cout << " на строке " << lexLine << "\n";
		//system("pause");
		exit(0);
	}

	//ТОЛЬКО для объявления переменных
	void to_idStack() {
		TI_STACK.push(table["TI"][LEX]);
	}

	//Для вставки нуля в стек
	void to_idStack(int num) {
		TI_STACK.push(0);
	}

	//ТОЛЬКО для проверки правильности и соответствия типов
	void to_typeStack(const string& type) {
		TYPE_STACK.push(type);
	}

	//Описание переменных из стека
	void set_declared(const string& type) {
		int curr = TI_STACK.top();
		string lex;
		TI_STACK.pop();
		while (curr != 0) {
			for (const auto& it : table["TI"])
				if (it.second == curr) { lex = it.first; break; }
			if (TI_SEM[lex]->isDeclared) err_proc(1, lex);
			else TI_SEM[lex] = shared_ptr<Var>(new Var(type));
			curr = TI_STACK.top();
			TI_STACK.pop();
		}
	}

	//Проверка идентификатора на существование (если не объявили будет ошибка)
	void checkid() {
		if (!TI_SEM[LEX]->isDeclared) err_proc(1, LEX);
		else to_typeStack(TI_SEM[LEX]->type);
	}

	//Метод проверки правильности и соответствия типов выражения
	void check_expr() {
		string types;
		for (int i = 0; i < 3; i++) {
			types += TYPE_STACK.top();
			TYPE_STACK.pop();
		}
		if (table_sem.find(types) == table_sem.end()) err_proc(2, types);
		else to_typeStack(table_sem[types]);
	}

	//Метод проверки правильности и соответствия типов выражения с отрицанием
	void check_not() {
		const string type = TYPE_STACK.top();
		TYPE_STACK.pop();
		if (type != "bool") err_proc(2);
		else to_typeStack("bool");
	}

	//Проверка между переменной и выражением на соответствие типов перед присваиванием
	void eqtype() {
		const string type1 = TYPE_STACK.top();
		TYPE_STACK.pop();
		const string type2 = TYPE_STACK.top();
		TYPE_STACK.pop();
		if (type1 == "bool" && type2 != "bool" || type2 == "bool" && type1 != "bool") err_proc(3, "\"" + type1 + "\" := \"" + type2 + "");
	}

	//Метод проверки соответствия выражения типу bool
	void eqbool() {
		const string type1 = TYPE_STACK.top();
		TYPE_STACK.pop();
		if (type1 != "bool") err_proc(2, type1);
	}


	void PR() {
		get_lexem();
		if (equal_lex("{")) {
			get_lexem();
			BODY();
			if (equal_lex("}") == 0) err_proc(5);
		}
		else err_proc(4);
	}

	void BODY() {

		if (S_TYPE()) {
		} else if (S_ID()) {
			OPER();
		} else {
			auto it = TW.begin();
			bool o = false;
			while (it != TW.end()) {
				if (equal_lex(it++->first)) o = true;
			}
			if (o) OPER();
			else err_proc(6);
		}

		while (equal_lex(";")) {
			get_lexem();
			if (S_TYPE()) {
			} else if (S_ID()) {
				OPER();
			}
			else {
				auto it = TW.begin();
				bool o = false;
				while (it != TW.end()) {
					if (equal_lex((it++)->first)) {
						o = true;
						break;
					}
				}
				if (o) OPER();
				else if (equal_lex("}")) return;
				else err_proc(6);
			}
		}
		err_proc(7);
	}

	void DECLARE() {
		const string type = TYPE_STACK.top();
		if (is_type(type)) {
			set_declared(type);
		} else err_proc(8);
	}

	bool S_TYPE() {
		if (is_type()) {
			to_typeStack(LEX);
			get_lexem();
			if (S_ID_AFTER_TYPE()) {
				DECLARE();
			} else err_proc(19);
			return true;
		}
		return false;
	}

	bool S_ID_AFTER_TYPE() {
		to_idStack(0);
		if (is_ID()) {
			to_idStack();
			get_lexem();
			while (equal_lex(",")) {
				get_lexem();
				if (is_ID()) {
					to_idStack();
					get_lexem();
				}
				else err_proc(9);
			}
		}
		else {
			TI_STACK.pop();
			return false;
		}
		return true;
	}

	bool S_ID() {
		to_idStack(0);
		if (is_ID()) {
			if (!TI_SEM[LEX]->isDeclared) err_proc(23);
			to_idStack();
			to_typeStack(TI_SEM[LEX]->type);
			get_lexem();
			if (!equal_lex(":=")) TYPE_STACK.pop();
			while (equal_lex(",")) {
				get_lexem();
				if (is_ID()) {
					if (!TI_SEM[LEX]->isDeclared) err_proc(23);
					to_idStack();
					get_lexem();
				} else err_proc(9);
			}
			while (!TI_STACK.empty()) TI_STACK.pop();
		} else {
			TI_STACK.pop();
			return false;
		}
		return true;
	}


	void FACT() {
		if (is_ID()) { checkid(); get_lexem(); }
		else if (is_NUM()) { to_typeStack(TN_SEM[LEX]->type); get_lexem(); }
		else if (equal_lex("true") || equal_lex("false")) { to_typeStack("bool"); get_lexem(); }
		else if (equal_lex("!")) {
			get_lexem();
			FACT();
			check_not();
		}
		else if (equal_lex("(")) {
			get_lexem();
			COMPARE();
			if (equal_lex(")")) get_lexem();
			else err_proc(10);
		}
		else err_proc(11);
	}

	void MULT() {
		FACT();
		if (equal_lex("/") || equal_lex("*") || equal_lex("&&")) {
			to_typeStack(LEX);
			get_lexem();
			FACT();
			check_expr();
		}
	}

	void ADD() {
		MULT();
		if (equal_lex("+") || equal_lex("-") || equal_lex("||")) {
			to_typeStack(LEX);
			get_lexem();
			MULT();
			check_expr();
		}
	}

	bool COMPARE() {
		ADD();
		if (equal_lex("!=") || equal_lex("==") || equal_lex(">") || equal_lex(">=") || equal_lex("<") || equal_lex("<=") || equal_lex("&&") || equal_lex("||")) {
			to_typeStack(LEX);
			get_lexem();
			ADD();
			check_expr();
		}
		return true;
	}

	void OPER() {

		if (equal_lex("begin")) {
			OPER1();

			if (!equal_lex("end")) err_proc(12);
			else get_lexem();

			if (is_ID()) to_typeStack(TI_SEM[LEX]->type);
			else if (is_NUM()) to_typeStack(TN_SEM[LEX]->type);
		}

		else if (S_ID()) {
			if (equal_lex(":=")) {
				get_lexem();
				COMPARE();
				eqtype();
			}
			else err_proc(13);
		}

		else if (equal_lex("readln")) {
			get_lexem();

			if (!S_ID()) err_proc(9);
			/*if (S_ID()) {
				if (TI_SEM[LEX]->isDeclared) {
					TI_SEM[LEX]->assign(string("0x0010")+to_string(rand()%100));
				} else err_proc(20);
				get_lexem();
				while (equal_lex(",")) {
					get_lexem();
					if (S_ID()) {
						if (TI_SEM[LEX]->isDeclared) {
							TI_SEM[LEX]->assign(string("0x0010") + to_string(rand() % 100));
						} else err_proc(20);
						get_lexem();
					} else err_proc(9);
				}
			} else err_proc(9);*/
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
			}
			else err_proc(14);

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
			}
			else err_proc(14);

			if (!equal_lex(")")) err_proc(10);
			else get_lexem();

			OPER();
			if (equal_lex("else")) {
				get_lexem();
				OPER();
			}
			else return;
		}

		else if (equal_lex("for")) {
			get_lexem();
			if (S_ID()) {
				if (TYPE_STACK.top() != "int") err_proc(3, TYPE_STACK.top());

				if (equal_lex(":=")) {
					get_lexem();
					COMPARE();
					eqtype();
				} else err_proc(16);

				if (equal_lex("to")) {
					get_lexem();
					COMPARE();
					if (TYPE_STACK.top() != "int") err_proc(3, TYPE_STACK.top());
					TYPE_STACK.pop();
				} else err_proc(15);

				if (equal_lex("[")) {
					get_lexem();
					if (equal_lex("step")) {
						get_lexem();
						COMPARE();
						if (TYPE_STACK.top() != "int") err_proc(3, TYPE_STACK.top());
						TYPE_STACK.pop();
					} else err_proc(21);
					if (equal_lex("]")) get_lexem();
					else err_proc(22);
				}

				auto it = TW.begin();
				bool o = false;
				for (; it != TW.end(); it++) {
					if (is_type(it->first)) continue;
					if (equal_lex(it->first)) {
						o = true;
						break;
					}
				}
				if (o) {
					OPER();
					if (equal_lex("next")) get_lexem();
					else err_proc(16);
				} else err_proc(6, LEX);
			}
		}

		else if (equal_lex(":=")) {
			get_lexem();
			COMPARE();
			eqtype();
		}
		else err_proc(6, LEX);
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
				}
				else err_proc(17);
			}
		}
		else err_proc(17);
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

	bool lexer(const string& filename) {
		cIn.open(filename);
		if (!cIn.is_open()) {
			throw exception(string("There is no input file \"" + filename + "\"").c_str());
		}
		string lexFilename = "lex." + filename;
		lOut.open(lexFilename, ios::out | ios::in | ios::trunc);
		if (!lOut.is_open()) {
			throw runtime_error(string("I can't open file \"" + lexFilename + "\" to write in!").c_str());
		}
		states_ STATE;
		gc();
		STATE = H;
		int flag = 0;
		do {
			switch (STATE) {
			case H: {
				while ((CH == ' ' || CH == '\n' || CH == '\t' || CH == '\b') && !cIn.eof()) {
					if (CH == '\n') lOut << "\n";
					gc();
				}
				if (cIn.eof()) {
					STATE = ER;
				} else if (let()) {
					nill();
					add();
					gc();
					STATE = I;
				}
				else if (CH == '0' || CH == '1') { nill(); STATE = N2; add(); gc(); }
				else if (CH >= '2' && CH <= '7') { nill(); STATE = N8; add(); gc(); }
				else if (CH >= '8' && CH <= '9') { nill(); STATE = N10; add(); gc(); }
				else if (CH == '.') { nill(); add(); gc(); STATE = P1; }
				else if (CH == '/') { gc(); STATE = C1; }
				else if (CH == '<') { gc(); STATE = M1; }
				else if (CH == '>') { gc(); STATE = M2; }
				else if (CH == '!') { gc(); STATE = M3; }
				else if (CH == ':') { gc(); STATE = M4; }
				else if (CH == '=') { gc(); STATE = M5; }
				else if (CH == '&') { gc(); STATE = B1; }
				else if (CH == '|') { gc(); STATE = B2; }
				else if (CH == '}') { out(2, TL["}"]); STATE = V; }
				else STATE = OG;
				break;
			}
			case I:
				while (let() || digit()) { add(); gc(); }
				look("TW");
				if (lexNum != 0) { out(1, lexNum); STATE = H; }
				else { put("TI"); out(4, lexNum); STATE = H; }
				break;
			case N2:
				while (CH == '0' || CH == '1') { add(); gc(); }
				if (CH >= '2' && CH <= '7') STATE = N8;
				else if (CH == '8' || CH == '9') STATE = N10;
				else if (CH == 'A' || CH == 'a' || CH == 'C' || CH == 'c' || CH == 'F' || CH == 'f') STATE = N16;
				else if (CH == 'E' || CH == 'e') { add(); gc(); STATE = E11; }
				else if (CH == 'D' || CH == 'd') { add(); gc(); STATE = D; }
				else if (CH == 'O' || CH == 'o') STATE = O;
				else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; }
				else if (CH == '.') { add(); gc(); STATE = P1; }
				else if (CH == 'B' || CH == 'b') { add(); gc(); STATE = B; }
				else if (let()) STATE = ER;
				else STATE = N10;
				break;
			case N8:
				while (CH >= '2' && CH <= '7') { add(); gc(); }
				if (CH == '8' || CH == '9') STATE = N10;
				else if (CH == 'A' || CH == 'a' || CH == 'B' || CH == 'b' || CH == 'C' || CH == 'c' || CH == 'F' || CH == 'f') STATE = N16;
				else if (CH == 'E' || CH == 'e') { add(); gc(); STATE = E11; }
				else if (CH == 'D' || CH == 'd') { add(); gc(); STATE = D; }
				else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; }
				else if (CH == '.') { add(); gc(); STATE = P1; }
				else if (CH == 'O' || CH == 'o') { gc(); STATE = O; }
				else if (let()) STATE = ER;
				else STATE = N10;
				break;
			case N10:
				while (CH == '8' || CH == '9') { add(); gc(); }
				if (CH == 'A' || CH == 'a' || CH == 'B' || CH == 'b' || CH == 'C' || CH == 'c' || CH == 'F' || CH == 'f') STATE = N16;
				else if (CH == 'E' || CH == 'e') { add(); gc(); STATE = E11; }
				else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; }
				else if (CH == '.') { add(); gc(); STATE = P1; }
				else if (CH == 'D' || CH == 'd') { add(); gc(); STATE = D; }
				else if (let()) STATE = ER;
				else { put("TN", "int"); out(3, lexNum); STATE = H; }
				break;
			case N16:
				while (check_hex()) { add(); gc(); }
				if (CH == 'H' || CH == 'h') { gc(); STATE = HX; }
				else STATE = ER;
				break;
			case B:
				if (check_hex()) STATE = N16;
				else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; }
				else if (let()) STATE = ER;
				else { translate(2); put("TN", "int"); out(3, lexNum); STATE = H; }
				break;
			case O:
				if (let() || digit()) STATE = ER;
				else { translate(8); put("TN", "int"); out(3, lexNum); STATE = H; }
				break;
			case D:
				if (CH == 'H' || CH == 'h') { gc(); STATE = HX; }
				else if (check_hex()) STATE = N16;
				else if (let()) STATE = ER;
				else { put("TN", "int"); out(3, lexNum); STATE = H; }
				break;
			case HX:
				if (let() || digit())STATE = ER;
				else { translate(16); put("TN", "int"); out(3, lexNum); STATE = H; }
				break;
			case E11:
				if (digit()) { add(); gc(); STATE = E12; }
				else if (CH == '+' || CH == '-') { add(); gc(); STATE = ZN; }
				else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; }
				else if (check_hex()) { add(); gc(); STATE = N16; }
				else STATE = ER;
				break;
			case ZN:
				if (digit()) { add(); gc(); STATE = E13; }
				else STATE = ER;
				break;
			case E12:
				while (digit()) { add(); gc(); }
				if (check_hex()) STATE = N16;
				else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; }
				else if (let()) STATE = ER;
				else { convert(); put("TN", "float"); out(3, lexNum); STATE = H; }
				break;
			case E13:
				while (digit()) { add(); gc(); }
				if (let() || CH == '.') STATE = ER;
				else { convert(); put("TN", "float"); out(3, lexNum); STATE = H; }
				break;
			case P1:
				if (digit()) STATE = P2; else STATE = ER;
				break;
			case P2:
				while (digit()) { add(); gc(); }
				if (CH == 'E' || CH == 'e') { add(); gc(); STATE = E21; }
				else if (let() || CH == '.') STATE = ER;
				else { convert(); put("TN", "float"); out(3, lexNum); STATE = H; }
				break;
			case E21:
				if (CH == '+' || CH == '-') { add(); gc(); STATE = ZN; }
				else if (digit()) STATE = E22;
				else STATE = ER;
				break;
			case E22:
				while (digit()) { add(); gc(); }
				if (let() || CH == '.') STATE = ER;
				else { convert(); put("TN", "float"); out(3, lexNum); STATE = H; }
				break;
			case C1:
				if (CH == '*') { gc(); STATE = C2; }
				else { out(2, TL["/"]); STATE = H; }
				break;
			case C2:
				flag = 1;
				while (CH != '*' && flag && CH != '}')
					flag = gc();
				if (CH == '}' || !flag) STATE = ER;
				else { gc(); STATE = C3; }
				break;
			case C3:
				if (CH == '/') { gc(); STATE = H; }
				else STATE = C2;
				break;
			case M1:
				if (CH == '=') { gc(); out(2, TL["<="]); STATE = H; }
				else { out(2, TL["<"]); STATE = H; }
				break;
			case M2:
				if (CH == '=') { gc(); out(2, TL[">="]); STATE = H; }
				else { out(2, TL[">"]); STATE = H; }
				break;
			case M3:
				if (CH == '=') { gc(); out(2, TL["!="]); STATE = H; }
				else { out(2, TL["!"]); STATE = H; }
				break;
			case M4:
				if (CH == '=') { gc(); out(2, TL[":="]); STATE = H; }
				else { out(2, TL["=="]); STATE = H; }
				break;
			case M5:
				if (CH == '=') { gc(); out(2, TL["=="]); STATE = H; }
				else { STATE = ER; }
				break;
			case B1:
				if (CH == '&') { gc(); out(2, TL["&&"]); STATE = H; }
				else { STATE = ER; }
				break;
			case B2:
				if (CH == '|') { gc(); out(2, TL["||"]); STATE = H; }
				else { STATE = ER; }
				break;
			case OG:
				nill();
				add();
				look("TL");
				if (lexNum != 0) {
					gc();
					out(2, lexNum);
					STATE = H;
				}
				else STATE = ER;
				break;
			case V: break;
			case ER:
				cout << "An Lex error occurred after character \"" + to_string(CH) + "\"";
			}
		} while (STATE != V && STATE != ER);
		if (STATE == ER) {
			lOut << "Не распознанная лексема \"" << CH << "\"\n";
			cout << "ЛЕКСИЧЕСКИЙ АНАЛИЗАТОР: Не распознанная лексема \"" << CH << "\"\n";
			return false;
		}
		lOut.close();
		cIn.close();
		return true;
	}

	bool syntaxAndSeman(const string& filename) {
		const string lexFilename = "lex." + filename;
		lIn.open(lexFilename, ios::in | ios::binary);
		if (!lIn.is_open()) {
			throw runtime_error(string("What a heck?! Where is lex analyzed file \"" + lexFilename + "\"?").c_str());
		}
		PR();
		return true;
	}
};

int main(const int argc, char* argv[]) {
	setlocale(LC_ALL, "RUS");
	try {
		if (argc < 2) throw exception("Not enough arguments: 1");
		const string filename = argv[1];
		if (compiler comp; comp.lexer(filename)) {
			if (comp.syntaxAndSeman(filename)) {

			}
		}
	} catch (runtime_error e) {
		cout << "RUNTIME ERROR " << e.what() << "\n";
		throw e;
	} catch (exception e) {
		cout << "ERROR " << e.what() << "\n";
	}
	//system("pause");
}