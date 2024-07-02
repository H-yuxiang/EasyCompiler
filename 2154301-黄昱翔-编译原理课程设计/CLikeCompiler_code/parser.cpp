#include "tool.h"
#include "parser.h"

/* BEGIN - class Parser - 函数实现 */
// 构造
Parser::Parser()
{
	this->tmp_number = 0;
	this->nxtquad = QUAD_SATRT;
	this->Item_group_num = 0;
	this->gra_root = NULL;
}

// 析构
Parser::~Parser()
{
}

// 变元映射表
void Parser::load_argument_def()
{
	for (int i = 0; i < Grammar_symbol_NUM; i++) {
		Grammar_symbol_mp[Grammar_symbol_name[i]] = Grammar_symbol_def[i];
		if (Grammar_symbol_name[i] != "ID" && Grammar_symbol_name[i] != "num" && Grammar_symbol_name[i] != "epsilon")
			this->Argument_type[Grammar_symbol_def[i]] = ARGUMENT;
		else
			this->Argument_type[Grammar_symbol_def[i]] = TERMINAL;
	}
}

// 初始文法
void Parser::load_production()
{
	// 扩展
	Production_raw.push_back(production(0, ARGUMENT, -1, (std::string)("Start"), std::vector<std::string>{Grammar_symbol_mp[Grammar_rule[0][0]]}));
	this->Argument_garmmar_mp[(std::string)("Start")].push_back((int)Production_raw.size() - 1);
	// 其他
	for (int i = 0; i < Grammar_rule_NUM; ++i) {
		production prod_tmp;
		prod_tmp.id = i + 1;
		if (Grammar_rule[i][0] == "ID" || Grammar_rule[i][0] == "num" || Grammar_rule[i][0] == "epsilon")
			prod_tmp.type = TERMINAL;
		else
			prod_tmp.type = ARGUMENT;
		prod_tmp.left = Grammar_symbol_mp[Grammar_rule[i][0]];
		for (int j = 1; j < Grammar_rule[i].size(); ++j) {
			if (Grammar_symbol_mp.count(Grammar_rule[i][j]))
				prod_tmp.right.push_back(Grammar_symbol_mp[Grammar_rule[i][j]]);
			else
				prod_tmp.right.push_back(Grammar_rule[i][j]);
		}
		Production_raw.push_back(prod_tmp);
		if (prod_tmp.type == ARGUMENT)
			Argument_garmmar_mp[prod_tmp.left].push_back((int)Production_raw.size() - 1);
	}
	// for LR1_table
	Prod_acc_mp.resize(Production_raw.size());
}

// 打印初始文法（调试）
void Parser::show_production()
{
	std::ofstream outfile;
	outfile.open(PS_Production_Path, std::ios::out | std::ios::binary);
	for (int i = 0; i < Production_raw.size(); ++i) {
		outfile << i << ":" << Production_raw[i].left << "->";
		for (int j = 0; j < Production_raw[i].right.size(); ++j) {
			outfile << Production_raw[i].right[j];
		}
		outfile << "\n";
	}
	outfile.close();
}

// 获取单个变元的FIRST集
std::vector<std::string> Parser::get_single_first(std::string argument)
{
	std::vector<std::string> ret_first_set;
	// 遍历所有 argument 为 left 的产生式
	for (int idx = 0; idx < Argument_garmmar_mp[argument].size(); ++idx) {
		int i = Argument_garmmar_mp[argument][idx]; // 该变元的所有产生式索引
		// Production_raw[i].right[0] 本身是终结符
		if (!Argument_type.count(Production_raw[i].right[0]) || Production_raw[i].right[0] == Grammar_symbol_mp["ID"]
			|| Production_raw[i].right[0] == Grammar_symbol_mp["num"] || Production_raw[i].right[0] == Grammar_symbol_mp["epsilon"]) {
			if (!is_string_in_vector(ret_first_set, Production_raw[i].right[0]))
				ret_first_set.push_back(Production_raw[i].right[0]);
		}
		// Production_raw[i].right[0] 本身是变元
		else {
			// 向后看，直到【1.某个终结符】或【2.某个变元的FIRST不含epsilon】
			for (int j = 0; j < Production_raw[i].right.size(); ++j) {
				// 如果是终结符
				if (!Argument_type.count(Production_raw[i].right[j]) || Production_raw[i].right[j] == Grammar_symbol_mp["ID"]
					|| Production_raw[i].right[j] == Grammar_symbol_mp["num"] || Production_raw[i].right[j] == Grammar_symbol_mp["epsilon"]) {
					if (!is_string_in_vector(ret_first_set, Production_raw[i].right[j]))
						ret_first_set.push_back(Production_raw[i].right[j]);
					break; //1.
				}
				// 如果是变元
				else {
					std::vector<std::string> tmp_first_set;
					// 该变元已经求过FIRST
					if (First_set.count(Production_raw[i].right[j]))
						tmp_first_set = First_set[Production_raw[i].right[j]];
					else {
						// 递归
						tmp_first_set = get_single_first(Production_raw[i].right[j]);
						First_set[Production_raw[i].right[j]] = tmp_first_set;
					}
					// 补充FIRST
					for (int k = 0; k < tmp_first_set.size(); ++k) {
						if (!is_string_in_vector(ret_first_set, tmp_first_set[k]))
							ret_first_set.push_back(tmp_first_set[k]);
					}
					if (!is_string_in_vector(tmp_first_set, Grammar_symbol_mp["epsilon"]))
						break; //2.
				}
			}
		}
	}
	return ret_first_set;
}

// 获取所有变元的FIRST集
void Parser::get_first()
{
	for (int i = 0; i < this->Production_raw.size(); ++i) {
		// 产生式 raw 的 left 必然是变元
		if (First_set.count(Production_raw[i].left))
			continue;// 已经求过
		First_set[Production_raw[i].left] = get_single_first(Production_raw[i].left);
	}
}

// 打印所有变元的FIRST集
void Parser::show_first()
{
	for (auto it = First_set.begin(); it != First_set.end(); ++it) {
		std::cout << (*it).first << "{";
		for (int i = 0; i < (*it).second.size(); ++i) {
			std::cout << (*it).second[i] << ",";
		}
		std::cout << "}\n";
	}
}

// 获取一个项目的闭包（产生的 item 中 dot的位置都是在 right 的开头）
itemGroup Parser::get_single_closer(item im)
{
	itemGroup ret_ig;
	// 点的位置在末尾
	if (im.prod.dot_pos >= im.prod.right.size()) {
		ret_ig.item_group.insert(im);
		return ret_ig;
	}
	std::queue<item> qwait; // 等待队列
	qwait.push(im);
	while (qwait.size()) {
		item im_tmp = qwait.front();
		// 添加到 ret_ig
		ret_ig.item_group.insert(im_tmp);
		qwait.pop();
		production prod_tmp = im_tmp.prod;
		std::string lookforward_tmp = im_tmp.lookforward;
		// 情形1：点在产生式的末尾，则后续不入队
		if (prod_tmp.dot_pos >= prod_tmp.right.size()) {
			continue;
		}
		// 情形2：点后为终结符，则后续不入队
		if (!Argument_type.count(prod_tmp.right[prod_tmp.dot_pos]) || Argument_type[prod_tmp.right[prod_tmp.dot_pos]] == TERMINAL) {
			continue;
		}
		// 情形3：点后为变元
		// 需要点后移后的"剩余序列first集"
		std::vector<std::string> seqence_first;
		// 一般
		for (int j = prod_tmp.dot_pos + 1; j < prod_tmp.right.size(); ++j) {
			// 非终结符
			if (!Argument_type.count(prod_tmp.right[j]) || Argument_type[prod_tmp.right[j]] == TERMINAL) {
				seqence_first.push_back(prod_tmp.right[j]);
				break;
			}
			// 终结符
			else {
				for (int k = 0; k < First_set[prod_tmp.right[j]].size(); ++k) {
					if (!is_string_in_vector(seqence_first, First_set[prod_tmp.right[j]][k])) {
						seqence_first.push_back(First_set[prod_tmp.right[j]][k]);
					}
				}
				// 当前变元first集不含epsilon
				if (!is_string_in_vector(First_set[prod_tmp.right[j]], this->Grammar_symbol_mp["epsilon"]))
					break;
			}
			// lf
			if (j == prod_tmp.right.size() - 1) {
				seqence_first.push_back(lookforward_tmp);
			}
		}
		// 特判
		if (seqence_first.size() == 0)
			seqence_first.push_back(lookforward_tmp);
		// 获取该变元的产生式索引
		std::vector<int> argument_production_idx = this->Argument_garmmar_mp[prod_tmp.right[prod_tmp.dot_pos]];
		for (int i = 0; i < argument_production_idx.size(); ++i) {
			production prod_new = this->Production_raw[argument_production_idx[i]];
			item im_new = item(prod_new, 0);
			// qwait.push
			for (int j = 0; j < seqence_first.size(); ++j) {
				im_new.lookforward = seqence_first[j];
				qwait.push(im_new);
			}
		}// 遍历产生式
	}// end of while(q)
	return ret_ig;
}

// 打印"Start"的闭包（调试）
void Parser::show_closure() {
	production toprint = Production_raw[0];
	item im;
	im.prod = toprint;
	im.prod.dot_pos = 0;
	im.lookforward = "#";
	// itemGroup ret_ig = get_single_closer(im);
	itemGroup ig_tmp;
	ig_tmp.id = -1;
	ig_tmp.item_group.insert(im);
	itemGroup ret_ig = get_closer(ig_tmp);
	for (auto it = ret_ig.item_group.begin(); it != ret_ig.item_group.end(); ++it) {
		// (*it) - item
		// item - production + lookforward
		// production - left + dot + right
		std::cout << (*it).prod.left << "->";
		for (int i = 0; i < (*it).prod.right.size(); ++i) {
			if (i == (*it).prod.dot_pos) std::cout << "·";
			std::cout << (*it).prod.right[i];
		}
		if ((*it).prod.dot_pos == (*it).prod.right.size())
			std::cout << "·";
		std::cout << "," << (*it).lookforward << "\n";
	}
}

// 获取 get_closer 的输入项目集（根据·后不同的字符会产生多个itemGroup）
// 输入：I_i
std::vector<itemGroup> Parser::get_GOTO_input(const itemGroup& in_ig)
{
	std::vector<itemGroup> ret_igv;
	std::map<std::string, int> Item_idx;
	// 根据点后的字符分类获取转移索引
	for (auto it = in_ig.item_group.begin(); it != in_ig.item_group.end(); ++it) {
		item im = (*it);
		// 点在末尾（添加一张表记录[I_i,s,j]，创建LR1分析表时有用）
		// Prod_acc_mp[j]map<lf,vector<I_i>> -> ACTION[i,lf]=j
		if (im.prod.dot_pos >= im.prod.right.size()) {
			// for ACTION case3
			// A!=S'
			if (im.prod.left == "Start")
				continue;
			// lf.type == TERMINAL
			if (!Argument_type.count(im.lookforward) || Argument_type[im.lookforward] == TERMINAL) {
				this->Prod_acc_mp[im.prod.id][im.lookforward].push_back(in_ig.id);
			}
			continue;
		}
		// 获取点后字符
		std::string nxt = im.prod.right[im.prod.dot_pos];
		// 如果是空产生式，忽略
		if (nxt == this->Grammar_symbol_mp["epsilon"])
			continue;
		// 全局唯一挪"点位置"的语句
		im.prod.dot_pos++;
		// 已有，转移方向1
		if (Item_idx.count(nxt)) {
			int idx = Item_idx[nxt];
			ret_igv[idx].item_group.insert(im);
		}
		// 还未有，转移方向2
		else {
			Item_idx[nxt] = (int)ret_igv.size();
			itemGroup ig_new;
			ig_new.id = in_ig.id;
			ig_new.item_group.insert(im);
			ret_igv.push_back(ig_new);
		}
	}
	// debug
	//show_GOTO_input(ret_igv);
	// debug
	return ret_igv;
}

// 打印 get_closer 的输入项目集
void Parser::show_GOTO_input(std::vector<itemGroup> ig_v)
{
	for (int i = 0; i < ig_v.size(); ++i) {
		std::cout << i << std::endl;
		for (auto it = ig_v[i].item_group.begin(); it != ig_v[i].item_group.end(); ++it) {
			std::cout << (*it).prod.left << "->";
			for (int i = 0; i < (*it).prod.right.size(); ++i) {
				if (i == (*it).prod.dot_pos) std::cout << "·";
				std::cout << (*it).prod.right[i];
			}
			if ((*it).prod.dot_pos == (*it).prod.right.size())
				std::cout << "·";
			std::cout << "," << (*it).lookforward << "\n";
		}
	}
}

// 获取项目集的 closer
itemGroup Parser::get_closer(const itemGroup& in_ig)
{
	itemGroup ret_ig;
	ret_ig.id = in_ig.id;
	for (auto it = in_ig.item_group.begin(); it != in_ig.item_group.end(); ++it) {
		itemGroup ig_tmp = get_single_closer((*it));
		ret_ig.itemGroupUnion(ig_tmp);
	}
	return ret_ig;
}

// 获取所有的项目集 - 项目集族
void Parser::get_item_group()
{
	// I_i --s--> I_j	vector<map<string,int>> ig_shift_mp; // 状态 I_i 点后为 s 转移到状态 I_j
	// Start
	production p_start = Production_raw[0];
	item im_start;
	im_start.prod = p_start;
	im_start.prod.dot_pos = 0;
	im_start.lookforward = "#";
	itemGroup ig_start;
	ig_start.id = -1;
	ig_start.item_group.insert(im_start);
	// qwait
	std::queue<itemGroup> qwait;
	qwait.push(ig_start);
	// while until 项目集族不再扩大
	while (qwait.size()) {
		itemGroup ig_tmp = qwait.front();
		qwait.pop();
		// 获取闭包I_i
		itemGroup ret_ig = get_closer(ig_tmp);
		// 1.如果是空包
		if (ret_ig.item_group.size() == 0)
			continue;
		// 2.如果该闭包已经存在
		if (this->Item_group_set.count(ret_ig)) {
			auto ig_exist = this->Item_group_set.find(ret_ig);
			auto it = ig_tmp.item_group.begin();
			// for ACTION case1/2
			Item_group_shift_mp[ig_tmp.id][(*it).prod.right[(*it).prod.dot_pos - 1]] = (*ig_exist).id;
			continue;
		}
		// 3.如果该闭包不存在
		else {
			ret_ig.id = (this->Item_group_num++);
			this->Item_group_set.insert(ret_ig);
			this->Item_group_shift_mp.resize(this->Item_group_num);
			// for ACTION case1/2
			if (ig_tmp.id >= 0) {
				auto it = ig_tmp.item_group.begin();
				Item_group_shift_mp[ig_tmp.id][(*it).prod.right[(*it).prod.dot_pos - 1]] = ret_ig.id;
			}
		}
		// 获取新闭包的vector<转移输入集>
		std::vector<itemGroup> goto_input = get_GOTO_input(ret_ig);
		// 如果产生式的点均已到结尾
		if (goto_input.size() == 0)
			continue;
		// 尚有待转移的
		for (int i = 0; i < goto_input.size(); ++i) {
			qwait.push(goto_input[i]);
		}
	}
}

// 打印项目集族
void Parser::show_item_group()
{
	std::ofstream outfile;
	outfile.open(PS_ItemGroup_Path, std::ios::out | std::ios::binary);
	for (auto it1 = this->Item_group_set.begin(); it1 != this->Item_group_set.end(); ++it1) {
		//if ((*it1).id != 80 && (*it1).id != 44)continue;
		outfile << "I_" << (*it1).id << "：\n";
		itemGroup ig_tmp = (*it1);
		for (auto it2 = ig_tmp.item_group.begin(); it2 != ig_tmp.item_group.end(); ++it2) {
			outfile << (*it2).prod.left << "->";
			for (int i = 0; i < (*it2).prod.right.size(); ++i) {
				if (i == (*it2).prod.dot_pos) outfile << "·";
				outfile << (*it2).prod.right[i];
			}
			if ((*it2).prod.dot_pos == (*it2).prod.right.size())
				outfile << "·";
			outfile << ",lf=" << (*it2).lookforward << "\n";
		}
	}
	outfile.close();
}

// 打印DFA状态转移表
void Parser::show_item_group_shift_mp()
{
	// std::vector<std::map<std::string, int>> Item_group_shift_mp;
	for (int i = 0; i < this->Item_group_shift_mp.size(); ++i) {
		std::cout << "State" << i << "：\n";
		if (this->Item_group_shift_mp[i].size() == 0) {
			std::cout << "Accepted State.\n";
			continue;
		}
		for (auto it = Item_group_shift_mp[i].begin(); it != Item_group_shift_mp[i].end(); ++it) {
			std::cout << (*it).first << "--" << (*it).second << "\n";
		}
	}
}

// 获取LR1分析表
void Parser::get_LR1_table()
{
	// 通过 Item_group_shift_mp 获取 LR1 分析表
	this->LR1_table.resize(this->Item_group_shift_mp.size());
	// 分4种情况
	// 1. [A->..·a..,b] && goto(I_i,a)=I_j -> ACTION[i,a]=sj
	// 2. [A->..·B..,b] && goto(I_i,B)=I_j -> GOTO[i,B]=gj
	for (int i = 0; i < this->Item_group_shift_mp.size(); ++i) {
		if (this->Item_group_shift_mp[i].size() == 0) {
			continue;
		}
		for (auto it = Item_group_shift_mp[i].begin(); it != Item_group_shift_mp[i].end(); ++it) {
			if (!Argument_type.count((*it).first) || Argument_type[(*it).first] == TERMINAL)
				this->LR1_table[i][(*it).first] = { 's',(*it).second };
			else
				this->LR1_table[i][(*it).first] = { 'g',(*it).second };
		}
	}
	// 3, [A->..·,a] && A!="Start" - ACTION[i,a]=rj
	// 格式：Prod_acc_mp[i]map<lf, vector<j>> -> ACTION[j, lf] = i
	for (int i = 0; i < Prod_acc_mp.size(); ++i) {
		// 相同的产生式共享展望符
		std::set<std::string> lf_set;
		for (auto it = Prod_acc_mp[i].begin(); it != Prod_acc_mp[i].end(); ++it) {
			lf_set.insert((*it).first);
		}
		for (auto it = Prod_acc_mp[i].begin(); it != Prod_acc_mp[i].end(); ++it) {
			for (auto it1 = lf_set.begin(); it1 != lf_set.end(); ++it1) {
				for (int j = 0; j < (*it).second.size(); ++j) {
					this->LR1_table[(*it).second[j]][(*it1)] = { 'r',i };
				}
			}
		}
	}
	// 4. [Start->mp[Program]·,#] -> ACTION[i,#]=ax
	this->LR1_table[1]["#"] = { 'a',0 };
}

// 打印LR1分析表
void Parser::show_LR1_table()
{
	//std::cout << this->LR1_table.size();
	std::ofstream outfile;
	outfile.open(PS_LR1table_Path, std::ios::out | std::ios::binary);
	for (int i = 0; i < this->LR1_table.size(); ++i) {
		outfile << "State" << i << ":\n";
		for (auto it = this->LR1_table[i].begin(); it != this->LR1_table[i].end(); ++it) {
			outfile << (*it).first << " - " << (*it).second.first << (*it).second.second << "\n";
		}
	}
	outfile.close();
}

// 使用LR1分析表进行分析
bool Parser::Analyse(std::vector<token> tk)
{
	// 栈底状态
	int cur_state = 0;
	// 语法分析的状态
	int als_state = PARSER_GOING;
	// 状态栈 vector<int> StateStack;
	std::vector<int> StateStack;
	// 符号栈 vector<string> SymbolStack;
	std::vector<std::string> SymbolStack;
	// 语法树节点栈 vector<*treenode> NodeStack
	std::vector<treenode*> NodeStack;
	// 步骤数
	int StepCount = 0;
	// 输入串指针（索引token[] tk）
	int p_tk = 0;
	// 初始化
	StateStack.push_back(cur_state);
	SymbolStack.push_back("#");
	// 输出控制
	std::ofstream outfile;
	outfile.open(PS_Rignt_Path, std::ios::out | std::ios::binary);
	// 分析
	while (als_state == 0 && p_tk < tk.size()) {
		StepCount++;
		// file - 输出栈的形态
		outfile << "Step" << StepCount << ":\n";
		outfile << "StateStack:";
		for (int i = 0; i < StateStack.size(); ++i) {
			outfile << StateStack[i] << " ";
		}
		outfile << "\n";
		outfile << "SymbolStack:";
		for (int i = 0; i < SymbolStack.size(); ++i) {
			outfile << SymbolStack[i] << " ";
		}
		outfile << "\n";
		// file - 输出栈的形态
		// 状态栈顶
		cur_state = StateStack[StateStack.size() - 1];
		token tk_tmp = tk[p_tk];
		// 输入串首元素处理
		std::string tk_tmp_str;
		std::string tk_tmp_str_val;
		if (tk_tmp.token_type == "identifier")
			tk_tmp_str = this->Grammar_symbol_mp["ID"];
		else if (tk_tmp.token_type == "digit")
			tk_tmp_str = this->Grammar_symbol_mp["num"];
		else
			tk_tmp_str = tk_tmp.token_type;
		tk_tmp_str_val = tk_tmp.token_value;
		// 判断err
		if (!this->LR1_table[cur_state].count(tk_tmp_str)) {
			// err
			als_state = ERROR_SHIFT;
			break;
		}
		// 移进/规约/goto/acc
		char action = this->LR1_table[cur_state][tk_tmp_str].first;
		int nxt_state = this->LR1_table[cur_state][tk_tmp_str].second;
		// file - action  eg.s1/r2...
		outfile << "action:" << action << nxt_state << "\n";
		// file
		// 1.移进
		if (action == 's') {
			SymbolStack.push_back(tk_tmp_str);
			StateStack.push_back(nxt_state);
			p_tk++;
			// Semantic
			treenode* p = new treenode;
			p->type = tk_tmp_str;
			p->value = tk_tmp_str_val;
			p->pos_x = tk[p_tk - 1].token_x;
			p->pos_y = tk[p_tk - 1].token_y;
			NodeStack.push_back(p);
			// Semantic
		}
		// 2.规约
		else if (action == 'r') {
			production prod_to_use = Production_raw[nxt_state];
			for (int i = (int)prod_to_use.right.size() - 1; i >= 0; --i) {
				if (SymbolStack.size() > 1 && SymbolStack.back() == prod_to_use.right[i]) {
					SymbolStack.pop_back();
					StateStack.pop_back();
				}
				else {
					// err
					als_state = ERROR_REDUCE;
					break;
				}
			}
			// err
			if (als_state == ERROR_REDUCE)
				continue;
			SymbolStack.push_back(prod_to_use.left);
			// Semantic
			treenode* p = new treenode;
			p->type = "Argument"; // 同时可以标志是否到达leaf
			p->value = prod_to_use.left;
			std::stack<treenode*>revStack;
			for (int i = 0; i < prod_to_use.right.size(); ++i) {
				revStack.push(NodeStack[NodeStack.size() - 1]);
				NodeStack.pop_back();
			}
			while (revStack.size()) {
				p->child.push_back(revStack.top());
				revStack.pop();
			}
			NodeStack.push_back(p);
			// Semantic
			// 规约完要多做一步goto
			cur_state = StateStack[StateStack.size() - 1];
			tk_tmp_str = SymbolStack[SymbolStack.size() - 1];
			if (!this->LR1_table[cur_state].count(tk_tmp_str)) {
				// err
				als_state = ERROR_GOTO;
			}
			else {
				StateStack.push_back(this->LR1_table[cur_state][tk_tmp_str].second);
			}
		}
		// 3.goto 应该不会执行到，因为goto紧接在规约后做了
		else if (action == 'g') {
			StateStack.push_back(nxt_state);
		}
		// 4.acc
		else {
			als_state = PARSER_RIGHT;
			// Semantic
			this->gra_root = NodeStack[0];
			NodeStack.pop_back();
			// Semantic
			break;
		}
	}// while
	if (als_state == PARSER_RIGHT) {
		std::cout << "语法分析成功\n";
		outfile << "Parsing Success!\n";
		outfile.close();
		return true;
	}
	std::cout << "语法分析出错\n";
	std::cout << "错误代码:" << als_state << "\n";
	std::cout << "错误位置:(" << tk[p_tk].token_x << "," << tk[p_tk].token_y << ")\n";
	outfile.close();
	// file
	outfile.open(PS_Wrong_Path, std::ios::out | std::ios::binary);
	outfile << "Parsing Failed!\n";
	outfile << "Return code:" << als_state << "\n";
	outfile << "Wrong place:(" << tk[p_tk].token_x << "," << tk[p_tk].token_y << ")\n";
	outfile.close();
	return false;
}

// 遍历语法树
void Parser::show_tree(treenode* root, int depth)
{
	// 添加缩进
	for (int i = 0; i < depth; ++i) {
		if (i == 0) {
			std::cout << "|";
		}
		std::cout << "-";
	}
	std::cout << root->value << "\n";
	if (root->child.size()) {
		for (int i = (int)root->child.size() - 1; i >= 0; --i) {
			show_tree(root->child[i], depth + 1);
		}
	}
}

// Semantic Part
// 发射四元式（最后复制 this->root.code）
void Parser::emit(int is_cmd_out)
{
	std::ofstream outfile;
	outfile.open(SM_QUADRUPLE_Path, std::ios::out | std::ios::binary);
	// 特判 - 主函数返回值
	if (gra_root->code.back().second.result == "Func_Name") {
		gra_root->code.back().second.result = "main";
	}
	for (int i = 0; i < this->gra_root->code.size(); ++i) {
		this->Q_ruple.push_back(this->gra_root->code[i]);
		if (is_cmd_out) {
			// cmd
			std::cout << this->gra_root->code[i].first << " ";
			std::cout << "(" << this->gra_root->code[i].second.op << ", ";
			std::cout << this->gra_root->code[i].second.arg1 << ", ";
			std::cout << this->gra_root->code[i].second.arg2 << ", ";
			std::cout << this->gra_root->code[i].second.result << ")\n";
		}
		// file
		outfile << this->gra_root->code[i].first << " ";
		outfile << this->gra_root->code[i].second.op << " ";
		outfile << this->gra_root->code[i].second.arg1 << " ";
		outfile << this->gra_root->code[i].second.arg2 << " ";
		outfile << this->gra_root->code[i].second.result << "\n";
	}
	if (is_cmd_out) {
		// cmd
		std::cout << this->nxtquad << "\n";
	}
	// file
	outfile << this->nxtquad << "\n";
	outfile.close();
}
// 生成四元式
std::pair<int, Quadruple> Parser::gen(std::string _op, std::string _arg1, std::string _arg2, std::string _result)
{
	return std::make_pair(this->nxtquad++, Quadruple(_op, _arg1, _arg2, _result));
}

// dfs语法树生成四元式（后序遍历）
int Parser::dfs(treenode* root)
{
	// 初始化符号表
	if (this->tableSet.empty()) {
		this->mk_table();
	}
	// 是终结符，叶子节点
	if (root->type != "Argument")
	{
		root->place = root->value;
		root->code.clear();
		return SEMANTIC_RIGHT;// 分析正确 define
	}
	// 不是终结符，child.size>0
	if (root->value == this->Grammar_symbol_mp["函数声明"] || root->child[0]->value == "{") {
		this->mk_table();// 符号表扩充
	}

	// "声明"（针对不调用、仅声明的"函数声明"） 调用的函数先不dfs
	if (root->value == this->Grammar_symbol_mp["声明"] && root->child[1]->value != "main"
		&& root->child[2]->value == this->Grammar_symbol_mp["函数声明"]) {
		root->place = root->child[1]->place;
		// 函数是否已定义
		int is_def = 0;
		for (int i = 0; i < this->func.size(); ++i) {
			if (this->func[i].name == root->child[1]->value) { is_def = 1; break; }
		}
		if (is_def) {
			std::cout << "函数重定义：" << root->child[1]->value << std::endl;
			std::cout << "位置：(" << root->child[1]->pos_x << "," << root->child[1]->pos_y << ")\n";
			return ERROR_FUNC_REPEAT; // 函数重定义 define
		}
		this->add_argument(root->child[1]->value);
		funcInfo tmp;
		tmp.name = root->child[1]->value;
		// 取函数声明的code
		this->func.push_back(tmp);
		this->funcptr.push_back(root->child[2]); // "函数声明"
		return SEMANTIC_RIGHT; // 函数定义的部分先不dfs
	}

	// "声明"（全局数组）- new1 
	// 注：new2/3 服务于此条，数组信息已获取，空间申请在arrayinfo中，此后无需dfs此部分
	if (root->value == this->Grammar_symbol_mp["声明"] &&
		root->child[2]->value == this->Grammar_symbol_mp["数组声明"]) {
		root->place = root->child[1]->value;// 变量名id在child[1]
		// 数组是否已定义
		bool is_def = this->isArrayExist(root->child[1]->value);
		if (is_def) {
			std::cout << "数组重定义：" << root->child[1]->value << std::endl;
			std::cout << "位置：(" << root->child[1]->pos_x << "," << root->child[1]->pos_y << ")\n";
			return ERROR_ARRAY_REPEAT; // 数组重定义 
		}
		// 获取数组信息
		std::vector<int>arr_info = this->get_arr_sz(root->child[2]);
		// 检查数组是否合理
		if (!arr_check(arr_info)) {
			std::cout << "数组定义不合理：" << root->child[1]->value << std::endl;
			std::cout << "位置：(" << root->child[1]->pos_x << "," << root->child[1]->pos_y << ")\n";
			return ERROR_BAD_ARRAY; // 数组定义不合理
		}
		// 扩展数组符号表
		arrayInfo arrayinfo_item(arr_info);
		arrayinfo_item.name = root->child[1]->value;
		this->arrayinfo.push_back(arrayinfo_item);
		// 声明语句 - 申请数组空间
		int space_size = 4;
		for (int i = 0; i < arr_info.size(); ++i) { space_size *= arr_info[i]; }
		root->code.push_back(this->gen("dec", std::to_string(space_size), "-", root->place));
		// 没有继续dfs的必要 返回
		return SEMANTIC_RIGHT;
	}

	// "数组"（数组的访问） new67(89)
	// root->place
	if (root->value == this->Grammar_symbol_mp["数组"]) {
		std::string arg = root->child[0]->value;
		// 检查数组是否定义
		int is_def = -1;
		for (int i = 0; i < this->arrayinfo.size(); ++i) {
			if (this->arrayinfo[i].name == arg) { is_def = i; break; }
		}
		if (is_def == -1) {
			std::cout << "数组未定义：" << arg << std::endl;
			std::cout << "位置：(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
			return ERROR_ARRAY_NOT_FOUND; // 数组未定义
		}
		// 索引计算
		std::vector<int> arr_vis_info = this->get_arr_idx(root);
		int pos = this->get_arr_idx_pos(this->arrayinfo[is_def], arr_vis_info);
		// 检查访问合理性
		if (pos == -1) {
			std::cout << "数组访问索引错误：" << arg << std::endl;
			std::cout << "位置：(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
			return ERROR_ARRAY_DIM; // 访问和定义维数不一致
		}
		else if (pos == -2) {
			std::cout << "数组访问越界：" << arg << std::endl;
			std::cout << "位置：(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
			return ERROR_ARRAY_OVERFLOW; // 数组访问越界
		}
		root->place = this->arrayinfo[is_def].name + "[" + std::to_string(pos) + "]";
		// 没有继续dfs的必要 返回
		return SEMANTIC_RIGHT;
	}

	// 先遍历子节点
	for (int i = 0; i < root->child.size(); ++i) {
		int res = dfs(root->child[i]);
		// 分析出错
		if (res < 0) {
			if (root->value == this->Grammar_symbol_mp["函数声明"] || root->child.back()->value == "}")
				this->rm_table();// 符号表缩小
			return res;
		}
	}

	// 变元节点（子节点的 place/code 都已有）
	// default
	root->place = root->child[0]->place;
	root->code.clear();
	for (int i = 0; i < root->child.size(); ++i) {
		for (int j = 0; j < root->child[i]->code.size(); ++j) {
			root->code.push_back(root->child[i]->code[j]);
		}
	}

	// 枚举语法规则：写root的place/额外的code
	// "赋值语句"
	if (root->value == this->Grammar_symbol_mp["赋值语句"]) {
		// id = number
		if (root->child[0]->type == this->Grammar_symbol_mp["ID"]) {
			std::string arg = root->child[0]->value;
			if (this->lookup(arg)) {
				root->place = root->child[2]->place; //综合
				if (root->child[2]->place[root->child[2]->place.size() - 1] == ']')// new4
					root->code.push_back(this->gen("=[]", root->child[2]->place, "-", root->child[0]->place));
				else
					root->code.push_back(this->gen(":=", root->child[2]->place, "-", root->child[0]->place));
			}
			else {
				std::cout << "变量未声明：" << arg << std::endl;
				std::cout << "位置：(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
				return ERROR_ARGU_NOT_FOUND;// 变量未声明
			}
		}
		// arr = number - new4
		// 变址存数 - (:=,xxx,-,T),([]=,X1,-,T1[T]) - 相当于T1[T]:=X
		else if (root->child[0]->value == this->Grammar_symbol_mp["数组"]) {
			std::string arg = root->child[0]->child[0]->value;
			// 检查数组是否定义
			int is_def = -1;
			for (int i = 0; i < this->arrayinfo.size(); ++i) {
				if (this->arrayinfo[i].name == arg) { is_def = i; break; }
			}
			if (is_def == -1) {
				std::cout << "数组未定义：" << arg << std::endl;
				std::cout << "位置：(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
				return ERROR_ARRAY_NOT_FOUND; // 数组未定义
			}
			else {
				std::vector<int> arr_vis_info = this->get_arr_idx(root->child[0]);
				int pos = this->get_arr_idx_pos(this->arrayinfo[is_def], arr_vis_info);
				// 检查访问合理性
				if (pos == -1) {
					std::cout << "数组访问索引错误：" << arg << std::endl;
					std::cout << "位置：(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
					return ERROR_ARRAY_DIM; // 访问和定义维数不一致
				}
				else if (pos == -2) {
					std::cout << "数组访问越界：" << arg << std::endl;
					std::cout << "位置：(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
					return ERROR_ARRAY_OVERFLOW; // 数组访问越界
				}
				// 数组信息表 - 赋值
				//this->arrayinfo[is_def].p_arr[pos] = 
				// 产生赋值四元式
				root->place = root->child[0]->child[0]->place; //综合 数组名的place
				root->code.push_back(this->gen("[]=", root->child[2]->place, "-", root->child[0]->place));
			}
		}
	}
	// "Program"
	else if (root->value == this->Grammar_symbol_mp["Program"]) {
		// 检查main
		if (root->child.size() == 1) {
			int is_def = 0;
			for (int i = 0; i < func.size(); ++i) {
				if (func[i].name == "main") { is_def = 1; break; }
			}
			if (is_def == 0)
				return ERROR_MAIN_NOT_FOUND;
		}
	}
	// "类型"
	else if (root->value == this->Grammar_symbol_mp["类型"]) {
	}
	// "形参"
	else if (root->value == this->Grammar_symbol_mp["形参"]) {
		root->para.clear();
		if (root->child[0]->value != "void") {
			for (int i = 0; i < root->child[0]->para.size(); ++i) {
				root->para.push_back(root->child[0]->para[i]);
			}
		}
	}
	// "参数列表"
	else if (root->value == this->Grammar_symbol_mp["参数列表"]) {
		root->para.clear();
		root->para.push_back(root->child[0]->para[0]);
		if (root->child.size() > 1) {
			for (int i = 0; i < root->child[2]->para.size(); ++i) {
				root->para.push_back(root->child[2]->para[i]);
			}
		}
	}
	// "参数"
	else if (root->value == this->Grammar_symbol_mp["参数"]) {
		root->place = root->child[1]->place;
		this->add_argument(root->child[1]->value);
		root->para.clear();
		root->para.push_back(root->child[1]->value);
	}
	// "语句块" size==3/4
	else if (root->value == this->Grammar_symbol_mp["语句块"]) {
		root->place = root->child[root->child.size() - 2]->place;
	}
	// "内部声明" size==2/3
	else if (root->value == this->Grammar_symbol_mp["内部声明"]) {
	}
	// "内部变量声明" eg. int a
	else if (root->value == this->Grammar_symbol_mp["内部变量声明"]) {
		std::string arg = root->child[1]->place;
		if (this->lookup(arg)) {
			std::cout << "变量重定义：" << arg << std::endl;
			std::cout << "位置：(" << root->child[1]->pos_x << "," << root->child[1]->pos_y << ")" << std::endl;
			return ERROR_ARGU_REPEAT; // 变量重定义 define
		}
		this->add_argument(arg);
	}
	// "语句串" size==1/2
	else if (root->value == this->Grammar_symbol_mp["语句串"]) {
		if (root->child.size() == 2) {
			root->place = root->child[1]->place;
		}
	}
	// "语句" type==4
	else if (root->value == this->Grammar_symbol_mp["语句"]) {
	}
	// "return语句"
	else if (root->value == this->Grammar_symbol_mp["return语句"]) {
		if (root->child.size() == 3) {
			root->place = root->child[1]->place;
			root->code.push_back(this->gen(":=", root->child[1]->place, "-", "Func_Name"));
		}
	}
	// "while语句"
	else if (root->value == this->Grammar_symbol_mp["while语句"]) {
		root->code.clear();
		for (auto& it : root->child[2]->code) // 表达式
		{
			root->code.push_back(it); // 关系表达式的"j<,a,b,c"/"j,-,-,d"含在里面
		}
		for (auto& it : root->child[4]->code) // 语句块
		{
			root->code.push_back(it);
		}
		// 子块四元式长度确定，可以直接算 关系表达式、while中(j,-,-,xxx)的xxx
		root->code.push_back(gen("j", "-", "-", std::to_string(root->code[0].first)));
		for (auto& it : root->code) {
			//it.first == root->child[2]->false_flag
			if (it.second.result == "-1") {
				it.second.result = std::to_string(root->code.back().first + 1);// 关系表达式break(j,-,-,xxx)
				break;
			}
		}
	}
	// "if语句" size==5/7
	else if (root->value == this->Grammar_symbol_mp["if语句"]) {
		root->code.clear();
		for (auto& it : root->child[2]->code) // 表达式
		{
			root->code.push_back(it); // 关系表达式的"j<,a,b,c"/"j,-,-,d"含在里面
		}
		for (auto& it : root->child[4]->code) // 语句块
		{
			root->code.push_back(it);
		}

		if (root->child.size() == 7) {
			// 有else时if分支执行完之后跳转
			root->code.push_back(this->gen("j", "-", "-", std::to_string(root->code.back().first + root->child[6]->code.size() + 2)));
			root->code.back().first -= (int)root->child[6]->code.size();
			// else分支代码复制给root->code
			for (auto& it : root->child[6]->code) {
				it.first++; // 前面加了一句if后的无条件跳转
				if (it.second.op[0] == 'j') // 只要是跳转指令
				{
					int num = atoi(it.second.result.c_str());
					it.second.result = std::to_string(num + 1);
				}
				root->code.push_back(it);
			}
		}
		// 假出口
		for (auto& it : root->code) {
			// it.first == root->child[2]->false_flag
			if (it.second.result == "-1") {
				if (root->child.size() == 7) {
					// 跳转else的部分
					it.second.result = std::to_string(root->code.back().first - root->child[6]->code.size() + 1);
				}
				else
					it.second.result = std::to_string(root->code.back().first + 1);// 跳转后续的部分
				break;
			}
		}
	}
	// "表达式"
	else if (root->value == this->Grammar_symbol_mp["表达式"]) {
		if (root->child.size() == 1 || root->child[0]->value == "(") {
			root->place = root->child[int(root->child[0]->value == "(")]->place;
		}
		// "表达式" -> "加法表达式" "relop" "表达式"
		else {
			root->place = this->newtmp();
			treenode* t1 = root->child[0];
			treenode* t2 = root->child[2];
			root->code.push_back(this->gen("j" + root->child[1]->place, t1->place, t2->place, "0"));// 真分支
			root->code.back().second.result = std::to_string(root->code.back().first + 3);
			root->code.push_back(this->gen(":=", "0", "-", root->place));
			root->code.push_back(this->gen("j", "-", "-", "-1"));// 假分支
			root->code.push_back(this->gen(":=", "1", "-", root->place));
		}
	}
	// "relop"
	else if (root->value == this->Grammar_symbol_mp["relop"]) {

	}
	// "加法表达式"
	else if (root->value == this->Grammar_symbol_mp["加法表达式"]) {
		if (root->child.size() == 1) {
			// default
			root->place = root->child[0]->place;
		}
		else {
			treenode* t1 = root->child[0];
			treenode* t2 = root->child[2];
			root->place = this->newtmp();
			root->code.push_back(this->gen(root->child[1]->place, t1->place, t2->place, root->place));
		}
	}
	// "项"
	else if (root->value == this->Grammar_symbol_mp["项"]) {
		if (root->child.size() == 1) {
			// default
			root->place = root->child[0]->place;
		}
		else {
			treenode* t1 = root->child[0];
			treenode* t2 = root->child[2];
			root->place = this->newtmp();
			root->code.push_back(this->gen(root->child[1]->place, t1->place, t2->place, root->place));
		}
	}
	// "因子"
	else if (root->value == this->Grammar_symbol_mp["因子"]) {
		if (root->child.size() == 1) {
			// num / ID / 数组 new5
			root->place = root->child[0]->place;
		}
		else {
			// 表达式
			root->place = root->child[1]->place;
			// 如果调用了函数
			if (root->child[1]->value == this->Grammar_symbol_mp["call"]) {
				// 函数是否已定义
				int is_def = -1;
				for (int i = 0; i < this->func.size(); ++i) {
					if (this->func[i].name == root->child[0]->value) { is_def = i; break; }
				}
				if (is_def == -1) {
					std::cout << "函数未定义：" << root->child[0]->value << std::endl;
					std::cout << "位置：(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")\n";
					return ERROR_FUNC_NOT_FOUND; // 函数未定义 define
				}
				// ID
				root->place = root->child[0]->place;// 因子.place = ID.place 所以函数返回值的名字是函数名
				// 实参
				std::vector<std::string> real_para;
				for (int i = 0; i < root->child[1]->para.size(); ++i) {
					real_para.push_back(root->child[1]->para[i]);
				}
				// 形参
				std::vector<std::string> form_para = this->get_formpara(this->funcptr[is_def]);
				// 检查参数个数
				if (form_para.size() != real_para.size()) {
					std::cout << "函数参数个数不匹配\n";
					std::cout << "位置：(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")\n";
					return ERROR_FUNC_PARA_SIZE;// 函数参数个数不匹配
				}
				// 修改参数
				this->fixPara(this->funcptr[is_def], real_para, form_para);
				// dfs 被调用的函数
				int res = dfs(this->funcptr[is_def]);
				if (res < 0)return res;
				for (int i = 0; i < this->funcptr[is_def]->code.size(); ++i) {
					root->code.push_back(this->funcptr[is_def]->code[i]);
				}
				if (root->code.back().second.result == "Func_Name") {
					root->code.back().second.result = func[is_def].name;
				}
			}
		}
	}
	// "声明"（针对"变量声明"和调用的"函数声明"）
	// 调用的"函数声明"只有mian的，其余的都只在被call的时候才调用
	else if (root->value == this->Grammar_symbol_mp["声明"]) {
		root->place = root->child[1]->place;
		if (root->child[2]->value == Grammar_symbol_mp["函数声明"]) {
			// 函数是否已定义
			int is_def = 0;
			for (int i = 0; i < this->func.size(); ++i) {
				if (this->func[i].name == root->child[1]->value) { is_def = 1; break; }
			}
			if (is_def) {
				std::cout << "函数重定义：" << root->child[1]->value << std::endl;
				std::cout << "位置：(" << root->child[1]->pos_x << "," << root->child[1]->pos_y << ")\n";
				return ERROR_FUNC_REPEAT; // 函数重定义 define
			}
			this->add_argument(root->child[1]->value);
			funcInfo tmp;
			tmp.name = root->child[1]->value;
			// 取函数声明的code
			this->func.push_back(tmp);
			this->funcptr.push_back(root->child[2]); // "函数声明"
		}
	}
	// "函数声明"
	else if (root->value == this->Grammar_symbol_mp["函数声明"]) {
		root->place = root->child[3]->place; // "语句块"的place
	}
	// "声明串"
	else if (root->value == this->Grammar_symbol_mp["声明串"]) {
		root->place = root->child[0]->place;
		// 有过程调用 - 只要main的code
		/*if (root->child.size() == 2) {
			if (root->place != "main") {
				root->place = root->child[1]->place;
			}
			root->code.clear();
			for (int i = 0; i < root->child[1]->code.size(); ++i) {
				root->code.push_back(root->child[1]->code[i]);
			}
		}*/
	}
	// "call"
	else if (root->value == this->Grammar_symbol_mp["call"]) {
		if (root->child.size() == 3) {
			root->place = root->child[1]->place;
			root->para.clear();
			for (int i = 0; i < root->child[1]->para.size(); ++i) {
				root->para.push_back(root->child[1]->para[i]);
			}
		}
	}
	// "实参列表"
	else if (root->value == this->Grammar_symbol_mp["实参列表"]) {
		root->para.push_back(root->child[0]->place);
		if (root->child.size() > 1) {
			for (int i = 0; i < root->child[2]->para.size(); ++i) {
				root->para.push_back(root->child[2]->para[i]);
			}
		}
	}

	// to append...

	if (root->value == this->Grammar_symbol_mp["函数声明"] || root->child.back()->value == "}")
		this->rm_table();// 符号表缩小

	return SEMANTIC_RIGHT;// 默认正确
}

// 功能22
// 获取形参(root == "函数声明")
std::vector<std::string> Parser::get_formpara(treenode* root)
{
	std::vector<std::string> res;
	root = root->child[1]; // "形参"
	if (root->child[0]->value == "void")
		return res;
	dfs(root); // "参数列表"
	return root->child[0]->para;
}

// 功能23
// 修改形参(root == "函数声明")
bool Parser::fixPara(treenode* root, std::vector<std::string> real_para, std::vector<std::string> form_para)
{
	int cnt = 0;
	if (root == nullptr) {
		return false;
	}

	if (root->type == "identifier") {
		// 在 form_para 中查找对应的形参
		auto it = std::find(form_para.begin(), form_para.end(), root->value);

		if (it != form_para.end()) {
			// 找到对应的形参，替换为实参
			root->value = real_para[it - form_para.begin()];
			// 记录替换次数
			cnt++;
		}
	}
	// 递归处理子节点
	for (treenode* child : root->child) {
		fixPara(child, real_para, form_para);
	}
	return true;
}

/*
	功能24：isArrayExist
	查看数组声明是否重名
*/
bool Parser::isArrayExist(std::string s)
{
	for (int i = 0; i < this->arrayinfo.size(); ++i) {
		if (s == arrayinfo[i].name)return true;
	}
	return false;
}

// 语义分析顶层函数（错误处理）
bool Parser::semanticAnalyse() {
	int res = this->dfs(this->gra_root);
	if (res == SEMANTIC_RIGHT) {
		std::cout << "语义分析成功\n";
		this->emit(IS_SENMANTIC_CMD_SHOW);
		return true;
	}
	else if (res == ERROR_ARGU_NOT_FOUND) {
		std::cout << "变量未定义！\n";
	}
	else if (res == ERROR_ARGU_REPEAT) {
		std::cout << "变量重定义！\n";
	}
	else if (res == ERROR_FUNC_NOT_FOUND) {
		std::cout << "函数未定义！\n";
	}
	else if (res == ERROR_FUNC_REPEAT) {
		std::cout << "函数重定义！\n";
	}
	else if (res == ERROR_MAIN_NOT_FOUND) {
		std::cout << "找不到main函数！\n";
	}
	else if (res == ERROR_FUNC_PARA_SIZE) {
		std::cout << "实参个数错误！\n";
	}
	// to append
	else if (res == ERROR_ARRAY_REPEAT) {
		std::cout << "数组重定义！\n";
	}
	else if (res == ERROR_BAD_ARRAY) {
		std::cout << "数组定义不合理！\n";
	}
	else if (res == ERROR_ARRAY_NOT_FOUND) {
		std::cout << "数组重未义！\n";
	}
	else if (res == ERROR_ARRAY_DIM) {
		std::cout << "非预期的数组访问！\n";
	}
	else if (res == ERROR_ARRAY_OVERFLOW) {
		std::cout << "数组索引错误！\n";
	}

	return false;
}

/* END - class Parser - 函数实现 */

