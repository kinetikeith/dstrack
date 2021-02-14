#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include "AudioFile.h"

#include "DSTracker.hpp"

int main(int argc, char** argv)
{

	float sRate;
	unsigned int bSize = 1024;
	float* inBuffer = new float[bSize];

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

	DSTracker dst(300, 12, 2, sRate, bSize);
	float* argBuf = dst.getArgBuffer();
	float* magBuf = dst.getMagBuffer();

	int i = 0;
	int imod;
	float val;
	float phase;

	while(i < nSamples)
	{

		imod = i % bSize;

		inBuffer[imod] = fBuf[i];
		i++;
		if(imod == (bSize - 1))
		{

			dst.processFrame(inBuffer);

			//std::cout << dst.f4State[dst.fxPos2] << std::endl;
			//std::cout << dst.f4State[(dst.filtOrder * 3) + dst.fxPos0] << std::endl;
		}

		phase = fmod(phase + (220.0 / sRate), 1.0);
		val = std::sin(2 * M_PI * phase) * magBuf[imod];
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
