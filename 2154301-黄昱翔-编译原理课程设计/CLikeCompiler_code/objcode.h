#pragma once
#include "common.h"

// �� - Objcode Ŀ�����(mips)����
// ͨ����Ԫʽ����Ŀ�����
class Objcode {
	// ��Ԫʽ����
	std::vector<std::pair<int, Quadruple>> qruple;
	// ����1 - ��ȡDAG�������������
	void get_dag_entrance();
	// DAG����
	std::vector<int> dagEntrance;
	std::vector<DAGinfo> daginfo;

	// ��������ʱ����
	std::vector<VarInfo> varinfo;
	// �Ĵ������䣨��16�� $2-$17 ��������ռ䣩
	std::string RVALUE[REG_SIZE];
	// �����ռ���䣨��Ӧ�ļĴ�����ţ�
	std::map<std::string, int> AVALUE;
	// ��ǩ�������
	int label_num;
	// ����2 - ����Ƿ��Ǳ���
	bool is_var(std::string s);
	// ��ǩ-DAG(idx) ӳ���
	std::map<std::string, std::string> Label_DAG_mp;
	// ����3 - ��ȡ������
	std::string get_arr_name(std::string s);
	// ����4 - ��ȡ����ƫ��
	std::string get_arr_offset(std::string s);

public:
	// ���캯��
	Objcode() { this->label_num = 0; for (int i = 0; i < REG_SIZE; ++i) { RVALUE[i] = ""; } }
	// 1. ������Ԫʽ
	void get_qruple(std::vector<std::pair<int, Quadruple>> semantic_result);
	// 2. �����Ԫʽ�����ԣ�
	void show_qruple();
	// 3. ��������DAG
	// ע���������Ӧ��һ��ֻ�б�ǩ��DAG
	void get_dag();
	// 4. ���DAG��Ϣ�����ԣ�
	void show_dag();
	// 5. Ŀ���������
	int get_objcode();
};
