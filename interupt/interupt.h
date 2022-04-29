#include<functional>
#include<mutex>
using std::function;
using std::mutex;


/// �ж�����
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




/// ��ʼ���ж�
void init_interupt();

/// ���ж� 
void push_off();

/// ���ж�
void pop_off();

/// ��װһ���жϴ������
void set_handler(InteruptType type, std::function<void(InteruptType, int, int)>& f);

/// �������ȼ�
void set_priority(InteruptType,int);


/// ����һ���жϣ����ⲿ�豸ʹ��
void raise_interupt(InteruptType t, int device_id, int64_t value);

/// �����ж�; ��ִ��ָ��Ĳ��ֵ���
/// ������Ƕ���ж�
void handle_interupt();



