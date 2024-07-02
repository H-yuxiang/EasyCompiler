#include "tool.h"
#include "objcode.h"

// 类 - Objcode 函数实现
// 1. 接受四元式
void Objcode::get_qruple(std::vector<std::pair<int, Quadruple>> semantic_result)
{
	for (int i = 0; i < semantic_result.size(); ++i) {
		this->qruple.push_back(semantic_result[i]);
	}
}

// 2. 输出四元式（调试）
void Objcode::show_qruple()
{
	for (int i = 0; i < this->qruple.size(); ++i) {
		std::cout << this->qruple[i].first << " (";
		std::cout << this->qruple[i].second.op << ",";
		std::cout << this->qruple[i].second.arg1 << ",";
		std::cout << this->qruple[i].second.arg2 << ",";
		std::cout << this->qruple[i].second.result << ")\n";
	}
}

// 3. 划分语句块DAG
void Objcode::get_dag()
{
	this->get_dag_entrance();
	for (int i = 0; i < this->dagEntrance.size(); ++i) {
		DAGinfo daginfo_item;
		// idx
		daginfo_item.sp = this->dagEntrance[i];
		if (i == this->dagEntrance.size() - 1) {
			daginfo_item.ep = QUAD_SATRT + (int)this->qruple.size() - 1;
		}
		else {
			daginfo_item.ep = this->dagEntrance[i + 1] - 1;
		}
		// j to ...
		daginfo_item.fal_idx = daginfo_item.ep + 1;
		if (this->qruple[daginfo_item.ep - QUAD_SATRT].second.op[0] == 'j') {
			daginfo_item.suc_idx = std::stoi(this->qruple[daginfo_item.ep - QUAD_SATRT].second.result);
		}
		else {
			daginfo_item.suc_idx = daginfo_item.ep + 1;
		}
		this->daginfo.push_back(daginfo_item);
	}
}

// 4. 输出DAG信息（调试）
void Objcode::show_dag()
{
	for (int i = 0; i < this->daginfo.size(); ++i) {
		std::cout << this->daginfo[i].sp << "-" << this->daginfo[i].ep << "-" 
			<< this->daginfo[i].suc_idx << "-" << this->daginfo[i].fal_idx << "\n";
	}
}

// 5. 目标代码生成
// 以DAG块为单位，采用待用、活跃信息表
int Objcode::get_objcode()
{
	// （1）遍历所有四元式，确定每一个变量的声明、销毁时机 varinfo
	for (int i = 0; i < this->qruple.size(); ++i) {
		int tail = this->qruple[i].first;
		// result
		if (this->qruple[i].second.op == "=[]" // id = a[]
			|| this->qruple[i].second.op == ":=" || this->qruple[i].second.op == "+"
			|| this->qruple[i].second.op == "-"  || this->qruple[i].second.op == "*"
			|| this->qruple[i].second.op == "/"
		){
			for (int j = (int)this->qruple.size() - 1; j > i; --j) {
				if (this->qruple[j].second.arg1 == this->qruple[i].second.result ||
					this->qruple[j].second.arg2 == this->qruple[i].second.result ||
					this->qruple[j].second.result == this->qruple[i].second.result) {
					if (this->qruple[i].second.op == ":=" && this->qruple[j].second.result[this->qruple[j].second.result.size() - 1] == ']')continue;
					tail = this->qruple[j].first; break;
				}
			}
		}
		this->varinfo.push_back(VarInfo(this->qruple[i].second.result, this->qruple[i].first, tail));
	}
	// （2）按块生成目标代码
	// 打开文件 OC_Path...
	std::ofstream outfile;
	outfile.open(OC_Path, std::ios::out | std::ios::binary);
	// 生成标签
	for (int i = 0; i < this->daginfo.size(); ++i)
	{
		// 每个DAG有一个
		std::string label_tmp = "Label_" + std::to_string(this->label_num);
		this->label_num += 1;
		this->Label_DAG_mp[std::to_string(this->daginfo[i].sp)] = label_tmp;
	}
	this->label_num = 0;
	// 遍历DAG块
	for (int i = 0; i < this->daginfo.size(); ++i) {
		int j = this->daginfo[i].sp - QUAD_SATRT; // DAG块内索引偏移
		// 文件头
		if (i == 0) {
			// 先申请全局数组，如有必定在第一句（语法规定）
			if (this->qruple[j].second.op == "dec") {
				outfile << ".data\n";
				while (this->qruple[j].second.op == "dec") {
					outfile << this->qruple[j].second.result + ": .space " + this->qruple[j].second.arg1 + "\n"; // size
					++j;
				}
			}
			outfile << ".text\nmain:\n";
		}
		// 标签
		outfile << "Label_" + std::to_string(this->label_num) <<":\n";
		this->label_num += 1;
		// 语句 以DAG为单位生成
		while (j <= this->daginfo[i].ep - QUAD_SATRT) {
			// 根据varinfo分配寄存器
			int idx_new = 0;
			for (idx_new = 0; idx_new < REG_SIZE; ++idx_new) { if (this->RVALUE[idx_new] == "")break; }
			if (idx_new >= REG_SIZE) { std::cout << "寄存器不足！" << std::endl; outfile.close(); return ERROR_REG_FULL; }
			// arg1
			if (this->qruple[j].second.arg1 != "-" && this->is_var(this->qruple[j].second.arg1)) {
				if (this->AVALUE.count(this->qruple[j].second.arg1) == 0) {
					this->RVALUE[idx_new] = this->qruple[j].second.arg1;
					this->AVALUE[this->qruple[j].second.arg1] = idx_new + REG_START_IDX;
					for (idx_new = 0; idx_new < REG_SIZE; ++idx_new) { if (this->RVALUE[idx_new] == "")break; }
					if (idx_new >= REG_SIZE) { std::cout << "寄存器不足！" << std::endl; outfile.close(); return ERROR_REG_FULL; }
				}
			}
			// arg2
			if (this->qruple[j].second.arg2 != "-" && this->is_var(this->qruple[j].second.arg2)) {
				if (this->AVALUE.count(this->qruple[j].second.arg2) == 0) {
					this->RVALUE[idx_new] = this->qruple[j].second.arg2;
					this->AVALUE[this->qruple[j].second.arg2] = idx_new + REG_START_IDX;
					for (idx_new = 0; idx_new < REG_SIZE; ++idx_new) { if (this->RVALUE[idx_new] == "")break; }
					if (idx_new >= REG_SIZE) { std::cout << "寄存器不足！" << std::endl; outfile.close(); return ERROR_REG_FULL; }
				}
			}
			//result
			if (this->qruple[j].second.result != "-" && this->is_var(this->qruple[j].second.result)) {
				if (this->AVALUE.count(this->qruple[j].second.result) == 0) {
					this->RVALUE[idx_new] = this->qruple[j].second.result;
					this->AVALUE[this->qruple[j].second.result] = idx_new + REG_START_IDX;
					for (idx_new = 0; idx_new < REG_SIZE; ++idx_new) { if (this->RVALUE[idx_new] == "")break; }
					if (idx_new >= REG_SIZE) { std::cout << "寄存器不足！" << std::endl; outfile.close(); return ERROR_REG_FULL; }
				}
			}
			// 翻译为MIPS指令
			// [] = id / num
			if (this->qruple[j].second.op == "[]=")
			{
				std::string arg = this->qruple[j].second.arg1;
				std::string arr_name = this->get_arr_name(this->qruple[j].second.result);
				std::string offset = this->get_arr_offset(this->qruple[j].second.result);
				bool _is_var_ = this->is_var(arg);
				outfile << "addi $" + std::to_string(REG_START_IDX + REG_SIZE) + ",$zero," << std::stoi(offset) * 4 << "\n";
				if (_is_var_) { // id
					int idx = AVALUE[arg];
					outfile << "sw $" + std::to_string(idx) + "," + arr_name + "($" + std::to_string(REG_START_IDX + REG_SIZE) + ")\n";
				}
				else { // num
					outfile << "li $" + std::to_string(REG_START_IDX + REG_SIZE + 1) + "," + arg + "\n"; // arg1
					outfile << "sw $" + std::to_string(REG_START_IDX + REG_SIZE + 1) + "," + arr_name + "($" + std::to_string(REG_START_IDX + REG_SIZE) + ")\n";
				}
			}
			// id = []
			else if (this->qruple[j].second.op == "=[]")
			{
				std::string arg = this->qruple[j].second.result;
				std::string arr_name = this->get_arr_name(this->qruple[j].second.arg1);
				std::string offset = this->get_arr_offset(this->qruple[j].second.arg1);
				int idx = AVALUE[arg];
				outfile << "addi $" + std::to_string(REG_START_IDX + REG_SIZE) + ",$zero," << std::stoi(offset) * 4 << "\n";
				outfile << "lw $" + std::to_string(idx) + "," + arr_name + "($" + std::to_string(REG_START_IDX + REG_SIZE) + ")\n";
			}
			// id = id / num
			else if (this->qruple[j].second.op == ":=")
			{
				// a[]:=x 是数组传参，不执行
				if (this->qruple[j].second.result[this->qruple[j].second.result.size() - 1] != ']')
				{
					std::string left = this->qruple[j].second.result;
					int idx_left = AVALUE[left];
					if (this->is_var(this->qruple[j].second.arg1)) {
						std::string right = this->qruple[j].second.arg1;
						int idx_right = AVALUE[right];
						outfile << "move $" << idx_left << ",$" << idx_right << "\n";
					}
					else {
						outfile << "addi $" << idx_left << ",$zero," << this->qruple[j].second.arg1 << "\n";
					}
				}
			}
			// j - - xxx 无条件跳转
			else if (this->qruple[j].second.op == "j")
			{
				outfile << "j " << this->Label_DAG_mp[this->qruple[j].second.result] << "\n";
			}
			// a = b op c ,num / id / arr[]
			else if (this->qruple[j].second.op == "+" || this->qruple[j].second.op == "-" ||
				this->qruple[j].second.op == "*" || this->qruple[j].second.op == "/" || this->qruple[j].second.op[0] == 'j')
			{
				std::string res = this->qruple[j].second.result;
				int idx = AVALUE[res];
				// arg1 REG_START_IDX + REG_SIZE
				// case1 - arr[]
				if (this->qruple[j].second.arg1[this->qruple[j].second.arg1.size() - 1] == ']') {
					std::string arr_name = this->get_arr_name(this->qruple[j].second.arg1);
					std::string offset = this->get_arr_offset(this->qruple[j].second.arg1);
					outfile << "addi $" << REG_START_IDX + REG_SIZE + 1 << ",$zero," << std::stoi(offset) * 4 << "\n";
					outfile << "lw $" << REG_START_IDX + REG_SIZE << "," << arr_name << "($" << REG_START_IDX + REG_SIZE + 1 << ")\n";
				}
				// case2 - id
				else if (this->is_var(this->qruple[j].second.arg1)) {
					int idx_right = AVALUE[this->qruple[j].second.arg1];
					outfile << "add $" << REG_START_IDX + REG_SIZE << ",$zero,$" << idx_right << "\n";
				}
				// case3 - num
				else {
					outfile << "addi $" << REG_START_IDX + REG_SIZE << ",$zero," << this->qruple[j].second.arg1 << "\n";
				}
				// arg2 REG_START_IDX + REG_SIZE + 1
				// case1 - arr[]
				if (this->qruple[j].second.arg2[this->qruple[j].second.arg2.size() - 1] == ']') {
					std::string arr_name = this->get_arr_name(this->qruple[j].second.arg2);
					std::string offset = this->get_arr_offset(this->qruple[j].second.arg2);
					outfile << "addi $" << REG_START_IDX + REG_SIZE + 2 << ",$zero," << std::stoi(offset) * 4 << "\n";
					outfile << "lw $" << REG_START_IDX + REG_SIZE + 1 << "," << arr_name << "($" << REG_START_IDX + REG_SIZE + 2 << ")\n";
				}
				// case2 - id
				else if (this->is_var(this->qruple[j].second.arg2)) {
					int idx_right = AVALUE[this->qruple[j].second.arg2];
					outfile << "add $" << REG_START_IDX + REG_SIZE + 1 << ",$zero,$" << idx_right << "\n";
				}
				// case3 - num
				else {
					outfile << "addi $" << REG_START_IDX + REG_SIZE + 1 << ",$zero," << this->qruple[j].second.arg2 << "\n";
				}
				// mips
				if(this->qruple[j].second.op == "+")
				{
					outfile << "add $" << idx << ",$" << REG_START_IDX + REG_SIZE << ",$" << REG_START_IDX + REG_SIZE + 1 << "\n";
				}
				else if(this->qruple[j].second.op == "-")
				{
					outfile << "sub $" << idx << ",$" << REG_START_IDX + REG_SIZE << ",$" << REG_START_IDX + REG_SIZE + 1 << "\n";
				}
				else if (this->qruple[j].second.op == "*")
				{
					outfile << "mul $" << idx << ",$" << REG_START_IDX + REG_SIZE << ",$" << REG_START_IDX + REG_SIZE + 1 << "\n";
				}
				else if (this->qruple[j].second.op == "/") {
					outfile << "div $" << REG_START_IDX + REG_SIZE << ",$" << REG_START_IDX + REG_SIZE + 1 << "\n";
					outfile << "mflo $" << idx << "\n";
				}
				// jop
				else {
					bool is_solve = 0;
					if (!is_solve && (this->qruple[j].second.op == "j<" || this->qruple[j].second.op == "j<=")) {
						outfile << "slt $" << REG_START_IDX + REG_SIZE + 2 << ",$" << REG_START_IDX + REG_SIZE << ",$" << REG_START_IDX + REG_SIZE + 1 << "\n";
						outfile << "bne $" << REG_START_IDX + REG_SIZE + 2 << ",$zero," << this->Label_DAG_mp[res] << "\n";
						if (this->qruple[j].second.op == "<")is_solve = 1;
					}
					if (!is_solve && (this->qruple[j].second.op == "j>" || this->qruple[j].second.op == "j>=")) {
						outfile << "slt $" << REG_START_IDX + REG_SIZE + 2 << ",$" << REG_START_IDX + REG_SIZE + 1 << ",$" << REG_START_IDX + REG_SIZE << "\n";
						outfile << "bne $" << REG_START_IDX + REG_SIZE + 2 << ",$zero," << this->Label_DAG_mp[res] << "\n";
						if (this->qruple[j].second.op == ">")is_solve = 1;
					}
					if (!is_solve && (this->qruple[j].second.op == "j==" || this->qruple[j].second.op == "j>=" || this->qruple[j].second.op == "j<=")) {
						outfile << "sub $" << REG_START_IDX + REG_SIZE + 2 << ",$" << REG_START_IDX + REG_SIZE << ",$" << REG_START_IDX + REG_SIZE + 1 << "\n";
						outfile << "beq $" << REG_START_IDX + REG_SIZE + 2 << ",$zero," << this->Label_DAG_mp[res] << "\n";
						is_solve = 1;
					}
					if (!is_solve && this->qruple[j].second.op == "j!=") {
						outfile << "sub $" << REG_START_IDX + REG_SIZE + 2 << ",$" << REG_START_IDX + REG_SIZE << ",$" << REG_START_IDX + REG_SIZE + 1 << "\n";
						outfile << "bne $" << REG_START_IDX + REG_SIZE + 2 << ",$zero," << this->Label_DAG_mp[res] << "\n";
						is_solve = 1;
					}
				}
			}
			// 释放寄存器 RVALUE AVALUE j+QUAD_SATRT
			// arg1
			if (this->qruple[j].second.arg1 != "-" && this->is_var(this->qruple[j].second.arg1)) {
				int idx = 0;
				for (int idx = 0; idx < this->varinfo.size(); ++idx) { if (varinfo[idx].name == this->qruple[j].second.arg1)break; }
				if (this->varinfo[idx].ep == j + QUAD_SATRT) { 
					this->RVALUE[this->AVALUE[this->qruple[j].second.arg1] - REG_START_IDX] = "";
					this->AVALUE.erase(this->qruple[j].second.arg1);
				}
			}
			// arg2
			if (this->qruple[j].second.arg2 != "-" && this->is_var(this->qruple[j].second.arg2)) {
				int idx = 0;
				for (int idx = 0; idx < this->varinfo.size(); ++idx) { if (varinfo[idx].name == this->qruple[j].second.arg2)break; }
				if (this->varinfo[idx].ep == j + QUAD_SATRT) {
					this->RVALUE[this->AVALUE[this->qruple[j].second.arg2] - REG_START_IDX] = "";
					this->AVALUE.erase(this->qruple[j].second.arg2);
				}
			}
			//result
			if (this->qruple[j].second.result != "-" && this->is_var(this->qruple[j].second.result)) {
				int idx = 0;
				for (int idx = 0; idx < this->varinfo.size(); ++idx) { if (varinfo[idx].name == this->qruple[j].second.result)break; }
				if (this->varinfo[idx].ep == j + QUAD_SATRT) {
					this->RVALUE[this->AVALUE[this->qruple[j].second.result] - REG_START_IDX] = "";
					this->AVALUE.erase(this->qruple[j].second.result);
				}
			}
			++j; // DAG块内偏移
		}
	}
	outfile.close();
	std::cout << "目标代码生成成功！" << std::endl;
	return OBJGEN_RIGHT; // 生成成功
}

// 私有工具函数实现
// 函数1 - 获取DAG入口
void Objcode::get_dag_entrance()
{
	this->dagEntrance.clear();
	// 01 - 程序入口
	this->dagEntrance.push_back(QUAD_SATRT);
	// 02 - 能转移到的语句
	// 03 - 跟在条件转移语句后的语句
	for (int i = 1; i < this->qruple.size(); ++i) {
		if (this->qruple[i].second.op[0] == 'j') {
			this->dagEntrance.push_back(std::stoi(this->qruple[i].second.result)); //2
			if (this->qruple[i].second.op.size() > 1 && i < this->qruple.size() - 1 && this->qruple[i].second.op[0] != 'j') {
				this->dagEntrance.push_back(this->qruple[i + 1].first); //3
			}
		}
	}
	std::sort(this->dagEntrance.begin(), this->dagEntrance.end());
	auto last = std::unique(this->dagEntrance.begin(), this->dagEntrance.end());
	this->dagEntrance.erase(last, this->dagEntrance.end());

}

// 函数2 - 检查是否是变量
bool Objcode::is_var(std::string s)
{
	for (int i = 0; i < s.size(); ++i) {
		if (s[i] > '9' || s[i] < '0') return true;
	}
	return false;
}

// 函数3 - 获取数组名
std::string Objcode::get_arr_name(std::string s)
{
	int idx = 0;
	for (idx = 0; s[idx] != '['; ++idx){}
	return s.substr(0, idx);
}

// 函数4 - 获取数组偏移
std::string Objcode::get_arr_offset(std::string s)
{
	int idx = 0;
	for (idx = 0; s[idx] != '['; ++idx) {}
	return s.substr(idx + 1, s.size() - idx - 2);
}

