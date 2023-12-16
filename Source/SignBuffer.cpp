#include "SignBuffer.h"
#include <cmath>
#include <float.h>

const unsigned int SignBuffer::threshold = 10;

SignBuffer::SignBuffer(const unsigned int newLength)
{
	length = newLength;
	maxUsableIndex = newLength / 2;
	signs = signs = new bool[newLength];
	writePointer = 0;
}

bool SignBuffer::WriteSignToBuffer(const bool sign)
{
	signs[writePointer] = sign;
	writePointer++;
	writePointer %= length;

	if (writePointer == 0)
	{
		currentPeriod = CalculatePeriod();
		return true;
	}

	return false;
}

double SignBuffer::GetPitch(const double sampleRate)
{
	if (currentPeriod == 0.0) return 0.0f;
	const double secondsDifference = currentPeriod / sampleRate;
	const double frequency = 1 / secondsDifference;
	return frequency;
}

double SignBuffer::CalculatePeriod()
{
	bool exitedThreshold = false;
	unsigned int minimaIndex = 0;
	unsigned int minima = INT_MAX;

	for (unsigned int i = 1; i < maxUsableIndex; ++i)
	{
		const unsigned int modifiedAutocorrelationAtIndex = ModifiedAutocorrelation(i);
		const bool isInThreshold = modifiedAutocorrelationAtIndex <= threshold;

		if (!isInThreshold)
		{
			if (minimaIndex > 0)
			{
				return minimaIndex;
			}
			exitedThreshold = true;
		}
		else if (exitedThreshold && modifiedAutocorrelationAtIndex < minima)
		{
			minimaIndex = i;
			minima = modifiedAutocorrelationAtIndex;
		}
	}

	return 0.0f;
}

unsigned int SignBuffer::ModifiedAutocorrelation(const unsigned int lag)
{
	unsigned int sum = 0;

	for (unsigned int i = 0; i < maxUsableIndex; ++i)
	{
		sum += GetSignAtOffsetIndex(i) ^ GetSignAtOffsetIndex(i + lag);
	}

	return sum;
}

bool SignBuffer::GetSignAtOffsetIndex(const unsigned int index)
{
	unsigned int offsetIndex = (writePointer + index) % length;
	return signs[offsetIndex];
}
