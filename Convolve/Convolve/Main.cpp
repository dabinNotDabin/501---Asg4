#include <stdio.h>
#include "waveObject.h"

using namespace std;





int main(int argc, const char ** argv)
{
	if (argc != 2)
	{
		cout << "Usage: <inputFile> <impulseResponseFile> <outputFile>\n";
		return 0;
	}

	WaveObject w;

	w.getAudioDataFromFile(argv[1]);

	int x;
	cin >> x;

	return 0;
}