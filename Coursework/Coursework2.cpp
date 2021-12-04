#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <stack>

using namespace std;

class compiler {
private:
	ifstream cIn;
	fstream lIn;
	char CH = ' ';
	string BUFFER, LEX;
	int lexNum = 0;
	bool first = 0;

	stack <int> TI_STACK;
	stack <string> TYPE_STACK;

	//ќписание переменной
	struct Var {
		bool isdescr = 0;
		string type = "";
		string addres = "";
	};

	//ќписание числа
	struct Num {
		string type = "";
		string addres = "";

	unordered_map <string, int> TW = {
		{"readln", 1}, {"writeln", 2}, {"if", 3}, {"else", 4}, {"for", 5}, {"to", 6}, {"while", 7},
		{"begin", 8}, {"end", 9}, {"next", 10},{"true", 11}, {"false", 12},  {"step", 13},
		{"int", 14}, {"float", 15}, {"bool", 16}
	};
	unordered_map <string, int> TL = {
		{"{", 1}, {"}", 2}, {"&&", 3}, {"!", 4}, {"||", 5}, {",", 6},  {";", 7},  {"[", 8}, {"]", 9},
		{"(", 10}, {")", 11},  {"+", 12},  {"-", 13},  {"*", 14},  {"/", 15}, {"==", 16}, {"!=", 17},
		{">", 18}, {"<", 19}, {"<=", 20}, {">=", 21}, {"/*", 22}, {"*/", 23}, {":=", 24}
	};
	unordered_map <string, int> TN = {};
	unordered_map <string, int> TI = {};
	unordered_map <string, shared_ptr<Var>> TI_SEM = {};
	unordered_map <string, shared_ptr<Num>> TN_SEM = {};
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


	/*
	TW - таблица служебных слов ћ-€зыка, #1
	TL Ц таблица ограничителей ћ-€зыка, #2
	TN Ц таблица чисел, используемых в программе #3
	TI - таблица идентификаторов программы, #4
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
	SET Ц присваивание;
	M5 Ц равно;
	B1 Ц логическое »;
	B2 Ц логическое »Ћ»;
	P1 Ц точка;
	P2 Ц дробна€ часть числа;
	B Ц символ ЂBї или Ђbї;
	O Ц символ ЂOї или Ђoї;
	D Ц символ ЂDї или Ђdї;
	HX Ц символ ЂPROGї или Ђhї;
	E11 - символ ЂEї или Ђeї;
	E12,E13, E22 Ц пор€док числа в экспоненциальной форме;
	ZN, E21 Ц знак пор€дка числа в экспоненциальной форме;

	ќG Ц ограничитель;
	V Ц выход;
	ER Цошибка;
	*/

	int gc() {
		cIn.get(CH);
		if (CH != cIn.eof()) return 1;
		else return 0;
	}

	bool let() { return isalpha(CH); }

	bool digit() { return isdigit(CH); }

	void nill() { BUFFER = ""; }

	void add() { BUFFER += CH; }

	int look(string t) {
		if (table[t].find(BUFFER) == table[t].end()) lexNum = 0;
		else lexNum = table[t][BUFFER];
		return lexNum;
	}

	int put(string t) {
		if (table[t].find(BUFFER) == table[t].end()) table[t][BUFFER] = table[t].size() + 1;
		lexNum = table[t][BUFFER];
		return lexNum;
	}

	void out(int n, int k) {
		lIn << "(" << n << ", " << k << ")\n";
	}

	bool check_hex() { return isxdigit(CH); }

	bool AFH() { return (isxdigit(CH) || CH == 'H' || CH == 'h'); }

	int translate(int base) {
		int i = 0, rem = 0, number = 0, result = 0;
		stringstream stream;

		switch (base)
		{
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
			break;
		case 8:
			stream << BUFFER;
			stream >> number;
			while (number != 0)
			{
				rem = number % 10;
				number /= 10;
				result += rem * pow(8, i);
				++i;
			}
			return result;
			break;
		case 16:
			stream << BUFFER;
			stream >> std::hex >> result;
			return result;
			break;
		}
	}

	double convert() {
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
		lIn.open("lex." + filename, ios::out | ios::in | ios::trunc);
		states_ STATE;
		gc();
		STATE = H;
		int flag = 0;
		do {
			switch (STATE) {
			case H: {
				while ((CH == ' ' || CH == '\n' || CH == '\t' || CH == '\b') && !cIn.eof()) gc();
				if (cIn.eof()) STATE = ER;
				if (let()) {
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
				else { put("TN"); out(3, lexNum); STATE = H; }
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
				else { translate(2); put("TN"); out(3, lexNum); STATE = H; }
				break;
			case O:
				if (let() || digit()) STATE = ER;
				else { translate(8); put("TN"); out(3, lexNum); STATE = H; }
				break;
			case D:
				if (CH == 'H' || CH == 'h') { gc(); STATE = HX; }
				else if (check_hex()) STATE = N16;
				else if (let()) STATE = ER;
				else { put("TN"); out(3, lexNum); STATE = H; }
				break;
			case HX:
				if (let() || digit())STATE = ER;
				else { translate(16); put("TN"); out(3, lexNum); STATE = H; }
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
				else { convert(); put("TN"); out(3, lexNum); STATE = H; }
				break;
			case E13:
				while (digit()) { add(); gc(); }
				if (let() || CH == '.') STATE = ER;
				else { convert(); put("TN"); out(3, lexNum); STATE = H; }
				break;
			case P1:
				if (digit()) STATE = P2; else STATE = ER;
				break;
			case P2:
				while (digit()) { add(); gc(); }
				if (CH == 'E' || CH == 'e') { add(); gc(); STATE = E21; }
				else if (let() || CH == '.') STATE = ER;
				else { convert(); put("TN"); out(3, lexNum); STATE = H; }
				break;
			case E21:
				if (CH == '+' || CH == '-') { add(); gc(); STATE = ZN; }
				else if (digit()) STATE = E22;
				else STATE = ER;
				break;
			case E22:
				while (digit()) { add(); gc(); }
				if (let() || CH == '.') STATE = ER;
				else { convert(); put("TN"); out(3, lexNum); STATE = H; }
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
			}
		} while (STATE != V && STATE != ER);
		if (STATE == ER) lIn << "Ќе распознанна€ лексема \"" << CH << "\"\n";
		lIn.close();
		cIn.close();
		return STATE;
	}
	bool syntax(const string& filename) {

		return true;
	}
};

int main() {
	compiler comp;
	string filename = "mprogram.txt";
	if (comp.lexer(filename)) {
		if (comp.syntax(filename)) {

		}
	}
}