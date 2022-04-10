#include<ctime>
#include<iostream>
#include<cstdlib>
#include<ctime>
#include<string>
#include <stack>  
#include<map>
#include<vector>
#include"program.h"

#define READY 0 //定义各种状态代表的数值
#define RUN 1
#define BLOCK 2
#define END 3
#define SUSPEND 4


using namespace std;
unsigned int PID=0;
int time = 0;//当前时间，目前未设置更新方式
typedef struct cmd {
	int time;
	int num;//指令对应的编码
	int num2;//需要唤醒或阻塞的进程PID，文件size或申请的设备代码
	string path;
};

typedef struct PCB {
	int PID;//进程
	int	state; //进程状态
	int size;//进程所需内存
	int dataSize;//进程数据所占内存
	int nowSize;//进程剩余内存
	string path;//进程文件路径
	myFile* myFile; // 进程文件路径指针
	int arriveTime; // 进程到达时间
	int needTime; // 进程总共需要运行的时间
	int remainTime; // 进程还需运行的时间
	int finalTime; // 进程运行结束的时间
	stack<cmd> cmdStack; // 指令栈
};

// stack<PCB> readStack;
map<int, PCB> proMap;
vector<PCB> endVector;
vector<PCB> readVector;


int createPID(){
	return ++PID;
}
int getCmd(PCB newPCB) {//输入指令内容
	int buf[2];
	while (Fread(buf, sizeof(buf), newPCB.myFile)) {
		if (buf[0] == -1)break;//当指令为-1时代表退出
	cmd newCmd;
	newCmd.time = buf[0];
	newCmd.num = buf[1];
	switch (newCmd.num)
	{
	case 0://创建文件
		newCmd.path = Fread(newPCB.myFile);//读入字符串
		if (newCmd.path == "")return 0;
		if (!Fread(newCmd.num2, sizeof(newCmd.num2), newPCB.myFile)) return 0;
		break;
	case 1://删除文件
		newCmd.path = Fread(newPCB.myFile);//读入字符串
		if (newCmd.path == "")return 0;
		break;
	case 2://其他指令
	case 3:
	case 4:
	case 5:
		if (!Fread(newCmd.num2, sizeof(newCmd.num2), newPCB.myFile)) return 0;
		break;
	default:
		printf();//输入错误
		return 0;
		break;
	}	
	return 1;
}
int testPCB(PCB newPCB) {//测试PCB内的数据有无问题
	if (newPCB.needTime <= 0) {//服务时间小于0
		printf("currenttime is %d,parameter error.\n",newPCB.needTime);
		return 0;
	}
	if (newPCB.size< newPCB.dataSize){ //内存不足
		printf("size is %d %d\n",newPCB.size,dataSize);
		return 0;
	}
	if (!getCmd(newPCB))return 0;
	return 1;
}

int create(string path){//创建进程，返回1创建成功，0失败
	myFile* f = OpenFile(path);//打开进程所需的文件
	PCB newPCB;
	newPCB.path = path;
	newPCB.myFile = f;
	if (!f) {//判断文件是否存在
		printf("文件不存在");//后续由对应人员进行修改，后同,内部的内容之后也会修改
		return 0;
	}
	int buf[3];
	if (Fread(buf, sizeof(buf), f)){ // 读取前12个字节，并把对应位置的内容写入PCB中
		newPCB.needTime = buf[0];
		newPCB.arriveTime = time;
		newPCB.remainTime = newPCB.needTime;
		newPCB.size = buf[1];
		newPCB.dataSize = buf[2];
		newPCB.state = 0;
		newPCB.nowSize=newPCB.size-newPCB.dataSize;
		if (testPCB(newPCB,f)) {
			closeFile(f);
			return 0;
		}
		newPCB.PID= createPID();
	}else{//文件读取失败
		printf("file %s read initdata error.", path);
		closeFile(f);
		return 0;
	}
	closeFile(f);
	if (osMalloc(newPCB.PID, newPCB.size) {
		printf();//内存分配失败	
		return 0;
	}
	proMap[newPCB.PID]=newPCB;
	readVector.insert(readVector.end(),newPCB);
	//readStack.push(newPCB);
	return 1;
}
void eraseRead(int PID) {//删除readVector中对应PID的PCB
	for (auto i = readVector.begin(); i < readVector.end(); i++) {
		if (i->PID == PID) {
			readVector.erase(i);
			break;
		}
	}
}
int block(int PID) {//block进程，返回1 block成功，0失败
	if (proMap.find(PID) != proMap.end()&& proMap[PID].state==READY) {
		proMap[PID].state = BLOCK;
		eraseRead(PID);		//修改readVector
	}else {
		printf();//不存在对应进程或进程状态不对
		return 0;
	}
	return 1;
}
int wakeup(int PID) {//wakeup进程，返回1 wakeup成功，0失败
	if (proMap.find(PID) != proMap.end() && proMap[PID].state == BLOCK) {
		proMap[PID].state = READY;
		readVector.insert(readVector.end(), proMap[PID]);
	}
	else {
		printf();//不存在对应进程或进程状态不对
		return 0;
	}
	return 1;
}

int suspend(int PID) {//suspend进程，返回1 suspend成功，0失败
	if (proMap.find(PID) != proMap.end()) {
		if (proMap[PID].state = READY) {
			eraseRead(PID);
		}
		else if(proMap[PID].state != BLOCK) {
			printf();//进程状态不对
			return 0;
		}
		proMap[PID].state = SUSPEND;
		osRealse(PID);		//释放内存，释放设备
		realse(PID,0);//0 代表释放所有设备
	}
	else {
		printf();//不存在对应进程
		return 0;
	}
	return 1;
}

int active(int PID) {//active进程，返回1 active成功，0失败
	if (proMap.find(PID) != proMap.end()) {
		if (proMap[PID].state = SUSPEND) {
			if (osMalloc(proMap[PID].PID, proMap[PID].size) {
				printf();//内存分配失败	
				return 0;
			}
		}
		else if (proMap[PID].state != BLOCK) {
			printf();//进程状态不对
			return 0;
		}
		proMap[PID].state = READY;
		readVector.insert(readVector.end(), proMap[PID]);
	}
	else {
		printf();//不存在对应进程
		return 0;
	}
	return 1;
}

int stop(int PID) {//stop进程，返回1 stop成功，0失败
	if (proMap.find(PID) != proMap.end()) {
		if (proMap[PID].state == READY) {
			eraseRead(PID);
		}
		proMap[PID].state = END;
		proMap[PID].finalTime = time;
		osRealse(PID);		//释放内存，释放设备
		realse(PID, 0);//0 代表释放所有设备
		endVector.insert(endVector.end(), proMap[PID]);
		proMap.erase(PID);
	}
	else {
		printf();//不存在对应进程
		return 0;
	}
	return 1;
}

int runCmd(PCB runPCB) {//运行进程的指令，如无中断等情况则返回1，否则返回0
	int now = runPCB.needTime - runPCB.remainTime;//已运行时间片数量
	while(!runPCB.cmdStack.empty() && runPCB.cmdStack.top().time <= now) {//一个周期运行一个指令，如果不止一个此处可以改成while
		cmd nowCmd = runPCB.cmdStack.top();
		runPCB.cmdStack.pop();
		switch (nowCmd.num)
		{
		case 0://创建文件
			if (nowCmd.num2 <= runPCB.nowSize) {
				if (createFile(runPCB.PID, nowCmd.path, nowCmd.num2)) {
					runPCB -= nowCmd.num2;
				}
			}
			else {
				printf();//进程空间不足，文件创建失败
			}
			break;
		case 1://删除文件
			deleteFile(runPCB.PID, nowCmd.path);
			break;
		case 2://申请设备
			if (!apply(runPCB.PID,nowCmd.num2)) {//如果申请设备失败
				block(runPCB.PID);
			}
			break;
		case 3://释放设备
			release(runPCB.PID, nowCmd.num2);
			break;
		case 4://阻塞其他进程
			block(nowCmd.num2);
			break;
		case 5://唤醒其他进程
			wakeup(nowCmd.num2);
			break;
		default:
			printf();//指令输入出错
			break;
		}
	}
	return 1;
}

void run() {
	//需要定期调用此进程
	if (!readVector.empty()) {
		PCB runPCB = readVector[0];
		readVector.erase(readVector.begin());
		if (!runCmd(runPCB))return;
		runPCB.remainTime--;
		if (runPCB.remainTime == 0) {//如果已经运行结束，则结束进程，否则继续丢入栈中
			stop(runPCB.PID);
		}
		else {
			readVector.insert(readVector.end(), runPCB);
		}
	}
}