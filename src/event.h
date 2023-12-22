#pragma once
#include "util.h"
#

template <typename T>
class Event {
public:
	void invoke(T t) {
		for (auto& fn : work) {
			fn(t);
		}
	}

	void assign(void(*fn)(T)) {
		if (vec_contains<void(*)(T)>(work, fn))
			return;
		work.push_back(fn);
	}

	void unassign(void(*fn)(T)) {
		auto ele = std::find(work.begin(), work.end(), fn);
		if (ele != work.end())
			work.erase(ele);
	}

	void operator+=(void (*fn)(T)) { assign(fn); }
	void operator-=(void (*fn)(T)) { unassign(fn); }


private:
	std::vector<void (*)(T)> work;
};