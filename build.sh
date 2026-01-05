#!/bin/bash
mkdir -p ./build
cd ./build
cmake ..
make all -j$(nproc)
cd ..
echo 构建成功
