#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
using namespace std;

_declspec(dllimport) int add(int a, int b);

string proProcessIFC(string filename)
{
	string ret;
	vector<string> contents;
	FILE *fid = fopen(filename.c_str(), "r");
	char line[1024];
	memset(line, 0, 1024);
	bool begin = false;
	while (!feof(fid))
	{
		fgets(line, 1024, fid);
		ret += line;
		contents.push_back(line);
	}
	fclose(fid);
	vector<string> results;
	for (int i = 0; i < contents.size(); i++)
	{
		string cur = contents[i];
		if (cur == "DATA;")
		{
			begin = true;
			results.push_back(cur);
			continue;
		}
		if (i >= contents.size()-1 || !begin)
		{
			results.push_back(cur);
			continue;
		}
		int nextIndex = i;
		while (cur[sizeof(cur) - 1] != ';' && ++nextIndex < contents.size())
		{
			string next = contents[nextIndex];
			if (next.size() == 0 || next.size() > 0 && next[0] != '#')
			{
				cur += next;
				i = nextIndex;
			} 
			else
			{
				break;
			}
		}
		results.push_back(cur);
	}

	string newFilename = filename.substr(0, filename.length() - 4) + "_ProDealed.ifc";
	FILE *ftarget = fopen(newFilename.c_str(), "w");
	for (int i = 0; i < results.size(); i++)
	{
		fprintf(ftarget, results[i].c_str());
	}
	fclose(ftarget);
	return ret;
}

//int main()
//{
//	cout << add(2, 3) << endl;
//	proProcessIFC("myFile.ifc");
//	system("pause");
//	return 0;
//}