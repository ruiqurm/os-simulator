#include "device.h"
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

vector<long> allDevice[deviceNum];
string deviceName[] = { "disk","mouse","displayer","printer","keyboard" };

void log(string s) {
	s += '\n';
	FILE* fp = fopen(logname, "a");
	if (fp == nullptr) {
		log("无法打开文件" + string(logname));
		exit(1);
	}
	fwrite(s.c_str(), sizeof(char), s.length(), fp);
	fclose(fp);
}
void log(int a) {
	FILE* fp = fopen(logname, "a");
	if (fp == nullptr) {
		log("无法打开文件" + string(logname));
		exit(1);
	}
	string s;
	switch (a) {
	case 1:
		s = string("malloc申请空间失败");
		break;
	case 2:
		log("无法打开文件" + string(logname));
		break;
	case 3:
		log("无法打开文件" + string(diskname));
		break;
	}
	fwrite(s.c_str(), sizeof(char), s.length(), fp);
	fclose(fp);
}

void showDevice() {
	cout << "--------------------------" << endl;
	cout << "Devices' State" << endl;
	for (auto i = 0; i < deviceNum; i++) {
		cout << "Device name:" << deviceName[i] << endl;
		int length = allDevice[i].size();
		if (length == 0) {
			cout << "This device is free" << endl;
		}
		else {
			cout << "Used by progress: " << allDevice[i][0] << endl;
			if (length > 1) {
				cout << "Waiting queue:";
				for (int j = 1; j < length; j++) {
					cout << " " << allDevice[i][j];
				}
				cout << endl;
			}
		}
	}
	cout << "--------------------------" << endl;
}
bool acquire(long pid, int device) {
	//先检测该进程是否之前申请过该设备
	for (auto ptr = allDevice[device].begin(); ptr != allDevice[device].end(); ptr++) {
		if (*ptr == pid) {
			string e;
			e = "进程" + to_string(pid) + "重复申请" + deviceName[device] + ",申请设备操作失败";
			log(e);
			return false;
		}
	}
	allDevice[device].push_back(pid);
	if (allDevice[device].size() > 1) {
		string w;
		w = "进程" + to_string(pid) + "申请" + deviceName[device] + "并进入等待队列";
		log(w);
		return false; // 进程加入等待队列
	}
	else {
		return true; //进程使用设备
	}
}
bool release(long pid, int device) {
	int flag = 0; //判断该进程之前是否申请过该设备

	if (device == -1) { // 释放该进程申请的全部设备
		for (int device = 0; device < deviceNum; device++) {
			for (auto ptr = allDevice[device].begin(); ptr != allDevice[device].end(); ptr++) {
				if (*ptr == pid) {
					allDevice[device].erase(ptr);
				}
			}
		}
		return true;
	}
	else {
		for (auto ptr = allDevice[device].begin(); ptr != allDevice[device].end(); ptr++) {
			if (*ptr == pid) {
				flag = 1;
				allDevice[device].erase(ptr);
				if (allDevice[device].size() != 0) { // 激活等待队列中的第一个进程
					//raise_interupt(InteruptType::EXTERNAL_1, device, allDevice[device][0]);
					//wakeup(allDevice[device][0]);
				}
				return true;
			}
		}
	}
	
	string e;
	e = "进程" + to_string(pid) + "未申请过" + deviceName[device] + ",释放设备操作失败";
	log(e);
	return false;
}
	

//初始化iNode
void init_iNode(iNodeInDisk* inodeExample) {
	inodeExample->i_mode = 2;
	inodeExample->i_size = 0;
	inodeExample->nlinks = 0;
	inodeExample->mtime = time(NULL);
	for (int i = 0; i < 4; i++)
		inodeExample->i_zone[i] = maxBlockNum;
}
//磁盘初始化
void disk_init(int flag) {
	//初始化日志文件
	FILE* logptr = fopen(logname, "w");
	if (!logptr) { log(2); exit(1); }
	fclose(logptr);

	ifstream f(diskname);
	if (!f.good() || flag == 1) { // 之前未初始化过磁盘
		FILE* disk = fopen(diskname, "wb");
		if (!disk) { log(3); exit(1); }
		char* buf = (char*)malloc(blockSize * maxBlockNum);
		if (!buf) { log(1); exit(1); }
		fwrite(buf, blockSize * maxBlockNum, 1, disk); //写入0
		fclose(disk);

		//初始化超级块
		superBlock* sup = (superBlock*)malloc(sizeof(superBlock));
		if (!sup) { log(1); exit(1); }
		sup->blockNum = maxBlockNum;
		sup->inodeNum = maxFileNum;
		sup->maxfilesize = maxFileSize;
		//初始化iNodeBitMap
		char* iNodeBitMap = (char*)malloc(sizeof(char) * maxFileNum);
		if (!iNodeBitMap) { log(1); exit(1); }
		for (int i = 0; i < maxFileNum; i++) {
			if (i == 0)
				*(iNodeBitMap + i) = '1';
			else
				*(iNodeBitMap + i) = '0';
		}
		//初始化dataBitMap
		char* dataBitMap = (char*)malloc(sizeof(char) * maxDataBlockNum);
		if (!dataBitMap) { log(1); exit(1); }
		for (int i = 0; i < maxDataBlockNum; i++) {
			if (i == 0)
				*(dataBitMap + i) = '1';
			else
				*(dataBitMap + i) = '0';
		}
		//初始化iNode
		iNodeInDisk* iNodes = (iNodeInDisk*)malloc(iNodeSize * maxFileNum);
		if (!iNodes) { log(1); exit(1); }

		//写入磁盘
		disk = fopen(diskname, "rb+");
		if (!disk) { log(3); exit(1); }
		fwrite(sup, sizeof(superBlock), 1, disk);

		fseek(disk, iNodeBitMapStart * blockSize, SEEK_SET);
		fwrite(iNodeBitMap, sizeof(char) * maxFileNum, 1, disk);

		fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
		fwrite(dataBitMap, sizeof(char) * maxDataBlockNum, 1, disk);

		fseek(disk, iNodeStart * blockSize, SEEK_SET);
		fwrite(iNodes, iNodeSize * maxFileNum, 1, disk);

		free(sup);
		free(iNodes);
		free(dataBitMap);
		free(iNodeBitMap);

		fclose(disk);
		/*
		//初始化iNodeTable
		FILE* disk = fopen(diskname, "rb");
		fseek(disk, iNodeStart * blockSize, SEEK_SET);
		fread(&iNode_table, maxFileNum * sizeof(iNode), 1, disk);
		fclose(disk);
		*/

		//初始化根目录
		iNode *inode=new_iNode();
		inode->i_mode=0;
		inode->i_size=64;
		inode->nlinks=2;
		inode->i_zone[0]=new_block();
		write_iNode(inode);
		dir_entry *d=(dir_entry *)malloc(2*sizeof(dir_entry));
		strcpy(d[0].file_name,".");
		d[0].iNode_no=inode->i_num;
		strcpy(d[1].file_name,"..");
		d[1].iNode_no=inode->i_num;
		block_write(inode->i_zone[0],0,blockSize,(char *)d);
	}
}

// 读取超级块
void read_sup() {
	FILE* disk = fopen(diskname, "rb");
	if (!disk) { log(3); exit(1); }
	superBlock* sup = (superBlock*)malloc(sizeof(superBlock));
	if (!sup) { log(1); exit(1); }
	fread(sup, sizeof(superBlock), 1, disk);
	cout << "iNode节点数 " + to_string(sup->inodeNum) + " 物理块数 " + to_string(sup->blockNum);
	cout << " 文件最大长度 " + to_string(sup->maxfilesize) << endl;
	fclose(disk);
}

// 读取前十个位图
void read_map(int flag) {
	FILE* disk = fopen(diskname, "rb");
	if (!disk) { log(3); exit(1); }
	if (flag) {
		char* dataBitMap = (char*)malloc(sizeof(char) * maxDataBlockNum);
		if (!dataBitMap) { log(1); exit(1); }
		fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
		fread(dataBitMap, sizeof(char), maxDataBlockNum, disk);
		cout << "dataMap: ";
		for (int i = 0; i < 10; i++) {
			cout << dataBitMap[i];
		}
		cout << endl;
		free(dataBitMap);
	}
	else {
		char* iNodeBitMap = (char*)malloc(sizeof(char) * maxFileNum);
		if (!iNodeBitMap) { log(1); exit(1); }
		fseek(disk, iNodeBitMapStart * blockSize, SEEK_SET);
		fread(iNodeBitMap, sizeof(char), maxFileNum, disk);
		cout << "inodeMap:";
		for (int i = 0; i < 10; i++) {
			cout << iNodeBitMap[i];
		}
		cout << endl;
		free(iNodeBitMap);
	}
}

int new_block() {
	FILE* disk = fopen(diskname, "rb+");
	if (!disk) { log(3); exit(1); }

	char* dataBitMap = (char*)malloc(sizeof(char) * maxDataBlockNum);
	if (!dataBitMap) { log(1); exit(1); }
	fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
	fread(dataBitMap, sizeof(char), maxDataBlockNum, disk);

	int position = -1;
	for (int i = 0; i < maxDataBlockNum; i++) {
		if (dataBitMap[i] == '0') {
			position = i;
			dataBitMap[i] = '1';
			break;
		}
	}
	// 更新磁盘中的逻辑块位图
	fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
	fwrite(dataBitMap, sizeof(char), maxDataBlockNum, disk);
	fclose(disk);
	free(dataBitMap);
	if (position == -1) {
		log("无空闲数据块");
		return 0;
	}
	else {
		cout << "成功申请到逻辑块 " << position << endl;
		return position;
	}
}
bool free_block(int blockSeq) {
	FILE* disk = fopen(diskname, "rb+");
	if (!disk) { log(3); exit(1); }
	//更新位图
	char* dataBitMap = (char*)malloc(sizeof(char) * maxDataBlockNum);
	if (!dataBitMap) { log(1); exit(1); }
	fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
	fread(dataBitMap, sizeof(char), maxDataBlockNum, disk);
	if (dataBitMap[blockSeq] == '0') {
		log("错误：释放一个未申请的逻辑块");
		return false;
	}
	dataBitMap[blockSeq] = '0';
	fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
	fwrite(dataBitMap, sizeof(char), maxDataBlockNum, disk);
	free(dataBitMap);

	char* nullBlock = (char*)malloc(blockSize);
	if (!nullBlock) { log(1); exit(1); }
	fseek(disk, (dataStart + blockSeq) * blockSize, SEEK_SET);
	fwrite(nullBlock, blockSize, 1, disk);
	free(nullBlock);

	cout << "成功释放掉逻辑块 " << blockSeq << endl;
	fclose(disk);
}
//物理块写入函数，nr是块号，offset是开始读的位置在块中的偏移量，chars是要写入的字节数，最后一个参数是缓冲区指针
bool block_write(int blockSeq, int offset, int charNum, char* buf) {
	if (offset + charNum > blockSize) {
		log("写入操作过界");
		return false;
	}

	FILE* disk = fopen(diskname, "rb+");
	if (!disk) { log(3); exit(1); }
	
	char* dataBitMap = (char*)malloc(sizeof(char) * maxDataBlockNum);
	if (!dataBitMap) { log(1); exit(1); }
	fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
	fread(dataBitMap, sizeof(char), maxDataBlockNum, disk);
	if (dataBitMap[blockSeq] == '0') {
		log("发生错误，向未申请的逻辑块写入！");
		return false;
	}

	fseek(disk, (blockSeq + dataStart) * blockSize + offset, SEEK_SET);
	fwrite(buf, sizeof(char), charNum, disk);
	fclose(disk);
	cout << "向逻辑块写入成功" << endl;
	return true;
}
//物理块读出函数，返回读出的字节大小
bool block_read(int blockSeq, int offset, int charNum, char* buf) {
	if (offset + charNum > blockSize) {
		log("读取操作过界");
		return false;
	}

	FILE* disk = fopen(diskname, "rb");
	if (!disk) { log(3); exit(1); }

	char* dataBitMap = (char*)malloc(sizeof(char) * maxDataBlockNum);
	if (!dataBitMap) { log(1); exit(1); }
	fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
	fread(dataBitMap, sizeof(char), maxDataBlockNum, disk);
	if (dataBitMap[blockSeq] == '0') {
		log("发生错误，读取未申请的逻辑块！");
		return false;
	}

	fseek(disk, (blockSeq + dataStart) * blockSize + offset, SEEK_SET);
	fread(buf, sizeof(char), charNum, disk);
	fclose(disk);
	cout << "从逻辑块读取成功" << endl;
	return true;
}

void br(int num, char* buf) {
	FILE* disk = fopen(diskname, "rb");
	fseek(disk, num * blockSize, SEEK_SET);
	fread(buf, blockSize, 1, disk);
	fclose(disk);
}
	
iNode* new_iNode() {
	FILE* disk = fopen(diskname, "rb+");
	if (!disk) { log(3); exit(1); }

	char* iNodeBitMap = (char*)malloc(sizeof(char) * maxFileNum);
	if (!iNodeBitMap) { log(1); exit(1); }
	fseek(disk, iNodeBitMapStart * blockSize, SEEK_SET);
	fread(iNodeBitMap, sizeof(char), maxFileNum, disk);

	int position = -1;
	for (int i = 0; i < maxFileNum; i++) {
		if (iNodeBitMap[i] == '0') {
			position = i;
			iNodeBitMap[i] = '1';
			break;
		}
	}
	//更新磁盘中的iNode位图
	fseek(disk, iNodeBitMapStart * blockSize, SEEK_SET);
	fwrite(iNodeBitMap, sizeof(char), maxFileNum, disk);
	fclose(disk);
	free(iNodeBitMap);

	if (position == -1) {
		log("无空闲iNode");
		return nullptr;
	}
	else {
		iNode* ptr = (iNode*)malloc(sizeof(iNode));
		if (!ptr) { log(1); exit(1); }
		ptr->i_num = position;
		cout << "成功申请到iNode块 " << position << endl;
		return ptr;
	}
}
void free_iNode(iNode* inode) {
	FILE* disk = fopen(diskname, "rb+");
	if (!disk) { log(3); exit(1); }

	int position = inode->i_num;

	char* iNodeBitMap = (char*)malloc(sizeof(char) * maxFileNum);
	if (!iNodeBitMap) { log(1); exit(1); }
	fseek(disk, iNodeBitMapStart * blockSize, SEEK_SET);
	fread(iNodeBitMap, sizeof(char), maxFileNum, disk);
	if (iNodeBitMap[position] == '0') {
		log("错误：释放一个未申请的iNode块");
		exit(1);
	}
	iNodeBitMap[position] = '0';
	//更新磁盘中的iNode位图
	fseek(disk, iNodeBitMapStart * blockSize, SEEK_SET);
	fwrite(iNodeBitMap, sizeof(char), maxFileNum, disk);
	free(iNodeBitMap);

	iNodeInDisk* nullBlock = (iNodeInDisk*)malloc(iNodeSize);
	if (!nullBlock) { log(1); exit(1); }
	init_iNode(nullBlock);
	fseek(disk, iNodeStart * blockSize, SEEK_SET);
	fseek(disk, position * iNodeSize, SEEK_CUR);
	fwrite(nullBlock, iNodeSize, 1, disk);
	free(nullBlock);

	free(inode);
	cout << "成功释放掉iNode块 " << position << endl;
	fclose(disk);
}
bool read_iNode(iNode* inode) {
	int position = inode->i_num;

	FILE* disk = fopen(diskname, "rb");
	if (!disk) { log(3); exit(1); }

	char* iNodeBitMap = (char*)malloc(sizeof(char) * maxFileNum);
	if (!iNodeBitMap) { log(1); exit(1); }
	fseek(disk, iNodeBitMapStart * blockSize, SEEK_SET);
	fread(iNodeBitMap, sizeof(char), maxFileNum, disk);
	if (iNodeBitMap[position] == '0') {
		log("错误：读取一个未申请的iNode块");
		return false;
	}
	free(iNodeBitMap);

	iNodeInDisk* temp = (iNodeInDisk*)malloc(iNodeSize);
	if (!temp) { log(1); exit(1); }
	fseek(disk, iNodeStart * blockSize, SEEK_SET);
	fseek(disk, position * iNodeSize, SEEK_CUR);
	fread(temp, iNodeSize, 1, disk);
	inode->i_mode = temp->i_mode;
	inode->i_size = temp->i_size;
	inode->nlinks = temp->nlinks;
	inode->mtime = temp->mtime;
	for (int i = 0; i < 4; i++)
		inode->i_zone[i] = temp->i_zone[i];

	free(temp);
	fclose(disk);
	cout << "读出iNode节点成功" << endl;
	return true;
}
bool write_iNode(iNode* inode) {
	int position = inode->i_num;

	FILE* disk = fopen(diskname, "rb+");
	if (!disk) { log(3); exit(1); }

	char* iNodeBitMap = (char*)malloc(sizeof(char) * maxFileNum);
	if (!iNodeBitMap) { log(1); exit(1); }
	fseek(disk, iNodeBitMapStart * blockSize, SEEK_SET);
	fread(iNodeBitMap, sizeof(char), maxFileNum, disk);
	if (iNodeBitMap[position] == '0') {
		log("错误：向一个未申请的iNode块写入");
		return false;
	}
	free(iNodeBitMap);

	iNodeInDisk* temp = (iNodeInDisk*)malloc(iNodeSize);
	if (!temp) { log(1); exit(1); }
	temp->i_mode = inode->i_mode;
	temp->i_size = inode->i_size;
	temp->nlinks = inode->nlinks;
	temp->mtime = inode->mtime;
	for (int i = 0; i < 4; i++)
		temp->i_zone[i] = inode->i_zone[i];
	fseek(disk, iNodeStart * blockSize, SEEK_SET);
	fseek(disk, position * iNodeSize, SEEK_CUR);
	fwrite(temp, iNodeSize, 1, disk);

	free(temp);
	fclose(disk);
	cout << "写入iNode节点成功" << endl;
	return true;
}
