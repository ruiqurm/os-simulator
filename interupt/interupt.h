#include<mutex>
using std::mutex;

/// �ж�����
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
class InteruptValid {
	public:
		InteruptValid() {}
		InteruptValid& set(InteruptType);
		InteruptValid& unset(InteruptType);
		bool is_set(InteruptType);
};




/// ��ʼ���ж�
void init_interupt();

/// ���ж� 
void push_off();

/// ���ж�
void pop_off();

/// ��װһ���жϴ������
void set_handler(InteruptType type, InteruptFunc f);

/// �������ȼ�
void set_priority(InteruptType,int);


/// ����һ���жϣ����ⲿ�豸ʹ��
void raise_interupt(InteruptType t, int device_id, int64_t value);

/// �����ж�; ��ִ��ָ��Ĳ��ֵ���
/// Ϊ��ֹ�жϹ��࣬����ᴦ��ȫ���ɴ�����ж�
void handle_interupt();


///  ��ʱ�����
void enable_timer();
void disable_timer();
void stop_timer();