// --------------外部接口--------------
//展示所有设备状态
void showDevice();
//进程申请设备,返回true代表申请成功,返回false代表设备已被占用,进程加入等待队列
bool acquire(long pid, int device);
//进程释放设备,device为-1则释放该进程申请的所有设备
bool release(long pid, int device);

// 磁盘初始化
void disk_init(int flag = 0);

// 申请一个数据块,返回块号;如果返回0说明无空闲数据块
int new_block();
// 输入块号,释放该物理块
bool free_block(int blockSeq);
//写入物理块，形参列表:块号,开始写的位置在块中的偏移量,要写入的字节数,缓冲区指针;返回true代表操作成功
bool block_write(int blockSeq, int offset, int charNum, char* buf);
//读出物理块，形参列表:块号,开始读的位置在块中的偏移量,要读入的字节数,缓冲区指针;返回true代表操作成功
bool block_read(int blockSeq, int offset, int charNum, char* buf);

// 申请一个iNode,返回iNode指针;指针中的i_num是iNode编号;如果返回空指针说明无空闲iNode块
iNode* new_iNode();
// 释放一个iNode,根据参数中的inode->i_num释放磁盘中的iNode节点,并释放参数中的iNode指针
void free_iNode(iNode* inode);
// 读iNode,即根据参数中的inode->i_num在磁盘对应位置读出该iNode的全部数据;返回true代表操作成功
bool read_iNode(iNode* inode);
// 写iNode,即根据参数中的inode->i_num在磁盘对应位置写入该iNode的全部数据;返回true代表操作成功
bool write_iNode(iNode* inode);
