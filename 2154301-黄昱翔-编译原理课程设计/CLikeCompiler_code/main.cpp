#include "tool.h"
#include "lexical.h"
#include "parser.h"
#include "objcode.h"

int main() {
    // �Ĵ���Ԫʽ����
    std::vector<std::pair<int, Quadruple>> semantic_result;
    if (START_FROM_LEX) {
        // ����Դ����������Ԫʽ
        // 0. Input
        std::string file_path;
        std::cout << "�������ļ�·����\n";
        std::cin >> file_path;
        std::string input = file_to_str(file_path);
        
        // ָ��·��
        //std::string input = file_to_str(PATH_TEST_ARRAY);

        // 1. Lexer Analyse
        Lexical LC;
        std::pair<int, std::vector<token>> is_LC_right = LC.Analyze(input);
        if (is_LC_right.first > 0) {
            std::cout << "�ʷ������ɹ�\n";
        }
        else {
            std::cout << "�ʷ�����ʧ��\n";
            system("pause");
            return 0;
        }

        // 2. Parser Analyse
        Parser PS;
        PS.load_argument_def();
        PS.load_production();
        //PS.show_production();
        PS.get_first();
        //PS.show_first();
        //PS.show_closure();
        PS.get_item_group();
        PS.show_item_group();
        //PS.show_item_group_shift_mp();
        PS.get_LR1_table();
        //PS.show_LR1_table();
        bool is_PS_right = PS.Analyse(is_LC_right.second);
        if (is_PS_right == 0) {
            system("pause");
            return 0;
        }

        // 3. Semantic Analyse
        //PS.show_tree(PS.get_tree());
        bool is_SM_right = PS.semanticAnalyse();
        if (!is_SM_right) {
            system("pause");
            return 0;
        }
        semantic_result = PS.get_Q_ruple();
    }
    else {
        // ���ļ���ȡ��Ԫʽ
        std::ifstream infile;
        infile.open(SM_QUADRUPLE_Path, std::ios::in | std::ios::binary);
        std::string s;
        int num;
        Quadruple q_item;
        while (!infile.eof()) {
            infile >> s; num = std::stoi(s);
            infile >> s; q_item.op = s;
            if (infile.eof())break;
            infile >> s; q_item.arg1 = s;
            infile >> s; q_item.arg2 = s;
            infile >> s; q_item.result = s;
            semantic_result.push_back({ num,q_item });
        }
        infile.close();
    }

    // 4. Object Code Generation
    Objcode OC;
    OC.get_qruple(semantic_result);
    //OC.show_qruple();
    OC.get_dag();
    //OC.show_dag();
    int is_OC_right = OC.get_objcode();
    if (is_OC_right < 0) {
        system("pause");
        return 0;
    }

    // 5. End
    std::cout << "����ɹ���" << std::endl;
    
    system("pause");
    return 0;
}
