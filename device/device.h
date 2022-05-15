#pragma once
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <ctime>
#include "file_management.h"
using namespace std;

#define diskname "disk.txt"
#define maxDataBlockNum 1024 //最多1024个物理块
#define maxBlockNum (1024 + 132)
#define BLOCK_SIZE 64 //每个物理块大小为64字节
#define maxFileNum 256 //最多文件数量、最大inode数量
#define maxFileSize 256 //每个文件的最大大小
#define blockNumInOneFile (maxFileSize / blockSize) //每个文件占用物理块数
#define deviceNum 5

#define superBlockStart 0 //超级块
#define iNodeBitMapStart 1 //iNode位图
#define dataBitMapStart 2 //数据块位图
#define iNodeStart 4 //dataBitMapStart + ceil(1.0 * maxDataBlockNum / (sizeof(char) * blockSize));
#define dataStart (iNodeStart + 128) //132, 共128块用来存inode

vector<long> allDevice[deviceNum];
string deviceName[] = { "disk","mouse","displayer","printer","keyboard" };

typedef struct superBlock {  //超级块结构
	unsigned short inodeNum; //磁盘中inode节点数
	unsigned short blockNum; //磁盘中的物理块数
	unsigned long maxfilesize; //文件的最大长度
}superBlock;

//展示所有设备状态
void show(); 
//进程申请设备,返回true代表申请成功,返回false代表设备已被占用,进程加入等待队列
bool acquire(long pid, int device);
//进程释放设备,device为-1则释放该进程申请的所有设备
bool release(long pid, int device);

// 磁盘初始化
void disk_init();

// 申请一个数据块,返回块号;如果返回-1说明无空闲数据块
int new_block();
// 输入块号,释放该物理块
void free_block(int blockSeq);
//写入物理块，形参列表:块号,开始写的位置在块中的偏移量,要读入的字节数,缓冲区指针;返回true代表操作成功
bool block_write(int blockSeq, int offset, int charNum, char* buf);
//读出物理块，形参列表:块号,开始读的位置在块中的偏移量,要读入的字节数,缓冲区指针;返回true代表操作成功
bool block_read(int blockSeq, int offset, int charNum, char* buf);

// 申请一个iNode,返回iNode指针;指针中的i_num是iNode编号;如果i_num为-1说明无空闲iNode块
iNode* new_iNode();
// 释放一个iNode,根据参数中的inode->i_num释放磁盘中的iNode节点,并释放参数中的iNode指针
void free_iNode(iNode* inode);
// 读iNode,即根据参数中的inode->i_num在磁盘对应位置读出该iNode的全部数据;返回true代表操作成功
bool read_iNode(iNode* inode);
// 写iNode,即根据参数中的inode->i_num在磁盘对应位置写入该iNode的全部数据;返回true代表操作成功
bool write_iNode(iNode* inode);
