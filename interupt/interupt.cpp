#include "interupt.h"
#include <atomic>
#include<cstdint>
#include<vector>
#include<queue>
#include<ctime>

using std::vector;
using std::uint16_t;
using std::priority_queue;
const auto InteruptVectorTableSize = 128;

mutex mu;
std::atomic<uint16_t> mask;
struct InteruptVector {
	int interupt_id;
	std::function<void(InteruptType,int,int)> handler = nullptr;
	int priority;
};

InteruptVector InteruptVectorTable[InteruptVectorTableSize];

struct Interupt {
	InteruptType type;
	int device_id;
	int64_t time;
	int64_t value;
	bool operator < (const Interupt& b)const;
	bool is_blocking() const;
};

bool Interupt::operator < (const Interupt& b)const {
	if (type == b.type) {
		return time < b.time || (time == b.time && device_id < b.device_id);
	}
	else {
		return InteruptVectorTable[static_cast<int>(type)].priority < InteruptVectorTable[static_cast<int>(b.type)].priority;
	}
}
bool Interupt::is_blocking() const {
	if (static_cast<int>(type) > static_cast<int>(InteruptType::Non_Maskable)) {
		return false;
	}
	else {
		return !(mask & 1 && (mask & (1 << static_cast<int>(type))));
	}
}

priority_queue<Interupt, std::vector<Interupt>, std::greater<Interupt>> queue;



bool InteruptMask::is_set(InteruptType t) {
	uint16_t offset = 1<<static_cast<int>(t);
	switch (t)
	{
	case InteruptType::TIMER:
	case InteruptType::SOFTWARE:
	case InteruptType::EXTERNAL_1:
		return mask & offset;
	case InteruptType::ERROR:
	case InteruptType::EXTERNAL_0:
		return false;
	default:
		return false;
	}
}
InteruptMask& InteruptMask::set(InteruptType t) {
	uint16_t m = 1 << static_cast<int>(t);
	switch (t)
	{
	case InteruptType::TIMER:
	case InteruptType::SOFTWARE:
	case InteruptType::EXTERNAL_1:
		mask |= m;
		break;
	case InteruptType::ERROR:
	case InteruptType::EXTERNAL_0:
	default:
		break;
	}
	return *this;
}

InteruptMask& InteruptMask::unset(InteruptType t) {
	uint16_t m = ~(1 << static_cast<int>(t));
	switch (t)
	{
	case InteruptType::TIMER:
	case InteruptType::SOFTWARE:
	case InteruptType::EXTERNAL_1:
		mask&= m;
		break;
	case InteruptType::ERROR:
	case InteruptType::EXTERNAL_0:
	default:
		break;
	}
	return *this;
}

void init_interupt() {
	mask = 1 |
		(1 << static_cast<int>(InteruptType::TIMER)) |
		(1 << static_cast<int>(InteruptType::SOFTWARE)) |
		(1 << static_cast<int>(InteruptType::EXTERNAL_1));
	InteruptVectorTable[static_cast<int>(InteruptType::TIMER)].priority = 0;
	InteruptVectorTable[static_cast<int>(InteruptType::SOFTWARE)].priority = 0;
	InteruptVectorTable[static_cast<int>(InteruptType::EXTERNAL_1)].priority = 1;
	InteruptVectorTable[static_cast<int>(InteruptType::EXTERNAL_0)].priority = 100;
	InteruptVectorTable[static_cast<int>(InteruptType::ERROR)].priority = 101;


}

void push_off() {
	mask &= 0xfffe;
}

void pop_off() {
	mask |= 1;
}

void raise_interupt(InteruptType t,int device_id,int64_t value) {
	mu.lock();
	queue.push(Interupt{ t,device_id,value });
	mu.unlock();
}


void handle_interupt() {
	vector<Interupt> should_delay;
	while (!queue.empty()) {
		auto top = queue.top();
		if (top.is_blocking()) {
			should_delay.push_back(top);
			queue.pop();
		}
		else {
			InteruptVectorTable[static_cast<int>(top.type)].handler(top.type, top.device_id, top.value);
			queue.pop();
			break;
		}
	}
	for (auto& v : should_delay) {
		queue.push(v);
	}
}

void set_handler(InteruptType type, std::function<void(InteruptType, int, int)> &f) {
	InteruptVectorTable[static_cast<int>(type)].handler = f;
}

void set_priority(InteruptType type, int v) {
	if (static_cast<int>(type) > static_cast<int>(InteruptType::Non_Maskable) || v >10 || v<0) {
		return;
	}
	InteruptVectorTable[static_cast<int>(type)].priority = v;
}