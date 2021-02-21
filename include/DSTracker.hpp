#ifndef DSTRACKER_HPP
#define DSTRACKER_HPP

#include "Biquad.hpp"

class DSTracker
{
public:

		DSTracker(float minFreq, float maxFreq, int winSize, int filtOrder, float sampRate);
		~DSTracker();

	void	processSample(float);
	void	processFrame(float*, float*, float*, int);

	void	setFreqRange(float, float);

	/* Result Vars */
	float	resMag;
	float	resArg;

private:

	void	calcCoefs();

	void	autocorrelate();

	void	filterAC();

	void	calcResult();
	void	filterResult();

	float		minFreq;
	const float	minFreqOrig;
	float		maxFreq;

	float		minDelay;
	float		maxDelay;

	float		sampRate;
	int		winSize;

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

	/* Signal Filters */
	Biquad		sigLowpass;
	Biquad		sigHighpass;

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
