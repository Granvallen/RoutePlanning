#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include "RoadGraph.hpp"

using namespace std;

class Scheduler
{
public:
	Scheduler(string cartxt, string roadtxt, string crosstxt): time(0), isDone(false) 
	{
		initScheduler(cartxt, roadtxt, crosstxt);
	}

	long time; // ����ʱ��
	bool isDone; //�Ƿ�������
	void step(); // ��һ��ʱ��Ƭ��

	CarList carwaitgo; // �ȴ���·�ĳ�
	CarList carsetoff; // ��·�ϵĳ�
	RoadList roadlist;
	CrossList crosslist;
	RoadGraph roadgraph;

private:
	void initScheduler(string cartxt, string roadtxt, string crosstxt);

	void scheduleCarwaitgo(); // ���ȵȴ���·�ĳ�

	void scheduleInit(); // ÿ�ε���ǰ׼��
	void scheduleRoad(); // ����ÿһ����·
	void scheduleLane(Road& road, Lane& lane); // ����·һ�������ϵ����г���
	bool scheduleCross(); // ��������·��
	bool scheduleCrossRoad(Cross& cross);

	void enterRoad(Car& car, Lane& lane, int lane_num, int speed); // �������·
	void leaveRoad(Car& car, Lane& lane); // ���뿪��·
	long getfirstCar(Road& road, Cross& to); // ����һ������ĳ��������ȵ��ȵĳ�
	bool tryEnterRoad(Cross& cross, Road& road, Car& car);

	void answer(Car& car);
	void showstatus();

	bool flag1; // ��¼���� ����cross ��һ��ѭ���� �Ƿ����˵��� ��������ٽ���һ�����ѭ��
	bool flag2; // ��¼���� һ��cross����· ��ѭ���� �Ƿ����˵���
	bool flag3; // ��¼����һ��cross����·��ѭ���� �Ƿ����˵��� ��������ٽ���һ�����ѭ�� ֱ��û�е���

	long carnum = 0;
	long donenum = 0;
	long carwaitgonum = 0;
	long carsetoffnum = 0;

};

void Scheduler::initScheduler(string cartxt, string roadtxt, string crosstxt)
{
	carwaitgo.initCarList(cartxt);
	roadlist.initRoadList(roadtxt);
	crosslist.initCrossList(crosstxt);
	carnum = carwaitgo.size();
	cout << "load txt done!" << endl;

	roadgraph.initGraph(crosslist, roadlist);
	cout << "initGraph done!" << endl;

	// Ԥ�ȸ�ÿ�����趨��·��
	for (auto& car : carwaitgo)
		roadgraph.findFastestRoute(crosslist, roadlist, car.second);

	cout << "init car route done!" << endl;
}

void Scheduler::showstatus()
{
	cout << "--- time: " << time << " ---" << endl;
	cout << "carwaitgo: " << carwaitgo.size() << " carsetoff: " << carsetoff.size()
		<< " done: " << carnum - carwaitgo.size() - carsetoff.size()
		<< " acc: "
		<< carwaitgo.size() - carwaitgonum << " "
		<< carsetoff.size() - carsetoffnum << " "
		<< carnum - carwaitgo.size() - carsetoff.size() - donenum << endl;
	donenum = carnum - carwaitgo.size() - carsetoff.size();
	carwaitgonum = carwaitgo.size();
	carsetoffnum = carsetoff.size();
}

void Scheduler::step()
{
	time++;
	showstatus();

	if (carwaitgo.empty() && carsetoff.empty()) // ·��û�� ͬʱ û��Ҫ��·
	{
		isDone = true;
		return;
	}
	else
	{
		scheduleInit(); // ����ǰ��ʼ��

		if (!carsetoff.empty()) // ·���г��ȴ���
		{
			scheduleRoad(); // �ȵ���һ������·
			while (scheduleCross()); // ѭ������·�� ֱ������·�ڶ�����
		}

		if (!carwaitgo.empty()) // �г��ȴ�����
			scheduleCarwaitgo();
	}
}

// ����û��·�ĳ�
void Scheduler::scheduleCarwaitgo()
{
	list<long> carid; // ��¼��������id

	for (auto& carinfo : carwaitgo) // �������еȴ���·�ĳ�
	{
		Car& car = carinfo.second;
		if (car.plan_time > time) // ��û������ʱ��
			continue;

		Road& road = roadlist[car.next_road];
		if (tryEnterRoad(crosslist[car.from_id], road, car))
			carid.push_back(car.id);
	}

	// �������� ���б�
	// �Ӵ������ĳ��б���ɾ���Ѿ������ĳ� �Լ� �������ĳ�����carsetoff
	// ֮�����������������Ϊ �����ڱ���ʱ���˵����� ��ѭ��ʱɾ���б�Ԫ�ػ������
	for (auto id : carid)
	{
		carsetoff.add(carwaitgo[id]);
		carwaitgo.remove(id);
	}
}

// ����ǰ׼��
void Scheduler::scheduleInit()
{
	for (auto& car : carsetoff)
		car.second.isScheduled = false; // ���г��� �Ƿ������� ����Ϊ false
}

void Scheduler::scheduleRoad()
{
	for (auto& roadinfo : roadlist) // �������е�·
	{
		// �ȴ���from��to�ڵ�ĳ���
		Road& road = roadinfo.second;

		vector<Lane>& lanes_from_to = road.getRoadCars(road.to_id);
		for (auto& lane : lanes_from_to) // �����г�������
			scheduleLane(road, lane);

		if (road.isDuplex)
		{
			vector<Lane>& lanes_to_from = road.getRoadCars(road.from_id);
			for (auto& lane : lanes_to_from) // �����г�������
				scheduleLane(road, lane);
		}
	}
}

void Scheduler::scheduleLane(Road& road, Lane& lane)
{
	long before = -1;
	for (auto carid : lane) // ��ÿ�������ϵĳ�����
	{
		Car& car = carsetoff[carid];
		if (car.isScheduled) // �Ѿ����ȹ��Ļ�������һ����
			continue;

		int maxspeed = min(road.max_speed, car.max_speed); // ����������ٶ�

		// �Ƿ��Ǹó����ĵ�һ�����ֿ�����
		if (before < 0) // beforeС��0 Ϊ�ó����ĵ�һ����
		{
			if (car.pos == road.length ||
				road.length - car.pos < maxspeed) // ����·�� �� �ó���һ��������·��
				before = carid;
			else
			{
				car.pos += maxspeed;
				car.isScheduled = true;
				car.isStoped = false;
				before = carid;
			}
		}
		else
		{
			Car& carbefore = carsetoff[before];
			if (carbefore.pos - car.pos - 1 >= maxspeed) // ����ǰ���㹻Զ
			{
				car.pos += maxspeed;
				car.isScheduled = true;
				car.isStoped = false;
				before = carid;
			}
			else if (!carbefore.isScheduled) // ����ǰ������Զ ǰ��û�е������
				before = carid;
			else if (carbefore.isStoped) // ����ǰ������Զ ǰ��������� ͣ��
			{
				car.isStoped = true;
				car.isScheduled = true;
				before = carid;
			}
			else
			{
				car.pos = carbefore.pos - 1;
				car.isStoped = false;
				car.isScheduled = true;
				before = carid;
			}
		}
	} // ��һ�����ϵĳ��������
}

bool Scheduler::scheduleCross()
{
	flag1 = false; // �ж����cross��ѭ���Ƿ�����˵��� ����������� ��Ҫ�ٽ�һ�δ�ѭ��
	for (auto& crossinfo : crosslist) // ��id˳�����·�� ��ѭ��
	{
		Cross& cross = crossinfo.second;

		flag1 = scheduleCrossRoad(cross) || flag1;
	}
	return flag1;
}

bool Scheduler::scheduleCrossRoad(Cross& cross)
{
	flag2 = false;
	flag3 = true;
	while (flag3) // û�е��ȷ������˳�
	{
		flag3 = false;
		// id�������·�ڵĸ���·
		for (auto& roadinfo : cross.road_id)
		{
			// ��· �� ��ͨ����·�ڷ���ĳ���
			if (roadinfo.second > 0 && roadlist[roadinfo.second].isTo(cross.id))
			{
				Road& road = roadlist[roadinfo.second];
				long carid = getfirstCar(road, cross); // ����-1 ����·û�г���·��
				
				if (carid >= 0)
				{
					Car& car = carsetoff[carid];
					int from = cross.getRoadIdx(road.id); // ����·�����
					int lane_num = car.lane_num; // �����³�ԭ����ʻ�ĳ�����
					Lane& lane = road.getRoadCars(cross.id)[lane_num];

					// ���ж����·���ǲ��Ǿ���Ŀ�ĵ� �ǵĻ����ý�����һ��·
					if (car.next_road < 0) // û����һ��· ˵����Ŀ�ĵ�
					{
						answer(car);

						leaveRoad(car, lane);
						scheduleLane(road, lane);
						carsetoff.remove(car.id);
						continue;
					}

					// ���ж� ��·�� �����
					if (car.next_turn == 0) // ֱ��
					{
						bool done = tryEnterRoad(cross, road, car);
						flag3 = done || flag3;
						if (done) // �����ɹ���·�� ���̶�ԭ��������һ�ε���
							scheduleLane(road, lane);
					}
					else if (car.next_turn == 1) // ��ת �����ұߵ�·���ȼ���ߵĳ��ǲ���ֱ��
					{
						int right = from == 0 ? 3 : from - 1;
						// �ҵ��ұ�����·���ȼ���ߵĳ�
						// �ұ��г� �� ֱ�� ��������ǰ���ĵ���
						long rightroadid = cross.road_id[right];
						if (rightroadid >= 0)
						{
							long rightcarid = getfirstCar(roadlist[rightroadid], cross);
							if (rightcarid >= 0 && carsetoff[rightcarid].next_turn == 0)
								continue;
						}

						bool done = tryEnterRoad(cross, road, car);
						flag3 = done || flag3;
						if (done) // �����ɹ���·�� ���̶�ԭ��������һ�ε���
							scheduleLane(road, lane);
					}
					else // ��ת ��Ҫ������Ƿ�ֱ�� �Լ� �����Ƿ���ת
					{
						int left = from == 3 ? 0 : from + 1;
						int front = from > 1 ? from - 2 : from + 2;

						long leftroadid = cross.road_id[left];
						if (leftroadid >= 0)
						{
							long leftcarid = getfirstCar(roadlist[leftroadid], cross);
							if (leftcarid >= 0 && carsetoff[leftcarid].next_turn == 0)
								continue;
						}

						long frontroadid = cross.road_id[front];
						if (frontroadid >= 0)
						{
							long frontcarid = getfirstCar(roadlist[frontroadid], cross);
							if (frontcarid >= 0 && carsetoff[frontcarid].next_turn == 0)
								continue;
						}

						bool done = tryEnterRoad(cross, road, car);
						flag3 = done || flag3;
						if (done) // �����ɹ���·�� ���̶�ԭ��������һ�ε���
							scheduleLane(road, lane);
					}
				}
			}
		} // ·�ڸ���·ѭ������һ��
		if (flag3) flag2 = true;
	} // whileѭ��
	return flag2;
}

// ���Խ�����һ��· �����Ƿ�ɹ�
bool Scheduler::tryEnterRoad(Cross& cross, Road& road, Car& car)
{
	// ȡ����һ��·�ĳ���
	Road& next_road = roadlist[car.next_road];
	vector<Lane>& roadcars = next_road.getRoadCars(car.next_cross);

	// �жϳ��ܲ��ܹ�·��
	int maxspeed = min(next_road.max_speed, car.max_speed);
	// �������г��� ����û�г����ܽ�ȥ
	int lane_num = 0;
	for (; lane_num < roadcars.size(); lane_num++)
	{
		Lane& lane = roadcars[lane_num];

		// ����������ĳ����ǿյ� �� ��ǰ�����㹻���ľ��� ֱ��������ٶ�
		if (lane.empty())
		{
			if (car.isSetoff) // ������Ѿ����� ��Ҫ���뿪֮ǰ��·
				leaveRoad(car, road.getRoadCars(cross.id)[car.lane_num]);
			enterRoad(car, lane, lane_num, maxspeed);
			return true;
		}
		else // �����ǿ�
		{
			Car& lastcar = carsetoff[lane.back()]; // ȡ������������һ����
			if (lastcar.pos - 1 > maxspeed)
			{
				if (car.isSetoff) // ������Ѿ����� ��Ҫ���뿪֮ǰ��·
					leaveRoad(car, road.getRoadCars(cross.id)[car.lane_num]);
				enterRoad(car, lane, lane_num, maxspeed);
				return true;
			}
			else if (!lastcar.isStoped) // �����ж�һ��ǰ����״̬ ������ʻ ����������ʻ
			{
				if (car.isSetoff)
					leaveRoad(car, road.getRoadCars(cross.id)[car.lane_num]);
				enterRoad(car, lane, lane_num, lastcar.pos - 1);
				return true;
			}
			else // ���ͣ�� �ó����϶��޷���· �ж���һ������
				continue;
		}
	}
	// ���г������ж���� �����ܽ����·
	car.isStoped = true;
	return false;
}

// ��������·��·�������ȵ��ȵĳ�(���������ڵȴ�����״̬)
// �������ͬʱ���� ����·�� �� ����·��ת�䷽��
long Scheduler::getfirstCar(Road& road, Cross& cross)
{
	vector<Lane>& lanes = road.getRoadCars(cross.id);
	long carid = -1;
	for (Lane& lane : lanes)
	{
		// ����ó���Ϊ�� �� ��ǰ��ĳ��Ѿ��������� ������һ������
		if (lane.empty() || carsetoff[lane.front()].isScheduled)
			continue;

		if (carid < 0) // �������������ĵ�һ����
		{
			carid = lane.front();
			if (carsetoff[carid].pos == road.length)
				break;
		}
		else if (carsetoff[carid].pos < carsetoff[lane.front()].pos) // ������·��
			carid = lane.front();
	}

	// �������·������ȼ��ĳ����� ���� ������· �� ·��ת�䷽��
	if (carid >= 0)
	{
		Car& car = carsetoff[carid];

		// ���³���·��
		roadgraph.findFastestRoute(crosslist, roadlist, car);

		// ���³�����ʻ����
		int from = cross.getRoadIdx(road.id);
		int to = cross.getRoadIdx(car.next_road);
		int diff = to - from;
		if (abs(diff) == 2) // ֱ��
			car.next_turn = 0;
		else if (diff == 1 || diff == -3) // ��ת
			car.next_turn = 1;
		else // ��ת
			car.next_turn = 2;
	}



	// �������·�������û�г��� �� �������ǿյ� �� ��������ǰ��ĳ����Ѿ������� �᷵��-1
	// ��֮ ����-1 ��ʾ����·û�г�Ҫ��·��
	return carid;
}

void Scheduler::enterRoad(Car& car, Lane& lane, int lane_num, int speed)
{
	// ���³�����Ϣ
	car.curr_speed = speed;
	car.pos = speed;
	car.isStoped = false;
	car.isScheduled = true;
	car.lane_num = lane_num;
	// ���µ�·��Ϣ
	lane.push_back(car.id);

	if (!car.isSetoff) // �շ���
	{
		car.isSetoff = true;
		car.real_time = time;
		car.dally_time = car.real_time - car.plan_time;
	}
}

void Scheduler::leaveRoad(Car& car, Lane& lane)
{
	lane.pop_front();
}

void Scheduler::answer(Car& car)
{
	// ������
	cout << car.id << " done! " << endl << car;
	car.showRoute();
	cout << endl;

	ofstream fout("answer.txt", ios::app);
	fout << "(" << car.id << ", " << car.real_time;
	long c1;
	long c2 = car.route_real.front();
	car.route_real.pop_front();
	while (!car.route_real.empty())
	{
		c1 = c2;
		c2 = car.route_real.front();
		fout << ", " << roadgraph.roadBetween[c1][c2];
		car.route_real.pop_front();
	}
	fout << ")" << endl;
}

#endif // SCHEDULER_HPP
