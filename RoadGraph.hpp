#ifndef ROADGRAPH_HPP
#define ROADGRAPH_HPP

#include <unordered_map>
#include <vector>
#include <list>

#include "cross.hpp"
#include "road.hpp"
#include "car.hpp"
#include "util.hpp"


class RoadGraph
{
public:
	RoadGraph() {}
	RoadGraph(CrossList& crosslist, RoadList& roadlist) { initGraph(crosslist, roadlist); }
	
	// 初始化路线图 neighborsOf 与 distBetween 便于之后搜索路径
	void initGraph(CrossList& crosslist, RoadList& roadlist);

	void findFastestRoute(CrossList& crosslist, RoadList& roadlist, Car& car);

	unordered_map<long, list<long> > neighborsOf; // 一个节点的相邻节点
	unordered_map<long, unordered_map<long, double> > distBetween; // 所有相邻节点的距离
	unordered_map<long, unordered_map<long, long> > roadBetween; // 所有相邻节点之间路的id

private:
	void dijkstra_search(CrossList& crosslist, RoadList& roadlist, Car& car); // dijkstra路径搜索方法

};

// 目前这个函数没有用到crosslist与roadlist的信息
void RoadGraph::findFastestRoute(CrossList& crosslist, RoadList& roadlist, Car& car)
{
	// Dijkstra算法 这里crosslist与roadlist car有道路实时信息 但这里先不用
	dijkstra_search(crosslist, roadlist, car);
}

void RoadGraph::initGraph(CrossList& crosslist, RoadList& roadlist)
{
	for (auto& cross : crosslist)
	{
		for (auto road_id : cross.second.road_id)
		{
			if (road_id.second < 0) continue;
			Road& road = roadlist[road_id.second];
			// 如果当前cross是road的from节点或者路是双向的 肯定能从这个节点到路另一个节点
			// 反过来 如果是单行线 而且 当前路口不是road的from 则不与路的另个路口相邻
			if (!road.isDuplex && road.from_id != cross.first)
				continue;

			// 判断下路另外一个路口id
			long cross_id_another = road.from_id != cross.first ? road.from_id : road.to_id;

			neighborsOf[cross.first].push_back(cross_id_another);
			distBetween[cross.first][cross_id_another] = road.length; // 注意单行路的问题 这里存的可通行方向的
			roadBetween[cross.first][cross_id_another] = road.id; // 存两路口之间道路的id
		}
	}
}

void RoadGraph::dijkstra_search(CrossList& crosslist, RoadList& roadlist, Car& car)
{
	long from = car.next_cross;
	long to = car.to_id;

	if (from == to) // 此时车已经到了目的地路口
	{
		car.route_real.push_back(to);
		car.route_plan.clear();
		car.next_cross = -1;
		car.next_road = -1;
		return;
	}

	unordered_map<long, long> cameFrom; // 记录父节点
	unordered_map<long, double> gScore; // 记录从起始点到每个节点的cost
	PriorityQueue<long, double> openSet;
	unordered_map<long, bool> closeSet;

	openSet.push(from, 0); // 起始点入队
	cameFrom[from] = from;
	gScore[from] = 0;

	while (!openSet.empty())
	{
		long current = openSet.pop();
		closeSet[current] = true;

		if (current == to) // 到达目标 更新car的路径
		{
			car.route_plan.clear();
			car.route_plan.push_front(current);
			while (cameFrom.find(current) != cameFrom.end() && current != from)
			{
				current = cameFrom[current];
				car.route_plan.push_front(current);
			}
			// 更新car的其他参数 更新car的路线信息都只在这里了!!!
			car.route_real.push_back(car.route_plan.front());
			car.route_plan.pop_front();
			car.next_cross = car.route_plan.front();
			car.next_road = roadBetween[current][car.next_cross];
			return;
		}

		for (long neighbor : neighborsOf[current]) // 遍历相邻节点
		{
			if (closeSet.find(neighbor) != closeSet.end()) continue; // 判断下这个节点是否处理过

			long roadid = roadBetween[current][neighbor];
			Road& road = roadlist[roadid];
			double tentative_gScore = gScore[current] + distBetween[current][neighbor] + road.getCarsNum(neighbor);
			
			if (gScore.find(neighbor) == gScore.end())
				openSet.push(neighbor, tentative_gScore);
			else if (tentative_gScore >= gScore[neighbor])
				continue;

			cameFrom[neighbor] = current;
			gScore[neighbor] = tentative_gScore;
		}
	}

}


#endif