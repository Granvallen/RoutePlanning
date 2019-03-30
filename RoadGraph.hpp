#ifndef ROADGRAPH_HPP
#define ROADGRAPH_HPP

#include <unordered_map>
#include <vector>
#include <list>

#include "corss.hpp"
#include "road.hpp"
#include "car.hpp"
#include "util.hpp"


class RoadGraph
{
public:
	RoadGraph() {}
	RoadGraph(CrossList& crosslist, RoadList& roadlist) { initGraph(crosslist, roadlist); }
	
	// ��ʼ��·��ͼ neighborsOf �� distBetween ����֮������·��
	void initGraph(CrossList& crosslist, RoadList& roadlist);

	void findFastestRoute(CrossList& crosslist, RoadList& roadlist, Car& car);

	unordered_map<long, list<long> > neighborsOf; // һ���ڵ�����ڽڵ�
	unordered_map<long, unordered_map<long, double> > distBetween; // �������ڽڵ�ľ���
	unordered_map<long, unordered_map<long, long> > roadBetween; // �������ڽڵ�֮��·��id

private:
	void dijkstra_search(CrossList& crosslist, RoadList& roadlist, Car& car); // dijkstra·����������

};

// Ŀǰ�������û���õ�crosslist��roadlist����Ϣ
void RoadGraph::findFastestRoute(CrossList& crosslist, RoadList& roadlist, Car& car)
{
	// Dijkstra�㷨 ����crosslist��roadlist car�е�·ʵʱ��Ϣ �������Ȳ���
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
			// �����ǰcross��road��from�ڵ����·��˫��� �϶��ܴ�����ڵ㵽·��һ���ڵ�
			// ������ ����ǵ����� ���� ��ǰ·�ڲ���road��from ����·�����·������
			if (!road.isDuplex && road.from_id != cross.first)
				continue;

			// �ж���·����һ��·��id
			long cross_id_another = road.from_id != cross.first ? road.from_id : road.to_id;

			neighborsOf[cross.first].push_back(cross_id_another);
			distBetween[cross.first][cross_id_another] = road.length; // ע�ⵥ��·������ �����Ŀ�ͨ�з����
			roadBetween[cross.first][cross_id_another] = road.id; // ����·��֮���·��id
		}
	}
}

void RoadGraph::dijkstra_search(CrossList& crosslist, RoadList& roadlist, Car& car)
{
	long from = car.next_cross;
	long to = car.to_id;

	if (from == to) // ��ʱ���Ѿ�����Ŀ�ĵ�·��
	{
		car.route_real.push_back(to);
		car.route_plan.clear();
		car.next_cross = -1;
		car.next_road = -1;
		return;
	}

	unordered_map<long, long> cameFrom; // ��¼���ڵ�
	unordered_map<long, double> gScore; // ��¼����ʼ�㵽ÿ���ڵ��cost
	PriorityQueue<long, double> openSet;
	unordered_map<long, bool> closeSet;

	openSet.push(from, 0); // ��ʼ�����
	cameFrom[from] = from;
	gScore[from] = 0;

	while (!openSet.empty())
	{
		long current = openSet.pop();
		closeSet[current] = true;

		if (current == to) // ����Ŀ�� ����car��·��
		{
			car.route_plan.clear();
			car.route_plan.push_front(current);
			while (cameFrom.find(current) != cameFrom.end() && current != from)
			{
				current = cameFrom[current];
				car.route_plan.push_front(current);
			}
			// ����car���������� ����car��·����Ϣ��ֻ��������!!!
			car.route_real.push_back(car.route_plan.front());
			car.route_plan.pop_front();
			car.next_cross = car.route_plan.front();
			car.next_road = roadBetween[current][car.next_cross];
			return;
		}

		for (long neighbor : neighborsOf[current]) // �������ڽڵ�
		{
			if (closeSet.find(neighbor) != closeSet.end()) continue; // �ж�������ڵ��Ƿ����

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