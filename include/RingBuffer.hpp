#ifndef DST_RINGBUFFER_HPP
#define DST_RINGBUFFER_HPP

#include <algorithm>
#include <cassert>
#include <cmath>

#include "compat.hpp"


template <typename T>
class RingBuffer
{
public:

		RingBuffer(unsigned long);
		~RingBuffer();

	void	push_back(T&);
	void	resize(unsigned long, bool = true);

	T	get(long);
	T	get(double);

private:

	T*		buffer;
	unsigned long	bufferSize;
	unsigned long	currentIndex;

} /* class RingBuffer */


template <typename T>
T truemod(T x, T y)
{

	T val = x % y;
	return x + (y * (val < 0));

}


template <typename T>
RingBuffer<T>::RingBuffer(unsigned long bufferSize_ = 0) :
	bufferSize(bufferSize_)
{

	assertm(newBufferSize != 0, "buffer size must be greater than zero");
	buffer = new T[bufferSize_];

}

template <typename T>
RingBuffer<T>::~RingBuffer()
{

	delete[] buffer;

}


template <typename T>
void RingBuffer<T>::push_back(T& val)
{

	currentIndex = (currentIndex + 1) % bufferSize;
	buffer[currentIndex] = val;

}


template <typename T>
T RingBuffer<T>::get(long offset)
{

	return buffer[truemod(currentIndex + offset, bufferSize)];

}


template <typename T>
T RingBuffer<T>::get(double offset)
{

	double offsetF;
	long offsetI;

	double t = std::fmod(offset, &offsetF);
	offsetI = offsetF;

	return std::lerp(get(offsetI), get(offsetI + 1), t);

}

template <typename T>
void RingBuffer<T>::resize(unsigned long newBufferSize, bool keepContents)
{

	assertm(newBufferSize != 0, "buffer size must be greater than zero");

	T* newBuffer = new T[newBufferSize];

	if(keepContents)
	{

		unsigned long copyRange = std::min(bufferSize, newBufferSize);

		for(unsigned long i = 0; i < copyRange; i++)
		{

			newBuffer[newBufferSize - i - 1] = get(-i);

		}

		currentIndex = newBufferSize - 1;

	}

	delete[] buffer;
	buffer = newBuffer;

}
