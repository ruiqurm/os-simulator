#include "interupt/interupt.h"
#include<cassert>
int main() {
	init_interupt();
	InteruptMask mask{};
	//mask.set(InteruptType::ERROR);
	//assert(mask.is_set(InteruptType::ERROR) == false);
	//mask.set(InteruptType::TIMER).set(InteruptType::SOFTWARE).set(InteruptType::EXTERNAL_1);
	//assert(mask.is_set(InteruptType::TIMER) == true);
	//mask.unset(InteruptType::TIMER).unset(InteruptType::SOFTWARE).unset(InteruptType::EXTERNAL_1);
	//assert(mask.is_set(InteruptType::TIMER) == false);



	//Interupt i{ InteruptType::ERROR,0,0,0 };
	//assert(i.is_blocking() == false);
	//push_off();
	//assert(i.is_blocking() == false);
	//pop_off();
	//i.type = InteruptType::SOFTWARE;
	//assert(i.is_blocking() == false);
	//push_off();
	//assert(i.is_blocking() == true);
	//pop_off();
	//mask.unset(InteruptType::SOFTWARE);
	//assert(i.is_blocking() == true);
	//i.type = InteruptType::EXTERNAL_1;
	//assert(i.is_blocking() == false);




}