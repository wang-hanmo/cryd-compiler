#include<bitmap.h>
BitMap::BitMap(size_t i_size) :m_size(i_size)
{
	m_array.clear();
	m_array.resize((i_size - 1 + unit_length) / unit_length);
}
BitMap::BitMap(const BitMap& rhs):m_size(rhs.m_size),m_array(rhs.m_array){}
size_t BitMap::size()const { return m_size; }
void BitMap::set(size_t index)
{
	assert(index >= 0 && index < m_size);
	m_array[index / unit_length] |= basic_t(1) << (index % unit_length);
}
void BitMap::reset(size_t index)
{
	assert(index >= 0 && index < m_size);
	m_array[index / unit_length] &= ~(basic_t(1) << (index % unit_length));
}
void BitMap::set_all()
{
	for (int i = 0; i < m_array.size(); ++i)
		m_array[i] = ~((basic_t)0);
}
void BitMap::reset_all()
{
	for (int i = 0; i < m_array.size(); ++i)
		m_array[i] = (basic_t)0;
}
bool BitMap::test(size_t index)const
{
	assert(index >= 0 && index < m_size);
	return (bool)((m_array[index / unit_length] >> (index % unit_length)) & basic_t(1));
}
bool BitMap::empty()
{
	for (auto unit : m_array)
		if (unit != 0)
			return false;
	return true;
}
std::string BitMap::get_string()
{
	std::string res("");
	for (int i = 0; i < m_size; ++i)
		res.push_back(test(i) ? '1' : '0');
	return res;
}
BitMap& BitMap::operator &=(const BitMap& rhs)
{
	assert(m_size == rhs.m_size);
	for (int i = 0; i < m_array.size(); ++i)
		m_array[i] &= rhs.m_array[i];
	return *this;
}
BitMap& BitMap::operator -=(const BitMap& rhs)
{
	assert(m_size == rhs.m_size);
	for (int i = 0; i < m_array.size(); ++i)
		m_array[i] = m_array[i] & ~(rhs.m_array[i]);
	return *this;
}
BitMap& BitMap::operator |=(const BitMap& rhs)
{
	assert(m_size == rhs.m_size);
	for (int i = 0; i < m_array.size(); ++i)
		m_array[i] |= rhs.m_array[i];
	return *this;
}
BitMap& BitMap::operator ^=(const BitMap& rhs)
{
	assert(m_size == rhs.m_size);
	for (int i = 0; i < m_array.size(); ++i)
		m_array[i] ^= rhs.m_array[i];
	return *this;
}
BitMap BitMap::operator &(const BitMap& rhs)const
{
	assert(m_size == rhs.m_size);
	BitMap res(m_size);
	for (int i = 0; i < m_array.size(); ++i)
		res.m_array[i] = m_array[i] & rhs.m_array[i];
	return res;
}
BitMap BitMap::operator |(const BitMap& rhs)const
{
	assert(m_size == rhs.m_size);
	BitMap res(m_size);
	for (int i = 0; i < m_array.size(); ++i)
		res.m_array[i] = m_array[i] | rhs.m_array[i];
	return res;
}
BitMap BitMap::operator ^(const BitMap& rhs)const
{
	assert(m_size == rhs.m_size);
	BitMap res(m_size);
	for (int i = 0; i < m_array.size(); ++i)
		res.m_array[i] = m_array[i] ^ rhs.m_array[i];
	return res;
}
BitMap BitMap::operator -(const BitMap& rhs)const
{
	assert(m_size == rhs.m_size);
	BitMap res(m_size);
	for (int i = 0; i < m_array.size(); ++i)
		res.m_array[i] = m_array[i] & ~(rhs.m_array[i]);
	return res;
}
bool BitMap::operator !=(const BitMap& rhs)const 
{
	if (m_size != rhs.m_size)
		return true;
	if (m_size == 0)
		return false;
	for (int i = 0; i < m_array.size() - 1; ++i)
		if (m_array[i] != rhs.m_array[i])
			return true;
	if (m_size % unit_length == 0) {
		return m_array.back() != rhs.m_array.back();
	} else {
		basic_t mask = (((basic_t)1) << (m_size % unit_length)) - 1;
		return (m_array.back() & mask) != (rhs.m_array.back() & mask);
	}
}