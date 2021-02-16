#include "Biquad.hpp"

#include <algorithm>

Coefs::Coefs()
{

}

Coefs::Coefs(float _b0, float _b1, float _b2, float _a1, float _a2) :
	b0(_b0), b1(_b1), b2(_b2), a1(_a1), a2(_a2)
{

}

Biquad::Biquad(size_t o) : order(o)
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
	
	std::copy(z1State, z1State + order + 1, z2State);
	std::copy(z0State, z0State + order + 1, z1State);
	z0State[0] = zn;

	for(unsigned int i = 0; i < order; i++)
	{

		z0State[i + 1] =
			(coefs.b0 * z0State[i]) +
			(coefs.b1 * z1State[i]) +
			(coefs.b2 * z2State[i]) -
			(coefs.a1 * z1State[i + 1]) -
			(coefs.a2 * z2State[i + 1]);

	}

	return z0State[order];

}

Biquad2D::Biquad2D(size_t o, size_t w) : 
	order(o), width(w), zSize((o + 1) * w)
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

float* Biquad2D::process(float* znState)
{

	std::copy(z1State, z1State + zSize, z2State);
	std::copy(z0State, z0State + zSize, z1State);	
	std::copy(znState, znState + width, z0State);

	for(unsigned int i = 0; i < order; i++)
	{

		for(unsigned int j = 0; j < width; j++)
		{

			z0State[((i + 1) * width) + j] =
			(coefs.b0 * z0State[(i * width) + j]) +
			(coefs.b1 * z1State[(i * width) + j]) +
			(coefs.b2 * z2State[(i * width) + j]) -
			(coefs.a1 * z1State[((i + 1) * width) + j]) -
			(coefs.a2 * z2State[((i + 1) * width) + j]);

		}

	}

	return z0State + (order * width);
	
}
