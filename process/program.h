#include<ctime>
#include<iostream>
#include<cstdlib>
#include<ctime>
#include<string>
#include <stack>  
#include<map>
#include<vector>
#include "windows.h"
#include "../memory/memory.h"


#define READY 0 //�������״̬�������ֵ
#define RUN 1
#define BLOCK 2
#define END 3
#define SUSPEND 4
#define CREATE 0 //ָ��ı���
#define DELETE 1
#define APPLY 2
#define REALESR 3
#define BLOCKCMD 4
#define WAKE 5

using namespace std;
int PID = 0;
int nowTime = 0;//��ǰʱ�䣬Ŀǰδ���ø��·�ʽ
typedef struct cmd {
	int time;
	int num;//ָ���Ӧ�ı���
	int num2;//��Ҫ���ѻ������Ľ���PID���ļ�size��������豸����
	//char* path;
	string path;
}cmd;

typedef struct PCB {
	int PID;//����
	int	state; //����״̬
	int size;//���������ڴ�
	int dataSize;//����������ռ�ڴ�
	int nowSize;//����ʣ���ڴ�
	//char* path;//�����ļ�·��
	string path;//�����ļ�·��
	// myFile* myFile; // �����ļ�·��ָ��
	FILE* myFile;
	int arriveTime; // ���̵���ʱ��
	int needTime; // �����ܹ���Ҫ���е�ʱ��
	int remainTime; // ���̻������е�ʱ��
	int finalTime; // �������н�����ʱ��
	stack<cmd> cmdStack; // ָ��ջ
	v_address address;   //�����ַ
}PCB;

// stack<PCB> readStack;
map<int, PCB> proMap;
vector<PCB> endVector;
vector<PCB> readVector;