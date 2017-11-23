#include "waveObject.h"
#include <windows.h>




WaveObject::WaveObject()
{
	pathToWaveFile = NULL;
	f = NULL;
}

WaveObject::~WaveObject()
{
	fclose(f);
}


bool WaveObject::associateFile(const char * filePath)
{
	pathToWaveFile = filePath;
	f = fopen(filePath, "r");

	cout << "File path: " << filePath << endl;

	if (f == NULL)
	{
		cout << "Open file FAILED.\n";
		return false;
	}

	if (!fileIsRiff())
		cout << "Not a RIFF file!\n";

	if (!fileIsWave())
		cout << "Not a WAVE file!\n";

	if (!fileIsPCM())
		cout << "Not in PCM format!\n";

	if (fclose(f) != 0)
	{
		cout << "Error closing file.\n";
		return false;
	}

	return true;
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




bool WaveObject::getRiffChunkFromFile()
{
	if (f == NULL)
	{
		cout << "Failed to open file to fetch riff chunk.\n";
		return false;
	}

	fseek(f, 0, SEEK_SET);

	if (feof(f))
	{
		cout << "EOF bit set.\n";
		clearerr(f);
	}

	if (ferror(f))
	{
		cout << "Error bit set.\n";
		clearerr(f);
	}

	int read = fread((void*)riffChunk.ID, 1, 4, f);
	if (read != 4)
		cout << "Bytes not read!\n";

	riffChunk.ID[4] = '\0';

	riffChunk.riffChunkSize = getIntLittleEndian();
	
	read = fread((void*)riffChunk.format, 1, 4, f);
	if (read != 4)
		cout << "Bytes not read!\n";

	riffChunk.format[4] = '\0';

	fseek(f, 0, SEEK_SET);

	return true;
}



bool WaveObject::getFormatChunkFromFile()
{
	if (f == NULL)
	{
		cout << "Failed to open file to fetch format chunk.\n";
		return false;
	}

	fseek(f, 12, SEEK_SET);

	if (feof(f))
	{
		cout << "EOF bit set.\n";
		clearerr(f);
	}

	if (ferror(f))
	{
		cout << "Error bit set.\n";
		clearerr(f);
	}

	int read = fread((void*)formatChunk.ID, 1, 4, f);

	if (read != 4)
		cout << "Bytes not read!\n";

	formatChunk.ID[4] = '\0';

	formatChunk.fmtChunkSize = getIntLittleEndian();
	formatChunk.format = getShortLittleEndian();
	formatChunk.numChannels = getShortLittleEndian();
	formatChunk.sampleRate = getIntLittleEndian();
	formatChunk.byteRate = getIntLittleEndian();
	formatChunk.blockAlign = getShortLittleEndian();
	formatChunk.bitsPerSample = getShortLittleEndian();

	fseek(f, 0, SEEK_SET);

	return true;
}



bool WaveObject::getDataChunkFromFile()
{
	if (f == NULL)
	{
		cout << "Failed to open file to fetch data chunk.\n";
		return false;
	}

	int start = 20 + formatChunk.fmtChunkSize;

	fseek(f, start, SEEK_SET);

	if (feof(f))
	{
		cout << "EOF bit set.\n";
		clearerr(f);
	}

	if (ferror(f))
	{
		cout << "Error bit set.\n";
		clearerr(f);
	}

	int read = fread((void*)dataChunkTwoByte.ID, 1, 4, f);

	if (read != 4)
		cout << "Bytes not read!\n";

	dataChunkTwoByte.ID[4] = '\0';

	dataChunkTwoByte.audioDataSize = getIntLittleEndian();

	// dataChunk.size is size of audio data in bytes
	size_t count = dataChunkTwoByte.audioDataSize;
	dataChunkTwoByte.dataShorts.reserve (count/2);

	clearerr(f);

//	dataChunkTwoByte.dataLarge = (unsigned char*)malloc(count);

//	fread((void*)dataChunkTwoByte.dataLarge, 1, count, f);

	int i = 0;
	while ( (i < dataChunkTwoByte.audioDataSize) )
	{
		int val = getShortLittleEndian();
		dataChunkTwoByte.dataShorts.push_back(val);
		i++;
	}

	fseek(f, 0, SEEK_SET);

	return true;
}



bool WaveObject::getWaveDataFromFile()
{
	bool success;

	f = fopen(pathToWaveFile, "r");

	if (f == NULL)
	{
		cout << "Failed to open file to get wave data from file.\n";
		return false;
	}

	success = getRiffChunkFromFile();
	success = getFormatChunkFromFile();
	success = getDataChunkFromFile();

	fclose(f);

	return success;
}


bool WaveObject::fileIsRiff()
{
	if (f == NULL)
		return false;

	char expected[] = "RIFF";

	fseek(f, 0, SEEK_SET);

	if (feof(f))
	{
		cout << "EOF bit set.\n";
		clearerr(f);
	}
	
	if (ferror(f))
	{
		cout << "Error bit set.\n";
		clearerr(f);
	}

	// Read 4 elements, 1 byte in size, from f and store in riffChunk.ID
	int read = fread((void*)riffChunk.ID, 1, 4, f);

	if (read != 4)
		cout << "Bytes not read!\n";

	riffChunk.ID[4] = '\0';

	fseek(f, 0, SEEK_SET);

	return 	(strcmp(riffChunk.ID, expected) == 0);
}


bool WaveObject::fileIsWave()
{
	if (f == NULL)
		return false;

	char expected[] = "WAVE";

	fseek(f, 8, SEEK_SET);

	if (feof(f))
	{
		cout << "EOF bit set.\n";
		clearerr(f);
	}

	if (ferror(f))
	{
		cout << "Error bit set.\n";
		clearerr(f);
	}

	// Read 4 elements, 1 byte in size, from f and store in riffChunk.Format
	int read = fread((void*)riffChunk.format, 1, 4, f);

	if (read != 4)
		cout << "Bytes not read!\n";

	riffChunk.format[4] = '\0';

	fseek(f, 0, SEEK_SET);

	return 	(strcmp(riffChunk.format, expected) == 0);
}


bool WaveObject::fileIsPCM()
{
	if (f == NULL)
		return false;

	fseek(f, 20, SEEK_SET);

	formatChunk.format = getShortLittleEndian();

	fseek(f, 0, SEEK_SET);

	return formatChunk.format == 1;
}


// This function advances the file stream position indicator by 4 bytes
int WaveObject::getIntBigEndian()
{
	unsigned char array[4];
	int x = 0;

	if (feof(f))
	{
		cout << "EOF bit set.\n";
		clearerr(f);
	}

	if (ferror(f))
	{
		cout << "Error bit set.\n";
		clearerr(f);
	}

	int read = fread((void*)array, 1, 4, f);

	if (read != 4)
		cout << "Bytes not read!\n";

	x = (array[3] << 0) + (array[2] << 8) + (array[1] << 16) + (array[0] << 24);

	return x;
}



// This function advances the file stream position indicator by 4 bytes
int WaveObject::getIntLittleEndian()
{
	unsigned char array[4];
	int x = 0;

	if (feof(f))
	{
		cout << "EOF bit set.\n";
		clearerr(f);
	}

	if (ferror(f))
	{
		cout << "Error bit set.\n";
		clearerr(f);
	}

//	int read = fread((void*)array, 1, 4, f);

	array[0] = fgetc(f);
	if (ferror(f))
		cout << "Error bit set.\n";
	if (feof(f))
		cout << "EOF bit set.\n";
	array[1] = fgetc(f);
	if (ferror(f))
		cout << "Error bit set.\n";
	if (feof(f))
		cout << "EOF bit set.\n";
	array[2] = fgetc(f);
	if (ferror(f))
		cout << "Error bit set.\n";
	if (feof(f))
		cout << "EOF bit set.\n";
	array[3] = fgetc(f);
	if (ferror(f))
		cout << "Error bit set.\n";
	if (feof(f))
		cout << "EOF bit set.\n";

	if (array[0] == EOF)
		cout << "fgetc returned EOF.\n";
	else if (array[1] == EOF)
		cout << "fgetc returned EOF.\n";
	if (array[2] == EOF)
		cout << "fgetc returned EOF.\n";
	if (array[3] == EOF)
		cout << "fgetc returned EOF.\n";



//	if (read != 4)
//		cout << "Bytes not read!\n";

	x = (array[0] << 0) + (array[1] << 8) + (array[2] << 16) + (array[3] << 24);

	return x;
}



// This function advances the file stream position indicator by 2 bytes
short WaveObject::getShortBigEndian()
{
	unsigned char array[2];
	short x = 0;

	if (feof(f))
	{
		cout << "EOF bit set.\n";
		clearerr(f);
	}

	if (ferror(f))
	{
		cout << "Error bit set.\n";
		clearerr(f);
	}

	int read = fread((void*)array, 1, 2, f);

	if (read != 2)
		cout << "Bytes not read!\n";

	x = (array[1] << 0) + (array[0] << 8);

	return x;
}



// This function advances the file stream position indicator by 2 bytes
short WaveObject::getShortLittleEndian()
{
	unsigned char array[2];
	short x = 0;

	if (feof(f))
	{
		cout << "EOF bit set.\n";
		clearerr(f);
	}

	if (ferror(f))
	{
		cout << "Error bit set.\n";
		clearerr(f);
	}

	int read = fread((void*)array, 1, 2, f);

	if(read != 2)
		cout << "Bytes not read!\n";


	x = (array[0] << 0) + (array[1] << 8);

	return x;
}
