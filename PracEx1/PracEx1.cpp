#include <string>
#include <iostream>
#include <stack>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

using namespace std;

int main();
bool isOperator(char);
bool precedence(char, char);
string convertToPrefix(const string&);
string convertToPostfix(const string&);

istream* input = (istream*)&cin;//new ifstream("input.txt");

int main() {
    if (!input) throw runtime_error("No input stream");
    ifstream* fin = dynamic_cast<ifstream*>(input);
    if (fin) {
        if (!fin->is_open()) throw runtime_error("Invalid file");
    }
    istream& in = *input;
    for (; !in.eof();) {
        if (!std::cout.good()) break;
        std::cout << "Enter the Infix Expression: \n";
        string infix;
        getline(in, infix);
        if (infix.empty()) break;

        std::cout << "Postfix: '" << convertToPostfix(infix) << "'\n";
        break;
    }

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

// returns whether a `lOp` b `rOp` c == (a `lOp` b) `rOp` c
bool precedence(char leftOperator, char rightOperator) {
    if (leftOperator == '^') {
        return true;
    } else if (rightOperator == '^') {
        return false;
    } else if (leftOperator == '*' || leftOperator == '/' || leftOperator == '%') {
        return true;
    } else if (rightOperator == '*' || rightOperator == '/' || rightOperator == '%') {
        return false;
    }

    return true;
}



//Предусловие: infix - строка правильного формата инфиксного выражения, не пустая, перемнные макс в 1 символ
//Постусловие: Результат - строка префиксного выражения
string convertToPrefix(const std::string& infix) {
    stringstream prefix; // Our return string
    stack<char> stack;
    stack.push(')'); // Push a left parenthesis ‘)‘ onto the stack.
    for (long i = infix.size() - 1; i >= 0; i--) {
        const char current = infix[i];

        if (isspace(current)) {
            // ignore
        }
        // If it's a digit or '.' or a letter ("variables"), add it to the output
        else if (isalnum(current) || '.' == current) {
            string tmp = prefix.str();
            prefix.seekp(0);
            prefix << current;
            prefix << tmp;
        }

        else if (')' == current) {
            stack.push(current);
        }

        else if (isOperator(current)) {
            char leftOperator = current;
            while (!stack.empty() && isOperator(stack.top()) && precedence(stack.top(), leftOperator)) {
                string tmp = prefix.str();
                prefix.seekp(0);
                prefix << stack.top();
                prefix << tmp;
                stack.pop();
            }
            prefix;
            stack.push(leftOperator);
        }

        // We've hit a left parens
        else if ('(' == current) {
            // While top of stack is not a right parens
            while (!stack.empty() && ')' != stack.top()) {
                string tmp = prefix.str();
                prefix.seekp(0);
                prefix << stack.top();
                prefix << tmp;
                stack.pop();
            }
            if (stack.empty()) {
                throw runtime_error("missing left paren");
            }
            // Discard the right paren
            stack.pop();
            prefix;
        } else {
            throw runtime_error("invalid input character");
        }
    }


    // Started with a left paren, now close it:
    // While top of stack is not a right paren
    while (!stack.empty() && ')' != stack.top()) {
        string tmp = prefix.str();
        prefix.seekp(0);
        prefix << stack.top();
        prefix << tmp;
        stack.pop();
    }
    if (stack.empty()) {
        throw runtime_error("missing left paren");
    }
    // Discard the right paren
    stack.pop();

    // all open parens should be closed now -> empty stack
    if (!stack.empty()) {
        throw runtime_error("missing right paren");
    }

    return prefix.str();
}

string convertToPostfix(const std::string& infix) {
    std::stringstream postfix; // Our return string
    std::stack<char> stack;
    stack.push('('); // Push a left parenthesis ‘(‘ onto the stack.

    for (size_t i = 0, l = infix.size(); i < l; ++i) {
        const char current = infix[i];

        if (isspace(current)) {
            // ignore
        }
        // If it's a digit or '.' or a letter ("variables"), add it to the output
        else if (isalnum(current) || '.' == current) {
            postfix << current;
        }

        else if ('(' == current) {
            stack.push(current);
        }

        else if (isOperator(current)) {
            char rightOperator = current;
            while (!stack.empty() && isOperator(stack.top()) && precedence(stack.top(), rightOperator)) {
                postfix << stack.top();
                stack.pop();
            }
            postfix;
            stack.push(rightOperator);
        }

        // We've hit a right parens
        else if (')' == current) {
            // While top of stack is not a left parens
            while (!stack.empty() && '(' != stack.top()) {
                postfix << stack.top();
                stack.pop();
            }
            if (stack.empty()) {
                throw runtime_error("missing left paren");
            }
            // Discard the left paren
            stack.pop();
            postfix;
        } else {
            throw runtime_error("invalid input character");
        }
    }


    // Started with a left paren, now close it:
    // While top of stack is not a left paren
    while (!stack.empty() && '(' != stack.top()) {
        postfix << stack.top();
        stack.pop();
    }
    if (stack.empty()) {
        throw runtime_error("missing left paren");
    }
    // Discard the left paren
    stack.pop();

    // all open parens should be closed now -> empty stack
    if (!stack.empty()) {
        throw runtime_error("missing right paren");
    }

    return postfix.str();
}