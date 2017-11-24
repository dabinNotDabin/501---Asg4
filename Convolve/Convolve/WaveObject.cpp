#include "waveObject.h"
#include <windows.h>




WaveObject::WaveObject()
{
	pathToWaveFile = NULL;
}

WaveObject::~WaveObject()
{
	in.close();
	out.close();
}


vector<short> WaveObject::getAudioDataFromFile(const char * filePath)
{
	if (!associateFile(filePath))
		return vector<short>();

	if (!getWaveDataFromFile())
		return vector<short>();

	if (formatChunk.bitsPerSample == 16)
		return 	vector<short>(dataChunkTwoByte.dataShorts);
	else
		cout << "Only support for data formatted at 16 bits per sample.\n";

	return vector<short>();
}



bool WaveObject::associateFile(const char * filePath)
{
	pathToWaveFile = filePath;

	if (!fileIsRiff())
		cout << "Not a RIFF file!\n";

	if (!fileIsWave())
		cout << "Not a WAVE file!\n";

	if (!fileIsPCM())
		cout << "Not in PCM format!\n";

	return true;
}


bool WaveObject::getRiffChunkFromFile()
{
	in.open(pathToWaveFile, in.in);

	if (!in.good())
	{
		cout << "Failed to open file to fetch riff chunk.\n";
		in.close();
		return false;
	}

	in.read (riffChunk.ID, 4);
	riffChunk.ID[4] = '\0';

	riffChunk.riffChunkSize = getIntLittleEndian();
	
	in.read(riffChunk.format, 4);
	riffChunk.format[4] = '\0';

	if (!in.good())
	{
		in.close();
		return false;
	}

	in.close();
	return true;
}



bool WaveObject::getFormatChunkFromFile()
{
	in.open(pathToWaveFile, in.in);

	if (!in.good())
	{
		cout << "Failed to open file to fetch format chunk.\n";
		in.close();
		return false;
	}

	in.read(formatChunk.ID, 4);
	formatChunk.ID[4] = '\0';

	formatChunk.fmtChunkSize = getIntLittleEndian();
	formatChunk.format = getShortLittleEndian();
	formatChunk.numChannels = getShortLittleEndian();
	formatChunk.sampleRate = getIntLittleEndian();
	formatChunk.byteRate = getIntLittleEndian();
	formatChunk.blockAlign = getShortLittleEndian();
	formatChunk.bitsPerSample = getShortLittleEndian();

	if (!in.good())
	{
		in.close();
		return false;
	}

	in.close();
	return true;
}



bool WaveObject::getDataChunkFromFile()
{
	in.open(pathToWaveFile, in.in);

	if (!in.good())
	{
		cout << "Failed to open file to fetch data chunk.\n";
		in.close();
		return false;
	}

	int start = 20 + formatChunk.fmtChunkSize;

	in.read = (dataChunkTwoByte.ID, 4);
	dataChunkTwoByte.ID[4] = '\0';

	dataChunkTwoByte.audioDataSize = getIntLittleEndian();

	// dataChunk.size is size of audio data in bytes
	size_t count = dataChunkTwoByte.audioDataSize;
	dataChunkTwoByte.dataShorts.reserve (count/2);

	int i = 0;
	while ( (i < dataChunkTwoByte.audioDataSize) )
	{
		int val = getShortLittleEndian();
		dataChunkTwoByte.dataShorts.push_back(val);
		i++;
	}

	if (!in.good())
	{
		in.close();
		return false;
	}

	in.close();
	return true;
}



bool WaveObject::getWaveDataFromFile()
{
	bool success;

	in.open(pathToWaveFile, in.in);

	if (!in.good())
	{
		cout << "Failed to open file to get wave data.\n";
		in.close();
		return false;
	}


	success = getRiffChunkFromFile();
	success = getFormatChunkFromFile();
	success = getDataChunkFromFile();

	in.close();

	return success;
}


bool WaveObject::fileIsRiff()
{
	in.open(pathToWaveFile, in.in);

	if (!in.good())
	{
		cout << "Failed to open file.\n";
		in.close();
		return false;
	}

	char expected[] = "RIFF";

	// Read 4 elements, 1 byte in size, from f and store in riffChunk.ID
	in.read = (riffChunk.ID, 4);
	riffChunk.ID[4] = '\0';

	in.close();

	return 	(strcmp(riffChunk.ID, expected) == 0);
}


bool WaveObject::fileIsWave()
{
	in.open(pathToWaveFile, in.in);

	if (!in.good())
	{
		cout << "Failed to open file.\n";
		in.close();
		return false;
	}

	char expected[] = "WAVE";

	in.seekg(12, in.beg);

	in.read (riffChunk.format, 4);
	riffChunk.format[4] = '\0';

	in.close();

	return 	(strcmp(riffChunk.format, expected) == 0);
}


bool WaveObject::fileIsPCM()
{
	in.open(pathToWaveFile, in.in);

	if (!in.good())
	{
		cout << "Failed to open file.\n";
		in.close();
		return false;
	}

	in.seekg(20, in.beg);

	formatChunk.format = getShortLittleEndian();

	in.close();

	return formatChunk.format == 1;
}


// This function advances the file stream position indicator by 4 bytes
int WaveObject::getIntBigEndian()
{
	unsigned char array[4];
	int x = 0;

	if (in.eof)
	{
		cout << "EOF bit set.\n";
		in.clear();
	}

	if (in.fail)
	{
		cout << "Fail bit set.\n";
		in.clear();
	}

	in.read = (array, 4);

	x = (array[3] << 0) + (array[2] << 8) + (array[1] << 16) + (array[0] << 24);

	return x;
}



// This function advances the file stream position indicator by 4 bytes
int WaveObject::getIntLittleEndian()
{
	unsigned char array[4];
	int x = 0;

	if (in.eof)
	{
		cout << "EOF bit set.\n";
		in.clear();
	}

	if (in.fail)
	{
		cout << "Fail bit set.\n";
		in.clear();
	}

	in.read = (array, 4);

	x = (array[0] << 0) + (array[1] << 8) + (array[2] << 16) + (array[3] << 24);

	return x;
}



// This function advances the file stream position indicator by 2 bytes
short WaveObject::getShortBigEndian()
{
	unsigned char array[2];
	short x = 0;

	if (in.eof)
	{
		cout << "EOF bit set.\n";
		in.clear();
	}

	if (in.fail)
	{
		cout << "Fail bit set.\n";
		in.clear();
	}

	in.read = (array, 2);

	x = (array[1] << 0) + (array[0] << 8);

	return x;
}



// This function advances the file stream position indicator by 2 bytes
short WaveObject::getShortLittleEndian()
{
	unsigned char array[2];
	short x = 0;

	if (in.eof)
	{
		cout << "EOF bit set.\n";
		in.clear();
	}

	if (in.fail)
	{
		cout << "Fail bit set.\n";
		in.clear();
	}

	in.read = (array, 2);

	x = (array[0] << 0) + (array[1] << 8);

	return x;
}
