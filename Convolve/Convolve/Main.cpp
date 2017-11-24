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

	vector <char> data = w.getAudioDataFromFile(argv[1]);


	for (int i = 0; i < data.size(); i++)
		cout << i << ": " << data[i] << endl;


	int x;
	cin >> x;

	return 0;
}