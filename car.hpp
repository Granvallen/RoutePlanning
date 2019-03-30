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
	Car() {} // Ϊʲôû��Ĭ�Ϲ��캯���ᱨ��
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

	long id; // ����id
	long from_id; // ��ʼ·��id
	long to_id; // �յ�·��id
	
	int max_speed; // �����
	int curr_speed; // ��ǰ����  ���������ĿǰΪֹ����û��ʲô���� ���ܵ�����ʵʱ·��ʱ������ο�

	// ����һ��·�ϵ����� ��һ����Ϊ4��·����Ϊ 1 2 3 4
	// ���ֵ�ڳ������·ʱ��ʼ�� �н������и���
	int pos;

	bool isSetoff; // �Ƿ��ѳ���
	long plan_time; // ԭ�ƻ�����ʱ��
	long real_time; // ʵ�ʳ���ʱ��
	long dally_time; // ���ӳٵ�ʱ��

	list<long> route_plan; // �ƻ�·�߾�����·��id �������Ѿ��߹���·�� ��Ȼ����Ҫ������ʻ��·��id
	list<long> route_real; // ʵ����ʻ����·��id
	int next_turn; // ��һ��·��ת�䷽�� 0-�� 1-�� 2-�� 3-
	long next_road; // ��һ����ʻ�ĵ�·id ��ʻ��·�ڸ���
	long next_cross; // ��һ��·��id
	int lane_num; // ����ʻ�ĳ����� ������·ʱ����

	bool isStoped; // �Ƿ�ͣ��
	bool isScheduled; // �Ƿ��ڵ�ǰʱ��Ƭ�ξ�������


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
	void add(Car& car) { carlist[car.id] = car; } // unordered_map����[]ʵ�����
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
	string line; // ��ȡ��һ����Ϣ

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