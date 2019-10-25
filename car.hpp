#ifndef CAR_HPP
#define CAR_HPP

#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string> // for getline
#include <unordered_map>


using namespace std;

class Car
{
public:
	Car() {} // 为什么没有默认构造函数会报错
	Car(long id, long from, long to, int speed, long planTime):
		id(id), from_id(from), to_id(to), max_speed(speed), 
		plan_time(planTime), next_cross(from), isSetoff(false), isStoped(false) {}
	Car(long* carinfo)
	{
		if (carinfo)
		{
			id = carinfo[0];
			from_id = next_cross = carinfo[1];
			to_id = carinfo[2];
			max_speed = static_cast<int>(carinfo[3]);
			plan_time = carinfo[4];
			isSetoff = isStoped = false;
		}
	}

	void showRoute();

	friend ostream & operator<<(ostream &out, Car &car);

	long id; // 车的id
	long from_id; // 起始路口id
	long to_id; // 终点路口id
	
	int max_speed; // 最大车速
	int curr_speed; // 当前车速  这个变量到目前为止好像没起什么作用 可能到考虑实时路况时才能起参考

	// 车在一条路上的坐标 如一条长为4的路坐标为 1 2 3 4
	// 这个值在车进入道路时初始化 行进过程中更新
	int pos;

	bool isSetoff; // 是否已出发
	long plan_time; // 原计划出发时间
	long real_time; // 实际出发时间
	long dally_time; // 已延迟的时间

	list<long> route_plan; // 计划路线经过的路口id 不包括已经走过的路口 虽然答案里要求是行驶道路的id
	list<long> route_real; // 实际行驶经过路口id
	int next_turn; // 下一个路口转弯方向 0-上 1-右 2-下 3-
	long next_road; // 下一条行驶的道路id 行驶到路口更新
	long next_cross; // 下一个路口id
	int lane_num; // 所行驶的车道号 车在上路时更新

	bool isStoped; // 是否停车
	bool isScheduled; // 是否在当前时间片段经过调度


private:

};

void Car::showRoute()
{
	if (!route_real.empty())
	{
		auto iter = route_real.begin();
		cout << "car " << id << " route: ";
		cout << *iter;
		for (iter++; iter != route_real.end(); iter++)
			cout << " -> " << *iter;
		cout << endl;
	}
}




ostream & operator<<(ostream &out, Car &car)
{
	out << "car id: " << car.id << "\n"
		<< "from: " << car.from_id << "\n"
		<< "to: " << car.to_id << "\n"
		<< "speed: " << car.max_speed << "\n"
		<< "planTime: " << car.plan_time << endl;
	return out;
}


class CarList
{
public:
	CarList() {}
	CarList(string carfile) { initCarList(carfile); }

	void initCarList(string carfile);
	Car& operator[](long i) { return carlist[i]; }
	void add(Car& car) { carlist[car.id] = car; } // unordered_map这里[]实现深复制
	void remove(long id) { carlist.erase(id); }
	const unordered_map<long, Car>& getList() { return carlist; }
	bool empty() { return carlist.empty(); }
	long size() { return carlist.size(); }

	unordered_map<long, Car>::iterator begin() { return carlist.begin(); }
	unordered_map<long, Car>::iterator end() { return carlist.end(); }

private:


	unordered_map<long, Car> carlist;
};

void CarList::initCarList(string carfile)
{
	const int N = 5;
	ifstream car_in(carfile);
	string line; // 读取的一行信息

	if (car_in)
		while (getline(car_in, line))
		{
			if (line[0] == '#')
				continue;

			line = line.substr(1, line.length() - 2);

			istringstream iss(line);
			string s;
			long carinfo[N];
			for (int i = 0; i < N && getline(iss, s, ','); i++)
				carinfo[i] = atol(s.c_str());

			carlist[carinfo[0]] = Car(carinfo);
		}
	else
		cout << "the car.txt open failed." << endl;
}






#endif