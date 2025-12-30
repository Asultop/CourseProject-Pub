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

// 重绘当前输入区域：先清空之前显示的字符，再显示当前长度的*，并把光标放回 pos 位置
// 重绘当前输入区域：从当前光标位置(cur_pos)回到起始，清除 prev_len，打印 len 个 '*'，
// 最后把光标放到 target_pos 位置。
static void redraw_masked(int prev_len, int len, int cur_pos, int target_pos) {
    // 从当前光标位置移动到起始位置
    move_cursor(-cur_pos);
    // 清除之前的显示（用空格覆盖）
    for (int i = 0; i < prev_len; i++) putchar(' ');
    // 光标回到起始位置（因为打印了 prev_len 个空格）
    move_cursor(-prev_len);
    // 打印当前的*掩码
    for (int i = 0; i < len; i++) putchar('*');
    // 将光标移动到正确的位置
    move_cursor(target_pos - len);
    fflush(stdout);
}
void getpwd(char *pwd, int pwdlen) {
    if (!pwd || pwdlen <= 0) return;

    char pass[PWDLEN + 1] = {0}; // 存储密码
    int len = 0;                 // 密码长度
    int pos = 0;                 // 光标位置（0~len）
    int prev_len = 0;            // 上次显示的长度，用于重绘清除残留
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
            // 保存当前光标位置（当前光标在 pos 处）
            int cur_pos = pos;
            // 删除内存中的字符并保持以\0结尾
            memmove(&pass[pos-1], &pass[pos], len - pos);
            len--;
            pos--;
            pass[len] = '\0';

            // 使用重绘函数：告诉它当前光标位置 cur_pos，以及删除后目标位置 pos
            redraw_masked(prev_len, len, cur_pos, pos);
            prev_len = len;
            continue;
        }

        // 4. 超出长度：忽略
        if (len >= pwdlen) continue;

        // 5. 可打印字符：插入到光标位置
        if (isprint((unsigned char)ch)) {
            // 保存当前光标位置（当前光标在 pos 处）
            int cur_pos = pos;
            // 内存中插入字符并保持以\0结尾
            memmove(&pass[pos+1], &pass[pos], len - pos);
            pass[pos] = (char)ch;
            len++;
            pass[len] = '\0';
            pos++;

            // 重绘显示以修正*号偏移和残留，传入当前光标 cur_pos 和新目标 pos
            redraw_masked(prev_len, len, cur_pos, pos);
            prev_len = len;
        }
    }

    term_restore();
}
