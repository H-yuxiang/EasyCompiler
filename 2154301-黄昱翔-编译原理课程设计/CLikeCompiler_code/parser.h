#pragma once
#include "common.h"

/*
	类：语法分析 - Parser
*/
class Parser
{
public:
	Parser();
	~Parser();

	/*
		功能1：加载变元映射表
		return:	map<string, string>
	*/
	void load_argument_def();

	/*
		功能2：加载初始文法（已扩展）
		return:	vector<production>
	*/
	void load_production();
	void show_production();

	/*
		功能3：获取单个变元的FIRST集
		return: map<string, vector<string>>
	*/
	std::vector<std::string> get_single_first(std::string argument);

	/*
		功能4：获取所有变元的FIRST集
		return: map<string, vector<string>>
	*/
	void get_first();
	void show_first();

	/*
		功能5：获取一个项目的闭包
		return: 该项目的闭包构成的项目集
	*/
	itemGroup get_single_closer(item im);
	void show_closure();

	/*
		功能6：获取 get_closer 的输入 - get_item_group seris 1
		input: 一个项目集
		return: 一个<待生成闭包的项目>的集合
	*/
	std::vector<itemGroup> get_GOTO_input(const itemGroup& in_ig);
	void show_GOTO_input(std::vector<itemGroup> ig_v);

	/*
		功能7：获取项目集的 closer - get_item_group seris 2
		return: I_k - 一个属于规范族的项目集
	*/
	itemGroup get_closer(const itemGroup& in_ig);

	/*
		功能8：获取项目集规范族，结果存储在 std::set<itemGroup> Item_group_set;
	*/
	void get_item_group();
	void show_item_group();
	void show_item_group_shift_mp();

	/*
		功能9：获取LR1分析表，结果存储在 LR1_table
	*/
	void get_LR1_table();
	void show_LR1_table();

	/*
		功能10：使用LR1分析表进行分析
		return: 分析是否成功
	*/
	bool Analyse(std::vector<token> tk);

	/*
		功能11：遍历输出语法树
		return:
		注意：当前实现是dfs，不方便直接输出到文件
	*/
	void show_tree(treenode* root, int depth = 0);

	/*
		功能12：获取语法树
		return: this->root
	*/
	treenode* get_tree() { std::cout << "获取语法树!\n"; return this->gra_root; }

	/*
		功能13：辅助判断是否是终结符
		return: bool
	*/
	bool isTerminal(std::string s) {
		return (!Argument_type.count(s) || Argument_type[s] == TERMINAL);
	}

	/*
		功能14：四元式
		return:
	*/
	void emit(int is_cmd_out = 1);
	std::pair<int, Quadruple> gen(std::string _op, std::string _arg1, std::string _arg2, std::string _result);

	/*
		功能15：产生一个新的临时变量
		return: std::string
	*/
	std::string newtmp() {
		return "T" + std::to_string(this->tmp_number++);
	}

	/*
		功能16：寻找变量是否出现过（todo）
		return: bool
	*/
	bool lookup(std::string s) {
		for (int i = 0; i < this->tableSet.size(); ++i) {
			for (int j = 0; j < this->tableSet[i].size(); ++j) {
				if (this->tableSet[i][j] == s)return true;
			}
		}
		return false;
	}

	/*
		功能17：扩充符号表
		return:
	*/
	void mk_table() {
		std::vector<std::string> res(1, "");
		this->tableSet.push_back(res);
	}

	/*
		功能18：添加新变量
		return:
	*/
	void add_argument(std::string s) {
		this->tableSet.back().push_back(s);
	}

	/*
		功能19：删减符号表
		return:
	*/
	void rm_table() {
		if (this->tableSet.size())
			this->tableSet.pop_back();
	}

	/*
		功能20：dfs
		return: dfs的状态
	*/
	int dfs(treenode* root);

	/*
		功能21：semanticAnalyse
		return:
	*/
	bool semanticAnalyse();
	std::vector<std::pair<int, Quadruple>> get_Q_ruple() { return this->Q_ruple; }

	/*
		功能22：get_formpara
		该函数受语法规则制约
		获取某个函数的形参列表
	*/
	std::vector<std::string> get_formpara(treenode* root);

	/*
		功能23：fixPara
		修改某个函数的形参列表和其实现过程中的形参
	*/
	bool fixPara(treenode* root, std::vector<std::string> real_para, std::vector<std::string> form_para);

	/*
		功能24：isArrayExist
		查看数组声明是否重名
	*/
	bool isArrayExist(std::string s);

	// 工具函数1 - 声明时，数组合理性检查 合理返回1
	bool arr_check(std::vector<int> arr_sz) {
		long long total = 1;
		for (int i = 0; i < arr_sz.size(); ++i) {
			if (arr_sz[i] >= ARRAY_SIZE_MAX)return false;
			total *= arr_sz[i];
			if (total >= ARRAY_SIZE_MAX)return false;
		}
		return true;
	}

	// 工具函数2 - 访问时，数组索引位置计算
	// return	-1 - 维数不一致
	//			-2 - 某一维越界
	int get_arr_idx_pos(arrayInfo inf, std::vector<int> arr_idx) {
		if (inf.arr_sz.size() != arr_idx.size()) 
		{
			// std::cout << inf.arr_sz.size() << "-" << arr_idx.size() << std::endl;
			return -1;// 尺寸错误
		}
		int res = 0, total = 1;
		// 获取子立方体大小
		std::vector<int> blk_sz;
		for (int i = 0; i < inf.dim; ++i) total *= inf.arr_sz[i];
		for (int i = 0; i < inf.dim - 1; ++i) {
			total /= inf.arr_sz[i];
			blk_sz.push_back(total);
		}
		blk_sz.push_back(1);
		// 映射
		for (int i = 0; i < inf.dim; ++i) {
			if (arr_idx[i] < 0 || arr_idx[i] >= inf.arr_sz[i])return -2;// 索引越界
			res += arr_idx[i] * blk_sz[i];
		}
		return res;
	}

	// 工具函数3 - 声明时，获取数组的维数、每一维的大小
	std::vector<int> get_arr_sz(treenode* root) {
		std::vector<int> res;
		if (root->value != "array_declaration")// 调试
		{
			std::cout << "非预期声明数组的节点!" << std::endl;
			return res;
		}
		treenode* p = root;
		while (p->value != ";") {
			if (p->child[0]->value != "[")// 根
			{
				res.push_back(std::stoi(p->child[2]->value));
			}
			else {
				res.push_back(std::stoi(p->child[1]->value));
			}
			p = p->child.back();
		}
		return res;
	}

	// 工具函数4 - 访问时，获取数组每一维的索引
	std::vector<int> get_arr_idx(treenode* root) {
		std::vector<int> res;
		if (root->value != "array")// 调试
		{
			std::cout << "非预期访问数组的节点!" << std::endl;
			return res;
		}
		treenode* p = root;
		while (p->value != "]") {
			if (p->child[0]->value != "[")// 根
			{
				res.push_back(std::stoi(p->child[2]->value));
			}
			else {
				res.push_back(std::stoi(p->child[1]->value));
			}
			p = p->child.back();
		}
		return res;
	}

private:
	// 1.变元映射表（仅名称，一一对应）
	std::map<std::string, std::string> Grammar_symbol_mp;
	// 2."变元-文法"映射表 - 该变元为左部的所有产生式索引，在 load_production 加载
	//	  				   - 索引的是 Grammar_symbol_mp[...] 映射后的名称
	std::map<std::string, std::vector<int>> Argument_garmmar_mp;
	// 3.变元类型索引
	// Tips：判断 X 是否是终结符 -> !Argument_type.count(X) || Argument_type[X] == TERMINAL
	std::unordered_map<std::string, int> Argument_type;
	// 4.初始文法（已扩展）
	std::vector<production> Production_raw;
	// 5.所有变元的FIRST集
	std::map<std::string, std::vector<std::string>> First_set;

	// 项目集族
	// 1.项目集个数
	int Item_group_num;
	// 2.[I_i-(s)->I_j] - 状态I_i点后为s，转移到状态I_j - 索引表
	std::vector<std::map<std::string, int>> Item_group_shift_mp;
	// 3.项目集族set
	std::set<itemGroup> Item_group_set;
	// 4.产生式规约ACTION构造的依据（点在产生式右部的最后） - 行数(size)=产生式的个数
	// 记录的格式：Prod_acc_mp[j]map<lf,vector<I_i>> -> ACTION[i,a]=j
	std::vector<std::map<std::string, std::vector<int>>> Prod_acc_mp;

	// LR(1)分析表
	// ACTION和GOTO通过规则区分
	// 规则：pair<s(移进)/r(规约)/a(accept)/g(goto), int(next state)>
	std::vector< std::map<std::string, std::pair<char, int>> > LR1_table;

	// 语法树
	treenode* gra_root;
	// 符号表
	std::vector<std::vector<std::string>> tableSet;
	// 函数信息
	std::vector<funcInfo> func;
	std::vector<treenode*> funcptr;
	// 临时变量编号
	int tmp_number;
	// 四元式标号
	int nxtquad;
	// 记录四元式序列
	std::vector<std::pair<int, Quadruple>> Q_ruple;

	// 已声明的数组信息
	std::vector<arrayInfo> arrayinfo;
};

