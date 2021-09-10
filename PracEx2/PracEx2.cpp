#include <string>
#include <iostream>
#include <stack>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <queue>

using namespace std;

int main();
bool isOperator(char);
bool precedence(char, char);
int evaluatePostfix(const string&);

istream* input = (istream*)&cin;//new ifstream("input.txt");

int main() {
    if (!input) throw runtime_error("No input stream");
    ifstream* fin = dynamic_cast<ifstream*>(input);
    if (fin) {
        if (!fin->is_open()) throw runtime_error("Invalid file");
    }
    istream& in = *input;
    string postfix;
    cout << "Enter the Postfix Expression: \n";
    getline(in, postfix);
    cout << "Evaluate prefix: " << postfix << " = " << evaluatePostfix(postfix) << "\n";

    system("pause");
    return 0;
}

bool isOperator(char currentChar) {
    switch (currentChar) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '^':
    case '%':
        return true;
    default:
        return false;
    }
}






//Предусловие: prefix - строка правильного формата префиксного выражения, не пустая, число макс в 1 символ
//Постусловие: Результат - целое число, значение выражения
int evaluatePostfix(const string& postfix) {
    int result = 0;
    stack<int> vars;
    for (long long l = postfix.size(), i = 0; i < postfix.size(); i++) {
        const char current = postfix[i];

        if (isspace(current)) {
            continue;
        } else if (isalnum(current)) {
            int x = current - '0';
            vars.push(x);
        } else if (isOperator(current)) {
            char oper = current;
            int a = vars.top();
            vars.pop();
            int b = vars.top();
            vars.pop();
            int res = 0;
            switch (oper) {
            case '+':
                res = a + b;
                break;
            case '-':
                res = a - b;
                break;
            case '*':
                res = a * b;
                break;
            case '/':
                res = a / b;
                break;
            case '^':
                res = pow(a, b);
                break;
            }
            vars.push(res);
        }
    }
    result = vars.top();
    return result;
}