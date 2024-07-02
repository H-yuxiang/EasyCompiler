#pragma once
#include "common.h"

/*
	�ࣺ�﷨���� - Parser
*/
class Parser
{
public:
	Parser();
	~Parser();

	/*
		����1�����ر�Ԫӳ���
		return:	map<string, string>
	*/
	void load_argument_def();

	/*
		����2�����س�ʼ�ķ�������չ��
		return:	vector<production>
	*/
	void load_production();
	void show_production();

	/*
		����3����ȡ������Ԫ��FIRST��
		return: map<string, vector<string>>
	*/
	std::vector<std::string> get_single_first(std::string argument);

	/*
		����4����ȡ���б�Ԫ��FIRST��
		return: map<string, vector<string>>
	*/
	void get_first();
	void show_first();

	/*
		����5����ȡһ����Ŀ�ıհ�
		return: ����Ŀ�ıհ����ɵ���Ŀ��
	*/
	itemGroup get_single_closer(item im);
	void show_closure();

	/*
		����6����ȡ get_closer ������ - get_item_group seris 1
		input: һ����Ŀ��
		return: һ��<�����ɱհ�����Ŀ>�ļ���
	*/
	std::vector<itemGroup> get_GOTO_input(const itemGroup& in_ig);
	void show_GOTO_input(std::vector<itemGroup> ig_v);

	/*
		����7����ȡ��Ŀ���� closer - get_item_group seris 2
		return: I_k - һ�����ڹ淶�����Ŀ��
	*/
	itemGroup get_closer(const itemGroup& in_ig);

	/*
		����8����ȡ��Ŀ���淶�壬����洢�� std::set<itemGroup> Item_group_set;
	*/
	void get_item_group();
	void show_item_group();
	void show_item_group_shift_mp();

	/*
		����9����ȡLR1����������洢�� LR1_table
	*/
	void get_LR1_table();
	void show_LR1_table();

	/*
		����10��ʹ��LR1��������з���
		return: �����Ƿ�ɹ�
	*/
	bool Analyse(std::vector<token> tk);

	/*
		����11����������﷨��
		return:
		ע�⣺��ǰʵ����dfs��������ֱ��������ļ�
	*/
	void show_tree(treenode* root, int depth = 0);

	/*
		����12����ȡ�﷨��
		return: this->root
	*/
	treenode* get_tree() { std::cout << "��ȡ�﷨��!\n"; return this->gra_root; }

	/*
		����13�������ж��Ƿ����ս��
		return: bool
	*/
	bool isTerminal(std::string s) {
		return (!Argument_type.count(s) || Argument_type[s] == TERMINAL);
	}

	/*
		����14����Ԫʽ
		return:
	*/
	void emit(int is_cmd_out = 1);
	std::pair<int, Quadruple> gen(std::string _op, std::string _arg1, std::string _arg2, std::string _result);

	/*
		����15������һ���µ���ʱ����
		return: std::string
	*/
	std::string newtmp() {
		return "T" + std::to_string(this->tmp_number++);
	}

	/*
		����16��Ѱ�ұ����Ƿ���ֹ���todo��
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
		����17��������ű�
		return:
	*/
	void mk_table() {
		std::vector<std::string> res(1, "");
		this->tableSet.push_back(res);
	}

	/*
		����18������±���
		return:
	*/
	void add_argument(std::string s) {
		this->tableSet.back().push_back(s);
	}

	/*
		����19��ɾ�����ű�
		return:
	*/
	void rm_table() {
		if (this->tableSet.size())
			this->tableSet.pop_back();
	}

	/*
		����20��dfs
		return: dfs��״̬
	*/
	int dfs(treenode* root);

	/*
		����21��semanticAnalyse
		return:
	*/
	bool semanticAnalyse();
	std::vector<std::pair<int, Quadruple>> get_Q_ruple() { return this->Q_ruple; }

	/*
		����22��get_formpara
		�ú������﷨������Լ
		��ȡĳ���������β��б�
	*/
	std::vector<std::string> get_formpara(treenode* root);

	/*
		����23��fixPara
		�޸�ĳ���������β��б����ʵ�ֹ����е��β�
	*/
	bool fixPara(treenode* root, std::vector<std::string> real_para, std::vector<std::string> form_para);

	/*
		����24��isArrayExist
		�鿴���������Ƿ�����
	*/
	bool isArrayExist(std::string s);

	// ���ߺ���1 - ����ʱ����������Լ�� ������1
	bool arr_check(std::vector<int> arr_sz) {
		long long total = 1;
		for (int i = 0; i < arr_sz.size(); ++i) {
			if (arr_sz[i] >= ARRAY_SIZE_MAX)return false;
			total *= arr_sz[i];
			if (total >= ARRAY_SIZE_MAX)return false;
		}
		return true;
	}

	// ���ߺ���2 - ����ʱ����������λ�ü���
	// return	-1 - ά����һ��
	//			-2 - ĳһάԽ��
	int get_arr_idx_pos(arrayInfo inf, std::vector<int> arr_idx) {
		if (inf.arr_sz.size() != arr_idx.size()) 
		{
			// std::cout << inf.arr_sz.size() << "-" << arr_idx.size() << std::endl;
			return -1;// �ߴ����
		}
		int res = 0, total = 1;
		// ��ȡ���������С
		std::vector<int> blk_sz;
		for (int i = 0; i < inf.dim; ++i) total *= inf.arr_sz[i];
		for (int i = 0; i < inf.dim - 1; ++i) {
			total /= inf.arr_sz[i];
			blk_sz.push_back(total);
		}
		blk_sz.push_back(1);
		// ӳ��
		for (int i = 0; i < inf.dim; ++i) {
			if (arr_idx[i] < 0 || arr_idx[i] >= inf.arr_sz[i])return -2;// ����Խ��
			res += arr_idx[i] * blk_sz[i];
		}
		return res;
	}

	// ���ߺ���3 - ����ʱ����ȡ�����ά����ÿһά�Ĵ�С
	std::vector<int> get_arr_sz(treenode* root) {
		std::vector<int> res;
		if (root->value != "array_declaration")// ����
		{
			std::cout << "��Ԥ����������Ľڵ�!" << std::endl;
			return res;
		}
		treenode* p = root;
		while (p->value != ";") {
			if (p->child[0]->value != "[")// ��
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

	// ���ߺ���4 - ����ʱ����ȡ����ÿһά������
	std::vector<int> get_arr_idx(treenode* root) {
		std::vector<int> res;
		if (root->value != "array")// ����
		{
			std::cout << "��Ԥ�ڷ�������Ľڵ�!" << std::endl;
			return res;
		}
		treenode* p = root;
		while (p->value != "]") {
			if (p->child[0]->value != "[")// ��
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
	// 1.��Ԫӳ��������ƣ�һһ��Ӧ��
	std::map<std::string, std::string> Grammar_symbol_mp;
	// 2."��Ԫ-�ķ�"ӳ��� - �ñ�ԪΪ�󲿵����в���ʽ�������� load_production ����
	//	  				   - �������� Grammar_symbol_mp[...] ӳ��������
	std::map<std::string, std::vector<int>> Argument_garmmar_mp;
	// 3.��Ԫ��������
	// Tips���ж� X �Ƿ����ս�� -> !Argument_type.count(X) || Argument_type[X] == TERMINAL
	std::unordered_map<std::string, int> Argument_type;
	// 4.��ʼ�ķ�������չ��
	std::vector<production> Production_raw;
	// 5.���б�Ԫ��FIRST��
	std::map<std::string, std::vector<std::string>> First_set;

	// ��Ŀ����
	// 1.��Ŀ������
	int Item_group_num;
	// 2.[I_i-(s)->I_j] - ״̬I_i���Ϊs��ת�Ƶ�״̬I_j - ������
	std::vector<std::map<std::string, int>> Item_group_shift_mp;
	// 3.��Ŀ����set
	std::set<itemGroup> Item_group_set;
	// 4.����ʽ��ԼACTION��������ݣ����ڲ���ʽ�Ҳ������ - ����(size)=����ʽ�ĸ���
	// ��¼�ĸ�ʽ��Prod_acc_mp[j]map<lf,vector<I_i>> -> ACTION[i,a]=j
	std::vector<std::map<std::string, std::vector<int>>> Prod_acc_mp;

	// LR(1)������
	// ACTION��GOTOͨ����������
	// ����pair<s(�ƽ�)/r(��Լ)/a(accept)/g(goto), int(next state)>
	std::vector< std::map<std::string, std::pair<char, int>> > LR1_table;

	// �﷨��
	treenode* gra_root;
	// ���ű�
	std::vector<std::vector<std::string>> tableSet;
	// ������Ϣ
	std::vector<funcInfo> func;
	std::vector<treenode*> funcptr;
	// ��ʱ�������
	int tmp_number;
	// ��Ԫʽ���
	int nxtquad;
	// ��¼��Ԫʽ����
	std::vector<std::pair<int, Quadruple>> Q_ruple;

	// ��������������Ϣ
	std::vector<arrayInfo> arrayinfo;
};

