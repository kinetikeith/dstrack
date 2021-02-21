#include "DSTracker.hpp"

#include <algorithm>
#include <cmath>

#include <cassert>

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

DSTracker::DSTracker(float mnFreq, float mxFreq, int wSize, int fOrder, float sRate) :
	minFreq(mnFreq),
	minFreqOrig(mnFreq),
	maxFreq(mxFreq),
	winSize(wSize),
	sampRate(sRate),
	minDelay(sRate / (mxFreq * 4)),
	maxDelay(sRate / (mnFreq * 4)),
	sigSize(maxDelay + 1), sigPos(0),
	sigLowpass(fOrder),
	sigHighpass(fOrder),
	preMagHighpass(fOrder, wSize),
	preArgHighpass(fOrder, wSize),
	deltaMagLowpass(fOrder, wSize),
	deltaArgLowpass(fOrder, wSize),
	resMagLowpass(fOrder),
	resArgLowpass(fOrder)
{

	/* Input Signal */
	sigBuffer = new float[sigSize];
	std::fill_n(sigBuffer, sigSize, 0.0f);

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

	delete[] sigBuffer;

	delete[] rawArg;

	delete[] preMag;
	delete[] preArg;

	delete[] deltaMag;
	delete[] deltaArg;

	delete[] prob;

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

void DSTracker::processSample(float sample)
{

	sigBuffer[sigPos] = sigLowpass.process(sigHighpass.process(sample));

	autocorrelate();

	filterAC();

	calcResult();

	filterResult();

	sigPos = (sigPos + 1) % sigSize;

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

	minFreq = std::max(newMinFreq, minFreqOrig);
	assert(newMaxFreq >= minFreq);
	maxFreq = newMaxFreq;

	maxDelay = sampRate / (minFreq * 4);
	minDelay = sampRate / (maxFreq * 4);

	calcCoefs();

}


void DSTracker::autocorrelate()
{

	float delay;

	float a = sigBuffer[sigPos];
	if(a == 0.0)
	{
		a = 0.000000001;

	}

	float b;

	float t;
	float tI;
	int i0;
	int i1;

	float argDelta;
	float arg;

	for(int j = 0; j < winSize; j++)
	{

		delay = std::lerp(maxDelay, minDelay, (float(j + 1) / winSize));

		t = std::modf(delay, &tI);
		i0 = ((sigPos - int(tI)) + sigSize) % sigSize;
		i1 = ((sigPos - int(tI + 1)) + sigSize) % sigSize;

		b = std::lerp(sigBuffer[i0], sigBuffer[i1], t);

		arg = std::atan(b / a);
		argDelta = (arg - rawArg[j]) / M_PI;
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
