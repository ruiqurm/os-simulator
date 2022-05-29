#pragma once
#include<ctime>
#include<iostream>
#include<cstdlib>
#include<ctime>
#include<string>
#include <stack>  
#include<map>
#include<vector>
#include "../memory/memory.h"
#include "../filesystem/file_management.h"
#include "../interupt/interupt.h"
#include "../device/device.h"


#define READY 0 //定义各种状态代表的数值
#define RUN 1
#define BLOCK 2
#define END 3
#define SUSPEND 4
#define CREATE 0 //指令的编码
#define DELETE 1
#define APPLY 2
#define REALESR 3
#define BLOCKCMD 4
#define WAKE 5

using namespace std;
typedef struct cmd {//指令格式
	int time;//指令运行的时间
	int num;//指令对应的编码
	int num2;//需要唤醒或阻塞的进程PID，文件size或申请的设备代码
	string path;//创建或删除文件的路径
}cmd;

typedef struct PCB {
	int PID;//进程
	int state; //进程状态
	int size;//进程所需内存
	int dataSize;//进程数据所占内存
	int nowSize;//进程剩余内存
	//char* path;//进程文件路径
	string path;//进程文件路径
	myFile* myFile; // 进程文件路径指针
	//FILE* myFile;
	int arriveTime; // 进程到达时间
	int needTime; // 进程总共需要运行的时间
	int remainTime; // 进程还需运行的时间
	int finalTime; // 进程运行结束的时间
	stack<cmd> cmdStack; // 指令栈
	v_address address;   //虚拟地址
}PCB;

extern map<int, PCB> proMap;//存储未结束的所有PCB信息
extern vector<PCB> endVector;//存储已经结束的PCB信息
extern vector<PCB> readVector;//存储reafy状态的PCB信息



int getCmd(PCB* newPCB);//输入指令内容
int testPCB(PCB* newPCB);//测试PCB内的数据有无问题
int createProc(string path);//创建进程，返回1创建成功，0失败
void eraseRead(int PID);//删除readVector中对应PID的PCB
int wakeup(int PID);//wakeup进程，返回1 wakeup成功，0失败
int suspend(int PID, v_address address);//suspend进程，返回1 suspend成功，0失败
int active(int PID);//active进程，返回1 active成功，0失败
int stop(int PID, v_address address);//stop进程，返回1 stop成功，0失败
int runCmd(PCB* runPCB);//运行进程的指令，如无中断等情况则返回1，否则返回0
void run();//需要定期调用此进程
