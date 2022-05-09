#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <ctime>
#include "file_management.h"
using namespace std;

typedef struct superBlock {  //超级块结构
	unsigned short inodeNum; //磁盘中inode节点数
	unsigned short blockNum; //磁盘中的物理块数
	unsigned long maxFileSize; //文件的最大长度
}superBlock;

