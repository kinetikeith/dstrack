#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include "AudioFile.h"

#include "DSTracker.hpp"
#include "Biquad.hpp"

int main(int argc, char** argv)
{

	float sRate;

	std::string path;
	if(argc < 2)
	{

		std::cout << "Please supply a path." << std::endl;

	}

	AudioFile<float> aFile(argv[1]);

	if(aFile.load(argv[1]))
	{
		
		std::cout << "Loaded file " << argv[1] << " successfully" << std::endl;

	}
	else
	{

		std::cout << "Could not load file." << std::endl;
		return -1;

	}
	
	if(aFile.getNumChannels() != 1)
	{
		
		std::cout << "File must be mono.";
		return -1;
		
	}
	
	sRate = aFile.getSampleRate();
	unsigned int nSamples = aFile.getNumSamplesPerChannel();
	std::vector<float> fBuf = aFile.samples[0];
	std::vector<std::vector<float>> outBuffer = {{}};

	DSTracker dst(300, 12, 2, sRate);

	int i = 0;
	float val;
	float phase;

	while(i < nSamples)
	{

		dst.processSample(fBuf[i]);

		phase = fmod(phase + (220.0 / sRate), 1.0);
		val = std::sin(2 * M_PI * phase) * dst.resMag;
		outBuffer[0].push_back(val);

	}

	AudioFile<float> oFile;
	oFile.setNumChannels(1);
	oFile.setSampleRate(sRate);
	oFile.setBitDepth(aFile.getBitDepth());
	oFile.setAudioBuffer(outBuffer);

	oFile.save("outFile.wav");

	return 0;

}
