#pragma once
#include <vector>
#include <cassert>
#include <string>
#include <bitmap.h>
class Interval
{
private:
	std::vector<basic_t> m_array;
	size_t m_size;
public:
	Interval(const Interval& rhs);
	Interval(size_t i_size);
	size_t size()const;
	void set_all();
	void reset_all();
	void set(size_t index);
	void reset(size_t index);
	bool test(size_t index)const;
	bool empty();
	std::string get_string();
	//不相等
	bool operator !=(const Interval& rhs)const;
	//交集、并集、异或、差集
	Interval& operator &=(const Interval& rhs);
	Interval& operator |=(const Interval& rhs);
	Interval& operator ^=(const Interval& rhs);
	Interval& operator -=(const Interval& rhs);
	Interval operator &(const Interval& rhs)const;
	Interval operator |(const Interval& rhs)const;
	Interval operator ^(const Interval& rhs)const;
	Interval operator -(const Interval& rhs)const;

    Interval& xor_sum(int start_index, int end_index);
};