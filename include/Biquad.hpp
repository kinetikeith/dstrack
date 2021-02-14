#ifndef BIQUAD_HPP
#define BIQUAD_HPP

struct Biquad
{

		Biquad(size_t);
		~Biquad();

	float	process(float);

	float	b0;
	float	b1;
	float	b2;
	float	a1;
	float	a2;

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

	float	b0;
	float	b1;
	float	b2;
	float	a1;
	float	a2;

	float*	z0State;
	float*	z1State;
	float*	z2State;

	size_t	order;
	size_t	width;

	size_t	zSize;

}; /* class Biquad2D */

#endif /* BIQUAD_HPP */
