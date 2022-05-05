#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
using namespace std;


/*��Ҫ�����궨��*/
#define NAME_LEN 14	     //�ļ�������󳤶�
#define ROOT_INO 1       //��i�ڵ��
#define INODE_NUM 256	 //iNode������
#define DIR_NUM 8		 //ÿ��Ŀ¼�ļ��µ��ļ���������һ��blockΪ64���ֽڣ��ܷ�����32���ֽڵ�Ŀ¼��
#define PATH_LENGTH 100	 //·���ַ�����󳤶�
#define FBLK_NUM 4		 //�ļ���block��������
#define NR_FILE 64       //ϵͳ�ļ������������
#define NR_OPEN 20       //һ���������򿪵��ļ���
#define ENTRY_SIZE 32    //һ��Ŀ¼��Ĵ�С


/*��Ҫ���ݽṹ*/

//�ļ������ڵ�
typedef struct INODE
{
	unsigned short i_mode;		 //�ļ����ͣ�0Ŀ¼��1��ͨ
	int i_size;				     //�ļ���С���ֽ�����
    int nlinks;					 //�����������ж����ļ�Ŀ¼��ָ�����inode �ж�һ�� i�ڵ��Ƿ�Ӧ�ñ��ͷ�
	time_t mtime;				 //�ļ���ʱ������ļ�������һ�α䶯��ʱ��
	short i_zone[FBLK_NUM];      //�ļ�����block��λ��
	//�����ֶ�ֻ���ڴ���ʹ��
	unsigned short i_count;		 //inode�����ü���
	int i_dirt;					 //�޸ı�ʶ��1��ʾ���޸ģ�0��ʾδ�޸�
	int i_num;                   //i�ڵ��
} iNode;

//�ļ�Ŀ¼��ṹ
typedef struct DIR_ENTRY
{
	char file_name[28];			 //�ļ���
	unsigned int iNode_no;       //iNode���
} dir_entry;

//�ļ��ṹ��
typedef struct FILE
{
	int mode;                    //д��ģʽ��0Ϊ׷�ӣ�1Ϊ�ӵ�ǰλ��д�룬2Ϊ���ǣ�
	unsigned short f_count;		 //��Ӧ�ļ����ü���ֵ
	iNode* f_iNode;				 //ָ���ӦiNode
	long int f_pos;				 //��дָ��
} myFile;


/*ȫ�ֱ���*/
iNode iNode_table[INODE_NUM];    //iNode table�����飬�����±��ӦiNode���
myFile file_table[NR_FILE];      //ϵͳ�ļ�������
iNode* pwd;                      //��ǰ����Ŀ¼i�ڵ�ָ��
iNode* root;   					 //��ǰ�ĸ�Ŀ¼i�ڵ�ָ��


/*�ڲ���������*/

//���ļ����Ƚ�Ϊ0  ���룺�ļ���inodeָ��  �������
void truncate(iNode *inode);

//�ļ���ӳ�䵽�̿�Ĵ������  ���룺�ļ���inodeָ�룬�ļ��е����ݿ�ţ�������־�����Ϊ1�����߼��鲻����ʱ�����µĴ��̿飩  ���أ��豸�ϵ��߼����
int bmap(iNode *inode, int block, int create);

//��inode����������1�����Ի������д�ػ�ɾ��i�ڵ�  ���룺inode��ָ��  ���أ���
void iput(iNode *inode);

//��i�ڵ�Ż�ȡinode�ṹ��ָ��  ���룺i�ڵ��  ���أ�inode��ָ��
iNode *iget(int nr);

//��i�ڵ���л�ȡһ�����е�i�ڵ���  ���룺��  ���أ�inode��ָ��
iNode *get_empty_inode();

//��Ŀ¼������Ŀ¼��  ���룺Ŀ¼inodeָ�롢�ļ�����Ŀ¼��ָ��  ���أ��ɹ�����1��ʧ�ܷ���0
int find_entry(iNode *inode, string name, dir_entry* dir, int &de);

//��Ŀ¼���ĳλ�ü�һ����Ŀ¼��  ���룺Ŀ¼��ڵ�ָ�룬Ŀ¼���ţ�Ҫд������֣�Ҫд��Ľڵ���  ���أ��ɹ�����1��ʧ�ܷ���0
int add_entry(iNode *inode, int de, string name, unsigned int nr);

//Ѱ��ָ��·���ļ����ļ����͸�Ŀ¼��iNode  ���룺·��������Ŀ¼iNodeָ���ָ�룬�ļ�������������  ���أ��ɹ�����1��ʧ�ܷ���0
int namei(string path, iNode** father, string& filename);

//�ļ�ϵͳ�ڲ����ļ�  ���룺�ļ�inodeָ�롢д��ģʽ��0Ϊ׷�ӣ�1Ϊ�ӵ�ǰλ��д�룬2Ϊ���ǣ� ���أ������ļ��ṹ��ָ��  
myFile *openFile(iNode *inode, int mode);

//�ļ�ϵͳ�ڲ������ļ�  ���룺�ļ���Ŀ¼��inodeָ�롢�ļ������ļ�����  ���أ��ļ���inodeָ��
iNode *createFile(iNode *father, string name, int type);

//���ļ�  ���룺������ָ�롢�����ֽ������ļ�inodeָ�롢��дָ��λ��  ���أ��ɹ��������ֽ���
int readFile(void *v_buf, int count, iNode *inode, myFile *file);

//д�ļ�  ���룺������ָ�롢��д�ֽ������ļ�inodeָ�롢��дָ��λ��  ���أ��ɹ�д����ֽ���
int writeFile(void *v_buf, int count, iNode *inode, myFile *file);


/*ϵͳ��������*/

//�ļ�ϵͳ��ʼ��  ���룺��  ���أ��ɹ�����1��ʧ�ܷ���0
int init_filesystem();

//�����ļ�  ���룺�ļ�·�����ļ����ͣ�0��ʾĿ¼�ļ���1��ʾ��ͨ�ļ���  ���أ��ɹ�����1��ʧ�ܷ���0
int CreateFile(string path, int type);

//ɾ���ļ�  ���룺�ļ�·��  ���أ��ɹ�����1��ʧ�ܷ���0
int DeleteFile(string path);

//���ļ�  ���룺�ļ�·�������û���Ƿ񴴽���д��ģʽ��0Ϊ׷�ӣ�1Ϊ�ӵ�ǰλ��д�룬2Ϊ���ǣ� ���أ������ļ��ṹ��ָ��
myFile* OpenFile(string path, int create, int mode);

//�ر��ļ�  ���룺�ļ��ṹ��ָ��  ���أ��ɹ�����1��ʧ�ܷ���0
int CloseFile(myFile* file);

//��׼���ļ�  ���룺������ָ�롢�����ֽ������ļ��ṹ��ָ��  ���أ��ɹ��������ֽ���
int Fread(void* buf, int count, myFile* file);

//��׼д�ļ�  ���룺������ָ�롢��д�ֽ������ļ��ṹ��ָ��  ���أ��ɹ�д����ֽ���
int Fwrite(void* buf, int count, myFile* file);

//�޸ĵ�ǰĿ¼���ļ���  ���룺ԭ�������ơ����ڵ�����  ������ɹ�����1��ʧ�ܷ���0
int rename(string name, string now);

//Ŀ¼����  ���룺��  ���أ���ǰĿ¼������Ŀ¼������
vector<vector<string> > dir_ls();

//�л���ǰĿ¼  ���룺Ҫ�л�����Ŀ¼  ���أ��ɹ�����1��ʧ�ܷ���0
int dir_cd(string path);
                               