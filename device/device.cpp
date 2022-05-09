#include "device.h"

class Device {
public:
	map<int, vector<long>> allDevice;
	vector<string> deviceName;//尚未初始化，需在main中初始化

	void show() {
		for (auto i = allDevice.begin(); i != allDevice.end(); i++) {
			cout << "Devices' state" << endl;
			cout << "Device:" << deviceName[i->first] << " used by pid: " << i->second[0] << endl;
		}
		for (auto i = allDevice.begin(); i != allDevice.end(); i++) {
			int waitnum = i->second.size();
			if (waitnum > 1) {
				cout << "Waiting queue" << endl;
				cout << "Device:" << i->first << "'s waiting queue is: ";
				for (int j = 1; j < waitnum; j++)
					cout << i->second[j] << ' ';
				cout << endl;
			}
		}
	}
	bool acquire(int device, long pid) {
		allDevice[device].push_back(pid);
		if (allDevice[device].size() > 1)
			return false;
		else
			return true;
	}
	bool release(int device, long pid) {
		for (auto ptr = allDevice[device].begin(); ptr != allDevice[device].end(); ptr++) {
			if (*ptr == pid) {
				allDevice[device].erase(ptr);
				return true;
			}
		}
		return false;
	}
};

class Disk:public Device {
public:
	string diskname = "disk.txt";
	int maxDataBlockNum = 1024; //最多1024个物理块
	int maxBlockNum = 1024 + 132;
	int blockSize = 64; //每个物理块大小为64字节
	int maxFileNum = 256; //最多文件数量、最大inode数量
	int maxFileSize = 256; //每个文件的最大大小
	int blockNumInOneFile = maxFileSize / blockSize; //每个文件占用物理块数

	int superBlockStart = 0; //超级块
	int iNodeBitMapStart = 1; //iNode位图
	int dataBitMapStart = 2; //数据块位图
	int iNodeStart = 4; //dataBitMapStart + ceil(1.0 * maxDataBlockNum / (sizeof(char) * blockSize));
	int dataStart = iNodeStart + 128; //132, 共128块用来存inode

	//初始化iNode
	void init_iNode(iNode* inodeExample) {
		inodeExample->i_mode = 2;
		inodeExample->i_size = 0;
		inodeExample->nlinks = 0;
		inodeExample->mtime = time(NULL);
		for (int i = 0; i < 4; i++)
			inodeExample->i_zone[i] = maxBlockNum;
		inodeExample->i_count = 0;
		inodeExample->i_dirt = 0;
		inodeExample->i_num = 0;
	}
	//磁盘初始化
	void disk_init() {
		//写入0
		FILE* disk = fopen(diskname.c_str(), "wb");
		char* buf = (char*)malloc(blockSize * maxBlockNum);
		fwrite(buf, blockSize * maxBlockNum, 1, disk);
		fclose(disk);

		//初始化超级块
		superBlock* sup = (superBlock*)malloc(sizeof(superBlock));
		sup->blockNum = maxBlockNum;
		sup->inodeNum = maxFileNum;
		sup->maxFileSize = maxFileSize;
		//初始化iNodeBitMap
		char* iNodeBitMap = (char*)malloc(sizeof(char) * maxFileNum);
		for (int i = 0; i < maxFileNum; i++) {
			if (i == 0)
				*(iNodeBitMap + i) = 1;
			else
				*(iNodeBitMap + i) = 0;
		}
		//初始化dataBitMap
		char* dataBitMap = (char*)malloc(sizeof(char) * maxDataBlockNum);
		for (int i = 0; i < maxBlockNum; i++) {
			if (i == 0)
				*(dataBitMap + i) = 1;
			else
				*(dataBitMap + i) = 0;
		}
		//初始化iNode
		iNode* iNodes = (iNode*)malloc(sizeof(iNode) * maxFileNum);
		for (int i = 0; i < maxFileNum; i++) {
			init_iNode(iNodes + i);
		}

		//写入磁盘
		FILE* disk = fopen(diskname.c_str(), "rb+");
		fwrite(sup, sizeof(superBlock), 1, disk);

		fseek(disk, iNodeBitMapStart * blockSize, SEEK_SET);
		fwrite(iNodeBitMap, sizeof(char) * maxFileNum, 1, disk);

		fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
		fwrite(dataBitMap, sizeof(char) * maxDataBlockNum, 1, disk);

		fseek(disk, iNodeStart * blockSize, SEEK_SET);
		fwrite(iNodes, sizeof(iNode) * maxFileNum, 1, disk);
		
		free(sup);
		free(iNodes);
		free(dataBitMap);
		free(iNodeBitMap);

		fclose(disk);

		/*
		//初始化iNodeTable
		FILE* disk = fopen(diskname.c_str(), "rb");
		fseek(disk, iNodeStart * blockSize, SEEK_SET);
		fread(&iNode_table, maxFileNum * sizeof(iNode), 1, disk);
		fclose(disk);
		*/
	}


	int new_block() {
		char* dataBitMap = (char*)malloc(sizeof(char) * maxDataBlockNum);
		FILE* disk = fopen(diskname.c_str(), "rb+");
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
		fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
		fwrite(dataBitMap, sizeof(char), maxDataBlockNum, disk);
		fclose(disk);
		if (position == -1) {
			cout << "无空闲数据块";
		}
		free(dataBitMap);
		return position;
	}
	void free_block(int blockSeq) {
		FILE* disk = fopen(diskname.c_str(), "rb+");
		fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
		fseek(disk, blockSeq * sizeof(char), SEEK_CUR);
		char f = '0';
		char* nullBlock = (char*)malloc(blockSize * sizeof(char));
		fwrite(&f, sizeof(char), 1, disk);

		fseek(disk, dataStart * blockSize, SEEK_SET);
		fseek(disk, blockSeq * blockSize, SEEK_CUR);
		fwrite(nullBlock, blockSize, 1, disk);
		free(nullBlock);
		fclose(disk);
	}
	//物理块写入函数，nr是块号，offset是开始读的位置在块中的偏移量，chars是要读入的字节数，最后一个参数是缓冲区指针
	bool block_write(int blockSeq, int offset, int charNum, char* buf) {
		FILE* disk = fopen(diskname.c_str(), "rb+");
		if (disk) {
			fseek(disk, blockSeq * blockSize + offset, SEEK_SET);
			fwrite(buf, sizeof(char), charNum, disk);
			fclose(disk);
			return true;
		}
		else {
			printf("Error: Cannot operate on disk!\n"); //改成log
			return false;
		}
	}
	//物理块读出函数，返回读出的字节大小
	bool block_read(int blockSeq, int offset, int charNum, char* buf) {
		FILE* disk = fopen(diskname.c_str(), "r");
		if (disk) {
			fseek(disk, blockSeq * blockSize + offset, SEEK_SET);
			fread(buf, sizeof(char), charNum, disk);
			fclose(disk);
			return true;
		}
		else {
			printf("Error: Cannot operate on disk!\n"); //改成log
			return false;
		}
	}
	
	iNode* new_iNode() {
		iNode* ptr = (iNode*)malloc(sizeof(iNode));
		char* iNodeBitMap = (char*)malloc(sizeof(char) * maxFileNum);
		FILE* disk = fopen(diskname.c_str(), "rb+");
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
		fseek(disk, dataBitMapStart * blockSize, SEEK_SET);
		fwrite(iNodeBitMap, sizeof(char), maxFileNum, disk);
		fclose(disk);
		if (position == -1) {
			cout << "无空闲iNode";
		}
		ptr->i_num = position;
		free(iNodeBitMap);
		return ptr;
	}
	void free_iNode(iNode* inode) {
		FILE* disk = fopen(diskname.c_str(), "rb+");
		int position = inode->i_num;
		char f = '0';
		iNode* nullBlock = (iNode*)malloc(sizeof(iNode));
		init_iNode(nullBlock);
		fseek(disk, iNodeBitMapStart * blockSize + position, SEEK_SET);
		fwrite(&f, sizeof(char), 1, disk);

		fseek(disk, iNodeStart * blockSize, SEEK_SET);
		fseek(disk, position * sizeof(iNode), SEEK_CUR);
		fwrite(nullBlock, sizeof(iNode), 1, disk);
		free(inode);
		free(nullBlock);
		fclose(disk);
	}
	bool read_iNode(iNode* inode) {
		int position = inode->i_num;
		FILE* disk = fopen(diskname.c_str(), "r");
		fseek(disk, iNodeStart * blockSize, SEEK_SET);
		fseek(disk, position * sizeof(iNode), SEEK_CUR);
		fread(inode, sizeof(iNode), 1, disk);
		inode->i_num = position;
		fclose(disk);
	}
	bool write_iNode(iNode* inode) {
		int position = inode->i_num;
		FILE* disk = fopen(diskname.c_str(), "rb+");
		fseek(disk, iNodeStart * blockSize, SEEK_SET);
		fseek(disk, position * sizeof(iNode), SEEK_CUR);
		fwrite(inode, sizeof(iNode), 1, disk);
		fclose(disk);
	}
};
