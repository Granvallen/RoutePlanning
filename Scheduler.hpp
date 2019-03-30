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

	long time; // 调度时间
	bool isDone; //是否调度完成
	void step(); // 走一个时间片段

	CarList carwaitgo; // 等待上路的车
	CarList carsetoff; // 在路上的车
	RoadList roadlist;
	CrossList crosslist;
	RoadGraph roadgraph;

private:
	void initScheduler(string cartxt, string roadtxt, string crosstxt);

	void scheduleCarwaitgo(); // 调度等待上路的车

	void scheduleInit(); // 每次调度前准备
	void scheduleRoad(); // 调度每一条道路
	void scheduleLane(Road& road, Lane& lane); // 调度路一个方向上的所有车道
	bool scheduleCross(); // 调度所有路口
	bool scheduleCrossRoad(Cross& cross);

	void enterRoad(Car& car, Lane& lane, int lane_num, int speed); // 车进入道路
	void leaveRoad(Car& car, Lane& lane); // 车离开道路
	long getfirstCar(Road& road, Cross& to); // 返回一个方向的车道中最先调度的车
	bool tryEnterRoad(Cross& cross, Road& road, Car& car);

	void answer(Car& car);
	void showstatus();

	bool flag1; // 记录遍历 所有cross 的一个循环中 是否发生了调度 如果调度再进行一次这个循环
	bool flag2; // 记录遍历 一个cross所有路 的循环中 是否发生了调度
	bool flag3; // 记录遍历一个cross所有路的循环中 是否发生了调度 如果调度再进行一次这个循环 直到没有调度

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

	// 预先给每辆车设定好路线
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

	if (carwaitgo.empty() && carsetoff.empty()) // 路上没车 同时 没车要上路
	{
		isDone = true;
		return;
	}
	else
	{
		scheduleInit(); // 调度前初始化

		if (!carsetoff.empty()) // 路上有车先处理
		{
			scheduleRoad(); // 先调度一遍所有路
			while (scheduleCross()); // 循环调度路口 直到所有路口都不变
		}

		if (!carwaitgo.empty()) // 有车等待出发
			scheduleCarwaitgo();
	}
}

// 调度没上路的车
void Scheduler::scheduleCarwaitgo()
{
	list<long> carid; // 记录出发车的id

	for (auto& carinfo : carwaitgo) // 遍历所有等待上路的车
	{
		Car& car = carinfo.second;
		if (car.plan_time > time) // 若没到出发时间
			continue;

		Road& road = roadlist[car.next_road];
		if (tryEnterRoad(crosslist[car.from_id], road, car))
			carid.push_back(car.id);
	}

	// 更新两个 车列表
	// 从待出发的车列表中删除已经出发的车 以及 将出发的车加入carsetoff
	// 之所以在这里更新是因为 上面在遍历时用了迭代器 在循环时删除列表元素会出问题
	for (auto id : carid)
	{
		carsetoff.add(carwaitgo[id]);
		carwaitgo.remove(id);
	}
}

// 调度前准备
void Scheduler::scheduleInit()
{
	for (auto& car : carsetoff)
		car.second.isScheduled = false; // 所有车的 是否调度完毕 设置为 false
}

void Scheduler::scheduleRoad()
{
	for (auto& roadinfo : roadlist) // 遍历所有道路
	{
		// 先处理from到to节点的车道
		Road& road = roadinfo.second;

		vector<Lane>& lanes_from_to = road.getRoadCars(road.to_id);
		for (auto& lane : lanes_from_to) // 对所有车道处理
			scheduleLane(road, lane);

		if (road.isDuplex)
		{
			vector<Lane>& lanes_to_from = road.getRoadCars(road.from_id);
			for (auto& lane : lanes_to_from) // 对所有车道处理
				scheduleLane(road, lane);
		}
	}
}

void Scheduler::scheduleLane(Road& road, Lane& lane)
{
	long before = -1;
	for (auto carid : lane) // 对每个车道上的车处理
	{
		Car& car = carsetoff[carid];
		if (car.isScheduled) // 已经调度过的话处理下一辆车
			continue;

		int maxspeed = min(road.max_speed, car.max_speed); // 车理论最大速度

		// 是否是该车道的第一辆车分开考虑
		if (before < 0) // before小于0 为该车道的第一辆车
		{
			if (car.pos == road.length ||
				road.length - car.pos < maxspeed) // 车在路口 或 该车下一步将穿过路口
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
			if (carbefore.pos - car.pos - 1 >= maxspeed) // 距离前车足够远
			{
				car.pos += maxspeed;
				car.isScheduled = true;
				car.isStoped = false;
				before = carid;
			}
			else if (!carbefore.isScheduled) // 距离前车不够远 前车没有调度完毕
				before = carid;
			else if (carbefore.isStoped) // 距离前车不够远 前车调度完毕 停了
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
	} // 对一车道上的车处理完毕
}

bool Scheduler::scheduleCross()
{
	flag1 = false; // 判断这个cross大循环是否进行了调度 如果经过调度 需要再进一次大循环
	for (auto& crossinfo : crosslist) // 按id顺序调度路口 大循环
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
	while (flag3) // 没有调度发生才退出
	{
		flag3 = false;
		// id升序调度路口的各条路
		for (auto& roadinfo : cross.road_id)
		{
			// 有路 且 有通往该路口方向的车道
			if (roadinfo.second > 0 && roadlist[roadinfo.second].isTo(cross.id))
			{
				Road& road = roadlist[roadinfo.second];
				long carid = getfirstCar(road, cross); // 返回-1 这条路没有车过路口
				
				if (carid >= 0)
				{
					Car& car = carsetoff[carid];
					int from = cross.getRoadIdx(road.id); // 这条路的序号
					int lane_num = car.lane_num; // 保存下车原来行驶的车道号
					Lane& lane = road.getRoadCars(cross.id)[lane_num];

					// 先判断这个路口是不是就是目的地 是的话不用进入下一条路
					if (car.next_road < 0) // 没有下一条路 说明到目的地
					{
						answer(car);

						leaveRoad(car, lane);
						scheduleLane(road, lane);
						carsetoff.remove(car.id);
						continue;
					}

					// 再判断 过路口 的情况
					if (car.next_turn == 0) // 直行
					{
						bool done = tryEnterRoad(cross, road, car);
						flag3 = done || flag3;
						if (done) // 车若成功过路口 立刻对原车道进行一次调度
							scheduleLane(road, lane);
					}
					else if (car.next_turn == 1) // 左转 考虑右边道路优先级最高的车是不是直行
					{
						int right = from == 0 ? 3 : from - 1;
						// 找到右边这条路优先级最高的车
						// 右边有车 且 直行 先跳过当前车的调度
						long rightroadid = cross.road_id[right];
						if (rightroadid >= 0)
						{
							long rightcarid = getfirstCar(roadlist[rightroadid], cross);
							if (rightcarid >= 0 && carsetoff[rightcarid].next_turn == 0)
								continue;
						}

						bool done = tryEnterRoad(cross, road, car);
						flag3 = done || flag3;
						if (done) // 车若成功过路口 立刻对原车道进行一次调度
							scheduleLane(road, lane);
					}
					else // 右转 需要看左边是否直行 以及 对面是否左转
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
						if (done) // 车若成功过路口 立刻对原车道进行一次调度
							scheduleLane(road, lane);
					}
				}
			}
		} // 路口各条路循环处理一遍
		if (flag3) flag2 = true;
	} // while循环
	return flag2;
}

// 尝试进入下一条路 返回是否成功
bool Scheduler::tryEnterRoad(Cross& cross, Road& road, Car& car)
{
	// 取出下一条路的车道
	Road& next_road = roadlist[car.next_road];
	vector<Lane>& roadcars = next_road.getRoadCars(car.next_cross);

	// 判断车能不能过路口
	int maxspeed = min(next_road.max_speed, car.max_speed);
	// 遍历所有车道 看有没有车道能进去
	int lane_num = 0;
	for (; lane_num < roadcars.size(); lane_num++)
	{
		Lane& lane = roadcars[lane_num];

		// 如果遍历到的车道是空的 或 与前车有足够长的距离 直接以最大速度
		if (lane.empty())
		{
			if (car.isSetoff) // 如果车已经出发 需要先离开之前的路
				leaveRoad(car, road.getRoadCars(cross.id)[car.lane_num]);
			enterRoad(car, lane, lane_num, maxspeed);
			return true;
		}
		else // 车道非空
		{
			Car& lastcar = carsetoff[lane.back()]; // 取出这个车道最后一辆车
			if (lastcar.pos - 1 > maxspeed)
			{
				if (car.isSetoff) // 如果车已经出发 需要先离开之前的路
					leaveRoad(car, road.getRoadCars(cross.id)[car.lane_num]);
				enterRoad(car, lane, lane_num, maxspeed);
				return true;
			}
			else if (!lastcar.isStoped) // 否则判断一下前车的状态 如在行驶 则跟在其后行驶
			{
				if (car.isSetoff)
					leaveRoad(car, road.getRoadCars(cross.id)[car.lane_num]);
				enterRoad(car, lane, lane_num, lastcar.pos - 1);
				return true;
			}
			else // 如果停了 该车道肯定无法上路 判断下一个车道
				continue;
		}
	}
	// 所有车道都判断完毕 都不能进入道路
	car.isStoped = true;
	return false;
}

// 返回这条路过路口最优先调度的车(还必须是在等待调度状态)
// 这个函数同时更新 车的路线 和 车的路口转弯方向
long Scheduler::getfirstCar(Road& road, Cross& cross)
{
	vector<Lane>& lanes = road.getRoadCars(cross.id);
	long carid = -1;
	for (Lane& lane : lanes)
	{
		// 如果该车道为空 或 最前面的车已经经过调度 处理下一个车道
		if (lane.empty() || carsetoff[lane.front()].isScheduled)
			continue;

		if (carid < 0) // 处理满足条件的第一辆车
		{
			carid = lane.front();
			if (carsetoff[carid].pos == road.length)
				break;
		}
		else if (carsetoff[carid].pos < carsetoff[lane.front()].pos) // 更靠近路口
			carid = lane.front();
	}

	// 如果这条路最高优先级的车存在 更新 车的线路 与 路口转弯方向
	if (carid >= 0)
	{
		Car& car = carsetoff[carid];

		// 更新车的路线
		roadgraph.findFastestRoute(crosslist, roadlist, car);

		// 更新车的行驶方向
		int from = cross.getRoadIdx(road.id);
		int to = cross.getRoadIdx(car.next_road);
		int diff = to - from;
		if (abs(diff) == 2) // 直行
			car.next_turn = 0;
		else if (diff == 1 || diff == -3) // 左转
			car.next_turn = 1;
		else // 右转
			car.next_turn = 2;
	}



	// 如果这条路这个方向没有车道 或 车道都是空的 或 各车道最前面的车都已经过调度 会返回-1
	// 总之 返回-1 表示这条路没有车要过路口
	return carid;
}

void Scheduler::enterRoad(Car& car, Lane& lane, int lane_num, int speed)
{
	// 更新车的信息
	car.curr_speed = speed;
	car.pos = speed;
	car.isStoped = false;
	car.isScheduled = true;
	car.lane_num = lane_num;
	// 更新道路信息
	lane.push_back(car.id);

	if (!car.isSetoff) // 刚发车
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
	// 输出结果
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
