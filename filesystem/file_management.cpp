#include "file_management.h"
using namespace std;

//将文件长度截为0  输入：文件的inode指针  输出：无
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

//文件块映射到盘块的处理操作  输入：文件的inode指针，文件中的数据块号，创建标志（如果为1则在逻辑块不存在时申请新的磁盘块）  返回：设备上的逻辑块号
int bmap(iNode *inode, int block, int create)
{
	if(create && !inode->i_zone[block]){
		if(inode->i_zone[block]=new_block()){
			inode->i_dirt=1;
		}
	}
	return inode->i_zone[block];
}

//将inode的引用数减1，可以会视情况写回或删除i节点  输入：inode的指针  返回：无
void iput(iNode *inode)
{
	if(inode->i_count>1){ //如果引用数大于1，则将引用数减1后返回
		inode->i_count--;
		return;
	}
	if(!inode->nlinks){ //如果引用数为1且链接数为0，说明文件已被删除，则释放i节点及数据块
		truncate(inode);
		free_iNode(inode);
		return;
	}
	if(inode->i_dirt){ //如果引用数为1但链接数不为0，说明文件没被删除但inode不再使用，此时如果i_dirt为1则将inode写回
		write_iNode(inode);
	}
	inode->i_count--;
	return;
}

//由i节点号获取inode结构体指针  输入：i节点号  返回：inode的指针
iNode *iget(int nr)
{
	//首先从i节点表中取一个空闲节点备用
	iNode *empty=get_empty_inode();

	//接着扫描i节点表，寻找指定i节点号的inode
	for(int i=0; i<INODE_NUM; i++){
		if(iNode_table[i].i_num==nr){
			if(empty){ //如果找到了empty就没用了
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

//从i节点表中获取一个空闲的i节点项  输入：无  返回：inode的指针
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

//从目录表中找目录项  输入：目录inode指针、文件名、目录项指针  返回：成功返回1，失败返回0
int find_entry(iNode *inode, string name, dir_entry* dir, int &de)
{
	int num=(inode->i_size)/sizeof(dir_entry); //num表示此目录下有几个目录项
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

//在目录表的某位置加一个新目录项  输入：目录表节点指针，目录项编号，要写入的名字，要写入的节点编号  返回：成功返回1，失败返回0
int add_entry(iNode *inode, int de, string name, unsigned int nr)
{
	//新建一个目录项结构体指针
	dir_entry *dir=(dir_entry *)malloc(sizeof(dir_entry));
	dir->iNode_no=nr;
	//打开当前目录文件进行写操作
	myFile *file=openFile(inode,1);
	file->f_pos=ENTRY_SIZE*de;
	const char *buf=name.c_str();
	strcpy(dir->file_name,buf);
	int ret=writeFile(dir,sizeof(dir_entry),pwd,file);
	free(dir);
	CloseFile(file);
	return ret?1:0;
}

//寻找指定路径文件的文件名和父目录的iNode  输入：路径名，父目录iNode指针的指针，文件名变量的引用  返回：成功返回1，失败返回0
int namei(string path, iNode** father, string& filename)
{
	//如果路径名末尾有'/'的话要去掉
	int k=path.size() - 1;
	if (path[k] == '/') {
		path=path.substr(0,k);
	}
	
	//路径名不能为空
	int len=path.size();
	if(len==0) return NULL; 
	if(len>PATH_LENGTH) return NULL;

	iNode *inode;
	dir_entry *dir;
	int de; //目标文件在目录表中的位置
	int p; //路径名第一个元素的位置
	//如果路径名第一个字符是'/'，说明是绝对路径名，应该从根目录开始操作；否则是相对路径名，从当前目录开始操作
	if(path[0]=='/'){
		inode=root;
		p++;
	}else{
		inode=pwd;
	}

	inode->i_count++; //当前i节点正在使用，inode的引用数加一

	//开始对路径中的各个目录名和文件名进行循环处理
	while(1){
		int k=p; //记录p的初始值
		if(inode->i_mode){ //如果inode对应的是普通文件
			iput(inode);
			return NULL;
		}

		//从当前位置p开始寻找'/'
		int namelen=0;
		for( ; p<len && path[p]!='/'; namelen++) p++;

		string s=path.substr(k,namelen); 
		//如果p最终越界了，说明已到达路径名的末尾，则直接返回
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
		if(!inode){ //如果没有成功获得新的inode，也直接返回
			return 0;
		}
	}
}

//文件系统内部打开文件  输入：文件inode指针、写入模式（0为追加，1为从当前位置写入，2为覆盖） 返回：返回文件结构体指针  
myFile *openFile(iNode *inode, int mode)
{
	myFile *file;
	//在文件表中寻找空闲项
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

//文件系统内部创建文件  输入：文件父目录的inode指针、文件名、文件类型  返回：文件的inode指针
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
	if(!type){ //如果要创建的是目录文件，则还要设置两个新的默认目录项"."和".."
		if(!add_entry(inode,0,".",inode->i_num) || !add_entry(inode,1,"..",father->i_num)){
			return NULL;
		}
		inode->nlinks=2;
		father->nlinks++;
	}
	return inode;
}

//文件系统初始化  输入：无  返回：成功返回1，失败返回0
int init_filesystem()
{
	iNode *inode=iget(ROOT_INO);
	if(!inode) return 0;
	pwd=root=inode; //初始时当前工作目录为根目录
	return 1;
}

//创建文件  输入：文件路径、文件类型（0表示目录文件，1表示普通文件）  返回：成功返回1，失败返回0
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

//删除文件  输入：文件路径  返回：成功返回1，失败返回0
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
	//如果要删除的是目录且目录非空，则删除失败
	if(!inode->i_mode && inode->i_size!=2*ENTRY_SIZE){
		return 0;
	}
	//如果要删除当前目录，则删除失败
	if(inode==pwd) return 0;
	//首先从父目录中删除目录项
	int num=father->i_size/sizeof(dir_entry);
	for(int i=de; i<num; i++){
		dir[i]=dir[i+1];
		strcpy(dir[i].file_name,dir[i+1].file_name);
	}
	myFile *file=openFile(father,0);
	int ret=writeFile(dir,(num-1)*ENTRY_SIZE,father,file);
	father->mtime=time(0);
	if(!inode->i_mode) father->nlinks--; //如果要删除的是目录，则父目录链接数减1
	father->i_dirt=1;
	iput(father);
	free(dir);
	CloseFile(file);
	if(!ret) return 0;
	//之后将文件的i节点链接数减为0
	inode->nlinks=0;
	inode->i_dirt=1;
	iput(inode);
	return 1;
}

//打开文件  输入：文件路径、如果没有是否创建、写入模式（0为追加，1为从当前位置写入，2为覆盖） 返回：返回文件结构体指针
myFile* OpenFile(string path, int create, int mode)
{
	iNode *father;
	dir_entry *dir;
	string name;
	int de;
	if(!namei(path,&father,name)) return NULL;
	int ret=find_entry(father,name,dir,de);
	if(ret){ //此文件存在
		iput(father);
		int nr=dir[de].iNode_no;
		iNode *inode=iget(nr);
		free(dir);
		if (!inode) return NULL; //如果没有成功找到文件的inode，则出错
		if (!inode->i_mode){
			iput(inode);
			return NULL; //如果此inode对应的是一个目录文件，则出错
		}
		return openFile(inode,mode);
	}else{ //此文件不存在
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

//关闭文件  输入：文件结构体指针  返回：成功返回1，失败返回0
int CloseFile(myFile* file)
{
	if(file->f_count==0) return 0; //已经无人用
	if(--file->f_count) return 1; //还有人用
	iput(file->f_iNode);
	return 1;
}

//读文件  输入：缓冲区指针、欲读字节数、文件inode指针、读写指针位置  返回：成功读出的字节数
int readFile(void *v_buf, int count, iNode *inode, myFile *file)
{
	int left; //还剩多少字节没读
	int chars; //从当前块中读多少字节
	int nr; //当前读的块号
	char *buf=(char *)v_buf; //目标缓冲区指针
	char *bh; //临时读入缓冲区指针
	if((left=count)<=0){ //要读入的字节数小于等于0
		return 0;
	}
	while(left){
		int offset=file->f_pos%BLOCK_SIZE; //读指针在当前块中的偏移量
		int chars=min(BLOCK_SIZE-offset,left); //需要在当前块中读多少字节
		bh = (char*)malloc(chars+1);
		//如果所读取的数据块不存在，则将bh置为空。如果数据块存在但未成功读取，则退出循环。
		if(nr=bmap(inode,file->f_pos/BLOCK_SIZE,0)){
			if(!block_read(nr, offset, chars, bh)){
				break;
			}
			*(bh + chars) = '\0'; //如果成功读取数据到bh中，则将末尾置为'\0'
		}else{
			bh=NULL;
		}
		//如果成功读到数据，则将其复制到目标缓冲区，否则向目标缓冲区中填入chars个0值字节
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

//写文件  输入：缓冲区指针、欲写字节数、文件inode指针、读写指针位置  返回：成功写入的字节数
int writeFile(void *v_buf, int count, iNode *inode, myFile *file)
{
	int pos; //要写入的位置
	int i=0; //已写入字节数
	int nr; //当前要写入的块号
	char *buf=(char *)v_buf; //源缓冲区指针

	//首先根据写入模式确定pos的位置
	if(file->mode){
		pos=file->f_pos;
	}else{
		pos=inode->i_size;
	}

	//之后判断写入数据后文件大小会不会越界
	if(pos+count > FBLK_NUM*BLOCK_SIZE){
		return 0;
	}
	while(i<count){
		//如果要写入的块不存在且创建失败，则退出循环
		if(!(nr=bmap(inode,pos/BLOCK_SIZE,1))){
			break;
		}
		int offset=pos%BLOCK_SIZE; //要写入的位置在块中的偏移量
		int chars=min(BLOCK_SIZE-offset,count-i); //需要在当前块中写入多少字节
		//如果写入失败，则直接退出循环
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
	//如果不是在文件尾添加数据则修改文件读写指针
	if(file->mode){
		file->f_pos=pos;
	}
	return i;
}

//读入一行（从当前位置读到换行符）  输入：缓冲区指针，文件结构体指针  返回：成功读出的字节数(不包括换行符）
int fgets(void *v_buf, myFile *file)
{
	int count=0; //已读入的字节数
	char *buf=(char *)v_buf; //目标缓冲区指针
	char *bh; //临时读入缓冲区指针
	int nr; //当前读的块号
	while(1){
		int offset=file->f_pos%BLOCK_SIZE; //读指针在当前块中的偏移量
		int chars=BLOCK_SIZE-offset; //需要在当前块中读多少字节
		bh = (char*)malloc(chars + 1);
		if(!(nr=bmap(file->f_iNode,file->f_pos/BLOCK_SIZE,0)) || !block_read(nr, offset, chars, bh)){
			break;
		}
		//遍历读入的字符
		int i;
		for( i=0; i<chars; i++){
			char c=*(bh+i);
			if(c=='\n'){
				*(bh+i)='\0';
				break;
			}
		}
		//把临时缓冲区的内容读入目标缓冲
		strcpy(buf+count, bh);
		count+=i;
		file->f_pos+=i;
		if(i!=chars){ //说明读到了换行符
			file->f_pos++; //读指针移到换行符之后
			break;
		}
	}
	return count;
}

//标准读文件  输入：缓冲区指针、欲读字节数、文件结构体指针  返回：成功读出的字节数
int Fread(void* buf, int count, myFile* file)
{
	return readFile(buf, count, file->f_iNode, file);
}

//标准写文件  输入：缓冲区指针、欲写字节数、文件结构体指针  返回：成功写入的字节数
int Fwrite(void* buf, int count, myFile* file)
{
	if(file->mode==2){
		truncate(file->f_iNode);
		file->f_pos=0;
	}
	return writeFile(buf, count, file->f_iNode, file);
}

//修改当前目录下文件名  输入：原来的名称、现在的名称  输出：成功返回1，失败返回0
int rename(string name, string now)
{
	if(now.size()==0 || now.size()>NAME_LEN || now.find('/')!=string::npos){ //要改的名称为空、过长或包含分割符
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

//目录遍历  输入：无  返回：当前目录的所有目录项内容
vector<vector<string> > dir_ls()
{
	vector<vector<string> > v;

	iNode *inode=pwd;
	int num=(inode->i_size)/sizeof(dir_entry); //num表示此目录下有几个目录项
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

//切换当前目录  输入：要切换到的目录  返回：成功返回1，失败返回0
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
	if (!inode) return 0; //如果没有成功找到新路径的inode，则出错
	if (inode->i_mode){
		iput(inode);
		return 0; //如果此inode对应的是一个普通文件，则出错
	}
	iput(pwd);
	pwd = inode;
	return 1;
}	