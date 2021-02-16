#ifndef DSTRACKER_HPP
#define DSTRACKER_HPP

#include "Biquad.hpp"

class DSTracker
{
public:

		DSTracker(float minFreq, int winSize, int filtOrder, float sampRate, int bufSize);
		~DSTracker();

	void	processFrame(float* buf);

	float*	getMagBuffer();
	float*	getArgBuffer();

	void	calcCoefs();

	void	autocorrelate();

	void	filterAC();

	void	calcResult();
	void	filterResult();

	float	minFreq;
	float	sampRate;
	int	winSize;
	int	bufSize;

	float	maxDelay;

	float*	sigBuffer; // sigSize length
	int	sigSize;
	int	sigPos;

	float*	preMag;
	float*	preArg;

	float*	deltaMag;
	float*	deltaArg;

	float	postMag;
	float	postArg;

	float*	argPreBuffer; // winSize length
	float*	probBuffer; // winSize length

	float*	magResBuffer;
	float*	argResBuffer;

	Biquad2D	magPreHighpass;
	Biquad2D	argPreHighpass;
	Biquad2D	magLowpass;
	Biquad2D	argLowpass;
	Biquad		magPostLowpass;
	Biquad		argPostLowpass;

}; /* class DSTracker */


#endif /* DSTRACKER_HPP */
