#include"program.h"


map<int, PCB> proMap;//存储未结束的所有PCB信息
vector<PCB> endVector;//存储已经结束的PCB信息
vector<PCB> readVector;//存储reafy状态的PCB信息
int PID = 0;
int nowTime = 0;//当前时间

int createPID(){
	return ++PID;
}

//样例：1 2 3 xx
//提取上述样例的值，对应buf[0-2]，最后一个string作为返回值（有可能不存在）
string getNum(int buf[3], PCB *newPCB) {
	string temp = "";
	int flag = 1;
	char c[1000];
	fgets(c, newPCB->myFile);
	int j = 0;
	for (int i = 0; i < 3; i++) {
		buf[i] = 0;
		if (c[i] == '-') {
			buf[0] = -1;
			flag = 0;
			break;
		}
		while (c[j]!=' '&&c[j]!=EOF&&c[j]!='\n') {
			buf[i] = buf[i] * 10 + c[j] - '0';
			j++;
		}
		if (c[j] == EOF || c[j] == '\n')flag = 0;
		j++;
	}
	if (flag) {
		while (c[j] != EOF && c[j]!= '\n') {
			temp += c[j++];
		}
	}
	return temp;
}

int getCmd(PCB *newPCB) {//输入指令内容
	int buf[3];
	// fscanf(newPCB->myFile, "%d%d%d", &buf[0], &buf[1], &buf[2])
	while (true) {
		newCmd.path = getNum(buf, newPCB);
		if (buf[0] == -1)break;//当指令为-1时代表退出
		cmd newCmd;
		newCmd.time = buf[0];
		newCmd.num = buf[1];
		newCmd.num2 = buf[2];
		switch (newCmd.num)
		{
		case CREATE://创建文件
		case DELETE://删除文件
			char c[1000];
			//fgets(c, 100, newPCB->myFile);
			//fgets(c,newPCB->myFile);//读入字符串
			//newCmd.path = c;
			if (newCmd.path == "")return 0;
			newPCB->cmdStack.push(newCmd);
			break;
		case APPLY://其他指令
		case REALESR:
		case BLOCKCMD:
		case WAKE:
			newPCB->cmdStack.push(newCmd);
			break;
		default:
			printf("输入错误\n");//输入错误
			return 0;
			break;
		}
	}
	return 1;
}
int testPCB(PCB *newPCB){//测试PCB内的数据有无问题
	if (newPCB->needTime <= 0) {//服务时间小于0
		printf("currenttime is %d,parameter error.\n",newPCB->needTime);
		return 0;
	}
	if (newPCB->size< newPCB->dataSize){ //内存不足
		printf("size is %d %d\n",newPCB->size,newPCB->dataSize);
		return 0;
	}
	if (!getCmd(newPCB))return 0;
	return 1;
}

// !string path
int createProc(string path){//创建进程，返回1创建成功，0失败
	myFile* f = OpenFile(path,0,1);//打开进程所需的文件
	char c[100];
	strcpy(c, path.c_str());
	printf("%s\n", c);
	//FILE* f = fopen(c, "r");//打开进程所需的文件
	PCB newPCB;
	newPCB.path = path;
	newPCB.myFile = f;
	if (f==NULL) {//判断文件是否存在
		printf("文件不存在\n");//后续由对应人员进行修改，后同,内部的内容之后也会修改
		return 0;
	}
	int buf[3];
	printf("%d\n", f);
	//fscanf(newPCB.myFile, "%d%d%d", &buf[0], &buf[1], &buf[2]) == 3	 Fread(buf, sizeof(buf), newPCB.myFile)
	if (getNum(buf,&newPCB)!="") { // 读取前12个字节，并把对应位置的内容写入PCB中
		newPCB.needTime = buf[0];
		newPCB.arriveTime = nowTime;
		newPCB.remainTime = newPCB.needTime;
		newPCB.size = buf[1];
		newPCB.dataSize = buf[2];
		newPCB.state = READY;
		newPCB.nowSize=newPCB.size-newPCB.dataSize;
		if (!testPCB(&newPCB)) {
			CloseFile(f);
			return 0;
		}
		newPCB.PID= createPID();
	}else{//文件读取失败
		printf("file %s read initdata error.\n", path);
		CloseFile(f);
		return 0;
	}
	CloseFile(f);
	if (alloc(&(newPCB.address), newPCB.size, newPCB.PID) != 0 ) {
		printf("内存分配失败");//内存分配失败	
		return 0;
	}
	proMap[newPCB.PID]=newPCB;
	readVector.insert(readVector.end(),newPCB);
	//readStack.push(newPCB);
	return 1;
}
void eraseRead(int PID) {//删除readVector中对应PID的PCB
	for (auto i = readVector.begin(); i != readVector.end(); i++) {
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
		printf("BLOCK:不存在对应进程或进程状态不对\n");//不存在对应进程或进程状态不对
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
		printf("WAKEUP:不存在对应进程或进程状态不对\n");//不存在对应进程或进程状态不对
		return 0;
	}
	return 1;
}

int suspend(int PID, v_address address) {//suspend进程，返回1 suspend成功，0失败
	if (proMap.find(PID) != proMap.end()) {
		if (proMap[PID].state = READY) {
			eraseRead(PID);
		}
		else if(proMap[PID].state != BLOCK) {
			printf("进程状态不对\n");//进程状态不对
			return 0;
		}
		proMap[PID].state = SUSPEND;
		free(address, PID);		//释放内存，释放设备
		release(PID,-1);//-1 代表释放所有设备
	}
	else {
		printf("不存在对应进程\n");//不存在对应进程
		return 0;
	}
	return 1;
}

int active(int PID) {//active进程，返回1 active成功，0失败
	if (proMap.find(PID) != proMap.end()) {
		if (proMap[PID].state = SUSPEND) {
			if(alloc(&(proMap[PID].address), proMap[PID].size, proMap[PID].PID)) {
				printf("内存分配失败\n");//内存分配失败	
				return 0;
			}
		}
		else if (proMap[PID].state != BLOCK) {
			printf("进程状态不对\n");//进程状态不对
			return 0;
		}
		proMap[PID].state = READY;
		readVector.insert(readVector.end(), proMap[PID]);
	}
	else {
		printf("不存在对应进程\n");//不存在对应进程
		return 0;
	}
	return 1;
}

int stop(int PID,v_address address) {//stop进程，返回1 stop成功，0失败
	if (proMap.find(PID) != proMap.end()) {
		if (proMap[PID].state == READY) {
			eraseRead(PID);
		}
		proMap[PID].state = END;
		proMap[PID].finalTime = nowTime;
		free(address, PID);		//释放内存，释放设备
		release(PID, -1);//-1 代表释放所有设备
		endVector.insert(endVector.end(), proMap[PID]);
		proMap.erase(PID);
	}
	else {
		printf("不存在对应进程\n");//不存在对应进程
		return 0;
	}
	return 1;
}


int runCmd(PCB *runPCB) {//运行进程的指令，如无中断等情况则返回1，否则返回0
	int now = runPCB->needTime - runPCB->remainTime;//已运行时间片数量
	while(!runPCB->cmdStack.empty() && runPCB->cmdStack.top().time <= now) {//一个周期运行一个指令，如果不止一个此处可以改成while
		cmd nowCmd = runPCB->cmdStack.top();
		runPCB->cmdStack.pop();
		switch (nowCmd.num)
		{
		case CREATE://创建文件
			CreateFile(nowCmd.path, nowCmd.num2);
			printf("创建文件");
			break;
		case DELETE://删除文件
			DeleteFile(nowCmd.path);
			printf("删除文件");
			break;
		case APPLY://申请设备
			if (!acquire(runPCB->PID, nowCmd.num2)) {//如果申请设备失败
				block(runPCB->PID);
			}
			printf("申请设备");
			break;
		case REALESR://释放设备
			release(runPCB->PID, nowCmd.num2);
			printf("释放设备");
			break;
		case BLOCKCMD://阻塞其他进程
			block(nowCmd.num2);
			printf("block:%d",nowCmd.num2);
			break;
		case WAKE://唤醒其他进程
			wakeup(nowCmd.num2);
			printf("wakeup:%d",nowCmd.num2);
			break;
		default:
			printf("指令输入出错");//指令输入出错
			break;
		}
		handle_interupt();
	}
	return 1;
}

void run() {
	//需要定期调用此进程
	printf("nowTime：%d ", nowTime);
	if (!readVector.empty()) {
		PCB runPCB = readVector[0];
		printf("running process PID：%d needTime：%d ", runPCB.PID, runPCB.remainTime);
		readVector.erase(readVector.begin());
		runCmd(&runPCB);
		runPCB.remainTime--;
		proMap[runPCB.PID] = runPCB;
		if (runPCB.remainTime <= 0) {//如果已经运行结束，则结束进程，否则继续丢入栈中
			stop(runPCB.PID,runPCB.address);
		}
		else {
			if (runPCB.state == READY) {
				readVector.insert(readVector.end(), runPCB);
			}
		}
	}	
	printf("\n");
	nowTime++;
}
