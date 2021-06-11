#include "DSTracker.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "compat.hpp"

Coefs getLinkwitzRileyHPF(float f_c, float f_s)
{
	
	/* Linkwitz-Riley LPF and HPF */
	float theta_c = (M_PI * f_c) / f_s;
	float omega_c = M_PI * f_c;
	float k = omega_c / std::tan(theta_c);
	float delta = (k * k) + (omega_c * omega_c) + (2 * k * omega_c);

	return Coefs(
		(k * k) / delta,
		(-2.0f * (k * k)) / delta,
		(k * k) / delta,
		((-2.0f * (k * k)) + (2.0f * (omega_c * omega_c))) / delta,
		((-2.0f * k * omega_c) + (k * k) + (omega_c * omega_c)) / delta);

}

Coefs getLinkwitzRileyLPF(float f_c, float f_s)
{

	float theta_c = (M_PI * f_c) / f_s;
	float omega_c = M_PI * f_c;
	float k = omega_c / std::tan(theta_c);
	float delta = (k * k) + (omega_c * omega_c) + (2 * k * omega_c);

	return Coefs(
		(omega_c * omega_c) / delta,
		(2.0f * (omega_c * omega_c)) / delta,
		(omega_c * omega_c) / delta,
		((-2.0f * (k * k)) + (2.0f * (omega_c * omega_c))) / delta,
		((-2.0f * k * omega_c) + (k * k) + (omega_c * omega_c)) / delta);

}

DSTracker::DSTracker(float mnFreq, float mxFreq, int wSize, int fOrder,
		float sRate) :
	minFreq(mnFreq),
	maxFreq(mxFreq),
	winSize(wSize),
	sampRate(sRate),
	minDelay(sRate / (mxFreq * 4)),
	maxDelay(sRate / (mnFreq * 4)),
	distDelay((maxDelay - minDelay) / wSize),
	ringBuffer(((unsigned long) maxDelay) + 1),
	sigLowpass(fOrder),
	sigHighpass(fOrder),
	preMagHighpass(fOrder, wSize),
	preArgHighpass(fOrder, wSize),
	deltaMagLowpass(fOrder, wSize),
	deltaArgLowpass(fOrder, wSize),
	resMagLowpass(fOrder),
	resArgLowpass(fOrder)
{

	/* Autocorrelation Buffers */
	rawArg = new float[winSize];
	std::fill_n(rawArg, winSize, 0.0f);

	preMag = new float[winSize];
	preArg = new float[winSize];

	deltaMag = new float[winSize];
	deltaArg = new float[winSize];

	prob = new float[winSize];
	std::fill_n(prob, winSize, 0.0f);

	/* Might as well calculate filter coefs now... */
	calcCoefs();

}

DSTracker::~DSTracker()
{

	delete[] rawArg;

	delete[] preMag;
	delete[] preArg;

	delete[] deltaMag;
	delete[] deltaArg;

	delete[] prob;

}

void DSTracker::processSample(float sample)
{

	ringBuffer.push_back(sigLowpass.process(sigHighpass.process(sample)));

	autocorrelate();

	filterAC();

	calcResult();

	filterResult();

}

void DSTracker::processFrame(float* inBuf, float* outMagBuf, float* outArgBuf, int bufSize)
{

	for(int i = 0; i < bufSize; i++)
	{

		processSample(inBuf[i]);
		
		outMagBuf[i] = resMag;
		outArgBuf[i] = resArg;

	}

}

void DSTracker::setFreqRange(float newMinFreq, float newMaxFreq)
{

	minFreq = newMinFreq;
	maxFreq = newMaxFreq;
	assert(maxFreq >= minFreq);

	float newMaxDelay = sampRate / (minFreq * 4);

	if(newMaxDelay > maxDelay)
	{

		ringBuffer.resize(((unsigned long) newMaxDelay) + 1, true);

	}

	maxDelay = newMaxDelay;
	minDelay = sampRate / (maxFreq * 4);
	distDelay = (maxDelay - minDelay) / winSize;

	calcCoefs();

}

float DSTracker::getMagResult()
{

	return resMag;

}

float DSTracker::getArgResult()
{

	return resArg;

}

void DSTracker::calcCoefs()
{

	Coefs hpfCoefs = getLinkwitzRileyHPF(minFreq, sampRate);
	Coefs lpfCoefs = getLinkwitzRileyLPF(minFreq, sampRate);

	preMagHighpass.coefs = hpfCoefs;
	preArgHighpass.coefs = hpfCoefs;
	deltaMagLowpass.coefs = lpfCoefs;
	deltaArgLowpass.coefs = lpfCoefs;
	resMagLowpass.coefs = lpfCoefs;
	resArgLowpass.coefs = lpfCoefs;

	sigLowpass.coefs = getLinkwitzRileyLPF(maxFreq, sampRate);
	sigHighpass.coefs = getLinkwitzRileyHPF(minFreq, sampRate);

}

void DSTracker::autocorrelate()
{

	float a = ringBuffer.getCurrent();
	a = a + (1.e-9f * (a == 0.0));

	double delay = minDelay;

	for(int j = 0; j < winSize; j++)
	{

		delay += distDelay;

		float b = ringBuffer.getLerp(-delay);

		float arg = std::atan(b / a);
		float argDelta = (arg - rawArg[j]) / M_PI;
		rawArg[j] = arg;

		preMag[j] = std::sqrt((a * a) + (b * b));
		preArg[j] = argDelta - std::round(argDelta);

	}

}

void DSTracker::filterAC()
{

	float* m = preMagHighpass.process(preMag);
	float* a = preArgHighpass.process(preArg);

	for(unsigned int j = 0; j < winSize; j++)
	{

		deltaMag[j] = std::abs(m[j]);
		deltaArg[j] = std::abs(a[j]);

	}

	m = deltaMagLowpass.process(deltaMag);
	a = deltaArgLowpass.process(deltaArg);

	for(unsigned int j = 0; j < winSize; j++)
	{

		prob[j] = m[j] * a[j];

	}

}

void DSTracker::calcResult()
{
	
	int minJ = 0;
	float minVal = prob[minJ];
	
	for(int j = 0; j < winSize; j++)
	{

		if(prob[j] < minVal)
		{

			minJ = j;
			minVal = prob[j];

		}

	}

	resMag = std::abs(preMag[minJ]);
	resArg = std::abs(preArg[minJ]);

}

void DSTracker::filterResult()
{

	resMag = resMagLowpass.process(resMag);
	resArg = resArgLowpass.process(resArg) / 2.0;

}
