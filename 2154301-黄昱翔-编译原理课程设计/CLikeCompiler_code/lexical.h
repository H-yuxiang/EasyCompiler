#include "common.h"

//五类识别token的DFA的基类
class DFA
{
public:
    explicit DFA();
    //是否被该DFA接受，必须被子类重载
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) = 0;
};

//识别关键字的DFA
class KeyWordsDFA :public DFA
{
private:
    std::map<std::string, int>KeyWordsmap;
public:
    KeyWordsDFA();
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) override;
};

//识别标识符的DFA
class IdentifierDFA :public DFA
{
public:
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) override;
};

//识别数值的DFA
class DigitDFA :public DFA
{
private:
    //定义接受到数值的状态，整数/小数点前后/科学计数法前后/出错/终点
    int state;
    int SInteger = 0;
    int SDecimalBefore = 1;
    int SDecimalAfter = 2;
    int SSCIBefore = 3;
    int SSCIAfter = 4;
    int SERROR = 5;
    int STerminal = 6;
public:
    DigitDFA();
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) override;
};

//识别界符的DFA
class DelimiterDFA :public DFA
{
public:
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) override;
};

//识别算符的DFA
class OperatorDFA :public DFA
{
private:
    std::map<std::string, int> Operatormap;
public:
    OperatorDFA();
    virtual int isAccepted(const std::string& str, int& start, int& end, int& row, int& col) override;
};

//进行词法分析的类
class Lexical
{
private:
    DFA* myDFA[5];
public:
    Lexical();
    ~Lexical();
    std::pair<int, std::vector<token>> Analyze(std::string& inpput);
};