#include<functional>
#include<mutex>
using std::function;
using std::mutex;


/// 中断类型
enum class InteruptType {
	TIMER = 1,
	SOFTWARE, // useless
	EXTERNAL_1,

	Non_Maskable = 12,
	ERROR, // Non Maskable Interrupt
	EXTERNAL_0 // Non Maskable Interrupt
};

class InteruptMask {
	public:
		InteruptMask() {}
		InteruptMask& set(InteruptType);
		InteruptMask& unset(InteruptType);
		bool is_set(InteruptType);
};




/// 初始化中断
void init_interupt();

/// 关中断 
void push_off();

/// 开中断
void pop_off();

/// 安装一个中断处理程序
void set_handler(InteruptType type, std::function<void(InteruptType, int, int)>& f);

/// 设置优先级
void set_priority(InteruptType,int);


/// 产生一个中断；供外部设备使用
void raise_interupt(InteruptType t, int device_id, int64_t value);

/// 处理中断; 由执行指令的部分调用
/// 不会有嵌套中断
void handle_interupt();



