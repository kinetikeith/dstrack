#ifndef DST_DSTRACKER_HPP
#define DST_DSTRACKER_HPP

#include "Biquad.hpp"
#include "RingBuffer.hpp"

class DSTracker
{
public:

		DSTracker(float minFreq, float maxFreq, int winSize, int filtOrder, float sampRate);
		~DSTracker();

	void	processSample(float);
	void	processFrame(float*, float*, float*, int);

	void	setFreqRange(float, float);

	float	getMagResult();
	float	getArgResult();

private:

	void	calcCoefs();

	void	autocorrelate();

	void	filterAC();

	void	calcResult();
	void	filterResult();

	float		minFreq;
	float		maxFreq;

	float		minDelay;
	float		maxDelay;
	float		distDelay;

	float		sampRate;
	int		winSize;

	/* Input Signal */
	RingBuffer<float>	ringBuffer;
	
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


#endif /* DST_DSTRACKER_HPP */
