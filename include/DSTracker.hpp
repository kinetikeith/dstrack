#ifndef DSTRACKER_HPP
#define DSTRACKER_HPP

class DSTracker
{
public:

		DSTracker(float minFreq, int winSize, int filtOrder, float sampRate, int bufSize);
		~DSTracker();

	void	processFrame(float* buf);

	float*	getMagBuffer();
	float*	getArgBuffer();

	float lpf_b0;
	float lpf_b1;
	float lpf_b2;
	float lpf_a1;
	float lpf_a2;

	float hpf_b0;
	float hpf_b1;
	float hpf_b2;
	float hpf_a1;
	float hpf_a2;

	void	calcCoefs();

	void	autocorrelate();

	void	filterMag();
	void	filterArg();

	void	calcResult();
	void	filterResult();

	float	minFreq;
	float	sampRate;
	int	winSize;
	int	filtOrder;
	int	bufSize;

	float	maxDelay;

	int	sigPos;
	int	fxPos0;
	int	fxPos1;
	int	fxPos2;

	float*	sigBuffer; // sigSize length
	int	sigSize;

	float*	argPreBuffer; // winSize length
	float*	probBuffer; // winSize length

	float*	magResBuffer;
	float*	argResBuffer;

	float*	f0State; // winSize * (filtOrder + 1) * 3 length
	float*	f1State;
	float*	f2State;
	float*	f3State;
	float*	f4State; // (filtOrder + 1) * 3 length
	float*	f5State;

}; /* class DSTracker */


#endif /* DSTRACKER_HPP */
