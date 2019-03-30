#ifndef CROSS_HPP
#define CROSS_HPP

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
//#include <unordered_map>
#include <map>

using namespace std;

class Cross
{
public:
	Cross() {}
	//Cross(long id, long road0, long road1, long road2, long road3) : id(id)
	//{
	//	road_id[road0] = 0; road_id[road1] = 1;
	//	road_id[road2] = 2; road_id[road3] = 3;
	//}
	Cross(long* crossinfo)
	{
		if (crossinfo)
		{
			id = crossinfo[0];
			// ע������û·�� -1 Ҳ��Ž�ȥ �������Ϊ�� ·idΪֵ
			road_id[0] = crossinfo[1]; road_id[1] = crossinfo[2];
			road_id[2] = crossinfo[3]; road_id[3] = crossinfo[4];
		}
	}

	friend ostream & operator<<(ostream &out, Cross &cross);

	// ��·��ŵõ�·id
	long getRoadIdx(int roadid)
	{
		for (auto& i : road_id)
			if (i.second == roadid)
				return i.first;
		return -1;
	}

	long id; // ·��id
	map<long, int> road_id; // �ĸ����ӵ�·��id û·��Ϊ-1 ��map����ʵ��·id����

	//int road_num; // ���ӵ�·��Ŀ
	bool isBlock; // �Ƿ����
private:

};

ostream & operator<<(ostream &out, Cross &cross)
{
	out << "cross id: " << cross.id << endl
		<< "road0: " << cross.road_id[0] << endl
		<< "road1: " << cross.road_id[1] << endl
		<< "road2: " << cross.road_id[2] << endl
		<< "road3: " << cross.road_id[3] << endl;
	return out;
}



class CrossList
{
public:
	CrossList() {}
	CrossList(string crossfile) { initCrossList(crossfile); }

	void initCrossList(string crossfile);
	Cross & operator[](long i) { return crosslist[i]; }
	void add(Cross& cross) { crosslist[cross.id] = cross; }
	void remove(long id) { crosslist.erase(id); }
	const map<long, Cross>& getList() { return crosslist; }

	map<long, Cross>::iterator begin() { return crosslist.begin(); }
	map<long, Cross>::iterator end() { return crosslist.end(); }

private:

	map<long, Cross> crosslist; // ����֮��crossҪ��id��С������� ����������map

};

void CrossList::initCrossList(string crossfile)
{
	const int N = 5;
	ifstream cross_in(crossfile);
	string line;

	if (cross_in)
		while (getline(cross_in, line))
		{
			if (line[0] == '#')
				continue;

			line = line.substr(1, line.length() - 2);

			istringstream iss(line);
			string s;
			long crossinfo[N];
			for (int i = 0; i < N && getline(iss, s, ','); i++)
				crossinfo[i] = atol(s.c_str());

			crosslist[crossinfo[0]] = Cross(crossinfo);
		}
	else
		cout << "the cross.txt open failed." << endl;
}





#endif
