#pragma once
#include "common.h"

// 类 - Objcode 目标代码(mips)生成
// 通过四元式生成目标代码
class Objcode {
	// 四元式序列
	std::vector<std::pair<int, Quadruple>> qruple;
	// 函数1 - 获取DAG基本块的入口语句
	void get_dag_entrance();
	// DAG序列
	std::vector<int> dagEntrance;
	std::vector<DAGinfo> daginfo;

	// 变量销毁时机表
	std::vector<VarInfo> varinfo;
	// 寄存器分配（限16个 $2-$17 不含数组空间）
	std::string RVALUE[REG_SIZE];
	// 变量空间分配（对应的寄存器编号）
	std::map<std::string, int> AVALUE;
	// 标签管理相关
	int label_num;
	// 函数2 - 检查是否是变量
	bool is_var(std::string s);
	// 标签-DAG(idx) 映射表
	std::map<std::string, std::string> Label_DAG_mp;
	// 函数3 - 获取数组名
	std::string get_arr_name(std::string s);
	// 函数4 - 获取数组偏移
	std::string get_arr_offset(std::string s);

public:
	// 构造函数
	Objcode() { this->label_num = 0; for (int i = 0; i < REG_SIZE; ++i) { RVALUE[i] = ""; } }
	// 1. 接受四元式
	void get_qruple(std::vector<std::pair<int, Quadruple>> semantic_result);
	// 2. 输出四元式（调试）
	void show_qruple();
	// 3. 划分语句块DAG
	// 注：程序出口应有一个只有标签的DAG
	void get_dag();
	// 4. 输出DAG信息（调试）
	void show_dag();
	// 5. 目标代码生成
	int get_objcode();
};
