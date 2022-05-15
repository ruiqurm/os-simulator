#include "file_management.h"
using namespace std;

//���ļ����Ƚ�Ϊ0  ���룺�ļ���inodeָ��  �������
void truncate(iNode *inode)
{
	int i;
	for(i=0;i<FBLK_NUM;i++){
		if(inode->i_zone[i]){
			free_block(inode->i_zone[i]);
			inode->i_zone[i]=0;
		}
	}
	inode->i_size=0;
	inode->i_dirt=1;
	return;
}

//�ļ���ӳ�䵽�̿�Ĵ�������  ���룺�ļ���inodeָ�룬�ļ��е����ݿ�ţ�������־�����Ϊ1�����߼��鲻����ʱ�����µĴ��̿飩  ���أ��豸�ϵ��߼����
int bmap(iNode *inode, int block, int create)
{
	if(create && !inode->i_zone[block]){
		if(inode->i_zone[block]=new_block()){
			inode->i_dirt=1;
		}
	}
	return inode->i_zone[block];
}

//��inode����������1�����Ի������д�ػ�ɾ��i�ڵ�  ���룺inode��ָ��  ���أ���
void iput(iNode *inode)
{
	if(inode->i_count>1){ //�������������1������������1�󷵻�
		inode->i_count--;
		return;
	}
	if(!inode->nlinks){ //���������Ϊ1��������Ϊ0��˵���ļ��ѱ�ɾ�������ͷ�i�ڵ㼰���ݿ�
		truncate(inode);
		free_iNode(inode);
		return;
	}
	if(inode->i_dirt){ //���������Ϊ1����������Ϊ0��˵���ļ�û��ɾ����inode����ʹ�ã���ʱ���i_dirtΪ1��inodeд��
		write_iNode(inode);
	}
	inode->i_count--;
	return;
}

//��i�ڵ�Ż�ȡinode�ṹ��ָ��  ���룺i�ڵ��  ���أ�inode��ָ��
iNode *iget(int nr)
{
	//���ȴ�i�ڵ����ȡһ�����нڵ㱸��
	iNode *empty=get_empty_inode();

	//����ɨ��i�ڵ����Ѱ��ָ��i�ڵ�ŵ�inode
	for(int i=0; i<INODE_NUM; i++){
		if(iNode_table[i].i_num==nr){
			if(empty){ //����ҵ���empty��û����
				iput(empty);
			}
			return &iNode_table[i];
		}
	}

	if(!empty) return NULL;
	empty->i_num=nr;
	read_iNode(empty);
	return empty;
}

//��i�ڵ���л�ȡһ�����е�i�ڵ���  ���룺��  ���أ�inode��ָ��
iNode *get_empty_inode()
{
	iNode *inode=NULL;
	for(int i=0; i<INODE_NUM; i++){
		if(!iNode_table[i].i_count && !iNode_table[i].i_dirt){
			inode=&iNode_table[i];
			break;
		}
	}
	if(inode){
		memset(inode,0,sizeof(*inode));
		inode->i_count=1;
	}
	return inode;
}

//��Ŀ¼������Ŀ¼��  ���룺Ŀ¼inodeָ�롢�ļ�����Ŀ¼��ָ��  ���أ��ɹ�����1��ʧ�ܷ���0
int find_entry(iNode *inode, string name, dir_entry* dir, int &de)
{
	int num=(inode->i_size)/sizeof(dir_entry); //num��ʾ��Ŀ¼���м���Ŀ¼��
	dir = (dir_entry*)malloc(sizeof(dir_entry)*num);
	myFile *file=openFile(inode,0);
	int ret=readFile(dir,inode->i_size,inode,file);
	CloseFile(file);
	if(!ret){
		return 0;
	}
	int flag=0;
	for(int i=0;i<num;i++){
		if(dir[i].file_name==name){
			flag=1;
			de=i;
			break;
		}
	}
	return flag;
}

//��Ŀ¼����ĳλ�ü�һ����Ŀ¼��  ���룺Ŀ¼���ڵ�ָ�룬Ŀ¼���ţ�Ҫд������֣�Ҫд��Ľڵ���  ���أ��ɹ�����1��ʧ�ܷ���0
int add_entry(iNode *inode, int de, string name, unsigned int nr)
{
	//�½�һ��Ŀ¼��ṹ��ָ��
	dir_entry *dir=(dir_entry *)malloc(sizeof(dir_entry));
	dir->iNode_no=nr;
	//�򿪵�ǰĿ¼�ļ�����д����
	myFile *file=openFile(inode,1);
	file->f_pos=ENTRY_SIZE*de;
	const char *buf=name.c_str();
	strcpy(dir->file_name,buf);
	int ret=writeFile(dir,sizeof(dir_entry),pwd,file);
	free(dir);
	CloseFile(file);
	return ret?1:0;
}

//Ѱ��ָ��·���ļ����ļ����͸�Ŀ¼��iNode  ���룺·��������Ŀ¼iNodeָ���ָ�룬�ļ�������������  ���أ��ɹ�����1��ʧ�ܷ���0
int namei(string path, iNode** father, string& filename)
{
	//���·����ĩβ��'/'�Ļ�Ҫȥ��
	int k=path.size() - 1;
	if (path[k] == '/') {
		path=path.substr(0,k);
	}
	
	//·��������Ϊ��
	int len=path.size();
	if(len==0) return NULL; 
	if(len>PATH_LENGTH) return NULL;

	iNode *inode;
	dir_entry *dir;
	int de; //Ŀ���ļ���Ŀ¼���е�λ��
	int p; //·������һ��Ԫ�ص�λ��
	//���·������һ���ַ���'/'��˵���Ǿ���·������Ӧ�ôӸ�Ŀ¼��ʼ���������������·�������ӵ�ǰĿ¼��ʼ����
	if(path[0]=='/'){
		inode=root;
		p++;
	}else{
		inode=pwd;
	}

	inode->i_count++; //��ǰi�ڵ�����ʹ�ã�inode����������һ

	//��ʼ��·���еĸ���Ŀ¼�����ļ�������ѭ������
	while(1){
		int k=p; //��¼p�ĳ�ʼֵ
		if(inode->i_mode){ //���inode��Ӧ������ͨ�ļ�
			iput(inode);
			return NULL;
		}

		//�ӵ�ǰλ��p��ʼѰ��'/'
		int namelen=0;
		for( ; p<len && path[p]!='/'; namelen++) p++;

		string s=path.substr(k,namelen); 
		//���p����Խ���ˣ�˵���ѵ���·������ĩβ����ֱ�ӷ���
		if(p==len){
			filename=s;
			*father=inode;
			return 1;
		}
		if(!find_entry(inode,s,dir,de)){
			iput(inode);
			return 0;
		}
		iput(inode);
		int nr=dir[de].iNode_no;
		inode=iget(nr);
		free(dir);
		if(!inode){ //���û�гɹ�����µ�inode��Ҳֱ�ӷ���
			return 0;
		}
	}
}

//�ļ�ϵͳ�ڲ����ļ�  ���룺�ļ�inodeָ�롢д��ģʽ��0Ϊ׷�ӣ�1Ϊ�ӵ�ǰλ��д�룬2Ϊ���ǣ� ���أ������ļ��ṹ��ָ��  
myFile *openFile(iNode *inode, int mode)
{
	myFile *file;
	//���ļ�����Ѱ�ҿ�����
	int i;
	for(i=0;i<NR_FILE;i++){
		if(file_table[i].f_count==0) break;
	}
	if(i>=NR_FILE) return NULL;
	file=file_table+i;
	file->mode=mode;
	file->f_count=1;
	file->f_iNode=inode;
	file->f_pos=0;
	return file;
}

//�ļ�ϵͳ�ڲ������ļ�  ���룺�ļ���Ŀ¼��inodeָ�롢�ļ������ļ�����  ���أ��ļ���inodeָ��
iNode *createFile(iNode *father, string name, int type)
{
	iNode *inode=new_iNode();
	if(!inode) return NULL;
	inode->i_mode=type;
	inode->i_dirt=1;
	if(father->i_size/ENTRY_SIZE==DIR_NUM || !add_entry(father,father->i_size/ENTRY_SIZE,name,inode->i_num)){
		inode->nlinks--;
		iput(inode);
		return NULL;
	}
	if(!type){ //���Ҫ��������Ŀ¼�ļ�����Ҫ���������µ�Ĭ��Ŀ¼��"."��".."
		if(!add_entry(inode,0,".",inode->i_num) || !add_entry(inode,1,"..",father->i_num)){
			return NULL;
		}
		inode->nlinks=2;
		father->nlinks++;
	}
	return inode;
}

//�ļ�ϵͳ��ʼ��  ���룺��  ���أ��ɹ�����1��ʧ�ܷ���0
int init_filesystem()
{
	iNode *inode=iget(ROOT_INO);
	if(!inode) return 0;
	pwd=root=inode; //��ʼʱ��ǰ����Ŀ¼Ϊ��Ŀ¼
	return 1;
}

//�����ļ�  ���룺�ļ�·�����ļ����ͣ�0��ʾĿ¼�ļ���1��ʾ��ͨ�ļ���  ���أ��ɹ�����1��ʧ�ܷ���0
int CreateFile(string path, int type)
{
	iNode *father;
	dir_entry *dir;
	string name;
	int de;
	if(!namei(path,&father,name)) return NULL;
	int ret=find_entry(father,name,dir,de);
	if(ret){
		iput(father);
		free(dir);
		return 0;
	}else{
		iNode *inode=createFile(father,name,type);
		iput(father);
		free(dir);
		if(inode){
			return 1;
		}else{
			return 0;
		}
	}
}

//ɾ���ļ�  ���룺�ļ�·��  ���أ��ɹ�����1��ʧ�ܷ���0
int DeleteFile(string path)
{
	iNode *father;
	dir_entry *dir;
	string name;
	int de;
	if(!namei(path,&father,name)) return 0;
	int ret=find_entry(father,name,dir,de);
	if(ret==0) return 0;
	int nr=dir[de].iNode_no;
	iNode *inode=iget(nr);
	if(!inode) return 0;
	//���Ҫɾ������Ŀ¼��Ŀ¼�ǿգ���ɾ��ʧ��
	if(!inode->i_mode && inode->i_size!=2*ENTRY_SIZE){
		return 0;
	}
	//���Ҫɾ����ǰĿ¼����ɾ��ʧ��
	if(inode==pwd) return 0;
	//���ȴӸ�Ŀ¼��ɾ��Ŀ¼��
	int num=father->i_size/sizeof(dir_entry);
	for(int i=de; i<num; i++){
		dir[i]=dir[i+1];
		strcpy(dir[i].file_name,dir[i+1].file_name);
	}
	myFile *file=openFile(father,0);
	int ret=writeFile(dir,(num-1)*ENTRY_SIZE,father,file);
	father->mtime=time(0);
	if(!inode->i_mode) father->nlinks--; //���Ҫɾ������Ŀ¼����Ŀ¼��������1
	father->i_dirt=1;
	iput(father);
	free(dir);
	CloseFile(file);
	if(!ret) return 0;
	//֮���ļ���i�ڵ���������Ϊ0
	inode->nlinks=0;
	inode->i_dirt=1;
	iput(inode);
	return 1;
}

//���ļ�  ���룺�ļ�·�������û���Ƿ񴴽���д��ģʽ��0Ϊ׷�ӣ�1Ϊ�ӵ�ǰλ��д�룬2Ϊ���ǣ� ���أ������ļ��ṹ��ָ��
myFile* OpenFile(string path, int create, int mode)
{
	iNode *father;
	dir_entry *dir;
	string name;
	int de;
	if(!namei(path,&father,name)) return NULL;
	int ret=find_entry(father,name,dir,de);
	if(ret){ //���ļ�����
		iput(father);
		int nr=dir[de].iNode_no;
		iNode *inode=iget(nr);
		free(dir);
		if (!inode) return NULL; //���û�гɹ��ҵ��ļ���inode�������
		if (!inode->i_mode){
			iput(inode);
			return NULL; //�����inode��Ӧ����һ��Ŀ¼�ļ��������
		}
		return openFile(inode,mode);
	}else{ //���ļ�������
		if(!create) return NULL;
		iNode *inode=createFile(father,name,1);
		iput(father);
		free(dir);
		if(inode){
			return openFile(inode,mode);
		}else{
			return NULL;
		}
	}
}

//�ر��ļ�  ���룺�ļ��ṹ��ָ��  ���أ��ɹ�����1��ʧ�ܷ���0
int CloseFile(myFile* file)
{
	if(file->f_count==0) return 0; //�Ѿ�������
	if(--file->f_count) return 1; //��������
	iput(file->f_iNode);
	return 1;
}

//���ļ�  ���룺������ָ�롢�����ֽ������ļ�inodeָ�롢��дָ��λ��  ���أ��ɹ��������ֽ���
int readFile(void *v_buf, int count, iNode *inode, myFile *file)
{
	int left; //��ʣ�����ֽ�û��
	int chars; //�ӵ�ǰ���ж������ֽ�
	int nr; //��ǰ���Ŀ��
	char *buf=(char *)v_buf; //Ŀ�껺����ָ��
	char *bh; //��ʱ���뻺����ָ��
	if((left=count)<=0){ //Ҫ������ֽ���С�ڵ���0
		return 0;
	}
	while(left){
		int offset=file->f_pos%BLOCK_SIZE; //��ָ���ڵ�ǰ���е�ƫ����
		int chars=min(BLOCK_SIZE-offset,left); //��Ҫ�ڵ�ǰ���ж������ֽ�
		bh = (char*)malloc(chars+1);
		//�������ȡ�����ݿ鲻���ڣ���bh��Ϊ�ա�������ݿ���ڵ�δ�ɹ���ȡ�����˳�ѭ����
		if(nr=bmap(inode,file->f_pos/BLOCK_SIZE,0)){
			if(!block_read(nr, offset, chars, bh)){
				break;
			}
			*(bh + chars) = '\0'; //����ɹ���ȡ���ݵ�bh�У���ĩβ��Ϊ'\0'
		}else{
			bh=NULL;
		}
		//����ɹ��������ݣ����临�Ƶ�Ŀ�껺������������Ŀ�껺����������chars��0ֵ�ֽ�
		if(bh){
			strcpy(buf+count-left, bh);
		}else{
			memset(buf+count-left,0,chars);
		}
		free(bh);
		file->f_pos += chars;
		left -= chars;
	}
	return count-left;
}

//д�ļ�  ���룺������ָ�롢��д�ֽ������ļ�inodeָ�롢��дָ��λ��  ���أ��ɹ�д����ֽ���
int writeFile(void *v_buf, int count, iNode *inode, myFile *file)
{
	int pos; //Ҫд���λ��
	int i=0; //��д���ֽ���
	int nr; //��ǰҪд��Ŀ��
	char *buf=(char *)v_buf; //Դ������ָ��

	//���ȸ���д��ģʽȷ��pos��λ��
	if(file->mode){
		pos=file->f_pos;
	}else{
		pos=inode->i_size;
	}

	//֮���ж�д�����ݺ��ļ���С�᲻��Խ��
	if(pos+count > FBLK_NUM*BLOCK_SIZE){
		return 0;
	}
	while(i<count){
		//���Ҫд��Ŀ鲻�����Ҵ���ʧ�ܣ����˳�ѭ��
		if(!(nr=bmap(inode,pos/BLOCK_SIZE,1))){
			break;
		}
		int offset=pos%BLOCK_SIZE; //Ҫд���λ���ڿ��е�ƫ����
		int chars=min(BLOCK_SIZE-offset,count-i); //��Ҫ�ڵ�ǰ����д������ֽ�
		//���д��ʧ�ܣ���ֱ���˳�ѭ��
		if(!block_write(nr, offset, chars, buf+i)){
			break;
		}
		pos+=chars;
		if(pos>inode->i_size){
			inode->i_size=pos;
			inode->i_dirt=1;
		}
		i+=chars;
	}
	inode->mtime=time(0);
	//����������ļ�β�����������޸��ļ���дָ��
	if(file->mode){
		file->f_pos=pos;
	}
	return i;
}

//����һ�У��ӵ�ǰλ�ö������з���  ���룺������ָ�룬�ļ��ṹ��ָ��  ���أ��ɹ��������ֽ���(���������з���
int fgets(void *v_buf, myFile *file)
{
	int count=0; //�Ѷ�����ֽ���
	char *buf=(char *)v_buf; //Ŀ�껺����ָ��
	char *bh; //��ʱ���뻺����ָ��
	int nr; //��ǰ���Ŀ��
	while(1){
		int offset=file->f_pos%BLOCK_SIZE; //��ָ���ڵ�ǰ���е�ƫ����
		int chars=BLOCK_SIZE-offset; //��Ҫ�ڵ�ǰ���ж������ֽ�
		bh = (char*)malloc(chars + 1);
		if(!(nr=bmap(file->f_iNode,file->f_pos/BLOCK_SIZE,0) || !block_read(nr, offset, chars, bh)){
			break;
		}
		//����������ַ�
		for(int i=0; i<chars; i++){
			char c=*(bh+i);
			if(c=='\n'){
				*(bh+i)='\0';
				break;
			}
		}
		//����ʱ�����������ݶ���Ŀ�껺��
		strcpy(buf+count, bh);
		count+=i;
		file->f_pos+=i;
		if(i!=chars){ //˵�������˻��з�
			file->f_pos++; //��ָ���Ƶ����з�֮��
			break;
		}
	}
	return count;
}

//��׼���ļ�  ���룺������ָ�롢�����ֽ������ļ��ṹ��ָ��  ���أ��ɹ��������ֽ���
int Fread(void* buf, int count, myFile* file)
{
	return readFile(buf, count, file->f_iNode, file);
}

//��׼д�ļ�  ���룺������ָ�롢��д�ֽ������ļ��ṹ��ָ��  ���أ��ɹ�д����ֽ���
int Fwrite(void* buf, int count, myFile* file)
{
	if(file->mode==2){
		truncate(file->f_iNode);
		file->f_pos=0;
	}
	return writeFile(buf, count, file->f_iNode, file);
}

//�޸ĵ�ǰĿ¼���ļ���  ���룺ԭ�������ơ����ڵ�����  ������ɹ�����1��ʧ�ܷ���0
int rename(string name, string now)
{
	if(now.size()==0 || now.size()>NAME_LEN || now.find('/')!=string::npos){ //Ҫ�ĵ�����Ϊ�ա�����������ָ��
		return 0;
	}
	dir_entry *dir;
	int de;
	if(!find_entry(pwd,name,dir,de)){
		return 0;
	}
	unsigned int nr=dir[de].iNode_no;
	free(dir);
	return add_entry(pwd,de,now,nr);
}

//Ŀ¼����  ���룺��  ���أ���ǰĿ¼������Ŀ¼������
vector<vector<string> > dir_ls()
{
	vector<vector<string> > v;

	iNode *inode=pwd;
	int num=(inode->i_size)/sizeof(dir_entry); //num��ʾ��Ŀ¼���м���Ŀ¼��
	dir_entry* dir = (dir_entry*)malloc(sizeof(dir_entry)*num);
	if(!readFile(dir,inode->i_size,inode,0)){
		return v;
	}
	for(int i=0; i<num; i++){
		string name=dir[i].file_name;
		string date="";
		string type="";
		string size="";
		int nr=dir[i].iNode_no;
		iNode *t;
		if(t=iget(nr)){
			date=ctime(&t->mtime);
			type=t->i_mode?"file":"directory";
			size=to_string(t->i_size)+"B";
		}
		vector<string> w;
		w.push_back(name);
		w.push_back(date);
		w.push_back(type);
		w.push_back(size);
		v.push_back(w);
	}
	return v;
}

//�л���ǰĿ¼  ���룺Ҫ�л�����Ŀ¼  ���أ��ɹ�����1��ʧ�ܷ���0
int dir_cd(string path)
{
	string name;
	iNode *father;
	dir_entry *dir;
	int de;
	if(!namei(path,&father,name)) return 0;
	if(!find_entry(father,name,dir,de)) return 0; 
	iput(father);
	int nr=dir[de].iNode_no;
	iNode *inode=iget(nr);
	free(dir);
	if (!inode) return 0; //���û�гɹ��ҵ���·����inode�������
	if (inode->i_mode){
		iput(inode);
		return 0; //�����inode��Ӧ����һ����ͨ�ļ��������
	}
	iput(pwd);
	pwd = inode;
	return 1;
}	