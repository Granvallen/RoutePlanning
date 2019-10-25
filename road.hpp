#ifndef ROAD_HPP
#define ROAD_HPP

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <map>

using namespace std;

typedef list<long> Lane; // 车道

class Road
{
public:
	Road() {}
	Road(long* roadinfo)
	{
		if (roadinfo)
		{
			id = roadinfo[0];
			length = static_cast<int>(roadinfo[1]);
			max_speed = static_cast<int>(roadinfo[2]);
			channel = static_cast<int>(roadinfo[3]);
			from_id = roadinfo[4];
			to_id = roadinfo[5];
			isDuplex = roadinfo[6] > 0 ? true : false;
			road_from_to.resize(channel);
			if (isDuplex)
				road_to_from.resize(channel);
		}
	}

	friend ostream & operator<<(ostream &out, Road &road);

	long id;
	int length; // 路长
	int max_speed; // 最大车速
	int channel; // 车道数
	long from_id; // 起始路口id
	long to_id; // 终点路口id
	bool isDuplex; // 是否双向
	
	bool isBlock; // 是否堵路

	vector<Lane>& getRoadCars(long to); // 返回一个方向的所有车道
	int getCarsNum(long crossid);
	bool isTo(long crossid) { return crossid == to_id || (crossid == from_id && isDuplex); }

private:
	vector<Lane> road_from_to; // 记录从from节点到to节点 各个channel行驶车的队列 存车的id
	vector<Lane> road_to_from; // 如果是单行线的话这个vector是空的
};

vector<Lane>& Road::getRoadCars(long to)
{
	if (to == to_id)
		return road_from_to;
	else if (to == from_id)
		return road_to_from;
	else
		cout << "error! Road::getRoadCars()" << endl;
}

int Road::getCarsNum(long crossid)
{
	int n = 0;
	if (to_id == crossid)
		for (auto lane : road_from_to)
			n += lane.size();
	else
		for (auto lane : road_from_to)
			n += lane.size();
	return n;
}

ostream & operator<<(ostream &out, Road &road)
{
	out << "road id: " << road.id << endl
		<< "length: " << road.length << endl
		<< "speed: " << road.max_speed << endl
		<< "channel: " << road.channel << endl
		<< "from: " << road.from_id << endl
		<< "to: " << road.to_id << endl
		<< "isDuplex: " << road.isDuplex << endl;
	return out;
}


class RoadList
{
public:
	RoadList() {}
	RoadList(string roadfile) { initRoadList(roadfile); }

	void initRoadList(string roadfile);
	Road & operator[](long i) { return roadlist[i]; }
	void add(Road& road) { roadlist[road.id] = road; }
	void remove(long id) { roadlist.erase(id); }
	const map<long, Road>& getList() { return roadlist; }

	map<long, Road>::iterator begin() { return roadlist.begin(); }
	map<long, Road>::iterator end() { return roadlist.end(); }

private:

	map<long, Road> roadlist;
};

void RoadList::initRoadList(string roadfile)
{
	const int N = 7;
	ifstream road_in(roadfile);
	string line; // 读取的一行信息

	if (road_in)
		while (getline(road_in, line))
		{
			if (line[0] == '#')
				continue;

			line = line.substr(1, line.length() - 2);

			istringstream iss(line);
			string s;
			long roadinfo[N];
			for (int i = 0; i < N && getline(iss, s, ','); i++)
			{
				roadinfo[i] = atol(s.c_str());
			}
			roadlist[roadinfo[0]] = Road(roadinfo);
		}
	else
		cout << "the road.txt open failed." << endl;
}



#endif