#include "DSTracker.hpp"

#include <algorithm>
#include <cmath>

#include <iostream>

DSTracker::DSTracker(float mFreq, int wSize, int fOrder, float sRate, int bSize) :
	minFreq(mFreq), winSize(wSize), sampRate(sRate), bufSize(bSize),
	maxDelay(sRate / (mFreq * 4)), sigSize(maxDelay + 1), sigPos(0),
	magPreHighpass(fOrder, wSize), argPreHighpass(fOrder, wSize),
	magLowpass(fOrder, wSize), argLowpass(fOrder, wSize),
	magPostLowpass(fOrder), argPostLowpass(fOrder)
{

	sigBuffer = new float[sigSize];
	std::fill_n(sigBuffer, sigSize, 0.0f);

	argPreBuffer = new float[winSize];
	std::fill_n(argPreBuffer, winSize, 0.0f);

	probBuffer = new float[winSize];
	std::fill_n(probBuffer, winSize, 0.0f);

	magResBuffer = new float[bufSize];
	std::fill_n(magResBuffer, bufSize, 0.0f);
	argResBuffer = new float[bufSize];
	std::fill_n(argResBuffer, bufSize, 0.0f);

	preMag = new float[winSize];
	preArg = new float[winSize];
	deltaMag = new float[winSize];
	deltaArg = new float[winSize];

	float postMag = 0.0f;
	float postArg = 0.0f;

	calcCoefs();

}

DSTracker::~DSTracker()
{

	delete[] sigBuffer;

	delete[] argPreBuffer;
	delete[] probBuffer;

	delete[] preMag;
	delete[] preArg;
	delete[] deltaMag;
	delete[] deltaArg;

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

	Coefs hpfCoefs(
		(k * k) / delta,
		(-2.0f * (k * k)) / delta,
		(k * k) / delta,
		((-2.0f * (k * k)) + (2.0f * (omega_c * omega_c))) / delta,
		((-2.0f * k * omega_c) + (k * k) + (omega_c * omega_c)) / delta);

	Coefs lpfCoefs(
		(omega_c * omega_c) / delta,
		(2.0f * (omega_c * omega_c)) / delta,
		(omega_c * omega_c) / delta,
		((-2.0f * (k * k)) + (2.0f * (omega_c * omega_c))) / delta,
		((-2.0f * k * omega_c) + (k * k) + (omega_c * omega_c)) / delta);

	magPreHighpass.coefs = hpfCoefs;
	argPreHighpass.coefs = hpfCoefs;
	magLowpass.coefs = lpfCoefs;
	argLowpass.coefs = lpfCoefs;
	magPostLowpass.coefs = lpfCoefs;
	argPostLowpass.coefs = lpfCoefs;

}

void DSTracker::processFrame(float* buf)
{

	for(int i = 0; i < bufSize; i++)
	{

		sigBuffer[sigPos] = buf[i];
		
		autocorrelate();

		filterAC();

		calcResult();

		magResBuffer[i] = magPostLowpass.process(postMag);
		argResBuffer[i] = argPostLowpass.process(postArg);

		// Prepare for next frame
		sigPos = (sigPos + 1) % sigSize;

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

		preMag[j] = std::sqrt((sinVal * sinVal) + (cosVal * cosVal));

		arg = std::atan(cosVal / sinVal);
		argDelta = (arg - argPreBuffer[j]) / M_PI;
		argPreBuffer[j] = arg;

		preArg[j] = argDelta - std::round(argDelta);

	}

}

void DSTracker::filterAC()
{

	float* mBuf = magPreHighpass.process(preMag);
	float* aBuf = argPreHighpass.process(preArg);

	for(unsigned int j = 0; j < winSize; j++)
	{

		deltaMag[j] = std::abs(mBuf[j]);
		deltaArg[j] = std::abs(mBuf[j]);

	}

	mBuf = magLowpass.process(deltaMag);
	aBuf = argLowpass.process(deltaArg);

	for(unsigned int j = 0; j < winSize; j++)
	{

		probBuffer[j] = mBuf[j] * aBuf[j];

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

	postMag = std::abs(preMag[minJ]);
	postArg = std::abs(preArg[minJ]);

}	
