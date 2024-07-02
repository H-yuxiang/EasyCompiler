#include "tool.h"
#include "parser.h"

/* BEGIN - class Parser - ����ʵ�� */
// ����
Parser::Parser()
{
	this->tmp_number = 0;
	this->nxtquad = QUAD_SATRT;
	this->Item_group_num = 0;
	this->gra_root = NULL;
}

// ����
Parser::~Parser()
{
}

// ��Ԫӳ���
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

// ��ʼ�ķ�
void Parser::load_production()
{
	// ��չ
	Production_raw.push_back(production(0, ARGUMENT, -1, (std::string)("Start"), std::vector<std::string>{Grammar_symbol_mp[Grammar_rule[0][0]]}));
	this->Argument_garmmar_mp[(std::string)("Start")].push_back((int)Production_raw.size() - 1);
	// ����
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

// ��ӡ��ʼ�ķ������ԣ�
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

// ��ȡ������Ԫ��FIRST��
std::vector<std::string> Parser::get_single_first(std::string argument)
{
	std::vector<std::string> ret_first_set;
	// �������� argument Ϊ left �Ĳ���ʽ
	for (int idx = 0; idx < Argument_garmmar_mp[argument].size(); ++idx) {
		int i = Argument_garmmar_mp[argument][idx]; // �ñ�Ԫ�����в���ʽ����
		// Production_raw[i].right[0] �������ս��
		if (!Argument_type.count(Production_raw[i].right[0]) || Production_raw[i].right[0] == Grammar_symbol_mp["ID"]
			|| Production_raw[i].right[0] == Grammar_symbol_mp["num"] || Production_raw[i].right[0] == Grammar_symbol_mp["epsilon"]) {
			if (!is_string_in_vector(ret_first_set, Production_raw[i].right[0]))
				ret_first_set.push_back(Production_raw[i].right[0]);
		}
		// Production_raw[i].right[0] �����Ǳ�Ԫ
		else {
			// ��󿴣�ֱ����1.ĳ���ս������2.ĳ����Ԫ��FIRST����epsilon��
			for (int j = 0; j < Production_raw[i].right.size(); ++j) {
				// ������ս��
				if (!Argument_type.count(Production_raw[i].right[j]) || Production_raw[i].right[j] == Grammar_symbol_mp["ID"]
					|| Production_raw[i].right[j] == Grammar_symbol_mp["num"] || Production_raw[i].right[j] == Grammar_symbol_mp["epsilon"]) {
					if (!is_string_in_vector(ret_first_set, Production_raw[i].right[j]))
						ret_first_set.push_back(Production_raw[i].right[j]);
					break; //1.
				}
				// ����Ǳ�Ԫ
				else {
					std::vector<std::string> tmp_first_set;
					// �ñ�Ԫ�Ѿ����FIRST
					if (First_set.count(Production_raw[i].right[j]))
						tmp_first_set = First_set[Production_raw[i].right[j]];
					else {
						// �ݹ�
						tmp_first_set = get_single_first(Production_raw[i].right[j]);
						First_set[Production_raw[i].right[j]] = tmp_first_set;
					}
					// ����FIRST
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

// ��ȡ���б�Ԫ��FIRST��
void Parser::get_first()
{
	for (int i = 0; i < this->Production_raw.size(); ++i) {
		// ����ʽ raw �� left ��Ȼ�Ǳ�Ԫ
		if (First_set.count(Production_raw[i].left))
			continue;// �Ѿ����
		First_set[Production_raw[i].left] = get_single_first(Production_raw[i].left);
	}
}

// ��ӡ���б�Ԫ��FIRST��
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

// ��ȡһ����Ŀ�ıհ��������� item �� dot��λ�ö����� right �Ŀ�ͷ��
itemGroup Parser::get_single_closer(item im)
{
	itemGroup ret_ig;
	// ���λ����ĩβ
	if (im.prod.dot_pos >= im.prod.right.size()) {
		ret_ig.item_group.insert(im);
		return ret_ig;
	}
	std::queue<item> qwait; // �ȴ�����
	qwait.push(im);
	while (qwait.size()) {
		item im_tmp = qwait.front();
		// ��ӵ� ret_ig
		ret_ig.item_group.insert(im_tmp);
		qwait.pop();
		production prod_tmp = im_tmp.prod;
		std::string lookforward_tmp = im_tmp.lookforward;
		// ����1�����ڲ���ʽ��ĩβ������������
		if (prod_tmp.dot_pos >= prod_tmp.right.size()) {
			continue;
		}
		// ����2�����Ϊ�ս��������������
		if (!Argument_type.count(prod_tmp.right[prod_tmp.dot_pos]) || Argument_type[prod_tmp.right[prod_tmp.dot_pos]] == TERMINAL) {
			continue;
		}
		// ����3�����Ϊ��Ԫ
		// ��Ҫ����ƺ��"ʣ������first��"
		std::vector<std::string> seqence_first;
		// һ��
		for (int j = prod_tmp.dot_pos + 1; j < prod_tmp.right.size(); ++j) {
			// ���ս��
			if (!Argument_type.count(prod_tmp.right[j]) || Argument_type[prod_tmp.right[j]] == TERMINAL) {
				seqence_first.push_back(prod_tmp.right[j]);
				break;
			}
			// �ս��
			else {
				for (int k = 0; k < First_set[prod_tmp.right[j]].size(); ++k) {
					if (!is_string_in_vector(seqence_first, First_set[prod_tmp.right[j]][k])) {
						seqence_first.push_back(First_set[prod_tmp.right[j]][k]);
					}
				}
				// ��ǰ��Ԫfirst������epsilon
				if (!is_string_in_vector(First_set[prod_tmp.right[j]], this->Grammar_symbol_mp["epsilon"]))
					break;
			}
			// lf
			if (j == prod_tmp.right.size() - 1) {
				seqence_first.push_back(lookforward_tmp);
			}
		}
		// ����
		if (seqence_first.size() == 0)
			seqence_first.push_back(lookforward_tmp);
		// ��ȡ�ñ�Ԫ�Ĳ���ʽ����
		std::vector<int> argument_production_idx = this->Argument_garmmar_mp[prod_tmp.right[prod_tmp.dot_pos]];
		for (int i = 0; i < argument_production_idx.size(); ++i) {
			production prod_new = this->Production_raw[argument_production_idx[i]];
			item im_new = item(prod_new, 0);
			// qwait.push
			for (int j = 0; j < seqence_first.size(); ++j) {
				im_new.lookforward = seqence_first[j];
				qwait.push(im_new);
			}
		}// ��������ʽ
	}// end of while(q)
	return ret_ig;
}

// ��ӡ"Start"�ıհ������ԣ�
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
			if (i == (*it).prod.dot_pos) std::cout << "��";
			std::cout << (*it).prod.right[i];
		}
		if ((*it).prod.dot_pos == (*it).prod.right.size())
			std::cout << "��";
		std::cout << "," << (*it).lookforward << "\n";
	}
}

// ��ȡ get_closer ��������Ŀ�������ݡ���ͬ���ַ���������itemGroup��
// ���룺I_i
std::vector<itemGroup> Parser::get_GOTO_input(const itemGroup& in_ig)
{
	std::vector<itemGroup> ret_igv;
	std::map<std::string, int> Item_idx;
	// ���ݵ����ַ������ȡת������
	for (auto it = in_ig.item_group.begin(); it != in_ig.item_group.end(); ++it) {
		item im = (*it);
		// ����ĩβ�����һ�ű��¼[I_i,s,j]������LR1������ʱ���ã�
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
		// ��ȡ����ַ�
		std::string nxt = im.prod.right[im.prod.dot_pos];
		// ����ǿղ���ʽ������
		if (nxt == this->Grammar_symbol_mp["epsilon"])
			continue;
		// ȫ��ΨһŲ"��λ��"�����
		im.prod.dot_pos++;
		// ���У�ת�Ʒ���1
		if (Item_idx.count(nxt)) {
			int idx = Item_idx[nxt];
			ret_igv[idx].item_group.insert(im);
		}
		// ��δ�У�ת�Ʒ���2
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

// ��ӡ get_closer ��������Ŀ��
void Parser::show_GOTO_input(std::vector<itemGroup> ig_v)
{
	for (int i = 0; i < ig_v.size(); ++i) {
		std::cout << i << std::endl;
		for (auto it = ig_v[i].item_group.begin(); it != ig_v[i].item_group.end(); ++it) {
			std::cout << (*it).prod.left << "->";
			for (int i = 0; i < (*it).prod.right.size(); ++i) {
				if (i == (*it).prod.dot_pos) std::cout << "��";
				std::cout << (*it).prod.right[i];
			}
			if ((*it).prod.dot_pos == (*it).prod.right.size())
				std::cout << "��";
			std::cout << "," << (*it).lookforward << "\n";
		}
	}
}

// ��ȡ��Ŀ���� closer
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

// ��ȡ���е���Ŀ�� - ��Ŀ����
void Parser::get_item_group()
{
	// I_i --s--> I_j	vector<map<string,int>> ig_shift_mp; // ״̬ I_i ���Ϊ s ת�Ƶ�״̬ I_j
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
	// while until ��Ŀ���岻������
	while (qwait.size()) {
		itemGroup ig_tmp = qwait.front();
		qwait.pop();
		// ��ȡ�հ�I_i
		itemGroup ret_ig = get_closer(ig_tmp);
		// 1.����ǿհ�
		if (ret_ig.item_group.size() == 0)
			continue;
		// 2.����ñհ��Ѿ�����
		if (this->Item_group_set.count(ret_ig)) {
			auto ig_exist = this->Item_group_set.find(ret_ig);
			auto it = ig_tmp.item_group.begin();
			// for ACTION case1/2
			Item_group_shift_mp[ig_tmp.id][(*it).prod.right[(*it).prod.dot_pos - 1]] = (*ig_exist).id;
			continue;
		}
		// 3.����ñհ�������
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
		// ��ȡ�±հ���vector<ת�����뼯>
		std::vector<itemGroup> goto_input = get_GOTO_input(ret_ig);
		// �������ʽ�ĵ���ѵ���β
		if (goto_input.size() == 0)
			continue;
		// ���д�ת�Ƶ�
		for (int i = 0; i < goto_input.size(); ++i) {
			qwait.push(goto_input[i]);
		}
	}
}

// ��ӡ��Ŀ����
void Parser::show_item_group()
{
	std::ofstream outfile;
	outfile.open(PS_ItemGroup_Path, std::ios::out | std::ios::binary);
	for (auto it1 = this->Item_group_set.begin(); it1 != this->Item_group_set.end(); ++it1) {
		//if ((*it1).id != 80 && (*it1).id != 44)continue;
		outfile << "I_" << (*it1).id << "��\n";
		itemGroup ig_tmp = (*it1);
		for (auto it2 = ig_tmp.item_group.begin(); it2 != ig_tmp.item_group.end(); ++it2) {
			outfile << (*it2).prod.left << "->";
			for (int i = 0; i < (*it2).prod.right.size(); ++i) {
				if (i == (*it2).prod.dot_pos) outfile << "��";
				outfile << (*it2).prod.right[i];
			}
			if ((*it2).prod.dot_pos == (*it2).prod.right.size())
				outfile << "��";
			outfile << ",lf=" << (*it2).lookforward << "\n";
		}
	}
	outfile.close();
}

// ��ӡDFA״̬ת�Ʊ�
void Parser::show_item_group_shift_mp()
{
	// std::vector<std::map<std::string, int>> Item_group_shift_mp;
	for (int i = 0; i < this->Item_group_shift_mp.size(); ++i) {
		std::cout << "State" << i << "��\n";
		if (this->Item_group_shift_mp[i].size() == 0) {
			std::cout << "Accepted State.\n";
			continue;
		}
		for (auto it = Item_group_shift_mp[i].begin(); it != Item_group_shift_mp[i].end(); ++it) {
			std::cout << (*it).first << "--" << (*it).second << "\n";
		}
	}
}

// ��ȡLR1������
void Parser::get_LR1_table()
{
	// ͨ�� Item_group_shift_mp ��ȡ LR1 ������
	this->LR1_table.resize(this->Item_group_shift_mp.size());
	// ��4�����
	// 1. [A->..��a..,b] && goto(I_i,a)=I_j -> ACTION[i,a]=sj
	// 2. [A->..��B..,b] && goto(I_i,B)=I_j -> GOTO[i,B]=gj
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
	// 3, [A->..��,a] && A!="Start" - ACTION[i,a]=rj
	// ��ʽ��Prod_acc_mp[i]map<lf, vector<j>> -> ACTION[j, lf] = i
	for (int i = 0; i < Prod_acc_mp.size(); ++i) {
		// ��ͬ�Ĳ���ʽ����չ����
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
	// 4. [Start->mp[Program]��,#] -> ACTION[i,#]=ax
	this->LR1_table[1]["#"] = { 'a',0 };
}

// ��ӡLR1������
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

// ʹ��LR1��������з���
bool Parser::Analyse(std::vector<token> tk)
{
	// ջ��״̬
	int cur_state = 0;
	// �﷨������״̬
	int als_state = PARSER_GOING;
	// ״̬ջ vector<int> StateStack;
	std::vector<int> StateStack;
	// ����ջ vector<string> SymbolStack;
	std::vector<std::string> SymbolStack;
	// �﷨���ڵ�ջ vector<*treenode> NodeStack
	std::vector<treenode*> NodeStack;
	// ������
	int StepCount = 0;
	// ���봮ָ�루����token[] tk��
	int p_tk = 0;
	// ��ʼ��
	StateStack.push_back(cur_state);
	SymbolStack.push_back("#");
	// �������
	std::ofstream outfile;
	outfile.open(PS_Rignt_Path, std::ios::out | std::ios::binary);
	// ����
	while (als_state == 0 && p_tk < tk.size()) {
		StepCount++;
		// file - ���ջ����̬
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
		// file - ���ջ����̬
		// ״̬ջ��
		cur_state = StateStack[StateStack.size() - 1];
		token tk_tmp = tk[p_tk];
		// ���봮��Ԫ�ش���
		std::string tk_tmp_str;
		std::string tk_tmp_str_val;
		if (tk_tmp.token_type == "identifier")
			tk_tmp_str = this->Grammar_symbol_mp["ID"];
		else if (tk_tmp.token_type == "digit")
			tk_tmp_str = this->Grammar_symbol_mp["num"];
		else
			tk_tmp_str = tk_tmp.token_type;
		tk_tmp_str_val = tk_tmp.token_value;
		// �ж�err
		if (!this->LR1_table[cur_state].count(tk_tmp_str)) {
			// err
			als_state = ERROR_SHIFT;
			break;
		}
		// �ƽ�/��Լ/goto/acc
		char action = this->LR1_table[cur_state][tk_tmp_str].first;
		int nxt_state = this->LR1_table[cur_state][tk_tmp_str].second;
		// file - action  eg.s1/r2...
		outfile << "action:" << action << nxt_state << "\n";
		// file
		// 1.�ƽ�
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
		// 2.��Լ
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
			p->type = "Argument"; // ͬʱ���Ա�־�Ƿ񵽴�leaf
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
			// ��Լ��Ҫ����һ��goto
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
		// 3.goto Ӧ�ò���ִ�е�����Ϊgoto�����ڹ�Լ������
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
		std::cout << "�﷨�����ɹ�\n";
		outfile << "Parsing Success!\n";
		outfile.close();
		return true;
	}
	std::cout << "�﷨��������\n";
	std::cout << "�������:" << als_state << "\n";
	std::cout << "����λ��:(" << tk[p_tk].token_x << "," << tk[p_tk].token_y << ")\n";
	outfile.close();
	// file
	outfile.open(PS_Wrong_Path, std::ios::out | std::ios::binary);
	outfile << "Parsing Failed!\n";
	outfile << "Return code:" << als_state << "\n";
	outfile << "Wrong place:(" << tk[p_tk].token_x << "," << tk[p_tk].token_y << ")\n";
	outfile.close();
	return false;
}

// �����﷨��
void Parser::show_tree(treenode* root, int depth)
{
	// �������
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
// ������Ԫʽ������� this->root.code��
void Parser::emit(int is_cmd_out)
{
	std::ofstream outfile;
	outfile.open(SM_QUADRUPLE_Path, std::ios::out | std::ios::binary);
	// ���� - ����������ֵ
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
// ������Ԫʽ
std::pair<int, Quadruple> Parser::gen(std::string _op, std::string _arg1, std::string _arg2, std::string _result)
{
	return std::make_pair(this->nxtquad++, Quadruple(_op, _arg1, _arg2, _result));
}

// dfs�﷨��������Ԫʽ�����������
int Parser::dfs(treenode* root)
{
	// ��ʼ�����ű�
	if (this->tableSet.empty()) {
		this->mk_table();
	}
	// ���ս����Ҷ�ӽڵ�
	if (root->type != "Argument")
	{
		root->place = root->value;
		root->code.clear();
		return SEMANTIC_RIGHT;// ������ȷ define
	}
	// �����ս����child.size>0
	if (root->value == this->Grammar_symbol_mp["��������"] || root->child[0]->value == "{") {
		this->mk_table();// ���ű�����
	}

	// "����"����Բ����á���������"��������"�� ���õĺ����Ȳ�dfs
	if (root->value == this->Grammar_symbol_mp["����"] && root->child[1]->value != "main"
		&& root->child[2]->value == this->Grammar_symbol_mp["��������"]) {
		root->place = root->child[1]->place;
		// �����Ƿ��Ѷ���
		int is_def = 0;
		for (int i = 0; i < this->func.size(); ++i) {
			if (this->func[i].name == root->child[1]->value) { is_def = 1; break; }
		}
		if (is_def) {
			std::cout << "�����ض��壺" << root->child[1]->value << std::endl;
			std::cout << "λ�ã�(" << root->child[1]->pos_x << "," << root->child[1]->pos_y << ")\n";
			return ERROR_FUNC_REPEAT; // �����ض��� define
		}
		this->add_argument(root->child[1]->value);
		funcInfo tmp;
		tmp.name = root->child[1]->value;
		// ȡ����������code
		this->func.push_back(tmp);
		this->funcptr.push_back(root->child[2]); // "��������"
		return SEMANTIC_RIGHT; // ��������Ĳ����Ȳ�dfs
	}

	// "����"��ȫ�����飩- new1 
	// ע��new2/3 �����ڴ�����������Ϣ�ѻ�ȡ���ռ�������arrayinfo�У��˺�����dfs�˲���
	if (root->value == this->Grammar_symbol_mp["����"] &&
		root->child[2]->value == this->Grammar_symbol_mp["��������"]) {
		root->place = root->child[1]->value;// ������id��child[1]
		// �����Ƿ��Ѷ���
		bool is_def = this->isArrayExist(root->child[1]->value);
		if (is_def) {
			std::cout << "�����ض��壺" << root->child[1]->value << std::endl;
			std::cout << "λ�ã�(" << root->child[1]->pos_x << "," << root->child[1]->pos_y << ")\n";
			return ERROR_ARRAY_REPEAT; // �����ض��� 
		}
		// ��ȡ������Ϣ
		std::vector<int>arr_info = this->get_arr_sz(root->child[2]);
		// ��������Ƿ����
		if (!arr_check(arr_info)) {
			std::cout << "���鶨�岻����" << root->child[1]->value << std::endl;
			std::cout << "λ�ã�(" << root->child[1]->pos_x << "," << root->child[1]->pos_y << ")\n";
			return ERROR_BAD_ARRAY; // ���鶨�岻����
		}
		// ��չ������ű�
		arrayInfo arrayinfo_item(arr_info);
		arrayinfo_item.name = root->child[1]->value;
		this->arrayinfo.push_back(arrayinfo_item);
		// ������� - ��������ռ�
		int space_size = 4;
		for (int i = 0; i < arr_info.size(); ++i) { space_size *= arr_info[i]; }
		root->code.push_back(this->gen("dec", std::to_string(space_size), "-", root->place));
		// û�м���dfs�ı�Ҫ ����
		return SEMANTIC_RIGHT;
	}

	// "����"������ķ��ʣ� new67(89)
	// root->place
	if (root->value == this->Grammar_symbol_mp["����"]) {
		std::string arg = root->child[0]->value;
		// ��������Ƿ���
		int is_def = -1;
		for (int i = 0; i < this->arrayinfo.size(); ++i) {
			if (this->arrayinfo[i].name == arg) { is_def = i; break; }
		}
		if (is_def == -1) {
			std::cout << "����δ���壺" << arg << std::endl;
			std::cout << "λ�ã�(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
			return ERROR_ARRAY_NOT_FOUND; // ����δ����
		}
		// ��������
		std::vector<int> arr_vis_info = this->get_arr_idx(root);
		int pos = this->get_arr_idx_pos(this->arrayinfo[is_def], arr_vis_info);
		// �����ʺ�����
		if (pos == -1) {
			std::cout << "���������������" << arg << std::endl;
			std::cout << "λ�ã�(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
			return ERROR_ARRAY_DIM; // ���ʺͶ���ά����һ��
		}
		else if (pos == -2) {
			std::cout << "�������Խ�磺" << arg << std::endl;
			std::cout << "λ�ã�(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
			return ERROR_ARRAY_OVERFLOW; // �������Խ��
		}
		root->place = this->arrayinfo[is_def].name + "[" + std::to_string(pos) + "]";
		// û�м���dfs�ı�Ҫ ����
		return SEMANTIC_RIGHT;
	}

	// �ȱ����ӽڵ�
	for (int i = 0; i < root->child.size(); ++i) {
		int res = dfs(root->child[i]);
		// ��������
		if (res < 0) {
			if (root->value == this->Grammar_symbol_mp["��������"] || root->child.back()->value == "}")
				this->rm_table();// ���ű���С
			return res;
		}
	}

	// ��Ԫ�ڵ㣨�ӽڵ�� place/code �����У�
	// default
	root->place = root->child[0]->place;
	root->code.clear();
	for (int i = 0; i < root->child.size(); ++i) {
		for (int j = 0; j < root->child[i]->code.size(); ++j) {
			root->code.push_back(root->child[i]->code[j]);
		}
	}

	// ö���﷨����дroot��place/�����code
	// "��ֵ���"
	if (root->value == this->Grammar_symbol_mp["��ֵ���"]) {
		// id = number
		if (root->child[0]->type == this->Grammar_symbol_mp["ID"]) {
			std::string arg = root->child[0]->value;
			if (this->lookup(arg)) {
				root->place = root->child[2]->place; //�ۺ�
				if (root->child[2]->place[root->child[2]->place.size() - 1] == ']')// new4
					root->code.push_back(this->gen("=[]", root->child[2]->place, "-", root->child[0]->place));
				else
					root->code.push_back(this->gen(":=", root->child[2]->place, "-", root->child[0]->place));
			}
			else {
				std::cout << "����δ������" << arg << std::endl;
				std::cout << "λ�ã�(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
				return ERROR_ARGU_NOT_FOUND;// ����δ����
			}
		}
		// arr = number - new4
		// ��ַ���� - (:=,xxx,-,T),([]=,X1,-,T1[T]) - �൱��T1[T]:=X
		else if (root->child[0]->value == this->Grammar_symbol_mp["����"]) {
			std::string arg = root->child[0]->child[0]->value;
			// ��������Ƿ���
			int is_def = -1;
			for (int i = 0; i < this->arrayinfo.size(); ++i) {
				if (this->arrayinfo[i].name == arg) { is_def = i; break; }
			}
			if (is_def == -1) {
				std::cout << "����δ���壺" << arg << std::endl;
				std::cout << "λ�ã�(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
				return ERROR_ARRAY_NOT_FOUND; // ����δ����
			}
			else {
				std::vector<int> arr_vis_info = this->get_arr_idx(root->child[0]);
				int pos = this->get_arr_idx_pos(this->arrayinfo[is_def], arr_vis_info);
				// �����ʺ�����
				if (pos == -1) {
					std::cout << "���������������" << arg << std::endl;
					std::cout << "λ�ã�(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
					return ERROR_ARRAY_DIM; // ���ʺͶ���ά����һ��
				}
				else if (pos == -2) {
					std::cout << "�������Խ�磺" << arg << std::endl;
					std::cout << "λ�ã�(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")" << std::endl;
					return ERROR_ARRAY_OVERFLOW; // �������Խ��
				}
				// ������Ϣ�� - ��ֵ
				//this->arrayinfo[is_def].p_arr[pos] = 
				// ������ֵ��Ԫʽ
				root->place = root->child[0]->child[0]->place; //�ۺ� ��������place
				root->code.push_back(this->gen("[]=", root->child[2]->place, "-", root->child[0]->place));
			}
		}
	}
	// "Program"
	else if (root->value == this->Grammar_symbol_mp["Program"]) {
		// ���main
		if (root->child.size() == 1) {
			int is_def = 0;
			for (int i = 0; i < func.size(); ++i) {
				if (func[i].name == "main") { is_def = 1; break; }
			}
			if (is_def == 0)
				return ERROR_MAIN_NOT_FOUND;
		}
	}
	// "����"
	else if (root->value == this->Grammar_symbol_mp["����"]) {
	}
	// "�β�"
	else if (root->value == this->Grammar_symbol_mp["�β�"]) {
		root->para.clear();
		if (root->child[0]->value != "void") {
			for (int i = 0; i < root->child[0]->para.size(); ++i) {
				root->para.push_back(root->child[0]->para[i]);
			}
		}
	}
	// "�����б�"
	else if (root->value == this->Grammar_symbol_mp["�����б�"]) {
		root->para.clear();
		root->para.push_back(root->child[0]->para[0]);
		if (root->child.size() > 1) {
			for (int i = 0; i < root->child[2]->para.size(); ++i) {
				root->para.push_back(root->child[2]->para[i]);
			}
		}
	}
	// "����"
	else if (root->value == this->Grammar_symbol_mp["����"]) {
		root->place = root->child[1]->place;
		this->add_argument(root->child[1]->value);
		root->para.clear();
		root->para.push_back(root->child[1]->value);
	}
	// "����" size==3/4
	else if (root->value == this->Grammar_symbol_mp["����"]) {
		root->place = root->child[root->child.size() - 2]->place;
	}
	// "�ڲ�����" size==2/3
	else if (root->value == this->Grammar_symbol_mp["�ڲ�����"]) {
	}
	// "�ڲ���������" eg. int a
	else if (root->value == this->Grammar_symbol_mp["�ڲ���������"]) {
		std::string arg = root->child[1]->place;
		if (this->lookup(arg)) {
			std::cout << "�����ض��壺" << arg << std::endl;
			std::cout << "λ�ã�(" << root->child[1]->pos_x << "," << root->child[1]->pos_y << ")" << std::endl;
			return ERROR_ARGU_REPEAT; // �����ض��� define
		}
		this->add_argument(arg);
	}
	// "��䴮" size==1/2
	else if (root->value == this->Grammar_symbol_mp["��䴮"]) {
		if (root->child.size() == 2) {
			root->place = root->child[1]->place;
		}
	}
	// "���" type==4
	else if (root->value == this->Grammar_symbol_mp["���"]) {
	}
	// "return���"
	else if (root->value == this->Grammar_symbol_mp["return���"]) {
		if (root->child.size() == 3) {
			root->place = root->child[1]->place;
			root->code.push_back(this->gen(":=", root->child[1]->place, "-", "Func_Name"));
		}
	}
	// "while���"
	else if (root->value == this->Grammar_symbol_mp["while���"]) {
		root->code.clear();
		for (auto& it : root->child[2]->code) // ���ʽ
		{
			root->code.push_back(it); // ��ϵ���ʽ��"j<,a,b,c"/"j,-,-,d"��������
		}
		for (auto& it : root->child[4]->code) // ����
		{
			root->code.push_back(it);
		}
		// �ӿ���Ԫʽ����ȷ��������ֱ���� ��ϵ���ʽ��while��(j,-,-,xxx)��xxx
		root->code.push_back(gen("j", "-", "-", std::to_string(root->code[0].first)));
		for (auto& it : root->code) {
			//it.first == root->child[2]->false_flag
			if (it.second.result == "-1") {
				it.second.result = std::to_string(root->code.back().first + 1);// ��ϵ���ʽbreak(j,-,-,xxx)
				break;
			}
		}
	}
	// "if���" size==5/7
	else if (root->value == this->Grammar_symbol_mp["if���"]) {
		root->code.clear();
		for (auto& it : root->child[2]->code) // ���ʽ
		{
			root->code.push_back(it); // ��ϵ���ʽ��"j<,a,b,c"/"j,-,-,d"��������
		}
		for (auto& it : root->child[4]->code) // ����
		{
			root->code.push_back(it);
		}

		if (root->child.size() == 7) {
			// ��elseʱif��ִ֧����֮����ת
			root->code.push_back(this->gen("j", "-", "-", std::to_string(root->code.back().first + root->child[6]->code.size() + 2)));
			root->code.back().first -= (int)root->child[6]->code.size();
			// else��֧���븴�Ƹ�root->code
			for (auto& it : root->child[6]->code) {
				it.first++; // ǰ�����һ��if�����������ת
				if (it.second.op[0] == 'j') // ֻҪ����תָ��
				{
					int num = atoi(it.second.result.c_str());
					it.second.result = std::to_string(num + 1);
				}
				root->code.push_back(it);
			}
		}
		// �ٳ���
		for (auto& it : root->code) {
			// it.first == root->child[2]->false_flag
			if (it.second.result == "-1") {
				if (root->child.size() == 7) {
					// ��תelse�Ĳ���
					it.second.result = std::to_string(root->code.back().first - root->child[6]->code.size() + 1);
				}
				else
					it.second.result = std::to_string(root->code.back().first + 1);// ��ת�����Ĳ���
				break;
			}
		}
	}
	// "���ʽ"
	else if (root->value == this->Grammar_symbol_mp["���ʽ"]) {
		if (root->child.size() == 1 || root->child[0]->value == "(") {
			root->place = root->child[int(root->child[0]->value == "(")]->place;
		}
		// "���ʽ" -> "�ӷ����ʽ" "relop" "���ʽ"
		else {
			root->place = this->newtmp();
			treenode* t1 = root->child[0];
			treenode* t2 = root->child[2];
			root->code.push_back(this->gen("j" + root->child[1]->place, t1->place, t2->place, "0"));// ���֧
			root->code.back().second.result = std::to_string(root->code.back().first + 3);
			root->code.push_back(this->gen(":=", "0", "-", root->place));
			root->code.push_back(this->gen("j", "-", "-", "-1"));// �ٷ�֧
			root->code.push_back(this->gen(":=", "1", "-", root->place));
		}
	}
	// "relop"
	else if (root->value == this->Grammar_symbol_mp["relop"]) {

	}
	// "�ӷ����ʽ"
	else if (root->value == this->Grammar_symbol_mp["�ӷ����ʽ"]) {
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
	// "��"
	else if (root->value == this->Grammar_symbol_mp["��"]) {
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
	// "����"
	else if (root->value == this->Grammar_symbol_mp["����"]) {
		if (root->child.size() == 1) {
			// num / ID / ���� new5
			root->place = root->child[0]->place;
		}
		else {
			// ���ʽ
			root->place = root->child[1]->place;
			// ��������˺���
			if (root->child[1]->value == this->Grammar_symbol_mp["call"]) {
				// �����Ƿ��Ѷ���
				int is_def = -1;
				for (int i = 0; i < this->func.size(); ++i) {
					if (this->func[i].name == root->child[0]->value) { is_def = i; break; }
				}
				if (is_def == -1) {
					std::cout << "����δ���壺" << root->child[0]->value << std::endl;
					std::cout << "λ�ã�(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")\n";
					return ERROR_FUNC_NOT_FOUND; // ����δ���� define
				}
				// ID
				root->place = root->child[0]->place;// ����.place = ID.place ���Ժ�������ֵ�������Ǻ�����
				// ʵ��
				std::vector<std::string> real_para;
				for (int i = 0; i < root->child[1]->para.size(); ++i) {
					real_para.push_back(root->child[1]->para[i]);
				}
				// �β�
				std::vector<std::string> form_para = this->get_formpara(this->funcptr[is_def]);
				// ����������
				if (form_para.size() != real_para.size()) {
					std::cout << "��������������ƥ��\n";
					std::cout << "λ�ã�(" << root->child[0]->pos_x << "," << root->child[0]->pos_y << ")\n";
					return ERROR_FUNC_PARA_SIZE;// ��������������ƥ��
				}
				// �޸Ĳ���
				this->fixPara(this->funcptr[is_def], real_para, form_para);
				// dfs �����õĺ���
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
	// "����"�����"��������"�͵��õ�"��������"��
	// ���õ�"��������"ֻ��mian�ģ�����Ķ�ֻ�ڱ�call��ʱ��ŵ���
	else if (root->value == this->Grammar_symbol_mp["����"]) {
		root->place = root->child[1]->place;
		if (root->child[2]->value == Grammar_symbol_mp["��������"]) {
			// �����Ƿ��Ѷ���
			int is_def = 0;
			for (int i = 0; i < this->func.size(); ++i) {
				if (this->func[i].name == root->child[1]->value) { is_def = 1; break; }
			}
			if (is_def) {
				std::cout << "�����ض��壺" << root->child[1]->value << std::endl;
				std::cout << "λ�ã�(" << root->child[1]->pos_x << "," << root->child[1]->pos_y << ")\n";
				return ERROR_FUNC_REPEAT; // �����ض��� define
			}
			this->add_argument(root->child[1]->value);
			funcInfo tmp;
			tmp.name = root->child[1]->value;
			// ȡ����������code
			this->func.push_back(tmp);
			this->funcptr.push_back(root->child[2]); // "��������"
		}
	}
	// "��������"
	else if (root->value == this->Grammar_symbol_mp["��������"]) {
		root->place = root->child[3]->place; // "����"��place
	}
	// "������"
	else if (root->value == this->Grammar_symbol_mp["������"]) {
		root->place = root->child[0]->place;
		// �й��̵��� - ֻҪmain��code
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
	// "ʵ���б�"
	else if (root->value == this->Grammar_symbol_mp["ʵ���б�"]) {
		root->para.push_back(root->child[0]->place);
		if (root->child.size() > 1) {
			for (int i = 0; i < root->child[2]->para.size(); ++i) {
				root->para.push_back(root->child[2]->para[i]);
			}
		}
	}

	// to append...

	if (root->value == this->Grammar_symbol_mp["��������"] || root->child.back()->value == "}")
		this->rm_table();// ���ű���С

	return SEMANTIC_RIGHT;// Ĭ����ȷ
}

// ����22
// ��ȡ�β�(root == "��������")
std::vector<std::string> Parser::get_formpara(treenode* root)
{
	std::vector<std::string> res;
	root = root->child[1]; // "�β�"
	if (root->child[0]->value == "void")
		return res;
	dfs(root); // "�����б�"
	return root->child[0]->para;
}

// ����23
// �޸��β�(root == "��������")
bool Parser::fixPara(treenode* root, std::vector<std::string> real_para, std::vector<std::string> form_para)
{
	int cnt = 0;
	if (root == nullptr) {
		return false;
	}

	if (root->type == "identifier") {
		// �� form_para �в��Ҷ�Ӧ���β�
		auto it = std::find(form_para.begin(), form_para.end(), root->value);

		if (it != form_para.end()) {
			// �ҵ���Ӧ���βΣ��滻Ϊʵ��
			root->value = real_para[it - form_para.begin()];
			// ��¼�滻����
			cnt++;
		}
	}
	// �ݹ鴦���ӽڵ�
	for (treenode* child : root->child) {
		fixPara(child, real_para, form_para);
	}
	return true;
}

/*
	����24��isArrayExist
	�鿴���������Ƿ�����
*/
bool Parser::isArrayExist(std::string s)
{
	for (int i = 0; i < this->arrayinfo.size(); ++i) {
		if (s == arrayinfo[i].name)return true;
	}
	return false;
}

// ����������㺯����������
bool Parser::semanticAnalyse() {
	int res = this->dfs(this->gra_root);
	if (res == SEMANTIC_RIGHT) {
		std::cout << "��������ɹ�\n";
		this->emit(IS_SENMANTIC_CMD_SHOW);
		return true;
	}
	else if (res == ERROR_ARGU_NOT_FOUND) {
		std::cout << "����δ���壡\n";
	}
	else if (res == ERROR_ARGU_REPEAT) {
		std::cout << "�����ض��壡\n";
	}
	else if (res == ERROR_FUNC_NOT_FOUND) {
		std::cout << "����δ���壡\n";
	}
	else if (res == ERROR_FUNC_REPEAT) {
		std::cout << "�����ض��壡\n";
	}
	else if (res == ERROR_MAIN_NOT_FOUND) {
		std::cout << "�Ҳ���main������\n";
	}
	else if (res == ERROR_FUNC_PARA_SIZE) {
		std::cout << "ʵ�θ�������\n";
	}
	// to append
	else if (res == ERROR_ARRAY_REPEAT) {
		std::cout << "�����ض��壡\n";
	}
	else if (res == ERROR_BAD_ARRAY) {
		std::cout << "���鶨�岻����\n";
	}
	else if (res == ERROR_ARRAY_NOT_FOUND) {
		std::cout << "������δ�壡\n";
	}
	else if (res == ERROR_ARRAY_DIM) {
		std::cout << "��Ԥ�ڵ�������ʣ�\n";
	}
	else if (res == ERROR_ARRAY_OVERFLOW) {
		std::cout << "������������\n";
	}

	return false;
}

/* END - class Parser - ����ʵ�� */

