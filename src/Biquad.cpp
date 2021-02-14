#include "Biquad.hpp"

#include <algorithm>

Biquad::Biquad(size_t o) :
	order(o)
{

	z0State = new float[o + 1];
	std::fill_n(z0State, (o + 1), 0.0f);

	z1State = new float[o + 1];
	std::fill_n(z1State, (o + 1), 0.0f);

	z2State = new float[o + 1];
	std::fill_n(z2State, (o + 1), 0.0f);

}

Biquad::~Biquad()
{

	delete[] z0State;
	delete[] z1State;
	delete[] z2State;

}

float Biquad::process(float zn)
{
	
	std::copy(z1State, z1State + order, z2State);
	std::copy(z0State, z0State + order, z1State);
	z0State[0] = zn;

	for(unsigned int i = 0; i < order; i++)
	{

		z0State[i + 1] =
			(b0 * z0State[i]) +
			(b1 * z1State[i]) +
			(b2 * z2State[i]) -
			(a1 * z1State[i + 1]) -
			(a2 * z2State[i + 1]);

	}

	return z0State[order];

}

Biquad2D::Biquad2D(size_t o, size_t w) : 
	order(o), width(w), zSize(o * w)
{

	z0State = new float[zSize];
	std::fill_n(z0State, zSize, 0.0f);

	z1State = new float[zSize];
	std::fill_n(z1State, zSize, 0.0f);

	z2State = new float[zSize];
	std::fill_n(z2State, zSize, 0.0f);

}

Biquad2D::~Biquad2D()
{

	delete[] z0State;
	delete[] z1State;
	delete[] z2State;

}

float* process(float* znState)
{

	std::copy(z1State, z1State + zSize, z2State);
	std::copy(z0State, z0State + zSize, z1State);	
	std::copy(znState, znState + width, z0State);

	for(unsigned int i = 0; i < order; i++)
	{

		for(unsigned int j = 0; j < width; j++)
		{

			z0State[((i + 1) * width) + j] =
			(b0 * z0State[(i * width) + j]) +
			(b1 * z1State[(i * width) + j]) +
			(b2 * z2State[(i * width) + j]) -
			(a1 * z1State[((i + 1) * width) + j]) -
			(a2 * z2State[((i + 1) * width) + j]);

		}

	}

	return z0State + (order * width);
	
}
