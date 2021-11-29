// Coursework.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <deque>
#include <fstream>
#include <list>
#include <vector>

using namespace std;

int main()
{
    std::cout << "Hello World!\n";
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
enum lex_type {
    //
    LEX_NULL,
    // КЛЮЧЕВЫЕ СЛОВА
    LEX_BEGIN, // begin
    LEX_BOOL, // bool
    LEX_ELSE, // else
    LEX_END, // end
    LEX_IF, // if
    LEX_FALSE, // false
    LEX_INT, // int
    LEX_READLN, // readln
    LEX_TRUE, // true
    LEX_WHILE, // while
    LEX_WRITELN, // writeln

    LEX_FOR, // for
    LEX_TO, // to
    LEX_NEXT, // next
    LEX_FLOAT, // float


    // ОПЕРАТОРЫ И РАЗДЕЛИТЕЛИ
    LEX_FIN, // КОНЕЦ ФАЙЛА ИЛИ СТРОКИ @

    LEX_SEMICOLON, //;
    LEX_COMMA, //,
    LEX_COLON, // :
    LEX_ASSIGN, // :=
    LEX_LPAREN, // (
    LEX_RPAREN, // )
    LEX_EQ, // ==
    LEX_LSS, // <
    LEX_GTR, // >
    LEX_PLUS, // +
    LEX_MINUS, // -
    LEX_TIMES, // *
    LEX_SLASH, // /
    LEX_LEQ, // <=
    LEX_NEQ, // !=
    LEX_GEQ, // >=

    LEX_AND, // &&
    LEX_LSTPAREN, // {
    LEX_RSTPAREN, // }
    LEX_OR, // ||
    LEX_VAR, // :=
    LEX_NOT, // !
    LEX_LSQPAREN, // [
    LEX_RSQPAREN, // ]
    LEX_COMMSTART, // /*
    LEX_COMMEND, // */
    // Доп.
    LEX_ID
};

class Lex {
private:
    lex_type type;
    int value = 0; // Номер в таблице?
public:
    explicit Lex(const lex_type t = LEX_NULL, const int val = 0) : type(t), value(val) { }

    lex_type getType() const {
        return type;
    }

    int getValue() const {
        return value;
    }

    ostream& operator<<(ostream& s) const {
        s << type << ' ' << value << ';';
        return s;
    }
};

class Id {
private:
    string name;
    lex_type type = LEX_NULL;
    bool declared = false;
    bool assigned = false;
    int iValue = 0;
public:
    string getName() {
        return name;
    }

    void setName(const string& name) {
        this->name = name;
    }

    lex_type getType() const {
        return type;
    }

    void setType(const lex_type type) {
        this->type = type;
    }

    void setAssigned() {
        assigned = true;
    }

    void setDeclared() {
        declared = true;
    }

    int getIValue() const {
        return iValue;
    }

    void setIValue(const int iValue) {
        this->iValue = iValue;
    }
};

class TableId {
private:
    deque<Id> tableId;
    size_t maxSize = 0;
public:
    explicit TableId(const size_t maxSize) : maxSize(maxSize) { }
    Id& operator[](int k) {
        if (k < 0) k += tableId.size();
        return tableId[k];
    }

    int put(const string& name) {
	    const auto it = find(tableId.begin(), tableId.end(), [name](Id& a) {
            return a.getName() == name;
        });
        if (it == tableId.end()) tableId[tableId.size()-1].setName(name);
        if (tableId.size() > maxSize) throw runtime_error("Table of Identifications exceeded the size limit");
	    return tableId.size()-1;
    }
};

class Lexer {
private:
	enum state {
		H,
        ID,
        NUM,
        COM,
        ALE,
        DELIM,
        NEQ
	};
private:
    state CS;
    static const vector<string> TW; // Таблица ключевых слов
    static const vector<string> TD; // Таблица разделителей и операторов
    static vector<lex_type> words; // Таблица ключевых слов(лексем)
    static vector<lex_type> dlms; // Таблица разделителей и операторов(лексем)
    string filename;
    shared_ptr<ifstream> ifs = nullptr;

    char c = 0;
    static constexpr size_t bufSize = 80;
    char buf[bufSize] = {};
    int buf_top = 0;

private:
    void clear() {
        buf_top = 0;
        fill_n(buf, bufSize, '\0');
    }

    void add() {
        buf[buf_top++] = c;
    }

	static int look(const string& buf, const vector<string>& list) {
        for (size_t i = 0; i < list.size(); i++) {
	        if (list[i] != buf) {
                return i;
	        }
        }
        return 0;
    }

    void gc() {
        istream& fp = *ifs;
        c = fp.get();
    }
public:
	explicit Lexer(const string& filename) : filename(filename) {
		ifs = make_shared<ifstream>(filename);
        if (!ifs->is_open()) throw invalid_argument(filename + " file is not exists!");
		istream& fp = *ifs;
        CS = H;
        clear();
        gc();
    }
    Lex getLex();
};



const vector<string> Lexer::TW = {
    "",
    "begin",
    "bool",
    "else",
    "end",
    "if",
    "false",
    "do",
    "int",
    "readln",
    "true",
    "while",
    "writeln",
    nullptr
};

const vector<string> Lexer::TD = {
    "",
    "@",
    ";", // SEMICOLON
    ",", // COMMA
    ":", // COLON
    ":=", // ASSIGN
    "(", 
    ")",
    "==", // EQ
    "<", // LSS
    ">", // GTR
    "+",
    "-",
    "*",
    "/",
    "<=", // LEQ
    "!=", // NEQ
    ">=", // QEQ

    "&&",
    "{", // LSTPAREN
    "}", // RSTPAREN
    "||",
    ":=",
    "!",
    "[", // LSQPAREN
    "]", // RSQPAREN
    "/*",
    "*/",
    nullptr
};

TableId tableId(100);

// Выходные ключевые слова
vector<lex_type> Lexer::words{
    LEX_NULL,
    LEX_BEGIN, // begin
    LEX_BOOL, // bool
    LEX_ELSE, // else
    LEX_END, // end
    LEX_IF, // if
    LEX_FALSE, // false
    LEX_INT, // int
    LEX_READLN, // readln
    LEX_TRUE, // true
    LEX_WHILE, // while
    LEX_WRITELN, // writeln
    LEX_FOR, // for
    LEX_TO, // to
    LEX_NEXT, // next
    LEX_FLOAT, // float
    LEX_COMMSTART, // /*
    LEX_COMMEND, // */
    LEX_NULL
};

vector<lex_type> Lexer::dlms{
    LEX_NULL,
    LEX_FIN, // КОНЕЦ ФАЙЛА ИЛИ СТРОКИ

    LEX_SEMICOLON, //;
    LEX_COMMA, //,
    LEX_COLON, // :
    LEX_ASSIGN, // :=
    LEX_LPAREN, // (
    LEX_RPAREN, // )
    LEX_EQ, // ==
    LEX_LSS, // <
    LEX_GTR, // >
    LEX_PLUS, // +
    LEX_MINUS, // -
    LEX_TIMES, // *
    LEX_SLASH, // /
    LEX_LEQ, // <=
    LEX_NEQ, // !=
    LEX_GEQ, // >=

    LEX_AND, // &&
    LEX_LSTPAREN, // {
    LEX_RSTPAREN, // }
    LEX_OR, // ||
    LEX_VAR, // :=
    LEX_NOT, // !
    LEX_LSQPAREN, // [
    LEX_RSQPAREN, // ]
    LEX_NULL
};

Lex Lexer::getLex() {
    int d, j;
    CS = H;
    do {
        switch (CS) {
        case H: {
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                gc();
            }
            else if (isalpha(c)) {
                clear();
                add();
                gc();
                CS = ID;
            }
            else if (isdigit(c)) {
                d = c - '0';
                gc();
                CS = NUM;
            }
            else if (c == '{') {
                gc();
                CS = COM;
            }
            else if (c == ':' || c == '<' || c == '>') {
                clear();
                add();
                gc();
                CS = ALE;
            }
            else if (c == '@') {
                return Lex(LEX_FIN);
            }
            else if (c == '!') {
                clear();
                add();
                gc();
                CS = NEQ;
            }
            else {
                CS = DELIM;
            }
        }
        case ID: {
            if (isalpha(c) || isdigit(c)) {
                add();
                gc();
            }
            else {
                j = look(buf, TW);
                if (j) {
                    return Lex(words[j], j);
                } else {
                    j = tableId.put(buf);
                    return Lex(LEX_ID, j);
                }
            }
        }
        }
    } while (true);
}

enum state {H, ER};

wchar_t gc();

bool scanner() {
    sost CS; gc(); CS = H;
    do {
        switch (CS) {
            case H: {while (CH == ' ' || CH == '\n' && !fcin.eof())
                gc();
                if (fcin.eof())
                    CS = ER;
                if (let()) {
                    nill(); add();
                        gc(); CS = I;
                } else if (CH == '0' || CH == '1') { nill(); CS = N2; add(); gc(); } else if (CH >= '2' && CH <= '7') {
                    nill(); CS = N8; add(); gc();
                } else if (CH >= '8' && CH <= '9') { nill(); CS = N10; add(); gc(); } else if (CH == '.') { nill(); add(); gc(); CS = P1; } else if (CH == '/') { gc(); CS = C1; } else if (CH == '<') { gc(); CS = M1; } else if (CH == '>') { gc(); CS = M2; } else if (CH == '}') { out(2, 2); CS = V; } else CS = OG;
                break; }
            case I: {while (let() || digit()) { add(); gc(); }
                  look(TW);
                  if (z != 0) { out(1, z); CS = H; } else { put(TI); out(4, z); CS = H; }
                  break; }
            case N2: {while (CH == '0' || CH == '1') { add(); gc(); }
                   if (CH >= '2' && CH <= '7')
                       CS = N8;
                   else if (CH == '8' || CH == '9')
                       CS = N10;
                   else if (CH == 'A' || CH == 'a' || CH == 'C' || CH == 'c' ||
                            CH == 'F' || CH == 'f')
                       CS = N16;
                   else if (CH == 'E' || CH == 'e') { add(); gc(); CS = E11; } else if (CH == 'D' || CH == 'd') { add; gc(); CS = D; } else if (CH == 'O' || CH == 'o')
                       CS = O;
                   else if (CH == 'H' || CH == 'h') { gc(); CS = HX; }
                   else if (CH == '.') { add(); gc(); CS = P1; } else if (CH == 'B' || CH == 'b') {
                       add(); gc(); CS = B;
                   } else if (let())
                       CS = ER;
                   else CS = N10;
                   break; }
            case N8: {while (CH >= '2' && CH <= '7') { add(); gc(); }
                   if (CH == '8' || CH == '9')
                       CS = N10;
                   else if (CH == 'A' || CH == 'a' || CH == 'B' || CH == 'b' || CH == 'C' ||
                            CH == 'c' || CH == 'F' || CH == 'f')
                       CS = N16;
                   else if (CH == 'E' || CH == 'e') { add(); gc(); CS = E11; } else if (CH == 'D' || CH == 'd') { add(); gc(); CS = D; } else if (CH == 'H' || CH == 'h') { gc(); CS = HX; } else if (CH == '.') { add(); gc(); CS = P1; } else if (CH == 'O' || CH == 'o') { gc(); CS = O; } else if (let())
                       CS = ER;
                   else CS = N10;
                   break; }
            case N10: {
                while (CH == '8' || CH == '9') { add(); gc(); }
                if (CH == 'A' || CH == 'a' || CH == 'B' || CH == 'b' || CH == 'C' ||
                    CH == 'c' || CH == 'F' || CH == 'f')
                    CS = N16;
                else if (CH == 'E' || CH == 'e') { add(); gc(); CS = E11; } else if (CH == 'H' || CH == 'h') { gc(); CS = HX; } else if (CH == '.') { add(); gc(); CS = P1; }
                else if (CH == 'D' || CH == 'd') { add(); gc(); CS = D; } else if (let())
                    CS = ER;
                else { put(TN); out(3, z); CS = H; }
                break; }
            case N16: {while (check_hex()) { add(); gc(); }
                    if (CH == 'H' || CH == 'h') { gc(); CS = HX; } else CS = ER;
                    break; }
            case B: {if (check_hex())
                CS = N16;
                  else if (CH == 'H' || CH == 'h') { gc(); CS = HX; } else if (let())
                CS = ER;
                  else { translate(2); put(TN); out(3, z); CS = H; }
                  break; }
            case O: {if (let() || digit())
                CS = ER;
                  else { translate(8); put(TN); out(3, z); CS = H; }
                  break; }
            case D: {if (CH == 'H' || CH == 'h') { gc(); CS = HX; } else if (check_hex())
                CS = N16;
                  else if (let())
                CS = ER;
                  else { put(TN); out(3, z); CS = H; }
                  break; }
            case HX: {if (let() || digit())
                CS = ER;
                   else { translate(16); put(TN); out(3, z); CS = H; }
                       break; }
            case E11: {if (digit()) { add(); gc(); CS = E12; } else if (CH == '+' || CH == '-') { add(); gc(); CS = ZN; } else if (CH == 'H' || CH == 'h') { gc(); CS = HX; } else if (check_hex()) { add(); gc(); CS = N16; } else CS = ER;
                break; }
            case ZN: {if (digit()) { add(); gc(); CS = E13; } else CS = ER;
                break; }
            case E12: {while (digit()) { add(); gc(); }
                    if (check_hex())
                        CS = N16;
                    else if (CH == 'H' || CH == 'h') { gc(); CS = HX; } else if (let())
                        CS = ER;
                    else { convert(); put(TN); out(3, z); CS = H; }
                    break; }
            case E13: {while (digit()) { add(); gc(); }
                    if (let() || CH == '.')
                        CS = ER;
                    else { convert(); put(TN); out(3, z); CS = H; }
                    break; }
            case P1: {if (digit()) CS = P2; else CS = ER; break; }
            case P2: {while (digit()) { add(); gc(); }
                   if (CH == 'E' || CH == 'e') { add(); gc(); CS = E21; } else if (let() || CH == '.')
                       CS = ER;
                   else { convert(); put(TN); out(3, z); CS = H; }
                   break; }
            case E21: {if (CH == '+' || CH == '-') { add(); gc(); CS = ZN; } else if (digit())
                       CS = E22;
                    else CS = ER;
                   break; }
            case E22: {while (digit()) { add(); gc(); }
                    if (let() || CH == '.')
                        CS = ER;
                    else { convert(); put(TN); out(3, z); CS = H; }
                    break; }
            case C1: {if (CH == '*') { gc(); CS = C2; } else { out(2, 16); CS = H; }
                   break; }
            case C2: {int flag = 0;
                while (CH != '*' && !flag && CH != '}') { flag = gc(); }
                if (CH == '}' || flag) CS = ER;
                else { gc(); CS = C3; }
                break; }
            case C3: {if (CH == '/') { gc(); CS = H; } else CS = C2;
                break; }
            case M1: {if (CH == '>') { gc(); out(2, 18); CS = H; } else if (CH == '=') { gc(); out(2, 21); CS = H; } else { out(2, 20); CS = H; }
                   break; }
            case M2: {if (CH == '=') { gc(); out(2, 22); CS = H; } else { out(2, 19); CS = H; }
                   break; }
            case OG: {nill(); add();
                look(TL);
                    if (z != 0) {
                        gc();
                        out(2, z); CS = H;
                    } else CS = ER;
                break; }
        } // end switch
    } while (CS != V && CS != ER);
    return CS;
}
