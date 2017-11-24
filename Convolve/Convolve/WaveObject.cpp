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


vector<char> WaveObject::getAudioDataFromFile(const char * filePath)
{
	vector<char> data;

	if (!associateFile(filePath))
		return data;

	if (!getWaveDataFromFile())
		;
		//		return data;

	if (formatChunk.bitsPerSample == 16)
	{
		return vector<char>(dataChunkTwoByte.data, dataChunkTwoByte.data + dataChunkTwoByte.audioDataSize);
	}
	else
		cout << "Only support for data formatted at 16 bits per sample.\n";

	return data;
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
	in.open(pathToWaveFile, fstream::in);

	if (!in.is_open())
	{
		cout << "Failed to open file to fetch riff chunk.\n";
		in.close();
		return false;
	}

	noskipws(in);

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
	in.open(pathToWaveFile, fstream::in);

	if (!in.is_open())
	{
		cout << "Failed to open file to fetch format chunk.\n";
		in.close();
		return false;
	}

	noskipws(in);

	in.seekg(12, in.beg);

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
	in.open(pathToWaveFile, fstream::in);

	if (!in.is_open())
	{
		cout << "Failed to open file to fetch data chunk.\n";
		in.close();
		return false;
	}

	noskipws(in);

	int start = 20 + formatChunk.fmtChunkSize;

	in.seekg(start, in.beg);

	in.read(dataChunkTwoByte.ID, 4);
	dataChunkTwoByte.ID[4] = '\0';

	dataChunkTwoByte.audioDataSize = getIntLittleEndian();

	// dataChunk.size is size of audio data in bytes
	size_t count = dataChunkTwoByte.audioDataSize;
	dataChunkTwoByte.dataShorts.reserve (count/2);

	dataChunkTwoByte.data = new unsigned char[dataChunkTwoByte.audioDataSize];

	in.read((char*)dataChunkTwoByte.data, dataChunkTwoByte.audioDataSize);

	in.close();
	in.open(pathToWaveFile, fstream::in);

	in.seekg(start + 8, in.beg);

	unsigned int i = 0;
	while ( (i < count) )
	{
		int val = getShortLittleEndian();
		dataChunkTwoByte.dataShorts.push_back(val);
		i++;

		if (i == 42988)
			cout << "here\n";
	}

	checkStreamStatus();

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

	success = getRiffChunkFromFile();
	success = getFormatChunkFromFile();
	success = getDataChunkFromFile();

	return success;
}


bool WaveObject::fileIsRiff()
{
	in.open(pathToWaveFile, fstream::in);

	if (!in.is_open())
	{
		cout << "Failed to open file.\n";
		in.close();
		return false;
	}

	noskipws(in);

	char expected[] = "RIFF";

	// Read 4 elements, 1 byte in size, from f and store in riffChunk.ID
	in.read (riffChunk.ID, 4);
	riffChunk.ID[4] = '\0';

	in.close();

	return 	(strcmp(riffChunk.ID, expected) == 0);
}


bool WaveObject::fileIsWave()
{
	in.open(pathToWaveFile, fstream::in);

	if (!in.is_open())
	{
		cout << "Failed to open file.\n";
		in.close();
		return false;
	}

	noskipws(in);

	char expected[] = "WAVE";

	in.seekg(8, in.beg);

	in.read (riffChunk.format, 4);
	riffChunk.format[4] = '\0';

	in.close();

	return 	(strcmp(riffChunk.format, expected) == 0);
}


bool WaveObject::fileIsPCM()
{
	in.open(pathToWaveFile, fstream::in);

	if (!in.is_open())
	{
		cout << "Failed to open file.\n";
		in.close();
		return false;
	}

	noskipws(in);

	in.seekg(20, in.beg);

	formatChunk.format = getShortLittleEndian();

	in.close();

	return formatChunk.format == 1;
}


// This function advances the file stream position indicator by 4 bytes
int WaveObject::getIntBigEndian()
{
	char array[4];
	unsigned int x = 0;

	if (in.eof())
	{
		cout << "EOF bit set.\n";
		in.clear();
	}

	if (in.fail())
	{
		cout << "Fail bit set.\n";
		in.clear();
	}

	noskipws(in);

	in.read (array, 4);

	unsigned char uArray[4];
	uArray[0] = array[0];
	uArray[1] = array[1];
	uArray[2] = array[2];
	uArray[3] = array[3];

	x = (uArray[3] << 0) + (uArray[2] << 8) + (uArray[1] << 16) + (uArray[0] << 24);

	return x;
}



// This function advances the file stream position indicator by 4 bytes
int WaveObject::getIntLittleEndian()
{
	char array[4];
	unsigned int x = 0;

	if (in.eof())
	{
		cout << "EOF bit set.\n";
		in.clear();
	}

	if (in.fail())
	{
		cout << "Fail bit set.\n";
		in.clear();
	}

	noskipws(in);

//	in.read (array, 4);

	char c;
	unsigned char uArray[4];

	in.get(c);
	uArray[0] = c;
	in.get(c);
	uArray[1] = c;
	in.get(c);
	uArray[2] = c;
	in.get(c);
	uArray[3] = c;

	//uArray[0] = array[0];
	//uArray[1] = array[1];
	//uArray[2] = array[2];
	//uArray[3] = array[3];

	x = (uArray[0] << 0) + (uArray[1] << 8) + (uArray[2] << 16) + (uArray[3] << 24);

	return x;
}



// This function advances the file stream position indicator by 2 bytes
short WaveObject::getShortBigEndian()
{
	char array[2];
	unsigned short x = 0;

	if (in.eof())
	{
		cout << "EOF bit set.\n";
		in.clear();
	}

	if (in.fail())
	{
		cout << "Fail bit set.\n";
		in.clear();
	}

	noskipws(in);

	in.read (array, 2);

	unsigned char uArray[2];
	uArray[0] = array[0];
	uArray[1] = array[1];

	x = (uArray[1] << 0) + (uArray[0] << 8);

	return x;
}



// This function advances the file stream position indicator by 2 bytes
short WaveObject::getShortLittleEndian()
{
	char array[2];
	unsigned short x = 0;

	if (!in.is_open())
	{
		cout << "Failed to open file.\n";
		in.close();
		return false;
	}

	checkStreamStatus();

	cout << "Position in file: " << in.tellg() << endl;

//	in.read (array, 2);

	noskipws(in);

	char c;
	unsigned char uArray[2];

	if (in.peek() != 0)
		cout << in.peek();
	in.get(c);
	uArray[0] = c;
	checkStreamStatus();
	if (in.peek() != 0)
		cout << in.peek();
	in.get(c);
	uArray[1] = c;
	checkStreamStatus();
	cout << endl;
	//uArray[0] = array[0];
	//uArray[1] = array[1];

	x = (uArray[0] << 0) + (uArray[1] << 8);

	return x;
}



void WaveObject::checkStreamStatus()
{
	if (in.eof())
	{
		cout << "EOF bit set.\n";
	}

	if (in.fail())
	{
		cout << "Fail bit set.\n";
	}

	if (in.bad())
	{
		cout << "Bad bit set.\n";
	}
}

