#include<ctime>
#include<iostream>
#include<cstdlib>
#include<ctime>
#include<string>
#include <stack>  
#include<map>
#include<vector>
#include "../memory/memory.h"


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
int PID = 0;
int nowTime = 0;//当前时间，目前未设置更新方式
typedef struct cmd {
	int time;
	int num;//指令对应的编码
	int num2;//需要唤醒或阻塞的进程PID，文件size或申请的设备代码
	//char* path;
	string path;
}cmd;

typedef struct PCB {
	int PID;//进程
	int	state; //进程状态
	int size;//进程所需内存
	int dataSize;//进程数据所占内存
	int nowSize;//进程剩余内存
	//char* path;//进程文件路径
	string path;//进程文件路径
	// myFile* myFile; // 进程文件路径指针
	FILE* myFile;
	int arriveTime; // 进程到达时间
	int needTime; // 进程总共需要运行的时间
	int remainTime; // 进程还需运行的时间
	int finalTime; // 进程运行结束的时间
	stack<cmd> cmdStack; // 指令栈
	v_address address;   //虚拟地址
}PCB;

// stack<PCB> readStack;
map<int, PCB> proMap;
vector<PCB> endVector;
vector<PCB> readVector;
