#include <iostream>
using namespace std;

int main()
{
	FILE *fos = fopen("F:/data/test.txt","a");
	if (fos)
	{
		fprintf(fos, "%f", 1.2);
		fclose(fos);
	}

	return 0;
}