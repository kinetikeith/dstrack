#ifndef BIQUAD2D_HPP
#define BIQUAD2D_HPP

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

};

#endif
