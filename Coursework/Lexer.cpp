
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <stack>

using namespace std;

class Lexer {
private:
	ifstream cIn;
	ofstream lOut;
	ifstream lIn;
	char CH = ' ';
	string BUFFER, LEX;
	int lexNum = 0;
	bool first = false;
	int lexLine = 1;

	//—тек идентификаторов дл€ ???
	stack <int> TI_STACK;
	//—тек типов дл€ ???
	stack <string> TYPE_STACK;

	//ќписание переменной
	struct Var {
		bool isDeclared = false;
		bool isAssigned = false;
		string type;
		string address;
	public:
		Var() = default;
		explicit Var(string type) : isDeclared(true), type(std::move(type)) {}
		Var(string type, string address) : isDeclared(true), isAssigned(true), type(std::move(type)), address(std::move(address)) {}
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

	//ќписание числа
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
		{">", 18}, {"<", 19}, {"<=", 20}, {">=", 21}, {"/*", 22}, {"*/", 23}, {":=", 24}, {"Ц", 13}
	};
	unordered_map <string, int> TN = {};
	unordered_map <string, int> TI = {};
	unordered_map <string, shared_ptr<Var>> TI_SEM = {};
	unordered_map <string, shared_ptr<Num>> TN_SEM = {};


	/*
	TW - таблица служебных слов ћ-€зыка, #1
	TL Ц таблица ограничителей ћ-€зыка, #2
	TN Ц таблица чисел, используемых в программе #3
	TI - таблица идентификаторов программы, #4
	table - “аблица дл€ доступа к остальным таблицам
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
		OG, // «ј“џ„ ј ƒЋя »— Ћё„≈Ќ»… » — ќЅќ 
		V, // ¬џ’ќƒ
		ER, // ќЎ»Ѕ ј
	};

	/*
	CS - текущее состо€ние буфера
	Ќ Ц начало;
	I Ц идентификатор;
	N2 Ц двоичное число;
	N8 Ц восьмеричное число;
	N10 Цдес€тичное число;
	N16 Ц шестнадцатеричное число;
	—1, C2, C3 Ц комментарий;
	M1 Ц меньше, меньше или равно;
	M2 Ц больше, больше или равно;
	M3 Ц не равно;
	M4 Ц присваивание;
	M5 Ц равно;
	B1 Ц логическое »;
	B2 Ц логическое »Ћ»;
	P1 Ц точка;
	P2 Ц дробна€ часть числа;
	B Ц символ ЂBї или Ђbї;
	O Ц символ ЂOї или Ђoї;
	D Ц символ ЂDї или Ђdї;
	HX Ц символ ЂHї или Ђhї;
	E11 - символ ЂEї или Ђeї;
	E12,E13, E22 Ц пор€док числа в экспоненциальной форме;
	ZN, E21 Ц знак пор€дка числа в экспоненциальной форме;

	ќG Ц ограничитель;
	V Ц выход;
	ER Цошибка;
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
		sprintf_s(a, "„исло = %.8f", num);
		stream << a;
		stream >> num;
		return num;
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
			case H:
			{
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
				} else if (CH == '0' || CH == '1') { nill(); STATE = N2; add(); gc(); } else if (CH >= '2' && CH <= '7') { nill(); STATE = N8; add(); gc(); } else if (CH >= '8' && CH <= '9') { nill(); STATE = N10; add(); gc(); } else if (CH == '.') { nill(); add(); gc(); STATE = P1; } else if (CH == '/') { gc(); STATE = C1; } else if (CH == '<') { gc(); STATE = M1; } else if (CH == '>') { gc(); STATE = M2; } else if (CH == '!') { gc(); STATE = M3; } else if (CH == ':') { gc(); STATE = M4; } else if (CH == '=') { gc(); STATE = M5; } else if (CH == '&') { gc(); STATE = B1; } else if (CH == '|') { gc(); STATE = B2; } else if (CH == '}') { out(2, TL["}"]); STATE = V; } else STATE = OG;
				break;
			}
			case I:
				while (let() || digit()) { add(); gc(); }
				look("TW");
				if (lexNum != 0) { out(1, lexNum); STATE = H; } else { put("TI"); out(4, lexNum); STATE = H; }
				break;
			case N2:
				while (CH == '0' || CH == '1') { add(); gc(); }
				if (CH >= '2' && CH <= '7') STATE = N8;
				else if (CH == '8' || CH == '9') STATE = N10;
				else if (CH == 'A' || CH == 'a' || CH == 'C' || CH == 'c' || CH == 'F' || CH == 'f') STATE = N16;
				else if (CH == 'E' || CH == 'e') { add(); gc(); STATE = E11; } else if (CH == 'D' || CH == 'd') { add(); gc(); STATE = D; } else if (CH == 'O' || CH == 'o') STATE = O;
				else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; } else if (CH == '.') { add(); gc(); STATE = P1; } else if (CH == 'B' || CH == 'b') { add(); gc(); STATE = B; } else if (let()) STATE = ER;
				else STATE = N10;
				break;
			case N8:
				while (CH >= '2' && CH <= '7') { add(); gc(); }
				if (CH == '8' || CH == '9') STATE = N10;
				else if (CH == 'A' || CH == 'a' || CH == 'B' || CH == 'b' || CH == 'C' || CH == 'c' || CH == 'F' || CH == 'f') STATE = N16;
				else if (CH == 'E' || CH == 'e') { add(); gc(); STATE = E11; } else if (CH == 'D' || CH == 'd') { add(); gc(); STATE = D; } else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; } else if (CH == '.') { add(); gc(); STATE = P1; } else if (CH == 'O' || CH == 'o') { gc(); STATE = O; } else if (let()) STATE = ER;
				else STATE = N10;
				break;
			case N10:
				while (CH == '8' || CH == '9') { add(); gc(); }
				if (CH == 'A' || CH == 'a' || CH == 'B' || CH == 'b' || CH == 'C' || CH == 'c' || CH == 'F' || CH == 'f') STATE = N16;
				else if (CH == 'E' || CH == 'e') { add(); gc(); STATE = E11; } else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; } else if (CH == '.') { add(); gc(); STATE = P1; } else if (CH == 'D' || CH == 'd') { add(); gc(); STATE = D; } else if (let()) STATE = ER;
				else { put("TN", "int"); out(3, lexNum); STATE = H; }
				break;
			case N16:
				while (check_hex()) { add(); gc(); }
				if (CH == 'H' || CH == 'h') { gc(); STATE = HX; } else STATE = ER;
				break;
			case B:
				if (check_hex()) STATE = N16;
				else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; } else if (let()) STATE = ER;
				else { translate(2); put("TN", "int"); out(3, lexNum); STATE = H; }
				break;
			case O:
				if (let() || digit()) STATE = ER;
				else { translate(8); put("TN", "int"); out(3, lexNum); STATE = H; }
				break;
			case D:
				if (CH == 'H' || CH == 'h') { gc(); STATE = HX; } else if (check_hex()) STATE = N16;
				else if (let()) STATE = ER;
				else { put("TN", "int"); out(3, lexNum); STATE = H; }
				break;
			case HX:
				if (let() || digit())STATE = ER;
				else { translate(16); put("TN", "int"); out(3, lexNum); STATE = H; }
				break;
			case E11:
				if (digit()) { add(); gc(); STATE = E12; } else if (CH == '+' || CH == '-') { add(); gc(); STATE = ZN; } else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; } else if (check_hex()) { add(); gc(); STATE = N16; } else STATE = ER;
				break;
			case ZN:
				if (digit()) { add(); gc(); STATE = E13; } else STATE = ER;
				break;
			case E12:
				while (digit()) { add(); gc(); }
				if (check_hex()) STATE = N16;
				else if (CH == 'H' || CH == 'h') { gc(); STATE = HX; } else if (let()) STATE = ER;
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
				if (CH == 'E' || CH == 'e') { add(); gc(); STATE = E21; } else if (let() || CH == '.') STATE = ER;
				else { convert(); put("TN", "float"); out(3, lexNum); STATE = H; }
				break;
			case E21:
				if (CH == '+' || CH == '-') { add(); gc(); STATE = ZN; } else if (digit()) STATE = E22;
				else STATE = ER;
				break;
			case E22:
				while (digit()) { add(); gc(); }
				if (let() || CH == '.') STATE = ER;
				else { convert(); put("TN", "float"); out(3, lexNum); STATE = H; }
				break;
			case C1:
				if (CH == '*') { gc(); STATE = C2; } else { out(2, TL["/"]); STATE = H; }
				break;
			case C2:
				flag = 1;
				while (CH != '*' && flag && CH != '}')
					flag = gc();
				if (CH == '}' || !flag) STATE = ER;
				else { gc(); STATE = C3; }
				break;
			case C3:
				if (CH == '/') { gc(); STATE = H; } else STATE = C2;
				break;
			case M1:
				if (CH == '=') { gc(); out(2, TL["<="]); STATE = H; } else { out(2, TL["<"]); STATE = H; }
				break;
			case M2:
				if (CH == '=') { gc(); out(2, TL[">="]); STATE = H; } else { out(2, TL[">"]); STATE = H; }
				break;
			case M3:
				if (CH == '=') { gc(); out(2, TL["!="]); STATE = H; } else { out(2, TL["!"]); STATE = H; }
				break;
			case M4:
				if (CH == '=') { gc(); out(2, TL[":="]); STATE = H; } else { out(2, TL["=="]); STATE = H; }
				break;
			case M5:
				if (CH == '=') { gc(); out(2, TL["=="]); STATE = H; } else { STATE = ER; }
				break;
			case B1:
				if (CH == '&') { gc(); out(2, TL["&&"]); STATE = H; } else { STATE = ER; }
				break;
			case B2:
				if (CH == '|') { gc(); out(2, TL["||"]); STATE = H; } else { STATE = ER; }
				break;
			case OG:
				nill();
				add();
				look("TL");
				if (lexNum != 0) {
					gc();
					out(2, lexNum);
					STATE = H;
				} else STATE = ER;
				break;
			case V: break;
			case ER:
				cout << "An Lex error occurred after character \"" + to_string(CH) + "\"";
			}
		} while (STATE != V && STATE != ER);
		if (STATE == ER) {
			lOut << "Ќе распознанна€ лексема \"" << CH << "\"\n";
			cout << "Ћ≈ —»„≈— »… јЌјЋ»«ј“ќ–: Ќе распознанна€ лексема \"" << CH << "\"\n";
			return false;
		}
		lOut.close();
		cIn.close();
		return true;
	}
};