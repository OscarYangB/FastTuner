#pragma once
class SignBuffer
{
public:
	SignBuffer();
	void InitializeBuffer(const unsigned int newLength);

	~SignBuffer();

	void WriteSignToBuffer(const bool sign);
	double GetSampleDifference();

private:
	bool* signs;
	unsigned int length;
	unsigned int writePointer;

	bool GetSignAtOffsetIndex(const unsigned int index);
	double ModifiedAutocorrelation(const unsigned int lag);
	int GetMaxUsableIndex();

	static const double threshold;
};

