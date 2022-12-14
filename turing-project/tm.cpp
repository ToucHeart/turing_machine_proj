#include <fstream>
#include "message.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include "tm.h"
using namespace std;

// 1.Q S G F 有{} 以,分隔
// 2.q0 B N 无{}和,
// 3.delta function是5个字符串,之间以空格分隔,没有{} ,
#define DEBUG

static int lineCount = 0; //记录读取文件的行数

void removeBrackets(string &line) //得到{}中的部分 Q S G F
{
    size_t left = line.find('{');
    size_t right = line.find('}');
    if (left == string::npos || right == string::npos)
        printMessage(NO_BRACKETS, NO_BRACKETS);
    line = line.substr(left + 1, right - left - 1);
}

void removeComment(string &line) // q0 N delta function   去掉行尾注释和行尾多余空格
{
    int idx = line.find(';');
    if (idx != string::npos)
        line = line.substr(0, idx);
    int len = line.size() - 1;
    while (line[len] == ' ')
        len--;
    line.resize(len + 1);
}

void getLineElement(string &line, char split, vector<string> *strs = nullptr, vector<char> *chars = nullptr) // 0 char ,1 str
{
    stringstream ss(line);
    string token;
    if (strs)
    {
        while (getline(ss, token, split))
        {
            strs->push_back(token);
        }
    }
    else if (chars)
    {
        while (getline(ss, token, split))
        {
            chars->push_back(token[0]);
        }
    }
#if 0
    for (auto &i : receiver)
        cout << i << ' ';
    cout << endl;
#endif
}

void TM::addState(string &line, unordered_set<string> &states) // case 1 & case 3
{
    vector<string> tmp;
    getLineElement(line, ',', &tmp);
    for (const auto &s : tmp)
    {
        states.insert(s);
    }
}

void TM::addSymbol(string &line, unordered_set<char> &symbols) // case 1 & case 3
{
    vector<char> tmp;
    getLineElement(line, ',', nullptr, &tmp);
    for (const auto &c : tmp)
    {
        symbols.insert(c);
    }
}

void TM::addValue(string &line)
{
    char type = line[1];
    removeBrackets(line);
    switch (type)
    {
    case 'Q':
        addState(line, this->states);
        break;
    case 'F':
        addState(line, this->finalStates);
        break;
    case 'S':
        addSymbol(line, this->inputSymbols);
        break;
    case 'G':
        addSymbol(line, this->tapeSymbols);
        break;
    }
}

void TM::setValue(string &line) // q0 N
{
    int left = line.find('=');
    left++;
    while (left < line.size() && line[left] == ' ')
        left++;
    if (line[1] == 'q')
        startState = line.substr(left, line.size() - left);
    else if (line[1] == 'N')
    {
        int num = 0;
        while (left < line.size())
        {
            num = num * 10 + line[left] - '0';
            left++;
        }
        tapeNumber = num;
    }
}

void TM::reportDeltaError(const string &error, const string &type)
{
    printMessage(SYNTAX_ERROR, NORMAL, !verbose);
    if (verbose)
    {
        cerr << ": line " << lineCount << ": \'" << error << "\' , No such one in " << type << endl;
    }
    exit(SYNTAX_ERROR);
}

void TM::addDelta(string &line)
{
    vector<string> tmp;
    getLineElement(line, ' ', &tmp);

    if (tmp.size() != 5)
        printMessage(SYNTAX_ERROR, SYNTAX_ERROR);
#if 0
    for (auto &s : tmp)
        cout << s << ' ';
    cout << endl;
#endif

    //"<旧状态> <旧符号组> <新符号组> <方向组> <新状态>"，

    // check oldstate in states
    if (states.find(tmp[0]) == states.end())
    {
        reportDeltaError(tmp[0], "states");
    }
    // check oldsymbol
    for (int i = 0; i < tmp[1].size(); i++)
    {
        if (tmp[1][i] != '*' && tapeSymbols.find(tmp[1][i]) == tapeSymbols.end())
            reportDeltaError(tmp[1], "tapeSymbols");
    }
    // check newsymbol
    for (int i = 0; i < tmp[2].size(); i++)
    {
        if (tmp[2][i] != '*' && tapeSymbols.find(tmp[2][i]) == tapeSymbols.end())
            reportDeltaError(tmp[2], "tapeSymbols");
    }
    // check dir in l r *
    for (auto c : tmp[3])
    {
        if (c != 'l' && c != 'r' && c != '*')
            reportDeltaError(tmp[3], "directions");
    }
    // check newstate
    if (states.find(tmp[4]) == states.end())
    {
        reportDeltaError(tmp[4], "states");
    }
    // add to delta
    deltafunc.insert(pair<string, vector<string>>(tmp[0], {tmp[1], tmp[2], tmp[3], tmp[4]}));
}

void parseFile(const string &filename, TM *tm)
{
    ifstream input(filename, ios::in);
    if (!input)
    {
        cout << strerror(errno) << ": " << filename;
        printMessage(FILE_OPEN_ERROR, FILE_OPEN_ERROR);
    }
    string line;
    while (getline(input, line))
    {
        lineCount++;
        if (line.empty() || line[0] == ';')
            continue;
        removeComment(line);
        if (line[0] == '#')
        {
            switch (line[1])
            {
            case 'Q':
            case 'S':
            case 'G':
            case 'F':
            {
                tm->addValue(line);
            }
            break;
            case 'B':
                break;
            case 'q':
            case 'N':
                tm->setValue(line);
                break;
            default:
                printMessage(UNKNOWN_GRAMMAR, UNKNOWN_GRAMMAR);
                break;
            }
        }
        else // delta functions
        {
            tm->addDelta(line);
        }
    }
    input.close();
}

void TM::checkInput(const string &input) const
{
    if (verbose)
        cout << "Input: " << input << endl;
    for (int i = 0; i < input.length(); ++i)
    {
        if (inputSymbols.find(input[i]) == inputSymbols.end())
        {
            if (verbose)
            {
                cout << "==================== ERR ====================" << endl;
                cout << "error: \'" << input[i] << "\' was not declared in the set of input symbols" << endl;
                cout << "Input: " << input << endl;
                cout << string(7 + i, ' ') << '^' << endl;
                cout << "==================== END ====================" << endl;
                exit(ILLEGAL_INPUT);
            }
            else
            {
                printMessage(ILLEGAL_INPUT, ILLEGAL_INPUT);
            }
        }
    }
    if (verbose)
        cout << "==================== RUN ====================" << endl;
}

void TM::setTapes(const string &input)
{
    Tape *first = new Tape(input);
    tapes.push_back(first);
    for (int i = 1; i < tapeNumber; ++i)
    {
        Tape *p = new Tape();
        tapes.push_back(p);
    }
}

TM::~TM()
{
    for (int i = 0; i < tapeNumber; ++i)
    {
        delete tapes[i];
    }
}

TM::TM(const string &filename, bool v) : verbose(v), steps(0), BLANK('_')
{
    parseFile(filename, this);
    currentState = startState;
    // printSelf();
}
/*
Step   : 0
Index0 : 0 1 2 3 4 5 6
Tape0  : 1 0 0 1 0 0 1
Head0  : ^
Index1 : 0
Tape1  : _
Head1  : ^
State  : 0
*/
void Tape::getBorder(int &leftbeg, int &leftend, int &rightbeg, int &rightend) const
{
    while (leftbeg < 0 && leftHalf[-leftbeg - 1] == '_' && leftbeg != head)
        leftbeg++;
    while (rightend >= 0 && rightHalf[rightend] == '_' && rightend != head)
        --rightend;
    if (leftbeg >= 0) //只有右半部分
    {
        while (rightbeg < rightend && rightHalf[rightbeg] == '_' && rightbeg != head)
            rightbeg++;
    }
    else if (rightend < 0) //只有左半部分
    {
        while (leftend > leftbeg && leftHalf[-leftend - 1] == '_' && leftend != head)
            leftend--;
    }
}

void Tape::printTapeContent(const int &leftbeg, const int &leftend, const int &rightbeg, const int &rightend, string &ans, bool printBlank) const
{
    if (leftbeg < 0)
    {
        for (int i = -leftbeg - 1; i >= -leftend - 1; --i)
        {
            ans += leftHalf[i];
            if (printBlank)
            {
                ans += string(to_string(i + 1).size(), ' '); //上下对齐
            }
        }
    }
    if (rightend >= 0)
    {
        for (int i = rightbeg; i <= rightend; ++i)
        {
            ans += rightHalf[i];
            if (printBlank)
            {
                ans += string(to_string(i).size(), ' ');
            }
        }
    }
}

void Tape::printResult()
{
    int leftbeg = -leftHalf.size(), leftend = -1, rightbeg = 0, end = rightHalf.size() - 1;
    getBorder(leftbeg, leftend, rightbeg, end);
    string ans;
    printTapeContent(leftbeg, leftend, rightbeg, end, ans, false);
    if (ans.size() == 1 && ans[0] == '_')
        cout << "";
    else
        cout << ans;
}

void Tape::printSelf(int idx) const
{

#ifndef DEBUG
    cout << "leftHalf: " << leftHalf << endl;
    cout << "rightHalf: " << rightHalf << endl;
#endif

    int leftbeg = -leftHalf.size(), leftend = -1, rightbeg = 0, end = rightHalf.size() - 1;
    getBorder(leftbeg, leftend, rightbeg, end);

    cout << "Index" << idx << " : ";
    if (leftbeg < 0)
    {
        for (int i = -leftbeg; i >= -leftend; --i)
            cout << i << ' ';
    }
    if (end >= 0)
    {
        for (int i = rightbeg; i <= end; ++i)
            cout << i << ' ';
    }
    cout << endl
         << "Tape" << idx << "  : ";

    string ans;
    printTapeContent(leftbeg, leftend, rightbeg, end, ans);
    cout << ans;

    cout << endl
         << "Head" << idx << "  : ";
    if (leftbeg >= 0)
        leftbeg = rightbeg;
    int blankCount = 0;
    for (int j = leftbeg; j < head; ++j)
    {
        blankCount += to_string(abs(j)).size() + 1;
    }
    cout << string(blankCount, ' ') << '^' << endl;
}

void TM::printId() const
{
    cout << "Step   : " << steps << endl;
    cout << "State  : " << currentState << endl;
    for (int i = 0; i < tapes.size(); ++i)
    {
        tapes[i]->printSelf(i);
    }
    cout << "---------------------------------------------" << endl;
}

void TM::findFunc(multimap<string, vector<string>>::iterator &it) //找到合适的转移函数
{
    string curSym;
    for (int i = 0; i < tapes.size(); ++i)
    {
        curSym += tapes[i]->getCurrVal();
    }
    auto beg = deltafunc.lower_bound(currentState);
    auto end = deltafunc.upper_bound(currentState);
    for (; beg != end; ++beg)
    {
        string patternSym = beg->second[0];
        int i = 0;
        for (; i < curSym.size(); ++i)
        {
            if (patternSym[i] == curSym[i])
                continue;
            else if (patternSym[i] == '*')
            {
                if (curSym[i] != '_')
                    continue;
                else
                    break;
            }
            else
                break;
        }
        if (i == curSym.size())
            it = beg;
    }
}

void TM::Move(const string &newSym, const string &dir)
{
    for (int i = 0; i < newSym.size(); ++i)
    {
        tapes[i]->setNewSym(newSym[i]);
        tapes[i]->move(dir[i]);
    }
}

void TM::printResult()
{
    if (verbose)
        cout << "Result: ";
    tapes[0]->printResult();
    if (verbose)
        cout << endl
             << "==================== END ====================";
    cout << endl;
}

//"<旧状态> -> <旧符号组> <新符号组> <方向组> <新状态>"
void TM::run(const string &input)
{
    checkInput(input);
    setTapes(input);
    while (true)
    {
        if (verbose)
            printId();
        auto it = deltafunc.end();
        findFunc(it);
        if (it == deltafunc.end())
            break;
        string &newSym = it->second[1];
        string &dir = it->second[2];
        string &newstate = it->second[3];
        Move(newSym, dir);
        currentState = newstate;
        steps++;
    }
}

void TM::printSelf() const
{
    cout << "states: ";
    for (auto &i : states) // Q
        cout << i << ' ';
    cout << endl
         << "inputSymbols: ";
    for (auto &i : inputSymbols) // S
        cout << i << ' ';
    cout << endl
         << "tapeSymbols: ";
    for (auto &i : tapeSymbols) // G
        cout << i << ' ';
    cout << endl
         << "finalStates: ";
    for (auto &i : finalStates) // F
        cout << i << ' ';
    cout << endl
         << "startState: " << startState << endl;     // startState
    cout << "BLANK: " << BLANK << endl;               // B
    cout << "tapeNumber: " << tapeNumber << endl;     // N
    cout << "steps: " << steps << endl;               // steps
    cout << "currentState: " << currentState << endl; // currentState
    cout << "verbose: " << verbose << endl;           // verbose
}