#include<ctime>
#include<iostream>
#include<cstdlib>
#include<ctime>
#include<string>
#include <stack>  
#include<map>
#include<vector>
#include"program.h"

#define READY 0 //�������״̬�������ֵ
#define RUN 1
#define BLOCK 2
#define END 3
#define SUSPEND 4


using namespace std;
unsigned int PID=0;
int time = 0;//��ǰʱ�䣬Ŀǰδ���ø��·�ʽ
typedef struct cmd {
	int time;
	int num;//ָ���Ӧ�ı���
	int num2;//��Ҫ���ѻ������Ľ���PID���ļ�size��������豸����
	string path;
};

typedef struct PCB {
	int PID;//����
	int	state; //����״̬
	int size;//���������ڴ�
	int dataSize;//����������ռ�ڴ�
	int nowSize;//����ʣ���ڴ�
	string path;//�����ļ�·��
	myFile* myFile; // �����ļ�·��ָ��
	int arriveTime; // ���̵���ʱ��
	int needTime; // �����ܹ���Ҫ���е�ʱ��
	int remainTime; // ���̻������е�ʱ��
	int finalTime; // �������н�����ʱ��
	stack<cmd> cmdStack; // ָ��ջ
};

// stack<PCB> readStack;
map<int, PCB> proMap;
vector<PCB> endVector;
vector<PCB> readVector;


int createPID(){
	return ++PID;
}
int getCmd(PCB newPCB) {//����ָ������
	int buf[2];
	while (Fread(buf, sizeof(buf), newPCB.myFile)) {
		if (buf[0] == -1)break;//��ָ��Ϊ-1ʱ�����˳�
	cmd newCmd;
	newCmd.time = buf[0];
	newCmd.num = buf[1];
	switch (newCmd.num)
	{
	case 0://�����ļ�
		newCmd.path = Fread(newPCB.myFile);//�����ַ���
		if (newCmd.path == "")return 0;
		if (!Fread(newCmd.num2, sizeof(newCmd.num2), newPCB.myFile)) return 0;
		break;
	case 1://ɾ���ļ�
		newCmd.path = Fread(newPCB.myFile);//�����ַ���
		if (newCmd.path == "")return 0;
		break;
	case 2://����ָ��
	case 3:
	case 4:
	case 5:
		if (!Fread(newCmd.num2, sizeof(newCmd.num2), newPCB.myFile)) return 0;
		break;
	default:
		printf();//�������
		return 0;
		break;
	}	
	return 1;
}
int testPCB(PCB newPCB) {//����PCB�ڵ�������������
	if (newPCB.needTime <= 0) {//����ʱ��С��0
		printf("currenttime is %d,parameter error.\n",newPCB.needTime);
		return 0;
	}
	if (newPCB.size< newPCB.dataSize){ //�ڴ治��
		printf("size is %d %d\n",newPCB.size,dataSize);
		return 0;
	}
	if (!getCmd(newPCB))return 0;
	return 1;
}

int create(string path){//�������̣�����1�����ɹ���0ʧ��
	myFile* f = OpenFile(path);//�򿪽���������ļ�
	PCB newPCB;
	newPCB.path = path;
	newPCB.myFile = f;
	if (!f) {//�ж��ļ��Ƿ����
		printf("�ļ�������");//�����ɶ�Ӧ��Ա�����޸ģ���ͬ,�ڲ�������֮��Ҳ���޸�
		return 0;
	}
	int buf[3];
	if (Fread(buf, sizeof(buf), f)){ // ��ȡǰ12���ֽڣ����Ѷ�Ӧλ�õ�����д��PCB��
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
	}else{//�ļ���ȡʧ��
		printf("file %s read initdata error.", path);
		closeFile(f);
		return 0;
	}
	closeFile(f);
	if (osMalloc(newPCB.PID, newPCB.size) {
		printf();//�ڴ����ʧ��	
		return 0;
	}
	proMap[newPCB.PID]=newPCB;
	readVector.insert(readVector.end(),newPCB);
	//readStack.push(newPCB);
	return 1;
}
void eraseRead(int PID) {//ɾ��readVector�ж�ӦPID��PCB
	for (auto i = readVector.begin(); i < readVector.end(); i++) {
		if (i->PID == PID) {
			readVector.erase(i);
			break;
		}
	}
}
int block(int PID) {//block���̣�����1 block�ɹ���0ʧ��
	if (proMap.find(PID) != proMap.end()&& proMap[PID].state==READY) {
		proMap[PID].state = BLOCK;
		eraseRead(PID);		//�޸�readVector
	}else {
		printf();//�����ڶ�Ӧ���̻����״̬����
		return 0;
	}
	return 1;
}
int wakeup(int PID) {//wakeup���̣�����1 wakeup�ɹ���0ʧ��
	if (proMap.find(PID) != proMap.end() && proMap[PID].state == BLOCK) {
		proMap[PID].state = READY;
		readVector.insert(readVector.end(), proMap[PID]);
	}
	else {
		printf();//�����ڶ�Ӧ���̻����״̬����
		return 0;
	}
	return 1;
}

int suspend(int PID) {//suspend���̣�����1 suspend�ɹ���0ʧ��
	if (proMap.find(PID) != proMap.end()) {
		if (proMap[PID].state = READY) {
			eraseRead(PID);
		}
		else if(proMap[PID].state != BLOCK) {
			printf();//����״̬����
			return 0;
		}
		proMap[PID].state = SUSPEND;
		osRealse(PID);		//�ͷ��ڴ棬�ͷ��豸
		realse(PID,0);//0 �����ͷ������豸
	}
	else {
		printf();//�����ڶ�Ӧ����
		return 0;
	}
	return 1;
}

int active(int PID) {//active���̣�����1 active�ɹ���0ʧ��
	if (proMap.find(PID) != proMap.end()) {
		if (proMap[PID].state = SUSPEND) {
			if (osMalloc(proMap[PID].PID, proMap[PID].size) {
				printf();//�ڴ����ʧ��	
				return 0;
			}
		}
		else if (proMap[PID].state != BLOCK) {
			printf();//����״̬����
			return 0;
		}
		proMap[PID].state = READY;
		readVector.insert(readVector.end(), proMap[PID]);
	}
	else {
		printf();//�����ڶ�Ӧ����
		return 0;
	}
	return 1;
}

int stop(int PID) {//stop���̣�����1 stop�ɹ���0ʧ��
	if (proMap.find(PID) != proMap.end()) {
		if (proMap[PID].state == READY) {
			eraseRead(PID);
		}
		proMap[PID].state = END;
		proMap[PID].finalTime = time;
		osRealse(PID);		//�ͷ��ڴ棬�ͷ��豸
		realse(PID, 0);//0 �����ͷ������豸
		endVector.insert(endVector.end(), proMap[PID]);
		proMap.erase(PID);
	}
	else {
		printf();//�����ڶ�Ӧ����
		return 0;
	}
	return 1;
}

int runCmd(PCB runPCB) {//���н��̵�ָ������жϵ�����򷵻�1�����򷵻�0
	int now = runPCB.needTime - runPCB.remainTime;//������ʱ��Ƭ����
	while(!runPCB.cmdStack.empty() && runPCB.cmdStack.top().time <= now) {//һ����������һ��ָ������ֹһ���˴����Ըĳ�while
		cmd nowCmd = runPCB.cmdStack.top();
		runPCB.cmdStack.pop();
		switch (nowCmd.num)
		{
		case 0://�����ļ�
			if (nowCmd.num2 <= runPCB.nowSize) {
				if (createFile(runPCB.PID, nowCmd.path, nowCmd.num2)) {
					runPCB -= nowCmd.num2;
				}
			}
			else {
				printf();//���̿ռ䲻�㣬�ļ�����ʧ��
			}
			break;
		case 1://ɾ���ļ�
			deleteFile(runPCB.PID, nowCmd.path);
			break;
		case 2://�����豸
			if (!apply(runPCB.PID,nowCmd.num2)) {//��������豸ʧ��
				block(runPCB.PID);
			}
			break;
		case 3://�ͷ��豸
			release(runPCB.PID, nowCmd.num2);
			break;
		case 4://������������
			block(nowCmd.num2);
			break;
		case 5://������������
			wakeup(nowCmd.num2);
			break;
		default:
			printf();//ָ���������
			break;
		}
	}
	return 1;
}

void run() {
	//��Ҫ���ڵ��ô˽���
	if (!readVector.empty()) {
		PCB runPCB = readVector[0];
		readVector.erase(readVector.begin());
		if (!runCmd(runPCB))return;
		runPCB.remainTime--;
		if (runPCB.remainTime == 0) {//����Ѿ����н�������������̣������������ջ��
			stop(runPCB.PID);
		}
		else {
			readVector.insert(readVector.end(), runPCB);
		}
	}
}