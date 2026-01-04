# ACM 竞赛管理与训练系统  （课程设计）

# 快速开始编译


## 预先
```bash
git clone https://github.com/Asultop/CourseProject-Pub -b master 
cd CourseProject-Pub
mkdir -p build && cd build
cmake .. -G "Unix Makefiles"
```

## 编译所有项目 (主项目 + 工具 + 测试) (多线程)
```bash
make all -j$(nproc)
```

## 编译主工程项目 (多线程)
```bash
make course_project -j$(nproc)
```

## 编译工具
```bash
make tool
```

## 编译测试
```bash
make test -j$(nproc)
```




# ACM竞赛管理系统流程图设计

## 1. 项目目录架构

```
├── CMakeLists.txt              # 项目构建配置文件
├── main.c                      # 程序入口文件
├── DataBase/                   # 数据存储目录
│   ├── Platform/               # 平台数据目录
│   │   ├── ACMT/               # ACM竞赛资料目录
│   │   │   ├── 历届获奖.txt     # 历届获奖记录
│   │   │   ├── 参赛规则.txt     # 参赛规则
│   │   │   ├── 评分标准.txt     # 评分标准
│   │   │   ├── 赛事介绍.txt     # 赛事介绍
│   │   │   └── 赛事构成.txt     # 赛事构成
│   │   └── Problems/           # 题目数据目录
│   │       ├── 1/              # 题目ID为1的目录
│   │       │   ├── in/         # 测试用例输入目录
│   │       │   │   ├── test1.in
│   │       │   │   └── test2.in
│   │       │   ├── out/        # 测试用例输出目录
│   │       │   │   ├── test1.out
│   │       │   │   └── test2.out
│   │       │   ├── 1.c         # 参考源代码
│   │       │   ├── MetaData    # 题目元数据
│   │       │   ├── analyzing.txt # 题目分析
│   │       │   └── problem.txt # 题目描述
│   │       ├── 3485/           # 题目ID为3485的目录
│   │       └── 6293/           # 题目ID为6293的目录
│   └── usrData/                # 用户数据目录
│       └── userData.txt        # 用户数据文件
├── src-extends/                # 扩展源文件目录
│   ├── ACMLocalJudger.c/h      # 本地判题器模块
│   ├── championHistoryColManager.c/h # 冠军历史管理模块
│   ├── chineseSupport.c/h      # 中文支持模块
│   ├── codeRender.c/h          # 代码渲染模块
│   ├── colorPrint.c/h          # 彩色打印模块
│   ├── Def.h                   # 公共定义头文件
│   ├── fileHelper.c/h          # 文件辅助模块
│   ├── markdownPrinter.c/h     # Markdown打印模块
│   ├── md5.c/h                 # MD5加密模块
│   ├── passwordInputSimulator.c/h # 密码输入模拟器
│   ├── problemBankManager.c/h  # 题库管理模块
│   ├── releaseRuntime.c/h      # 运行时资源释放模块
│   ├── screenManager.c/h       # 屏幕管理模块
│   ├── stack.c/h               # 栈数据结构模块
│   └── usrManager.c/h          # 用户管理模块
├── src-test/                   # 测试源文件目录
│   ├── test_chineseSupport.c   # 中文支持测试
│   ├── test_codeRender.c       # 代码渲染测试
│   ├── test_input_simulator.c  # 输入模拟器测试
│   ├── test_markdown.c         # Markdown打印测试
│   ├── test_md5.c              # MD5加密测试
│   ├── test_printf.c           # 打印测试
│   ├── test_releaseDir.c       # 目录释放测试
│   ├── test_screenManager.c    # 屏幕管理测试
│   ├── test_stack.c            # 栈测试
│   └── tool_file2ShellCode.c   # 文件转ShellCode工具
└── WrongAnswerTest/            # 错误答案测试目录
    ├── CE.cpp                  # 编译错误测试
    ├── RE.cpp                  # 运行时错误测试
    └── TLE.cpp                 # 超时错误测试
```

## 2. 模块与文件对应关系

| 模块名称 | 对应文件 | 主要功能 |
|---------|----------|---------|
| 程序入口 | main.c | 程序启动、环境检查、用户认证、主菜单 |
| 用户管理 | usrManager.c/h | 用户创建、查询、删除、登录、注册 |
| 题库管理 | problemBankManager.c/h | 题目加载、添加、删除、交互式管理 |
| 本地判题器 | ACMLocalJudger.c/h | 代码判题、测试用例验证 |
| 文件辅助 | fileHelper.c/h | 文件读写、复制、删除、目录操作 |
| 屏幕管理 | screenManager.c/h | 屏幕清理、菜单显示、界面控制 |
| 中文支持 | chineseSupport.c/h | 中文编码处理、文件编码转换 |
| 代码渲染 | codeRender.c/h | 代码语法高亮渲染 |
| 彩色打印 | colorPrint.c/h | 彩色文本输出 |
| Markdown打印 | markdownPrinter.c/h | Markdown文件内容显示 |
| MD5加密 | md5.c/h | 密码MD5加密 |
| 冠军历史管理 | championHistoryColManager.c/h | 冠军记录查询、管理 |
| 运行时资源释放 | releaseRuntime.c/h | 环境修复、资源释放 |
| 栈数据结构 | stack.c/h | 栈的基本操作 |

## 3. 数据存储位置

| 数据类型 | 存储位置 | 存储格式 |
|---------|----------|---------|
| 用户数据 | DataBase/usrData/userData.txt | 文本文件，每行一个用户，包含用户名和密码 |
| 题目数据 | DataBase/Platform/Problems/ | 目录结构，每个题目一个子目录 |
| ACM资料 | DataBase/Platform/ACMT/ | 文本文件，包含竞赛相关信息 |

## 4. 节点定义

### 4.1 程序启动与初始化
- **开始节点**：程序入口
- **环境检查**（envCheck，main.c:124）：检查必要文件夹是否存在
- **环境修复**（releaseRuntimeResources，releaseRuntime.c）：修复缺失的环境资源
- **数据库初始化**（initDataBase，main.c:143）：初始化用户数据文件
- **默认管理员创建**（main.c:187）：创建默认管理员账号

### 4.2 用户认证模块
- **登录/注册界面**（printSplashScreen，screenManager.c）：显示登录注册选项
- **登录处理**（login，usrManager.c）：处理用户登录
- **注册处理**（registerUser，usrManager.c）：处理用户注册
- **密码修改**（modifyAccount，usrManager.c）：处理用户密码修改
- **用户删除**（deleteUserFlow，usrManager.c）：处理用户删除
- **获取所有用户**（getAllUsrByReadDataFile，usrManager.c:16）：从文件读取所有用户数据
- **保存所有用户**（saveAllUsrToDataFile，usrManager.c:17）：将所有用户数据保存到文件

### 4.3 主功能界面
- **主界面显示**（printMainScreen，screenManager.c）：显示主功能菜单
- **ACM竞赛简介**（getInACMIntroduction，main.c:45）：显示ACM竞赛相关信息
- **题库管理**（interactiveProblemBank，problemBankManager.c:12）：处理题库相关操作

### 4.4 题库管理模块
- **加载所有题目**（loadAllProblems，problemBankManager.c:9）：从目录加载所有题目信息
- **显示题目列表**：显示题目列表
- **查看题目详情**：查看题目详细信息
- **添加题目**（addProblemInteractive，problemBankManager.c:18）：交互式添加新题目
- **删除题目**（deleteProblemByID，problemBankManager.c:15）：按ID删除题目
- **判题处理**（acm_local_judge，ACMLocalJudger.c:14）：对用户提交的代码进行判题

### 4.5 辅助模块
- **文件内容显示**（displayFileContent，main.c:25）：显示文件内容
- **清理缓冲区**（cleanBuffer，main.c:20）：清理输入缓冲区
- **清理屏幕**（cleanScreen，main.c:21）：清理控制台屏幕
- **暂停屏幕**（pauseScreen，main.c:22）：暂停屏幕显示

## 5. 连接关系与数据流向

### 5.1 主流程
```
开始节点 → 环境检查(envCheck, main.c:124) → [环境正常?]
  → {是} → 数据库初始化(initDataBase, main.c:143) → 默认管理员创建(main.c:187)
  → {否} → 提示修复环境 → 环境修复(releaseRuntimeResources, releaseRuntime.c) → 重新检查环境
  → 登录/注册界面(printSplashScreen, screenManager.c)
```

### 5.2 用户认证流程
```
登录/注册界面 → [选择登录] → 登录处理(login, usrManager.c) → {成功} → 主界面显示(printMainScreen, screenManager.c)
登录/注册界面 → [选择注册] → 注册处理(registerUser, usrManager.c) → 保存所有用户(saveAllUsrToDataFile, usrManager.c:17) → 登录/注册界面
登录/注册界面 → [选择密码修改] → 密码修改(modifyAccount, usrManager.c) → 保存所有用户(saveAllUsrToDataFile, usrManager.c:17) → 登录/注册界面
登录/注册界面 → [选择用户删除] → 用户删除(deleteUserFlow, usrManager.c) → 保存所有用户(saveAllUsrToDataFile, usrManager.c:17) → 登录/注册界面
登录/注册界面 → [选择退出] → 结束节点
```

### 5.3 主功能流程
```
主界面显示(printMainScreen, screenManager.c) → [选择ACM竞赛简介] → ACM竞赛简介(getInACMIntroduction, main.c:45) → 文件内容显示(displayFileContent, main.c:25) → 主界面显示
主界面显示 → [选择题库管理] → 题库管理(interactiveProblemBank, problemBankManager.c:12) → 加载所有题目(loadAllProblems, problemBankManager.c:9) → 显示题目列表 → [选择操作]
  → [选择查看详情] → 查看题目详情 → 主界面显示
  → [选择添加题目] → 添加题目(addProblemInteractive, problemBankManager.c:18) → 主界面显示
  → [选择删除题目] → 删除题目(deleteProblemByID, problemBankManager.c:15) → 主界面显示
  → [选择提交代码] → 判题处理(acm_local_judge, ACMLocalJudger.c:14) → 显示判题结果 → 主界面显示
主界面显示 → [选择退出] → 登录/注册界面
```

### 5.4 代码判题流程
```
用户提交代码 → 判题处理(acm_local_judge, ACMLocalJudger.c:14) → 查找题目目录下的in子目录 → 遍历测试用例输入文件 → 查找对应的out子目录中的测试用例输出文件 → 对每个测试用例进行判题 → 返回JudgeSummary结构体 → 显示判题结果
```

## 6. 数据流向

### 6.1 用户数据流向
```
userData.txt(DataBase/usrData/) → getAllUsrByReadDataFile(usrManager.c:16) → globalUserGroup(内存数组) → saveAllUsrToDataFile(usrManager.c:17) → userData.txt
```

### 6.2 题目数据流向
```
Problems目录(DataBase/Platform/) → loadAllProblems(problemBankManager.c:9) → ProblemEntry数组(内存) → 显示题目列表 → 用户选择 → 查看题目详情/添加题目/删除题目
```

### 6.3 ACM资料数据流向
```
ACMT目录文件(DataBase/Platform/ACMT/) → displayFileContent(main.c:25) → mdcat_worker(markdownPrinter.c) → 控制台输出
```

### 6.4 代码判题数据流向
```
用户源代码 → acm_local_judge(ACMLocalJudger.c:14) → 题目in目录下的测试用例输入 → 程序执行 → 输出结果 → 与out目录下的测试用例输出对比 → JudgeResult → JudgeSummary → 控制台输出
```

## 7. 模块返回值类型信息

| 函数名 | 文件位置 | 返回值类型 | 说明 |
|-------|----------|-----------|------|
| envCheck | main.c:124 | bool | true=环境正常，false=环境异常 |
| releaseRuntimeResources | releaseRuntime.c | void | 无返回值 |
| initDataBase | main.c:143 | void | 无返回值 |
| getAllUsrByReadDataFile | usrManager.c:16 | UsrActionReturnInfo | ERR=-1，SUCCESS=0 |
| saveAllUsrToDataFile | usrManager.c:17 | UsrActionReturnInfo | ERR=-1，SUCCESS=0 |
| login | usrManager.c:23 | bool | true=登录成功，false=登录失败 |
| registerUser | usrManager.c:24 | bool | true=注册成功，false=注册失败 |
| modifyAccount | usrManager.c:25 | bool | true=修改成功，false=修改失败 |
| deleteUserFlow | usrManager.c:26 | bool | true=删除成功，false=删除失败 |
| loadAllProblems | problemBankManager.c:9 | int | 返回加载的题目数量 |
| deleteProblemByID | problemBankManager.c:15 | bool | true=删除成功，false=删除失败 |
| acm_local_judge | ACMLocalJudger.c:14 | JudgeSummary | 包含测试点数量和每个测试点的判题结果 |
| dirExists | fileHelper.c:22 | bool | true=目录存在，false=目录不存在 |
| fileExists | fileHelper.c:23 | bool | true=文件存在，false=文件不存在 |
| touchFile | fileHelper.c:24 | bool | true=操作成功，false=操作失败 |
| createFile | fileHelper.c:25 | bool | true=创建成功，false=创建失败 |
| copyFile | fileHelper.c:19 | bool | true=复制成功，false=复制失败 |
| removeFile | fileHelper.c:21 | bool | true=删除成功，false=删除失败 |

## 8. 异常处理流程

### 8.1 环境异常处理
```
envCheck返回false → 提示用户修复环境 → 调用releaseRuntimeResources → 重新调用envCheck → [环境正常?]
  → {是} → 继续执行
  → {否} → 提示修复失败 → 程序退出
```

### 8.2 文件操作异常处理
```
文件读取/写入失败 → 返回错误码ERR → 输出错误信息 → 程序退出或重试
```

### 8.3 用户操作异常处理
```
无效的选择 → 提示用户重新输入 → 重新显示菜单
```

### 8.4 判题异常处理
```
代码编译错误 → 返回JUDGE_RESULT_COMPILE_ERROR
代码运行时错误 → 返回JUDGE_RESULT_RUNTIME_ERROR
代码超时 → 返回JUDGE_RESULT_TIME_LIMIT_EXCEEDED
答案错误 → 返回JUDGE_RESULT_WRONG_ANSWER
答案正确 → 返回JUDGE_RESULT_ACCEPTED
```

## 9. 界面交互流程

### 9.1 登录/注册界面交互
```
显示登录/注册菜单 → 用户输入选择 → 处理对应操作 → 返回登录/注册界面或进入主界面
```

### 9.2 主功能界面交互
```
显示主功能菜单 → 用户输入选择 → 处理对应操作 → 返回主功能界面或退出到登录/注册界面
```

### 9.3 题库管理界面交互
```
显示题目列表 → 用户输入选择 → 处理对应操作（查看详情/添加/删除/提交代码） → 返回题库管理界面或主界面
```

### 9.4 ACM简介界面交互
```
显示ACM简介菜单 → 用户输入选择 → 显示对应文件内容 → 返回ACM简介菜单或主界面
```

## 10. 流程图绘制说明

1. **使用矩形表示处理节点**，并在节点下方标注返回值类型和文件位置
2. **使用菱形表示判断节点**，并在节点内标注判断条件
3. **使用平行四边形表示数据存储**，并标注存储位置
4. **使用箭头表示数据流向和控制流**，并在箭头上标注数据类型
5. **使用圆角矩形表示开始和结束节点**
6. **使用注释标注重要的代码模块和功能说明**
7. **使用不同颜色区分不同模块**：
   - 用户管理模块：蓝色
   - 题库管理模块：绿色
   - 本地判题模块：黄色
   - 辅助模块：灰色
   - 界面模块：紫色

## 11. 设计特点

1. **模块化设计**：各功能模块独立，便于维护和扩展
2. **交互式界面**：通过控制台实现用户交互，操作简单直观
3. **本地存储**：数据存储在本地文件系统，无需数据库支持
4. **跨平台支持**：通过条件编译实现Windows和Linux平台支持
5. **完整的错误处理**：对各种异常情况进行处理，提高程序健壮性
6. **清晰的目录结构**：按功能和数据类型组织目录，便于管理
7. **详细的文档**：包含题目描述、分析、参考代码等
8. **测试用例完整**：每个题目包含多个测试用例，确保判题准确性

## 12. 核心数据结构

### 12.1 用户数据结构
```c
typedef struct {
    char name[MAX_NAME_LEN];     // 用户名
    char password[MAX_PASSWORD_LEN]; // 密码
} UsrProfile;
```

### 12.2 题目数据结构
```c
typedef struct {
    char id[64];                 // 题目ID
    char title[256];             // 题目标题
    char difficulty[64];         // 题目难度
    char type[128];              // 题目类型
    char folderName[128];        // 文件夹名称
    char problemPath[512];       // 题目路径
} ProblemEntry;
```

### 12.3 判题结果数据结构
```c
typedef enum{
    JUDGE_RESULT_ACCEPTED=1,        // 答案正确
    JUDGE_RESULT_WRONG_ANSWER=2,     // 答案错误
    JUDGE_RESULT_TIME_LIMIT_EXCEEDED=3, // 超时
    JUDGE_RESULT_RUNTIME_ERROR=4,    // 运行时错误
    JUDGE_RESULT_COMPILE_ERROR=5     // 编译错误
} JudgeResult;

typedef struct{
    JudgeResult result;            // 判题结果
    char message[256];             // 结果消息
} JudgeReturnInfo;

typedef struct{
    int count;                     // 测试用例数量
    JudgeReturnInfo infos[MAX_JUDGES_PER_PROBLEM]; // 每个测试用例的结果
} JudgeSummary;
```

## 13. 常量定义

| 常量名 | 定义位置 | 值 | 说明 |
|-------|----------|-----|------|
| MAX_NAME_LEN | Def.h:89 | 100 | 用户名最大长度 |
| MAX_PASSWORD_LEN | Def.h:90 | 100 | 密码最大长度 |
| MAX_USER_COUNT | Def.h:91 | 1000 | 最大用户数量 |
| MAX_PROBLEMS | Def.h:93 | 2048 | 最大题目数量 |
| MAX_JUDGES_PER_PROBLEM | Def.h:94 | 100 | 每个题目最大测试用例数量 |
| DATABASE_DIR | Def.h:113 | "./DataBase" | 数据库目录 |
| PROBLEM_DIR | Def.h:115 | DATABASE_DIR "/Platform/Problems" | 题目目录 |
| USERDATA_DIR | Def.h:116 | DATABASE_DIR "/usrData" | 用户数据目录 |

## 14. 技术栈与依赖

| 技术/依赖 | 用途 | 来源 |
|----------|------|------|
| C语言 | 主要开发语言 | 标准C库 |
| CMake | 项目构建工具 | 外部依赖 |
| 标准C库 | 基础功能支持 | 系统提供 |
| ANSI转义序列 | 彩色终端输出 | 系统提供 |

## 15. 编译与运行

### 15.1 编译命令
```bash
mkdir build
cd build
cmake ..
make
```

### 15.2 运行命令
```bash
./course_project
```

### 15.3 测试命令
```bash
cd src-test/build
./test_xxx  # 其中xxx为测试模块名称
```

## 16. 扩展性设计

1. **支持更多编程语言**：可以扩展本地判题器，支持C++、Java等更多编程语言
2. **网络功能**：可以添加网络模块，支持远程判题和多人竞赛
3. **图形界面**：可以添加图形界面，提高用户体验
4. **数据库支持**：可以添加数据库支持，提高数据管理效率
5. **题目导入导出**：可以添加题目导入导出功能，方便题目管理

## 17. 安全性设计

1. **密码加密存储**：使用MD5算法加密存储用户密码
2. **输入验证**：对用户输入进行验证，防止无效输入
3. **文件权限控制**：合理设置文件权限，防止未授权访问
4. **错误信息隐藏**：不在错误信息中泄露敏感信息
5. **验证码机制**：防止暴力破解

## 18. 性能优化

1. **延迟加载**：题目数据采用延迟加载，提高启动速度
2. **缓存机制**：对频繁访问的数据进行缓存，提高访问速度
3. **多线程判题**：可以采用多线程进行判题，提高判题效率
4. **文件读写优化**：采用高效的文件读写方式，提高数据处理速度

## 19. 部署与维护

1. **环境要求**：支持C语言编译的系统，如Windows、Linux、macOS
2. **部署步骤**：编译生成可执行文件，复制DataBase目录到可执行文件同级目录
3. **维护建议**：定期备份userData.txt文件，防止数据丢失
4. **日志管理**：添加日志功能，便于调试和维护

## 20. 总结

本设计文档详细描述了ACM竞赛管理系统的流程图、架构、模块功能、数据流向和设计特点。该系统采用模块化设计，具有良好的扩展性和维护性，支持用户管理、题库管理和代码判题等核心功能。系统使用本地文件存储数据，无需数据库支持，部署简单，适合在各种环境下运行。

该流程图设计可以直接用于Visio或DrawIO等工具绘制详细的系统流程图，帮助开发人员和用户更好地理解系统的工作原理和流程。