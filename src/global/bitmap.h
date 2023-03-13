#pragma once
#include <vector>
#include <cassert>
#include <string>
typedef uint64_t basic_t;
constexpr int unit_length = sizeof(basic_t) * 8;
class BitMap
{
private:
	std::vector<basic_t> m_array;
	size_t m_size;
public:
	BitMap(const BitMap& rhs);
	BitMap(size_t i_size);
	size_t size()const;
	void set_all();
	void reset_all();
	void set(size_t index);
	void reset(size_t index);
	bool test(size_t index)const;
	bool empty();
	std::string get_string();
	//不相等
	bool operator !=(const BitMap& rhs)const;
	//交集、并集、异或、差集
	BitMap& operator &=(const BitMap& rhs);
	BitMap& operator |=(const BitMap& rhs);
	BitMap& operator ^=(const BitMap& rhs);
	BitMap& operator -=(const BitMap& rhs);
	BitMap operator &(const BitMap& rhs)const;
	BitMap operator |(const BitMap& rhs)const;
	BitMap operator ^(const BitMap& rhs)const;
	BitMap operator -(const BitMap& rhs)const;
};