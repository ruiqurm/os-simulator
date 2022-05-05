#include "pch.h"
//#include "../interupt/interupt.h"
#include "../interupt/interupt.cpp"
#include <functional>
TEST(InteruptValid, Interupt) {
	init_interupt();
	InteruptValid valid{};
	valid.set(InteruptType::ERROR);
	EXPECT_FALSE(valid.is_set(InteruptType::ERROR));
	valid.set(InteruptType::TIMER).set(InteruptType::SOFTWARE).set(InteruptType::EXTERNAL_1);
	EXPECT_TRUE(valid.is_set(InteruptType::TIMER));
	valid.unset(InteruptType::TIMER).unset(InteruptType::SOFTWARE).unset(InteruptType::EXTERNAL_1);
	EXPECT_FALSE(valid.is_set(InteruptType::TIMER));
	stop_timer();

}
TEST(Interupt_1, Interupt) {
	init_interupt();
	Interupt i{ InteruptType::ERROR,0,0,0 };
	EXPECT_FALSE(i.is_blocking());
	push_off();
	EXPECT_FALSE(i.is_blocking());
	pop_off();
	i.type = InteruptType::SOFTWARE;
	EXPECT_FALSE(i.is_blocking());
	push_off();
	EXPECT_TRUE(i.is_blocking());
	pop_off();
	InteruptValid valid{};
	valid.unset(InteruptType::SOFTWARE);
	EXPECT_TRUE(i.is_blocking());
	i.type = InteruptType::EXTERNAL_1;
	EXPECT_FALSE(i.is_blocking());
	stop_timer();
}

TEST(handleInterupt, Interupt) {
	init_interupt();
	set_handler(InteruptType::TIMER, [](InteruptType type, int a, int64_t v) {
		*(int*)v += 1;
		});
	set_handler(InteruptType::EXTERNAL_0, [](InteruptType type, int a, int64_t v) {
		*(int*)v += 5;
		});
	int v = 0;
	raise_interupt(InteruptType::TIMER, 0, (int64_t)(&v));

	handle_interupt();
	EXPECT_EQ(v, 1);

	// mulitiple
	InteruptValid valid{};
	valid.unset(InteruptType::TIMER);
	raise_interupt(InteruptType::TIMER, 0, (int64_t)(&v));
	raise_interupt(InteruptType::TIMER, 0, (int64_t)(&v));
	raise_interupt(InteruptType::TIMER, 0, (int64_t)(&v));
	raise_interupt(InteruptType::TIMER, 0, (int64_t)(&v));
	raise_interupt(InteruptType::EXTERNAL_0, 0, (int64_t)(&v));
	handle_interupt();
	EXPECT_EQ(v, 6);
	valid.set(InteruptType::TIMER);
	handle_interupt();
	EXPECT_EQ(v, 10);

	stop_timer();
}
TEST(Timer, Interupt) {

	init_interupt();
	enable_timer();
	auto pre = __cnt_timer_interupt;
	while (pre != 2) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		pre = __cnt_timer_interupt;
	}
	//handle_interupt();
	std::this_thread::sleep_for(std::chrono::milliseconds(90));
	EXPECT_EQ(__cnt_timer_interupt, pre);
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	EXPECT_EQ(__cnt_timer_interupt, pre+1);

	stop_timer();
}
