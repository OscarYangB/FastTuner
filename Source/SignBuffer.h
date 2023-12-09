#pragma once
class SignBuffer
{
public:
	SignBuffer(const unsigned int newLength);
	void WriteSignToBuffer(const bool sign);

	double DetectPitch();

private:
	bool* signs;
	unsigned int length;
	unsigned int writePointer;

	double DoModifiedAutocorrelationOnIndex(const unsigned int index);

	static double threshold;
};

