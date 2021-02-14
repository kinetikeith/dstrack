#include "DSTracker.hpp"

#include <algorithm>
#include <cmath>

#include <iostream>

DSTracker::DSTracker(float mFreq, int wSize, int fOrder, float sRate, int bSize) :
	minFreq(mFreq), winSize(wSize), filtOrder(fOrder), sampRate(sRate),
       	bufSize(bSize), maxDelay(sRate / (mFreq * 4)), sigSize(maxDelay + 1),
	sigPos(0), fxPos0(0), fxPos1(2), fxPos2(1)
{

	sigBuffer = new float[sigSize];
	std::fill_n(sigBuffer, sigSize, 0.0f);

	argPreBuffer = new float[winSize];
	std::fill_n(argPreBuffer, winSize, 0.0f);

	probBuffer = new float[winSize];
	std::fill_n(probBuffer, winSize, 0.0f);

	f0State = new float[winSize * (filtOrder + 1) * 3];
	std::fill_n(f0State, (winSize * (filtOrder + 1)), 0.0f);
	f1State = new float[winSize * (filtOrder + 1) * 3];
	std::fill_n(f1State, (winSize * (filtOrder + 1)), 0.0f);
	f2State = new float[winSize * (filtOrder + 1) * 3];
	std::fill_n(f2State, (winSize * (filtOrder + 1)), 0.0f);
	f3State = new float[winSize * (filtOrder + 1) * 3];
	std::fill_n(f3State, (winSize * (filtOrder + 1)), 0.0f);
	f4State = new float[(filtOrder + 1)];
	std::fill_n(f4State, (filtOrder + 1), 0.0f);
	f5State = new float[(filtOrder + 1)];
	std::fill_n(f5State, (filtOrder + 1), 0.0f);

	magResBuffer = new float[bufSize];
	std::fill_n(magResBuffer, bufSize, 0.0f);
	argResBuffer = new float[bufSize];
	std::fill_n(argResBuffer, bufSize, 0.0f);

	calcCoefs();

}

DSTracker::~DSTracker()
{

	delete[] sigBuffer;

	delete[] argPreBuffer;
	delete[] probBuffer;

	delete[] f0State;
	delete[] f1State;
	delete[] f2State;
	delete[] f3State;
	delete[] f4State;
	delete[] f5State;

	delete[] magResBuffer;

}

float* DSTracker::getMagBuffer()
{

	return magResBuffer;

}

float* DSTracker::getArgBuffer()
{

	return argResBuffer;

}

void DSTracker::calcCoefs()
{

	/* Linkwitz-Riley LPF and HPF */
	float theta_c = (float(M_PI) * minFreq) / sampRate;
	float omega_c = float(M_PI) * minFreq;
	float k = omega_c / std::tan(theta_c);
	float delta = (k * k) + (omega_c * omega_c) + (2 * k * omega_c);

	hpf_b0 = (k * k) / delta;
	hpf_b1 = (-2.0f * (k * k)) / delta;
	hpf_b2 = (k * k) / delta;
	hpf_a1 = ((-2.0f * (k * k)) + (2.0f * (omega_c * omega_c))) / delta;
	hpf_a2 = ((-2.0f * k * omega_c) + (k * k) + (omega_c * omega_c)) / delta;

	lpf_b0 = (omega_c * omega_c) / delta;
	lpf_b1 = 2.0 * lpf_b0;
	lpf_b2 = lpf_b0;
	lpf_a1 = hpf_a1;
	lpf_a2 = hpf_a2;

}

void DSTracker::processFrame(float* buf)
{

	for(int i = 0; i < bufSize; i++)
	{

		sigBuffer[sigPos] = buf[i];
		
		autocorrelate();

		filterMag();
		filterArg();

		calcResult();

		filterResult();

		magResBuffer[i] = f4State[(filtOrder * 3) + fxPos0];
		argResBuffer[i] = f5State[(filtOrder * 3) + fxPos0];

		// Prepare for next frame
		sigPos = (sigPos + 1) % sigSize;
		fxPos0 = (fxPos0 + 1) % 3;
		fxPos1 = (fxPos1 + 1) % 3;
		fxPos2 = (fxPos2 + 1) % 3;

	}


}

void DSTracker::autocorrelate()
{

	float delay;
	float sinVal = sigBuffer[sigPos];
	float cosVal;
	int cosPos1;
	int cosPos2;
	float argDelta;
	float arg;
	float t;
	float lower;

	for(int j = 0; j < winSize; j++)
	{

		delay = maxDelay * (float(j + 1) / winSize);
		t = std::modf(delay, &lower);
		cosPos1 = (sigPos - int(lower)) % sigSize;
		cosPos2 = (sigPos - int(std::ceil(delay))) % sigSize;
		cosVal = std::lerp(sigBuffer[cosPos1], sigBuffer[cosPos2], t);

		f0State[(fxPos0 * winSize) + j] = std::sqrt((sinVal * sinVal) + (cosVal * cosVal));
		
		arg = std::atan(cosVal / sinVal);
		argDelta = (arg - argPreBuffer[j]) / M_PI;
		argPreBuffer[j] = arg;
		f1State[(fxPos0 * winSize) + j] = argDelta - std::round(argDelta);

	}

}

void DSTracker::filterMag()
{

	float b0;
	float b1;
	float b2;
	float a1;
	float a2;

	for(int k = 0; k < filtOrder; k++)
	{

		for(int j = 0; j < winSize; j++)
		{

			b0 = hpf_b0 * f0State[(k * 3 * winSize) + (fxPos0 * winSize) + j];
			b1 = hpf_b1 * f0State[(k * 3 * winSize) + (fxPos1 * winSize) + j];
			b2 = hpf_b2 * f0State[(k * 3 * winSize) + (fxPos2 * winSize) + j];
			a1 = hpf_a1 * f0State[((k + 1) * 3 * winSize) + (fxPos1 * winSize) + j];
			a2 = hpf_a2 * f0State[((k + 1) * 3 * winSize) + (fxPos2 * winSize) + j];

			f0State[((k + 1) * 3 * winSize) + (fxPos0 * winSize) + j] = b0 + b1 + b2 - (a1 + a2);

		}
	
	}

	for(int j = 0; j < winSize; j++)
	{

		f2State[(fxPos0 * winSize) + j] = std::abs(f0State[(filtOrder * 3 * winSize) + (fxPos0 * winSize) + j]);

	}

	for(int k = 0; k < filtOrder; k++)
	{

		for(int j = 0; j < winSize; j++)
		{

			b0 = lpf_b0 * f2State[(k * 3 * winSize) + (fxPos0 * winSize) + j];
			b1 = lpf_b1 * f2State[(k * 3 * winSize) + (fxPos1 * winSize) + j];
			b2 = lpf_b2 * f2State[(k * 3 * winSize) + (fxPos2 * winSize) + j];
			a1 = lpf_a1 * f2State[((k + 1) * 3 * winSize) + (fxPos1 * winSize) + j];
			a2 = lpf_a2 * f2State[((k + 1) * 3 * winSize) + (fxPos2 * winSize) + j];
	
			f2State[((k + 1) * 3 * winSize) + (fxPos0 * winSize) + j] = b0 + b1 + b2 - (a1 + a2);

		}

	}

	for(int j = 0; j < winSize; j++)
	{

		probBuffer[j] = f2State[(filtOrder * 3 * winSize) + (fxPos0 * winSize) + j];

	}

}

void DSTracker::filterArg()
{

	float b0;
	float b1;
	float b2;
	float a1;
	float a2;

	for(int k = 0; k < filtOrder; k++)
	{

		for(int j = 0; j < winSize; j++)
		{

			b0 = hpf_b0 * f1State[(k * 3 * winSize) + (fxPos0 * winSize) + j];
			b1 = hpf_b1 * f1State[(k * 3 * winSize) + (fxPos1 * winSize) + j];
			b2 = hpf_b2 * f1State[(k * 3 * winSize) + (fxPos2 * winSize) + j];
			a1 = hpf_a1 * f1State[((k + 1) * 3 * winSize) + (fxPos1 * winSize) + j];
			a2 = hpf_a2 * f1State[((k + 1) * 3 * winSize) + (fxPos2 * winSize) + j];

			f1State[((k + 1) * 3 * winSize) + (fxPos0 * winSize) + j] = b0 + b1 + b2 - (a1 + a2);

		}
	
	}

	for(int j = 0; j < winSize; j++)
	{

		f3State[(fxPos0 * winSize) + j] = std::abs(f1State[(filtOrder * 3 * winSize) + (fxPos0 * winSize) + j]);

	}

	for(int k = 0; k < filtOrder; k++)
	{

		for(int j = 0; j < winSize; j++)
		{

			b0 = lpf_b0 * f3State[(k * 3 * winSize) + (fxPos0 * winSize) + j];
			b1 = lpf_b1 * f3State[(k * 3 * winSize) + (fxPos1 * winSize) + j];
			b2 = lpf_b2 * f3State[(k * 3 * winSize) + (fxPos2 * winSize) + j];
			a1 = lpf_a1 * f3State[((k + 1) * 3 * winSize) + (fxPos1 * winSize) + j];
			a2 = lpf_a2 * f3State[((k + 1) * 3 * winSize) + (fxPos2 * winSize) + j];

			f3State[((k + 1) * 3 * winSize) + (fxPos0 * winSize) + j] = b0 + b1 + b2 - (a1 + a2);

		
		}

	}

	for(int j = 0; j < winSize; j++)
	{

		probBuffer[j] *= f3State[(filtOrder * 3 * winSize) + (fxPos0 * winSize) + j];

	}

}

void DSTracker::calcResult()
{
	
	int minJ = 0;
	float minVal = probBuffer[minJ];
	
	for(int j = 0; j < winSize; j++)
	{

		if(probBuffer[j] < minVal)
		{

			minVal = probBuffer[j];

		}

	}

	f4State[fxPos0] = f0State[(fxPos0 * winSize) + minJ];
	f5State[fxPos0] = std::abs(f1State[(fxPos0 * winSize) + minJ]);

}
	
void DSTracker::filterResult()
{

	float b0, b1, b2, a1, a2;

	// todo?: Merge For loops for higher performance

	for(int k = 0; k < filtOrder; k++)
	{

		b0 = lpf_b0 * f4State[(k * 3) + fxPos0];
		b1 = lpf_b1 = f4State[(k * 3) + fxPos1];
		b2 = lpf_b2 = f4State[(k * 3) + fxPos2];
		a1 = lpf_b2 = f4State[((k + 1) * 3) + fxPos1];
		a2 = lpf_b2 = f4State[((k + 1) * 3) + fxPos2];

		f4State[((k + 1) * 3) + fxPos0] = b0 + b1 + b2 - (a1 + a2);

	}

	for(int k = 0; k < filtOrder; k++)
	{

		b0 = lpf_b0 * f5State[(k * 3) + fxPos0];
		b1 = lpf_b1 = f5State[(k * 3) + fxPos1];
		b2 = lpf_b2 = f5State[(k * 3) + fxPos2];
		a1 = lpf_b2 = f5State[((k + 1) * 3) + fxPos1];
		a2 = lpf_b2 = f5State[((k + 1) * 3) + fxPos2];

		f5State[((k + 1) * 3) + fxPos0] = b0 + b1 + b2 - (a1 + a2);

	}

}
