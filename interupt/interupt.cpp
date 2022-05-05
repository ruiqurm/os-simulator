#include "interupt.h"
#include <atomic>
#include<cstdint>
#include<vector>
#include<queue>
#include<ctime>
#include<thread>
using std::vector;
using std::uint16_t;
using std::priority_queue;

mutex mu;
std::atomic<uint16_t> valid;
std::atomic<bool> timer_is_valid;
std::atomic<bool> kill_timer;
int64_t __cnt_timer_interupt=0;

struct InteruptVector {
	InteruptFunc handler = nullptr;
	int priority;
};

InteruptVector InteruptVectorTable[InteruptVectorTableSize];


bool Interupt::operator < (const Interupt& b)const {
	if (type == b.type) {
		return time < b.time || (time == b.time && device_id < b.device_id);
	}
	else {
		return InteruptVectorTable[static_cast<int>(type)].priority < InteruptVectorTable[static_cast<int>(b.type)].priority;
	}
}
bool Interupt::operator > (const Interupt& b)const {
	if (type == b.type) {
		return time > b.time || (time == b.time && device_id > b.device_id);
	}
	else {
		return InteruptVectorTable[static_cast<int>(type)].priority > InteruptVectorTable[static_cast<int>(b.type)].priority;
	}
}

bool Interupt::is_blocking() const {
	if (static_cast<int>(type) > static_cast<int>(InteruptType::NON_MASKABLE)) {
		return false;
	}
	else {
		return !(valid & 1 && (valid & (1 << static_cast<int>(type))));
	}
}

priority_queue<Interupt, std::vector<Interupt>, std::greater<Interupt>> queue;

/// 默认中断函数
void do_nothing(InteruptType, int, int64_t);

/// 异常默认处理函数
void panic(InteruptType, int, int64_t);

bool InteruptValid::is_set(InteruptType t) {
	uint16_t offset = 1<<static_cast<int>(t);
	switch (t)
	{
	case InteruptType::TIMER:
	case InteruptType::SOFTWARE:
	case InteruptType::EXTERNAL_1:
		return valid & offset;
	case InteruptType::ERROR:
	case InteruptType::EXTERNAL_0:
		return false;
	default:
		return false;
	}
}
InteruptValid& InteruptValid::set(InteruptType t) {
	uint16_t m = 1 << static_cast<int>(t);
	switch (t)
	{
	case InteruptType::TIMER:
	case InteruptType::SOFTWARE:
	case InteruptType::EXTERNAL_1:
		valid |= m;
		break;
	case InteruptType::ERROR:
	case InteruptType::EXTERNAL_0:
	default:
		break;
	}
	return *this;
}

InteruptValid& InteruptValid::unset(InteruptType t) {
	uint16_t m = ~(1 << static_cast<int>(t));
	switch (t)
	{
	case InteruptType::TIMER:
	case InteruptType::SOFTWARE:
	case InteruptType::EXTERNAL_1:
		valid&= m;
		break;
	case InteruptType::ERROR:
	case InteruptType::EXTERNAL_0:
	default:
		break;
	}
	return *this;
}



void init_interupt() {
	valid = 1 |
		(1 << static_cast<int>(InteruptType::TIMER)) |
		(1 << static_cast<int>(InteruptType::SOFTWARE)) |
		(1 << static_cast<int>(InteruptType::EXTERNAL_1));
	InteruptVectorTable[static_cast<int>(InteruptType::TIMER)].priority = 0;
	InteruptVectorTable[static_cast<int>(InteruptType::SOFTWARE)].priority = 0;
	InteruptVectorTable[static_cast<int>(InteruptType::EXTERNAL_1)].priority = 1;
	InteruptVectorTable[static_cast<int>(InteruptType::EXTERNAL_0)].priority = 100;
	InteruptVectorTable[static_cast<int>(InteruptType::ERROR)].priority = 101;
	for (int i = 0; i < InteruptVectorTableSize; i++) {
		InteruptVectorTable[i].handler = do_nothing;
	}
	InteruptVectorTable[static_cast<int>(InteruptType::ERROR)].handler = panic;
	timer_is_valid.store(false);
	kill_timer.store(false);
	std::thread([=]()
		{
			while (!kill_timer.load()) {
				if (timer_is_valid.load()) {
					raise_interupt(InteruptType::TIMER, 0, time(NULL));
					__cnt_timer_interupt++;
				}
				std::this_thread::sleep_for(
					std::chrono::milliseconds(TIMER_INTERUPT_INTERVAL));
			}
		}).detach();

}
void enable_timer() {
	timer_is_valid.store(true);
}
void disable_timer() {
	timer_is_valid.store(false);
}
void stop_timer() {
	kill_timer.store(true);
}
void push_off() {
	valid &= 0xfffe;
}

void pop_off() {
	valid |= 1;
}

void raise_interupt(InteruptType t,int device_id,int64_t value) {
	mu.lock();
	queue.push(Interupt{ t,device_id,value,time(NULL)});
	mu.unlock();
}


void handle_interupt() {
	vector<Interupt> should_delay;
	// 将会一次处理完全部

	while (!queue.empty()) {
		const auto &top = queue.top();
		if (top.is_blocking()) {
			should_delay.push_back(top);
			queue.pop();
		}
		else {
			InteruptVectorTable[static_cast<int>(top.type)].handler(top.type, top.device_id, top.value);
			queue.pop();
		}
	}
	// 重填
	for (auto& v : should_delay) {
		queue.push(v);
	}
}

void set_handler(InteruptType type, InteruptFunc f) {
	InteruptVectorTable[static_cast<int>(type)].handler = f;
}

void set_priority(InteruptType type, int v) {
	if (static_cast<int>(type) > static_cast<int>(InteruptType::NON_MASKABLE) || v >10 || v<0) {
		return;
	}
	InteruptVectorTable[static_cast<int>(type)].priority = v;
}

void do_nothing(InteruptType t, int a, int64_t b) {

}
void panic(InteruptType, int, int64_t) {
	throw "panic";
}


InteruptSnapshot get_interupt_snapshot() {
	disable_timer();
	mu.lock();
	InteruptSnapshot snapshot{};
	while (!queue.empty()) {
		snapshot.interupts.push_back(queue.top());
		queue.pop();
	}
	// re-fill the queue
	for (auto& v : snapshot.interupts) {
		queue.push(v);
	}
	memcpy(snapshot.interuptVectorTable, InteruptVectorTable, sizeof(InteruptVectorTable));
	memset(snapshot.valid, false, sizeof(snapshot.valid));
	InteruptValid valid{};
	snapshot.valid[static_cast<int>(InteruptType::TIMER)] = valid.is_set(InteruptType::TIMER);
	snapshot.valid[static_cast<int>(InteruptType::SOFTWARE)] = valid.is_set(InteruptType::SOFTWARE);
	snapshot.valid[static_cast<int>(InteruptType::EXTERNAL_1)] = valid.is_set(InteruptType::EXTERNAL_1);
	snapshot.valid[static_cast<int>(InteruptType::ERROR)] = valid.is_set(InteruptType::ERROR);
	snapshot.valid[static_cast<int>(InteruptType::EXTERNAL_0)] = valid.is_set(InteruptType::EXTERNAL_0);
	mu.unlock();
	enable_timer();
}