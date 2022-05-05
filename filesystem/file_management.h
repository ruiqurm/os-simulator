#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
using namespace std;


/*重要参数宏定义*/
#define NAME_LEN 14	     //文件名称最大长度
#define ROOT_INO 1       //根i节点号
#define INODE_NUM 256	 //iNode的数量
#define DIR_NUM 8		 //每个目录文件下的文件最大个数，一个block为64个字节，能放两个32个字节的目录项
#define PATH_LENGTH 100	 //路径字符串最大长度
#define FBLK_NUM 4		 //文件中block的最大个数
#define NR_FILE 64       //系统文件表中最大项数
#define NR_OPEN 20       //一个进程最多打开的文件数
#define ENTRY_SIZE 32    //一个目录项的大小


/*重要数据结构*/

//文件索引节点
typedef struct INODE
{
	unsigned short i_mode;		 //文件类型：0目录，1普通
	int i_size;				     //文件大小（字节数）
    int nlinks;					 //链接数，即有多少文件目录项指向这个inode 判断一个 i节点是否应该被释放
	time_t mtime;				 //文件的时间戳，文件内容上一次变动的时间
	short i_zone[FBLK_NUM];      //文件数据block的位置
	//以下字段只在内存中使用
	unsigned short i_count;		 //inode的引用计数
	int i_dirt;					 //修改标识，1表示已修改，0表示未修改
	int i_num;                   //i节点号
} iNode;

//文件目录项结构
typedef struct DIR_ENTRY
{
	char file_name[28];			 //文件名
	unsigned int iNode_no;       //iNode编号
} dir_entry;

//文件结构体
typedef struct FILE
{
	int mode;                    //写入模式（0为追加，1为从当前位置写入，2为覆盖）
	unsigned short f_count;		 //对应文件引用计数值
	iNode* f_iNode;				 //指向对应iNode
	long int f_pos;				 //读写指针
} myFile;


/*全局变量*/
iNode iNode_table[INODE_NUM];    //iNode table的数组，数组下标对应iNode编号
myFile file_table[NR_FILE];      //系统文件表数组
iNode* pwd;                      //当前工作目录i节点指针
iNode* root;   					 //当前的根目录i节点指针


/*内部函数声明*/

//将文件长度截为0  输入：文件的inode指针  输出：无
void truncate(iNode *inode);

//文件块映射到盘块的处理操作  输入：文件的inode指针，文件中的数据块号，创建标志（如果为1则在逻辑块不存在时申请新的磁盘块）  返回：设备上的逻辑块号
int bmap(iNode *inode, int block, int create);

//将inode的引用数减1，可以会视情况写回或删除i节点  输入：inode的指针  返回：无
void iput(iNode *inode);

//由i节点号获取inode结构体指针  输入：i节点号  返回：inode的指针
iNode *iget(int nr);

//从i节点表中获取一个空闲的i节点项  输入：无  返回：inode的指针
iNode *get_empty_inode();

//从目录表中找目录项  输入：目录inode指针、文件名、目录项指针  返回：成功返回1，失败返回0
int find_entry(iNode *inode, string name, dir_entry* dir, int &de);

//在目录表的某位置加一个新目录项  输入：目录表节点指针，目录项编号，要写入的名字，要写入的节点编号  返回：成功返回1，失败返回0
int add_entry(iNode *inode, int de, string name, unsigned int nr);

//寻找指定路径文件的文件名和父目录的iNode  输入：路径名，父目录iNode指针的指针，文件名变量的引用  返回：成功返回1，失败返回0
int namei(string path, iNode** father, string& filename);

//文件系统内部打开文件  输入：文件inode指针、写入模式（0为追加，1为从当前位置写入，2为覆盖） 返回：返回文件结构体指针  
myFile *openFile(iNode *inode, int mode);

//文件系统内部创建文件  输入：文件父目录的inode指针、文件名、文件类型  返回：文件的inode指针
iNode *createFile(iNode *father, string name, int type);

//读文件  输入：缓冲区指针、欲读字节数、文件inode指针、读写指针位置  返回：成功读出的字节数
int readFile(void *v_buf, int count, iNode *inode, myFile *file);

//写文件  输入：缓冲区指针、欲写字节数、文件inode指针、读写指针位置  返回：成功写入的字节数
int writeFile(void *v_buf, int count, iNode *inode, myFile *file);


/*系统调用声明*/

//文件系统初始化  输入：无  返回：成功返回1，失败返回0
int init_filesystem();

//创建文件  输入：文件路径、文件类型（0表示目录文件，1表示普通文件）  返回：成功返回1，失败返回0
int CreateFile(string path, int type);

//删除文件  输入：文件路径  返回：成功返回1，失败返回0
int DeleteFile(string path);

//打开文件  输入：文件路径、如果没有是否创建、写入模式（0为追加，1为从当前位置写入，2为覆盖） 返回：返回文件结构体指针
myFile* OpenFile(string path, int create, int mode);

//关闭文件  输入：文件结构体指针  返回：成功返回1，失败返回0
int CloseFile(myFile* file);

//标准读文件  输入：缓冲区指针、欲读字节数、文件结构体指针  返回：成功读出的字节数
int Fread(void* buf, int count, myFile* file);

//标准写文件  输入：缓冲区指针、欲写字节数、文件结构体指针  返回：成功写入的字节数
int Fwrite(void* buf, int count, myFile* file);

//修改当前目录下文件名  输入：原来的名称、现在的名称  输出：成功返回1，失败返回0
int rename(string name, string now);

//目录遍历  输入：无  返回：当前目录的所有目录项内容
vector<vector<string> > dir_ls();

//切换当前目录  输入：要切换到的目录  返回：成功返回1，失败返回0
int dir_cd(string path);
                               