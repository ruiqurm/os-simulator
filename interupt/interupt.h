#include<mutex>
#include<vector>
#include "../process/program.h"
using std::mutex;

/// 中断类型
enum class InteruptType {
	TIMER = 1,
	SOFTWARE, // useless
	EXTERNAL_1,

	NON_MASKABLE = 12,
	ERROR, // Non Maskable Interrupt
	EXTERNAL_0 // Non Maskable Interrupt
};

typedef void (*InteruptFunc)(InteruptType, int, int64_t);
const unsigned int TIMER_INTERUPT_INTERVAL = 100;// 100ms
const int InteruptVectorTableSize = 32;

class InteruptValid {
	public:
		InteruptValid() {}
		InteruptValid& set(InteruptType);
		InteruptValid& unset(InteruptType);
		bool is_set(InteruptType);
};

struct Interupt {
	InteruptType type;
	int device_id;
	int64_t value;
	int64_t time;
	bool operator < (const Interupt& b)const;
	bool operator > (const Interupt& b)const;
	bool is_blocking() const;
};



/// 初始化中断
void init_interupt();

/// 关中断 
void push_off();

/// 开中断
void pop_off();

/// 安装一个中断处理程序
void set_handler(InteruptType type, InteruptFunc f);

/// 设置优先级
void set_priority(InteruptType,int);


/// 产生一个中断；供外部设备使用
void raise_interupt(InteruptType t, int device_id, int64_t value);

/// 处理中断; 由执行指令的部分调用
/// 为防止中断过多，这里会处理全部可处理的中断
void handle_interupt();


///  定时器相关
void enable_timer();
void disable_timer();
void stop_timer();


struct InteruptSnapshot {
	vector<Interupt> interupts;
	InteruptVector interuptVectorTable[InteruptVectorTableSize];
	bool valid[InteruptVectorTableSize];
};
/// 给前端
InteruptSnapshot get_interupt_snapshot();
void awake_on_external_interupt(InteruptType t, int device, int64_t value) {
	int pid = static_cast<int>(value);
	wakeup(pid);
}