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
