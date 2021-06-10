#ifndef DST_BIQUAD_HPP
#define DST_BIQUAD_HPP

#include <cstddef>

struct Coefs
{

		Coefs();
		Coefs(float, float, float, float, float);

	float	b0;
	float	b1;
	float	b2;
	float	a1;
	float	a2;

}; /* struct Coefs */

struct Biquad
{

		Biquad(size_t);
		~Biquad();

	float	process(float);

	Coefs	coefs;

	float*	z0State;
	float*	z1State;
	float*	z2State;

	size_t	order;

}; /* class Biquad */

struct Biquad2D
{

		Biquad2D(size_t, size_t);
		~Biquad2D();

	float*	process(float*);

	Coefs	coefs;

	float*	z0State;
	float*	z1State;
	float*	z2State;

	size_t	order;
	size_t	width;

	size_t	zSize;

}; /* class Biquad2D */

#endif /* DST_BIQUAD_HPP */
