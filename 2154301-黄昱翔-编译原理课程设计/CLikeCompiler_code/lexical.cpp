#include "tool.h"
#include "lexical.h"

/************************************************
类别：各DFA基类
*************************************************/
DFA::DFA()
{
}

/************************************************
类别：识别关键字的DFA实现
	创建keywords和索引的std::mapping关系
	判断是否被关键字DFA接受
*************************************************/
KeyWordsDFA::KeyWordsDFA()
{
	for (int i = 0; i < KeyWord_NUM; i++) {
		KeyWordsmap[KeyWords[i]] = i;
	}
}
int KeyWordsDFA::isAccepted(const std::string& str, int& start, int& end, int& row, int& col)
{
	int pos = start;//当前读入的位置
	int cur_col = col;//当前读入列
	std::string nowWord = "";//当前识别的单词

	while (true) {
		if (isLetter(str[pos]) || isDigit(str[pos])) {
			nowWord += str[pos++];
			cur_col++;
		}
		else
			break;
	}

	auto it = KeyWordsmap.find(nowWord);
	if (it != KeyWordsmap.end()) {
		//如果是关键字的话,返回关键字在关键字数组中的索引
		end = pos - 1;
		col = cur_col;
		return it->second;
	}

	return WRONG;
}

/*************************************************
类别：识别标识符的DFA实现
	判断是否被标识符DFA接受
	并将识别出来的标识符token加入标识符集里
*************************************************/
int IdentifierDFA::isAccepted(const std::string& str, int& start, int& end, int& row, int& col)
{
	int pos = start;//当前读入的位置
	int cur_col = col;//当前读入列
	std::string nowWord = "";//当前识别的单词
	while (true) {
		if (isLetter(str[pos]) || isDigit(str[pos])) {
			nowWord += str[pos++];
			cur_col++;
		}
		else
			break;
	}
	if (nowWord == "")
		return WRONG;
	end = pos - 1;
	col = cur_col;
	Identifier.insert(nowWord);//插入标识符
	return RIGHT;//若识别成功，返回正确
}

/*************************************************
类别：识别数值的DFA实现
	构造函数，将状态初始化为接受整数
	判断是否被数值DFA接受
*************************************************/
DigitDFA::DigitDFA()
{
	state = SInteger;
}
int DigitDFA::isAccepted(const std::string& str, int& start, int& end, int& row, int& col)
{
	if (state != SInteger)
		return WRONG;
	int pos = start;
	//接受字符直到出错或者终止
	while (1) {
		if (pos > str.size())
			return WRONG;
		if (state == SInteger) {
			if (str[pos] >= '0' && str[pos] <= '9');
			else if (str[pos] == '.') state = SDecimalBefore;//小数之前 形如235.
			else if (str[pos] == 'e')state = SSCIBefore;//科学计数法之前 形如455e
			else {
				state = STerminal;//数值读取结束
				break;
			}
		}
		else if (state == SDecimalBefore) {
			//此时形如235.
			if (str[pos] >= '0' && str[pos] <= '9') state = SDecimalAfter;//小数之后，形如235.5
			else if (str[pos] == 'e') state = SSCIBefore;//科学计数法之前 形如455.e  与455.0e相等
			else if (str[pos] == '.') {
				state = SERROR;
				break;
			}//形如123..  出错
			else {
				state = STerminal;
				break;
			}
		}
		else if (state == SDecimalAfter) {
			if (str[pos] >= '0' && str[pos] <= '9');
			else if (str[pos] == 'e') state = SSCIBefore;//科学计数法之前 形如455.1e
			else if (str[pos] == '.') {
				state = SERROR;
				break;
			}//形如123.1.  出错
			else {
				state = STerminal;
				break;
			}
		}
		else if (state == SSCIBefore) {
			if (str[pos] >= '0' && str[pos] <= '9') state = SSCIAfter;//科学计数法之后 形如123e1
			else {
				state = SERROR;
				break;
			}
		}
		else if (state == SSCIAfter) {
			if (str[pos] >= '0' && str[pos] <= '9');//科学计数法之后 形如123e12
			else if (str[pos] == 'e' || str[pos] == '.') {
				state = SERROR;
				break;
			}
			else {
				state = STerminal;
			}
		}
		pos++;
	}
	if (state == STerminal) {
		state = SInteger;
		end = pos - 1;
		col += pos - start;
		std::string tmp = str.substr(start, pos - start);
		//将识别到的字符加入数值集
		Digit.insert(tmp);
		return RIGHT;
	}
	//SERROR
	else {
		state = SInteger;
		return WRONG;
	}
}

/*************************************************
类别：识别界符的DFA实现
	查找该字符是否在界符集合中，
	判断是否被界符DFA接受
*************************************************/
int DelimiterDFA::isAccepted(const std::string& str, int& start, int& end, int& row, int& col)
{
	int pos = start;
	for (int i = 0; i < Delimiter_NUM; i++) {
		//找到了
		if (str.substr(pos).find(Delimiter[i]) != -1) {
			start = end = pos;
			col++;
			return RIGHT;
		}
	}
	return WRONG;
}

/*************************************************
类别：识别算符的DFA实现
	创建operator和索引的std::mapping关系
	判断是否能和算符表里的算符匹配上
	只要匹配上了，就返回
*************************************************/
OperatorDFA::OperatorDFA()
{
	for (int i = 0; i < Operator_NUM; i++)
		Operatormap[Operator[i]] = i;
}
int OperatorDFA::isAccepted(const std::string& str, int& start, int& end, int& row, int& col)
{
	int pos = start;
	char op = str[pos];
	if (op == '+') {
		char opp = str[pos + 1];
		if (opp == '+') end = pos + 1;// ++
		else if (opp == '=') end = pos + 1;// +=
		else end = pos;// +
	}
	else if (op == '-') {
		char opp = str[pos + 1];
		if (opp == '-') end = pos + 1;// --
		else if (opp == '=') end = pos + 1;// -=
		else if (opp == '>') end = pos + 1;// ->
		else end = pos;//  -
	}
	else if (op == '>') {
		char opp = str[pos + 1];
		if (opp == '=') end = pos + 1;// >=
		else if (opp == '>') {
			char oppp = str[pos + 2];
			if (oppp == '=') end = pos + 2;// >>=
			else end = pos + 1;// >>
		}
		else end = pos;// >
	}
	else if (op == '<') {
		char opp = str[pos + 1];
		if (opp == '=') end = pos + 1;// <=
		else if (opp == '<') {
			char oppp = str[pos + 2];
			if (oppp == '=') end = pos + 2;// <<=
			else end = pos + 1;// <<
		}
		else end = pos;// <
	}
	else if (op == '&') {
		char opp = str[pos + 1];
		if (opp == '&') end = pos + 1;//&&
		else if (opp == '=')end = pos + 1;//&=
		else end = pos;// &
	}
	else if (op == '|') {
		char opp = str[pos + 1];
		if (opp == '|')end = pos + 1;// ||
		else if (opp == '=')end = pos + 1;// |=
		else end = pos;// |
	}
	else if (op == '*' || op == '/' || op == '%' || op == '=' || op == '!' || op == '^') {
		char opp = str[pos + 1];
		if (opp == '=') end = pos + 1;// *= /= %= == != ^=
		else end = pos;//* / % = ! ^
	}
	else if (op == ',' || op == '?' || op == ':' || op == '.' || op == '~') {
		end = pos;
	}

	else
		return WRONG;

	col += end - start + 1;
	return RIGHT;
}

/*************************************************
类别：词法分析
*************************************************/
Lexical::Lexical()
{
	myDFA[ENTRANCE_KEYWORDS] = new KeyWordsDFA;
	myDFA[ENTRANCE_IDENTIFIER] = new IdentifierDFA;
	myDFA[ENTRANCE_OPERATOR] = new OperatorDFA;
	myDFA[ENTRANCE_DELIMITER] = new DelimiterDFA;
	myDFA[ENTRANCE_DIGIT] = new DigitDFA;
}
Lexical::~Lexical()
{
	for (int i = 0; i < 5; i++) {
		if (myDFA[i] != nullptr) {
			delete myDFA[i];
			myDFA[i] = nullptr;
		}
	}
}
std::pair<int, std::vector<token>> Lexical::Analyze(std::string& input) {
	// 变量定义
	std::string output = "";
	int row = 0, col = 0;
	int SIGN = LEXICAL_RIGHT;
	int start = 0, end = -1;
	int cur_row = 1, cur_col = 0;
	int last_row = cur_row, last_col = cur_col;
	std::stringstream final_str;

	// token信息
	std::vector<token> tk_info;

	// 预处理
	int pre_res = Preprocess(input, row, col, output);
	if (pre_res != RIGHT) {
		std::ofstream outfile(LC_Wrong_Path);
		if (!outfile.is_open()) {
			std::cout << LC_Wrong_Path << "打开失败";
			return { -1 ,tk_info };
		}
		outfile << row << ' ' << col << "preprocess fail!\n";
		outfile.close();
		return { ERROR_Pre, tk_info };
	}

	// 分析
	while (1) {
		start = end + 1;
		if (start >= output.size())
			break;
		//预处理输出output时，还留下空格和换行，在处理下一个token前，先去掉
		int pos = start;
		while (output[pos] == ' ' || output[pos] == '\r' || output[pos] == '\n') {
			if (output[pos] == ' ' || output[pos] == '\r') {
				pos++;
				cur_col++;
			}
			else {
				pos++;
				cur_col = 0;
				cur_row++;
			}
			if (pos >= output.size())
				break;
		}
		start = pos;
		end = pos;
		//前面没错，以空格或换行结尾，说明语法分析正确
		if (start >= output.size()) {
			SIGN = LEXICAL_RIGHT;
			break;
		}

		//正式判断当前有效字符的DFA入口
		int Entrance_DFA = -1;
		char ch = output[start];
		if (isOperator(ch)) Entrance_DFA = ENTRANCE_OPERATOR;
		else if (isDelimiter(ch)) Entrance_DFA = ENTRANCE_DELIMITER;
		else if (isDigit(ch)) Entrance_DFA = ENTRANCE_DIGIT;
		//关键字和标识符的DFA识别在这已经做了
		if (Entrance_DFA == -1) {
			//一定要先判断是不是关键字，再去判断是不是标识符
			int result = myDFA[ENTRANCE_KEYWORDS]->isAccepted(output, start, end, cur_row, cur_col);
			if (result == WRONG) {
				//不是关键字
				result = myDFA[ENTRANCE_IDENTIFIER]->isAccepted(output, start, end, cur_row, cur_col);
				if (result == RIGHT)//是标识符
					Entrance_DFA = ENTRANCE_IDENTIFIER;
			}
			else//是关键字
				Entrance_DFA = ENTRANCE_KEYWORDS;

			//假如说Entrance_DFA还是为-1
			if (Entrance_DFA == -1) {
				//todo,出错识别
				SIGN = error_sign(-1);
			}
		}
		//这里做算符、界符、数字的DFA识别
		else {
			int result = myDFA[Entrance_DFA]->isAccepted(output, start, end, cur_row, cur_col);
			if (result == WRONG) {
				//todo,出错识别
				SIGN = error_sign(Entrance_DFA);
			}
		}
		//判断本次循环是否到结束
		if (SIGN != LEXICAL_RIGHT)//在识别token的过程中出错
			break;
		//本次token识别成功
		else {
			std::string token_tmp = output.substr(start, end - start + 1);
			std::string token_type;
			create_token_line(Entrance_DFA, token_tmp, token_type);
			final_str << cur_row << ' ' << cur_col << ' ' << token_type << ' ' << token_tmp << "\n";
			tk_info.push_back(token(cur_row, cur_col, token_type, token_tmp));
			last_row = cur_row;
			last_col = cur_col;
		}
	}

	//文本识别完毕
	//词法分析正确，输出到文件
	if (SIGN == LEXICAL_RIGHT) {
		std::ofstream outfile(LC_Rignt_Path);
		if (!outfile.is_open()) {
			std::cout << LC_Wrong_Path << "打开失败";
			return { -1,tk_info };
		}
		outfile << final_str.str();
		outfile << last_row << ' ' << 1 << ' ' << "#" << ' ' << "#" << "\n";
		tk_info.push_back(token(last_row, 1, "#", "#"));
		outfile.close();
	}
	//词法分析错误
	else {
		std::ofstream outfile(LC_Wrong_Path);
		if (!outfile.is_open()) {
			std::cout << LC_Wrong_Path << "打开失败";
			return { -1,tk_info };
		}
		std::string token = output.substr(start, end - start + 1);
		std::string token_type;
		create_sign_error_line(SIGN, token, token_type);
		outfile << last_row << ' ' << last_col + 1 << ' ' << token_type << ' ' << token << "\n";
		outfile.close();
	}
	return { SIGN,tk_info };
}
