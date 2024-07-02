1、进入 CLikeCompiler_exe 目录下，点击 CLikeCompiler.exe 文件；
2、此时提示输入文件路径，输入以下的任意一个，编译对应类C源程序；

正确代码（第一个含“过程调用+数组”，第二个仅含“过程调用”）路径：
./test/test-code-array.txt
./test/test-code-call.txt

静态语义错误代码路径：
./test/test-code-call-argument_not_found.txt
./test/test-code-call-argument_repeat.txt
./test/test-code-call-function_not_found.txt
./test/test-code-call-function_repeat.txt
./test/test-code-call-main_not_found.txt
./test/test-code-call-real_para_num_error.txt

3、输出的四元式在 CLikeCompiler_exe/parser/parser-4_ruple.txt 中；
4、输出的目标代码在 CLikeCompiler_exe/objcode/objcode_mips.txt 中；