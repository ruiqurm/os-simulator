#include<iostream>
#include<string>
#include <stdio.h>
#include "memory.h"
using namespace std;
void Error(string x) {
	cout << "ERROR: " << x << endl;
	exit(-1);
}
void OK(string x) {
	cout << "OK: " << x << endl;
}
int test1() {
	init();
	v_address a;
	if (alloc(&a, 1024, 1) != 0) {
		Error("test1 fail alloc");
		exit(-1);
	}
	atom_data b;
	if (read(&b, a, 1) != 0) {
		Error("test1 fail read normal");
		exit(-1);
	}
	if (read(&b, a + 1024, 1) != -1) {
		Error("test1 fail read no normal");
	}
	OK("test1");
}
int test2() {
	init();
	v_address a;
	if (alloc(&a, 1024, 1) != 0) {
		Error("test2 fail alloc");
	}

	atom_data b = 'w';
	if (write(b, a, 1) != 0) {
		Error("test2 fail write");
	}

	atom_data c;
	if (read(&c, a, 1) != 0 || c != b) {
		Error("test2 fail read");
	}
	OK("test2");
}
int test3() {
	init();
	v_address a;
	if (alloc(&a, 1024 * 70, 1) != 0) {
		Error("test3 fail alloc");
	}
	atom_data b = 'w';
	atom_data c;
	for (int i = 0; i < 1000; ++i) {
		if (write(b, a, 1) != 0
			|| write(b, a + 1024 * 60, 1) != 0) {
			Error("test3 fail write");
		}

		if (read(&c, a, 1) != 0
			|| read(&c, a + 1024 * 60, 1) != 0) {
			Error("test3 fail read");
		}
	}
	OK("test3");
}
int test4() {
	init();

	v_address addr[91];
	m_size size = 1024 * 8;

	for (m_size i = 0; i < 64; ++i) {
		if (alloc(addr + i, 1024 * 8, i + 1) != 0) {
			Error("test6 fail alloc");
		}
	}
	for (m_size j = 0; j < 32; ++j) {
		if (free(addr[j], j + 1) != 0) {
			Error("test6 fail, free");

		}
	}
	for (m_size i = 0; i < 32; ++i) {
		if (alloc(addr + i, 1024 * 8, i + 1) != 0) {
			Error("test6 fail alloc2");
		}
	}
	OK("test4");


}

int main() {
	test1();
	test2();
	test3();
	test4();
}