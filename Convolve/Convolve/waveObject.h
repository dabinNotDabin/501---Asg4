#pragma once
#include <stdio.h>
#include <iostream>
#include <vector>
#include <fstream>


using namespace std;




// Note that all char[] are stored with a null terminator hence the length of 5.

typedef struct
{
	char			ID[5];
	unsigned int	riffChunkSize;
	char			format[5];

} RiffChunk;


typedef struct
{
	char			ID[5];
	unsigned int	fmtChunkSize;
	short			format;
	short			numChannels;
	int				sampleRate;
	unsigned int	byteRate;
	short			blockAlign;
	short			bitsPerSample;

} FormatChunk;


typedef struct
{
	char			ID[5];
	unsigned int	audioDataSize;
	unsigned char * data;
	vector<short>	dataShorts;

} DataChunkTwoByte;


typedef struct
{
	char			ID[5];
	unsigned int	audioDataSize;
	int *			data;
	vector<int>		dataInts;

} DataChunkFourByte;



class WaveObject
{
private:
	RiffChunk riffChunk{ -1, -1 };
	FormatChunk formatChunk{ -1, -1 };
	DataChunkTwoByte dataChunkTwoByte{ -1, -1 };
	DataChunkFourByte dataChunkFourByte{ -1, -1 };

	
	const char * pathToWaveFile;
	ifstream in;
	ofstream out;



	bool getRiffChunkFromFile();
	bool getFormatChunkFromFile();
	bool getDataChunkFromFile();

	bool getWaveDataFromFile();

	bool fileIsRiff();
	bool fileIsWave();
	bool fileIsPCM();

	bool associateFile(const char * filePath);


	int getIntLittleEndian();
	int getIntBigEndian();

	short getShortLittleEndian();
	short getShortBigEndian();

	void checkStreamStatus();

public:

	WaveObject();
	~WaveObject();


	vector<char> getAudioDataFromFile(const char * filePath);

};