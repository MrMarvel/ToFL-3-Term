// PracEx5.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <sstream>
#include <fstream>

using namespace std;


class State {
public:
    static size_t id_inc;
    size_t id = id_inc++;
    string state = "";
public:
    State(char s) {
        state = "";
        state += s;
    }
    State(string s) {
        state = "";
        state += s;
    }
    State() {}
    friend bool operator!=(State& s1, State& s2);
    bool isRight() {
        return !state.empty();
    }
    bool operator<(const State& s2) const {
        return state < s2.state;
    }
    string to_string() {
        return string("State{") + "state=" + state + "}";
    }

};

size_t State::id_inc = 1;

class CompoundState : public State{
public:
    set<State> states;
public:
    CompoundState(char s) : State(s){
        states.insert(s);
    }
    CompoundState(string s) : State(s){
        states.insert(s);
    }
    CompoundState(State s) : State(s){
        states.insert(s);
    }
    CompoundState() {}
    bool isRight() {
        for (auto st : states) {
            if (!st.isRight()) return false;
        }
        return true;
    }
    bool operator==(const CompoundState& s2) {
        if (states.size() != s2.states.size()) return false;
        for (auto s : states) {
            auto it = s2.states.find(s);
            if (it == s2.states.end()) return false;
        }
        return true;
    }
    bool operator!=(const CompoundState& s2) {
        return !operator==(s2);
    }
    bool operator<(const CompoundState& s2) const {
        return states < s2.states;
    }
    void insert(State st) {
        states.insert(st);
        if (states.size() < 2) {
            state = st.state;
            return;
        }
        state = "(";
        bool o;
        o = false;
        for (auto s : states) {
            if (o) state += "+";
            state += s.state;
            o = true;
        }
        state += ")";
    }
    void insert(CompoundState st) {
        insert((State)st);
    }
    string to_string() {
        string stS = "";
        bool o = false;
        for (auto s : states) {
            if (o) stS += ", ";
            stS += s.to_string();
            o = true;
        }
        return string("CompoundState{") + "states=[" + stS + "]" + "}";
    }
};

class Transition {
public:
    shared_ptr<CompoundState> from;
    char ch = ' ';
    shared_ptr<CompoundState> to;
public:
    Transition(CompoundState from, char ch, CompoundState to) {
        this->from = shared_ptr<CompoundState>(new CompoundState(from));
        this->ch = ch;
        this->to = shared_ptr<CompoundState>(new CompoundState(to));
    }
    Transition() {}
    bool static hasSamePrefix(Transition& s1, Transition& s2) {
        if (*s1.from != *s2.from) return false;
        if (s1.ch != s2.ch) return false;
        return true;
    }
    bool isRight() {
        if (!from) return false;
        if (!from->isRight()) return false;
        if (!to) return false;
        if (!to->isRight()) return false;
    }
    string to_string() {
        return string("Transition{") + "from=" + from->to_string() + ", ch=" + ch + ", to=" + to->to_string() + "}";
    }
};

class TransitionFunction {
public:
    vector<Transition> transitions;
public:
    vector<CompoundState> transition(CompoundState from, char ch) {
        vector<CompoundState> list;
        for (auto t : transitions) {
            if (*t.from != from) continue;
            if (t.ch != ch) continue;
            list.push_back(*t.to);
        }
        return list;
    }
    void addTransition(Transition t) {
        if (!t.isRight()) return;
        /*for (size_t i = 0; i < transitions.size(); i++) {
            Transition tr = transitions[i];
            if (!Transition::hasSamePrefix(t, tr)) continue;
            transitions.erase(transitions.begin() + i);
            i--;
        }*/ //Не позволяет делать НКА
        transitions.push_back(t);
    }
    string to_string(string tab = "") {
        string mt = "  ";
        string trS = "";
        bool o = false;
        for (auto s : transitions) {
            if (o) trS += ",\n" + tab + mt;
            trS += s.to_string();
            o = true;
        }
        return string("TransitionFunction{") +"transitions=[" + 
            "\n" + tab + mt + trS + 
            "\n" + tab + mt + "]" +
            "\n" + tab + "}";
    }
};

class Automation {
public:
    set<CompoundState> Q; //Множество состояний
    set<char> A; //Алфавит входных символов
    TransitionFunction F; //Множество переходов(функция переходов)
    CompoundState S; //Стартовое состояние
    set<State> E; //Конечные состояния
    string to_string() {
        string qS = "";
        bool o = false;
        for (auto s : Q) {
            if (o) qS += ", ";
            qS += s.to_string();
            o = true;
        }
        string aS = "";
        o = false;
        for (auto s : A) {
            if (o) aS += ", ";
            aS += s;
            o = true;
        }
        string eS = "";
        o = false;
        for (auto s : E) {
            if (o) eS += ", ";
            eS += s.to_string();
            o = true;
        }
        string t = "\t";
        return string("Automation{") + "\n\tQ=[" + qS + "]"+", \n\tA=["+aS+"]"+", \n\tF="+F.to_string(t)+", \n\tS="+S.to_string()+", \n\tE=["+eS+"]"+"\n}";
    }
};


Automation get_DFA_By_NFA_depr(Automation nfa) {
    Automation dfa;
    queue<CompoundState> P;
    TransitionFunction Fd;
    P.push({ nfa.S });
    vector<CompoundState> Qd;
    while (!P.empty()) {
        CompoundState pd = P.front();
        P.pop();
        for (char c : nfa.A) {
            CompoundState qd;
            for (CompoundState p : pd.states) {
                vector<CompoundState> list = nfa.F.transition(p, c);
                if (list.size() == 0) continue;
                //qd.insert(*);
                Fd.addTransition(Transition());
            }
        }
    }
    return dfa;
}

Automation get_DFA_By_NFA_depr2(Automation nfa) {
    Automation dfa;
    vector<CompoundState> Q;
    TransitionFunction F;
    Q.push_back({nfa.S});
    for (size_t i = 0; i < Q.size(); i++) {
        CompoundState compoundState = Q[i];
        for (char ch : nfa.A) {
            CompoundState resultStates;
            for (CompoundState cst : compoundState.states) {
                vector<CompoundState> list = nfa.F.transition(cst, ch);
                for (State st : cst.states) {
                    vector<CompoundState> psevList = nfa.F.transition(cst, ch);
                    for (auto fcst : psevList) {
                        auto it = find(list.begin(), list.end(), fcst);
                        if (it != list.end()) continue;
                        list.push_back(fcst);
                    }
                }
                if (list.size() == 0) continue;
                if (list.size() > 1) {
                    CompoundState resSt;
                    for (auto s : list) {
                        resSt.insert(s);
                    }
                    resultStates = (resSt);
                } else resultStates = (list.front());
            }
            Transition t = Transition(compoundState, ch, resultStates);
            F.addTransition(t);
            auto it = find(Q.begin(), Q.end(), resultStates);
            if (it != Q.end()) continue;
            Q.push_back(resultStates);
        }
    }
    dfa.A = nfa.A;
    dfa.Q = set<CompoundState>(Q.begin(), Q.end());
    dfa.F = F;
    dfa.S = nfa.S;
    return dfa;
}

template<typename T>
shared_ptr<T> findInVector(vector<T> v, T val) {
    auto it = find(v.begin(), v.end(), val);
    if (it == it.end()) return nullptr;
    return *it;
}

Automation get_DFA_By_NFA(Automation nfa) {
    Automation dfa;
    vector<vector<CompoundState>> Q;
    TransitionFunction F;
    Q.push_back({ nfa.S });
    for (size_t i = 0; i < Q.size(); i++) {
        vector<CompoundState> states = Q[i];
        for (char ch : nfa.A) {
            vector<CompoundState> resultStates;
            for (CompoundState state : states) {
                vector<CompoundState> list = nfa.F.transition(state, ch);
                if (list.size() == 0) continue;
                for (auto s : list) {
                    if (findInVector(resultStates, s)) continue;
                    resultStates.push_back(s);
                }
            }
            Transition t = Transition(states, ch, resultStates);
            F.addTransition(t);
            if (findInVector(Q, resultStates)) continue;
            Q.push_back(resultStates);
        }
    }
    dfa.A = nfa.A;
    dfa.Q = set<CompoundState>(Q.begin(), Q.end());
    dfa.F = F;
    dfa.S = nfa.S;
    return dfa;
}

Automation getFilledNFA() {
    const string FILE = "input.txt";
    shared_ptr<ifstream> input = shared_ptr<ifstream>(new ifstream(FILE));
    ifstream& in = *input;
    string line;
    istringstream ss;
    Automation nfa;
    printf("Enter set of states:\n");
    {
        getline(in, line);
        ss.str(line);
        string state;
        while (!ss.eof()) {
            if (!ss.good()) {
                ss.clear();
                ss.str();
                break;
            }
            ss >> state;
            nfa.Q.insert(State(state));
        }
    }
    printf("Enter the input alphabet:\n");
    {
        char ch;
        getline(in, line);
        ss = istringstream(line);
        while (!ss.eof()) {
            if (!ss.good()) {
                ss.clear();
                ss.str();
                break;
            }
            ss >> ch;
            if (ch == ' ') continue;
            nfa.A.insert(ch);
        }
    }
    printf("Enter state-transitions function <current state input character next state> :\n");
    printf("Example: 5 a 4\n");
    {
        string state1, state2;
        string str;
        char ch;
        while (!in.eof()) {
            getline(in, line);
            ss = istringstream(line);
            ss.clear();

            ss >> state1 >> str;
            if (!ss.good()) break;
            if (state1 == "0") break;
            ss >> state2;
            if (state2.length() < 1) break;
            ch = str[0];
            State st1, st2;
            st1 = State(state1);
            st2 = State(state2);
            Transition t = Transition(st1, ch, st2);
            nfa.F.addTransition(t);
            printf("Added transition: ");
            cout << t.to_string();
            printf("\n");
        }
    }
    printf("Enter a set of initial states(only 1):\n");
    {
        string state;
        getline(in, line);
        ss = istringstream(line);
        while (!ss.eof()) {
            if (!ss.good()) {
                ss.clear();
                ss.str();
                break;
            }
            ss >> state;
            State start = State(state);
            nfa.S = start;
            break;
        }
    }
    printf("Enter a set of final states:\n");
    {
        string state;
        getline(in, line);
        ss = istringstream(line);
        while (!ss.eof()) {
            if (!ss.good()) {
                ss.clear();
                ss.str();
                break;
            }
            ss >> state;
            State end = State(state);
            nfa.E.insert(end);
        }
    }
    return nfa;
}

void printAutomation(Automation a) {
    cout << "Set of states: ";
    bool o;
    o = false;
    for (auto s : a.Q) {
        if (o) cout << ", ";
        cout << s.state;
        o = true;
    }
    cout << "\n";

    cout << "\nInput alphabet: ";
    o = false;
    for (auto ch : a.A) {
        if (o) cout << ", ";
        cout << ch;
        o = true;
    }
    cout << "\n";

    printf("\nState-transitions function:\n");
    for (auto tr : a.F.transitions) {
        cout << "D(" << tr.from->state << ", " << tr.ch << ") = " << tr.to->state << "\n";
    }

    printf("\nInitial states: ");
    cout << a.S.state;
    printf("\n");

    printf("\nFinal states: ");
    o = false;
    for (auto s : a.E) {
        if (o) cout << ", ";
        cout << s.state;
        o = true;
    }
    printf("\n");
}

int main() {
    setlocale(LC_ALL, "RUS");
    cout << "NFA to DFA converter!\n";

    Automation nfa = getFilledNFA();
    printf("\n\nNFA:\n");
    printAutomation(nfa);
    printf("\n\nDFA:\n");
    Automation dfa = get_DFA_By_NFA(nfa);
    printAutomation(dfa);
    system("pause");
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

bool operator!=(State& s1, State& s2) {
    return s1.state != s2.state;
}