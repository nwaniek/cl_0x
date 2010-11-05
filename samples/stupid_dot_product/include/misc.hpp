#ifndef __MISC_HPP__9289FB36_2DC8_4275_90FB_1E96B7E64B67
#define __MISC_HPP__9289FB36_2DC8_4275_90FB_1E96B7E64B67


template <typename ...Args>
struct count;

template <>
struct count<> { static const int value = 0; };

template <typename T, typename ...Args>
struct count<T, Args...>
{
	static const int value = 1 + count<Args...>::value;
};


#endif /* __MISC_HPP__9289FB36_2DC8_4275_90FB_1E96B7E64B67 */

