#include "passwordInputSimulator.h"
static void term_init() {
#if !IS_WINDOWS
    if (g_terminal_inited) return;
    tcgetattr(STDIN_FILENO, &g_old_tio);
    struct termios new_tio = g_old_tio;
    new_tio.c_lflag &= ~(ICANON | ECHO);  // 关闭行缓冲、回显
    new_tio.c_cc[VMIN] = 1;               // 最少读1字符
    new_tio.c_cc[VTIME] = 0;              // 无超时
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    g_terminal_inited = 1;
#endif
}
static void term_restore() {
#if !IS_WINDOWS
    if (g_terminal_inited) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_old_tio);
        g_terminal_inited = 0;
    }
#endif
}
static int getch_safe() {
#if IS_WINDOWS
    return _getch();
#else
    char ch;
    ssize_t ret = read(STDIN_FILENO, &ch, 1);
    return (ret == 1) ? (unsigned char)ch : EOF;
#endif
}
static void move_cursor(int n) {
    if (n > 0) {
        for (int i = 0; i < n; i++) printf("\033[C"); // 右移
    } else if (n < 0) {
        for (int i = 0; i < -n; i++) printf("\033[D"); // 左移
    }
    fflush(stdout);
}
void getpwd(char *pwd, int pwdlen) {
    if (!pwd || pwdlen <= 0) return;

    char pass[PWDLEN + 1] = {0}; // 存储密码
    int len = 0;                 // 密码长度
    int pos = 0;                 // 光标位置（0~len）
    memset(pwd, 0, pwdlen + 1);

    term_init();

    // 清空行缓冲（防止残留字符干扰）
    setbuf(stdout, NULL);

    while (1) {
        int ch = getch_safe();

        // 1. 回车/换行：结束输入
        if (ch == '\r' || ch == '\n') {
            printf("\n");
            strncpy(pwd, pass, pwdlen);
            break;
        }

        // 2. 处理方向键
        if (ch == ARROW_PRE) {
#if IS_WINDOWS
            // Windows方向键：先读前缀再读键码
            ch = getch_safe();
            if (ch == ARROW_LEFT && pos > 0) {
                move_cursor(-1); // 左移光标
                pos--;
            } else if (ch == ARROW_RIGHT && pos < len) {
                move_cursor(1);  // 右移光标
                pos++;
            }
#else
            // Linux/macOS方向键：ESC [ D/C
            char seq[2];
            read(STDIN_FILENO, seq, 2);
            if (seq[0] == '[' && seq[1] == 'D' && pos > 0) { // 左箭头
                move_cursor(-1);
                pos--;
            } else if (seq[0] == '[' && seq[1] == 'C' && pos < len) { // 右箭头
                move_cursor(1);
                pos++;
            }
#endif
            continue;
        }

        // 3. 退格键：删除光标前字符（核心修复显示）
        if ((ch == BS_CHAR || ch == '\b') && pos > 0) {
            // 1. 删除内存中的字符
            memmove(&pass[pos-1], &pass[pos], len - pos);
            len--;
            pos--;

            // 2. 终端显示修正（精确控制光标）
            move_cursor(-1);          // 光标左移到要删除的*位置
            printf(" ");              // 空格覆盖*
            move_cursor(-1);          // 光标回退
            // 重绘后面的*（如果有）
            for (int i = pos; i < len; i++) {
                printf("*");
            }
            // 光标移回正确位置
            move_cursor(pos - len);
            continue;
        }

        // 4. 超出长度：忽略
        if (len >= pwdlen) continue;

        // 5. 可打印字符：插入到光标位置
        if (isprint((unsigned char)ch)) {
            // 1. 内存中插入字符
            memmove(&pass[pos+1], &pass[pos], len - pos);
            pass[pos] = ch;
            len++;

            // 2. 终端显示修正
            // 先重绘光标后的所有*（腾出位置）
            for (int i = pos; i < len; i++) {
                printf("*");
            }
            // 光标移回插入位置的下一位
            move_cursor(pos - len + 1);
            pos++;
        }
    }

    term_restore();
}
