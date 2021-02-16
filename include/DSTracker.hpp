#ifndef DSTRACKER_HPP
#define DSTRACKER_HPP

#include "Biquad.hpp"

class DSTracker
{
public:

		DSTracker(float minFreq, int winSize, int filtOrder, float sampRate);
		~DSTracker();

	void	processSample(float);
	void	processFrame(float*, float*, float*, int);

	void	calcCoefs();

	void	autocorrelate();

	void	filterAC();

	void	calcResult();
	void	filterResult();

	float	minFreq;
	float	sampRate;
	int	winSize;

	float	maxDelay;

	/* Input Signal */
	float*	sigBuffer;
	int	sigSize;
	int	sigPos;
	
	/* Autocorrelation Buffers (of winSize length) */
	float*	rawArg;

	float*	preMag;
	float*	preArg;

	float*	deltaMag;
	float*	deltaArg;

	float*	prob;

	/* Result Vars */
	float	resMag;
	float	resArg;

	/* Autocorrelation Filters */
	Biquad2D	preMagHighpass;
	Biquad2D	preArgHighpass;
	Biquad2D	deltaMagLowpass;
	Biquad2D	deltaArgLowpass;

	/* Result Filters */
	Biquad		resMagLowpass;
	Biquad		resArgLowpass;

}; /* class DSTracker */


#endif /* DSTRACKER_HPP */
