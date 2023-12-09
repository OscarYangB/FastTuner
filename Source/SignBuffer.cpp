#include "SignBuffer.h"
#include <cmath>

static double threshold = 400.0;

SignBuffer::SignBuffer(const unsigned int newLength)
{
	length = newLength;
	signs = new bool[newLength];
	writePointer = 0;
}

void SignBuffer::WriteSignToBuffer(const bool sign)
{
	signs[writePointer] = sign;
	writePointer++;
	writePointer %= length;
}

double SignBuffer::DetectPitch()
{
	int indexUnderThreshold = -1;

	for (int i = 0; i < length; ++i)
	{
		const double modifiedAutocorrelationAtIndex = DoModifiedAutocorrelationOnIndex(i);
		
		if (modifiedAutocorrelationAtIndex < threshold)
		{
			if (indexUnderThreshold < 0)
			{
				indexUnderThreshold = i;
			}
			else
			{
				return 1.0 / (i - indexUnderThreshold);
			}
		}
	}

	return 0.0;
}

double SignBuffer::DoModifiedAutocorrelationOnIndex(const unsigned int index)
{
	int sum = 0;

	for (int i = 0; i < length - index; ++i) 
	{
		sum += abs(signs[i] - signs[i + index]);
	}

	return sum;
}
