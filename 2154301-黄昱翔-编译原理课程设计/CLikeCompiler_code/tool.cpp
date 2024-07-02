#include "tool.h"

bool isLetter(char ch)
{
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
		return true;
	else
		return false;
}

bool isDigit(char ch)
{
	if (ch >= '0' && ch <= '9')
		return true;
	else
		return false;
}

bool isOperator(char ch)
{
	for (int i = 0; i < Operator_NUM; i++) {
		if (ch == Operator[i][0])
			return true;
	}
	return false;
}

bool isDelimiter(char ch)
{
	for (int i = 0; i < Delimiter_NUM; i++) {
		if (ch == Delimiter[i][0])
			return true;
	}
	return false;
}

int error_sign(int entrance)
{
	if (entrance == ENTRANCE_KEYWORDS) return ERROR_KEYWORDS;
	else if (entrance == ENTRANCE_IDENTIFIER) return ERROR_IDENTIFIER;
	else if (entrance == ENTRANCE_OPERATOR) return ERROR_OPERATOR;
	else if (entrance == ENTRANCE_DELIMITER) return ERROR_DELIMITER;
	else if (entrance == ENTRANCE_DIGIT) return ERROR_DIGIT;
	else if (entrance == -1) return ERROR_NODFA;
	return ERROR_NODFA;// for completment
}

void create_token_line(int entrance_dfa, std::string token, std::string& token_type)
{
	if (entrance_dfa == ENTRANCE_KEYWORDS) token_type = token;
	else if (entrance_dfa == ENTRANCE_IDENTIFIER) token_type = "identifier";
	else if (entrance_dfa == ENTRANCE_OPERATOR) token_type = token;
	else if (entrance_dfa == ENTRANCE_DELIMITER) token_type = token;
	else if (entrance_dfa == ENTRANCE_DIGIT) token_type = "digit";
}

void create_sign_error_line(int sign, std::string token, std::string& token_type)
{
	if (sign == ERROR_NODFA) token_type = "dont know";
	else if (sign == ERROR_KEYWORDS) token_type = token;
	else if (sign == ERROR_IDENTIFIER) token_type = "identifier";
	else if (sign == ERROR_OPERATOR) token_type = token;
	else if (sign == ERROR_DELIMITER) token_type = token;
	else if (sign == ERROR_DIGIT) token_type = "digit";
}

int Preprocess(std::string& input, int& row, int& col, std::string& output)
{
	int pos = 0;
	int cur_row = row, cur_col = col;
	while (pos < input.size()) {
		char ch = input[pos];
		//去注释
		if (ch == '/') {
			//先往后移一步
			if (++pos >= input.size()) {
				row = cur_row;
				col = cur_col;
				return WRONG;
			}
			char ch1 = input[pos];
			//单行注释
			if (ch1 == '/') {
				while (input[++pos] != '\n') cur_col++;//读到末尾
				output += '\n';//留下一个换行符
				cur_row++;
				cur_col = 0;
			}
			//多行注释
			else if (ch1 == '*') {// /*
				if (++pos >= input.size()) {
					row = cur_row;
					col = cur_col;
					return WRONG;
				}
				char ch2 = input[pos];
				while (1) {
					while (ch2 != '*') {
						if (ch2 == '\n') {
							cur_row++;
							cur_col = 0;
							output += '\n';
						}
						if (++pos >= input.size()) {
							row = cur_row;
							col = cur_col;
							return WRONG;
						}
						ch2 = input[pos];
						cur_col++;
					}
					if (++pos >= input.size()) {
						row = cur_row;
						col = cur_col;
						return WRONG;
					}
					ch2 = input[pos];
					cur_col++;
					if (ch2 == '/')
						break;
					else if (ch2 == '\n') {
						cur_row++;
						cur_col = 0;
						output += '\n';
					}
				}
			}
		}
		//去tab
		else if (ch == '\t') {
			output += ' ';
			cur_col++;
		}
		//去换行
		else if (ch == '\n') {
			output += '\n';
			cur_row++;
			cur_col = 0;
		}
		//读取内容
		else {
			output += ch;
			cur_col++;
		}
		pos++;
	}
	row = cur_row;
	col = cur_col;
	return RIGHT;
}

std::string file_to_str(std::string path)
{
	std::ifstream file(path);
	std::string input;
	if (file) {
		std::stringstream buffer;
		buffer << file.rdbuf();
		input = buffer.str();
	}
	else
		std::cout << path << "打开错误\n";
	return input;
}

/* 以下是语法分析的部分 */
/* BEGIN - class token - 函数实现 */
token::token() {
	this->token_x = -1, this->token_y = -1;
	this->token_type = "";
	this->token_value = "";
}

token::~token()
{
}

token::token(int tx, int ty, std::string type, std::string value) {
	this->token_x = tx, this->token_y = ty;
	this->token_type = type;
	this->token_value = value;
}
/* END - class token - 函数实现 */

/* BEGIN - class production - 函数实现 */
production::production() {
	this->id = -1;
	this->type = 0;
	this->dot_pos = -1;
}

production::production(int _id, int _type, int _dot_pos, std::string _left, std::vector<std::string> _right)
{
	this->id = _id;
	this->type = _type;
	this->dot_pos = _dot_pos;
	this->left = _left;
	this->right = _right;
}

production::~production() {

}
/* END - class production - 函数实现 */

// 工具函数
bool is_string_in_vector(std::vector<std::string> string_set, std::string s)
{
	for (int i = 0; i < string_set.size(); ++i) {
		if (string_set[i] == s)
			return true;
	}
	return false;
}

/* BEGIN - class item - 函数实现 */
item::item()
{
}

item::~item()
{
}

item::item(production p, int _dot_pos)
{
	this->prod = p;
	this->prod.dot_pos = _dot_pos;
}
/* END - class item - 函数实现 */

/* BEGIN - class itemGroup - 函数实现 */
itemGroup::itemGroup()
{
	this->id = -1;
}

itemGroup::~itemGroup()
{
}

itemGroup::itemGroup(int _id)
{
	this->id = _id;
}

// 合并项目集
bool itemGroup::itemGroupUnion(const itemGroup& otherIG)
{
	auto tmpIGsize = (*this).item_group.size();
	for (auto it = otherIG.item_group.begin(); it != otherIG.item_group.end(); ++it) {
		this->item_group.insert(*it); // set会自动去重
	}
	return this->item_group.size() == tmpIGsize; // 合并后是否有新的元素加入
}

/* END - class itemGroup - 函数实现 */

/* BEGIN - class treenode - 函数实现 */
treenode::treenode()
{
	this->type = "";
	this->value = "";
	this->child.clear();
}

treenode::~treenode()
{
	if (this->child.size()) {
		for (int i = 0; i < this->child.size(); ++i) {
			delete this->child[i];
		}
	}
}

bool treenode::clear()
{
	this->type = "";
	this->value = "";
	this->child.clear();
	return 1;
}
/* END - class treenode -函数实现 */

