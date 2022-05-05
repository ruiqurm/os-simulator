#include"program.h"

int createPID(){
	return ++PID;
}
int getCmd(PCB *newPCB) {//����ָ������
	int buf[3];
	// !Fread(buf, sizeof(buf), newPCB.myFile)
	while (fscanf(newPCB->myFile, "%d%d%d", &buf[0], &buf[1], &buf[2])) {
		if (buf[0] == -1)break;//��ָ��Ϊ-1ʱ�����˳�
		cmd newCmd;
		newCmd.time = buf[0];
		newCmd.num = buf[1];
		newCmd.num2 = buf[2];
		switch (newCmd.num)
		{
		case CREATE://�����ļ�
		case DELETE://ɾ���ļ�
			char c[1000];
			fgets(c, 100, newPCB->myFile);
			newCmd.path = c;
			//!newCmd.path = Fread(newPCB.myFile);//�����ַ���
			if (newCmd.path == "")return 0;
			newPCB->cmdStack.push(newCmd);
			break;
		case APPLY://����ָ��
		case REALESR:
		case BLOCKCMD:
		case WAKE:
			newPCB->cmdStack.push(newCmd);
			break;
		default:
			printf("�������\n");//�������
			return 0;
			break;
		}
	}
	return 1;
}
int testPCB(PCB *newPCB){//����PCB�ڵ�������������
	if (newPCB->needTime <= 0) {//����ʱ��С��0
		printf("currenttime is %d,parameter error.\n",newPCB->needTime);
		return 0;
	}
	if (newPCB->size< newPCB->dataSize){ //�ڴ治��
		printf("size is %d %d\n",newPCB->size,newPCB->dataSize);
		return 0;
	}
	if (!getCmd(newPCB))return 0;
	return 1;
}

// !string path
int create(string path){//�������̣�����1�����ɹ���0ʧ��
	//!myFile* f = OpenFile(path);//�򿪽���������ļ�
	char c[100];
	strcpy(c, path.c_str());
	printf("%s\n", c);
	FILE* f = fopen(c, "r");//�򿪽���������ļ�
	PCB newPCB;
	newPCB.path = path;
	newPCB.myFile = f;
	if (f==NULL) {//�ж��ļ��Ƿ����
		printf("�ļ�������\n");//�����ɶ�Ӧ��Ա�����޸ģ���ͬ,�ڲ�������֮��Ҳ���޸�
		return 0;
	}
	int buf[3];
	printf("%d\n", f);
	// !Fread(buf, sizeof(buf), newPCB.myFile)
	if (fscanf(newPCB.myFile, "%d%d%d", &buf[0], &buf[1], &buf[2]) == 3){ // ��ȡǰ12���ֽڣ����Ѷ�Ӧλ�õ�����д��PCB��
		newPCB.needTime = buf[0];
		newPCB.arriveTime = nowTime;
		newPCB.remainTime = newPCB.needTime;
		newPCB.size = buf[1];
		newPCB.dataSize = buf[2];
		newPCB.state = READY;
		newPCB.nowSize=newPCB.size-newPCB.dataSize;
		if (!testPCB(&newPCB)) {
			fclose(f);
			//!closeFile(f);
			return 0;
		}
		newPCB.PID= createPID();
	}else{//�ļ���ȡʧ��
		printf("file %s read initdata error.\n", path);
		fclose(f);
		//!closeFile(f);
		return 0;
	}
	fclose(f);
	//!closeFile(f);
	/*!if (osMalloc(newPCB.PID, newPCB.size) {
		printf("�ڴ����ʧ��");//�ڴ����ʧ��	
		return 0;
	}*/
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
		printf("BLOCK:�����ڶ�Ӧ���̻����״̬����\n");//�����ڶ�Ӧ���̻����״̬����
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
		printf("WAKEUP:�����ڶ�Ӧ���̻����״̬����\n");//�����ڶ�Ӧ���̻����״̬����
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
			printf("����״̬����\n");//����״̬����
			return 0;
		}
		proMap[PID].state = SUSPEND;
		//!osRealse(PID);		//�ͷ��ڴ棬�ͷ��豸
		//!realse(PID,0);//0 �����ͷ������豸
	}
	else {
		printf("�����ڶ�Ӧ����\n");//�����ڶ�Ӧ����
		return 0;
	}
	return 1;
}

int active(int PID) {//active���̣�����1 active�ɹ���0ʧ��
	if (proMap.find(PID) != proMap.end()) {
		if (proMap[PID].state = SUSPEND) {
			/*if (osMalloc(proMap[PID].PID, proMap[PID].size) {
				printf();//�ڴ����ʧ��	
				return 0;
			}*/
		}
		else if (proMap[PID].state != BLOCK) {
			printf("����״̬����\n");//����״̬����
			return 0;
		}
		proMap[PID].state = READY;
		readVector.insert(readVector.end(), proMap[PID]);
	}
	else {
		printf("�����ڶ�Ӧ����\n");//�����ڶ�Ӧ����
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
		proMap[PID].finalTime = nowTime;
		//!osRealse(PID);		//�ͷ��ڴ棬�ͷ��豸
		//!realse(PID, 0);//0 �����ͷ������豸
		endVector.insert(endVector.end(), proMap[PID]);
		proMap.erase(PID);
	}
	else {
		printf("�����ڶ�Ӧ����\n");//�����ڶ�Ӧ����
		return 0;
	}
	return 1;
}


int runCmd(PCB *runPCB) {//���н��̵�ָ������жϵ�����򷵻�1�����򷵻�0
	int now = runPCB->needTime - runPCB->remainTime;//������ʱ��Ƭ����
	while(!runPCB->cmdStack.empty() && runPCB->cmdStack.top().time <= now) {//һ����������һ��ָ������ֹһ���˴����Ըĳ�while
		cmd nowCmd = runPCB->cmdStack.top();
		runPCB->cmdStack.pop();
		switch (nowCmd.num)
		{
		case CREATE://�����ļ�
			//!createFile(runPCB.PID, nowCmd.path, nowCmd.num2);
			printf("�����ļ�\n");
			break;
		case DELETE://ɾ���ļ�
			//!deleteFile(runPCB.PID, nowCmd.path);
			printf("ɾ���ļ�\n");
			break;
		case APPLY://�����豸
			/*!if (!apply(runPCB.PID, nowCmd.num2)) {//��������豸ʧ��
				block(runPCB.PID);
			}*/
			printf("�����豸\n");
			break;
		case REALESR://�ͷ��豸
			//! release(runPCB.PID, nowCmd.num2);
			printf("�ͷ��豸\n");
			break;
		case BLOCKCMD://������������
			block(nowCmd.num2);
			break;
		case WAKE://������������
			wakeup(nowCmd.num2);
			break;
		default:
			printf("ָ���������\n");//ָ���������
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
		if (!runCmd(&runPCB))return;
		runPCB.remainTime--;
		if (runPCB.remainTime == 0) {//����Ѿ����н�������������̣������������ջ��
			stop(runPCB.PID);
		}
		else {
			readVector.insert(readVector.end(), runPCB);
		}
		printf("%d ", runPCB.PID);
	}
	printf("%d\n", nowTime);
	nowTime++;
}
int main() {
	create("C:\\Users\\86131\\Desktop\\1.txt");
	create("C:\\Users\\86131\\Desktop\\2.txt");
	create("C:\\Users\\86131\\Desktop\\1.txt");
	create("C:\\Users\\86131\\Desktop\\3.txt");
	create("C:\\Users\\86131\\Desktop\\2.txt");
	for (int i = 0; i < 100; i++) {
		run();
		Sleep(200);
	}
}