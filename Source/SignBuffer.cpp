#include "SignBuffer.h"
#include <cmath>
#include <float.h>
#include "PluginProcessor.h"

const double SignBuffer::threshold = 100;

SignBuffer::SignBuffer()
{
	length = 0;
	signs = nullptr;
	writePointer = 0;
}

void SignBuffer::InitializeBuffer(const unsigned int newLength)
{
	length = newLength;
	signs = new bool[newLength];
	writePointer = 0;
}

SignBuffer::~SignBuffer()
{
	delete[] signs;
}

void SignBuffer::WriteSignToBuffer(const bool sign)
{
	signs[writePointer] = sign;
	writePointer++;
	writePointer %= length;

	if (writePointer == 0)
	{
		currentSampleDifference = GetSampleDifference();
	}
}

double SignBuffer::GetSampleDifference()
{
	int enterThresholdIndexes[2] = { -1 , -1 };
	int exitThresholdIndexes[2] = { -1 , -1 };

	bool isBelowThreshold = false;

	for (unsigned int i = 0; i < GetMaxUsableIndex(); ++i)
	{
		const double modifiedAutocorrelationAtIndex = ModifiedAutocorrelation(i);

		//DBG(modifiedAutocorrelationAtIndex);

		const bool newIsBelowThreshold = modifiedAutocorrelationAtIndex < threshold;
		
		if (newIsBelowThreshold != isBelowThreshold)
		{
			isBelowThreshold = !isBelowThreshold;

			if (isBelowThreshold)
			{
				for (int j = 0; j < 2; ++j) 
				{
					if (enterThresholdIndexes[j] < 0) 
					{
						//DBG(modifiedAutocorrelationAtIndex);
						enterThresholdIndexes[j] = i;
						break;
					}
				}
			}
			else
			{
				for (int j = 0; j < 2; ++j)
				{
					if (exitThresholdIndexes[j] < 0)
					{
						exitThresholdIndexes[j] = i;
						break;
					}
				}
			}
		}
	}

	double firstIndex = (enterThresholdIndexes[0] + exitThresholdIndexes[0]) / 2.0;
	double secondIndex = (enterThresholdIndexes[1] + exitThresholdIndexes[1]) / 2.0;
	return secondIndex - firstIndex;
}

bool SignBuffer::GetSignAtOffsetIndex(const unsigned int index)
{
	int offsetIndex = (writePointer + index) % length;
	return signs[offsetIndex];
}

double SignBuffer::ModifiedAutocorrelation(const unsigned int lag)
{
	int sum = 0;

	for (unsigned int i = 0; i < GetMaxUsableIndex(); ++i)
	{
		sum += std::abs(GetSignAtOffsetIndex(i) - GetSignAtOffsetIndex(i + lag));
	}

	return sum;
}

unsigned int SignBuffer::GetMaxUsableIndex()
{
	return (unsigned int) floor((double) length / 2.0);
}
