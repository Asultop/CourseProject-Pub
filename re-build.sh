echo "编译 CourseProject 主程序..."
gcc -std=c11 main.c src-extends/codeRender.c src-extends/chineseSupport.c src-extends/screenManager.c src-extends/stack.c src-extends/md5.c src-extends/usrManager.c src-extends/fileHelper.c src-extends/passwordInputSimulator.c src-extends/championHistoryColManager.c src-extends/problemBankManager.c src-extends/markdownPrinter.c src-extends/ACMLocalJudger.c -o CourseProject -lm

echo "编译测试程序..."
mkdir -p src-test/build
echo "  编译 test_input_simulator..."
gcc -std=c11 src-extends/passwordInputSimulator.c src-test/test_input_simulator.c -o src-test/build/test_input_simulator -lm
echo "  编译 test_markdown..."
gcc -std=c11 src-extends/stack.c src-extends/markdownPrinter.c src-test/test_markdown.c -o src-test/build/test_markdown -lm
echo "  编译 test_md5..."
gcc -std=c11 src-extends/md5.c src-test/test_md5.c -o src-test/build/test_md5 -lm
echo "  编译 test_printf..."
gcc -std=c11 src-test/test_printf.c -o src-test/build/test_printf -lm 
echo "  编译 test_stack..."
gcc -std=c11 src-extends/stack.c src-test/test_stack.c -o src-test/build/test_stack -lm
echo "  编译 test_screenManager..."
gcc -std=c11 src-extends/chineseSupport.c src-extends/screenManager.c src-test/test_screenManager.c -o src-test/build/test_screenManager -lm
echo "  编译 test_chineseSupport..."
gcc -std=c11 src-extends/chineseSupport.c src-test/test_chineseSupport.c -o src-test/build/test_chineseSupport -lm
echo "  编译 test_codeRender..."
gcc -std=c11 src-extends/codeRender.c src-extends/fileHelper.c src-extends/chineseSupport.c src-test/test_codeRender.c -o src-test/build/test_codeRender -lm