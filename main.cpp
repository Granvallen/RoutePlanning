#include <iostream>
#include <iterator>
#include <unordered_map>
#include <map>
//#include "car.hpp"
//#include "corss.hpp"
//#include "road.hpp"
//#include "util.hpp"
//#include "RoadGraph.hpp"
#include "Scheduler.hpp"

using namespace std;

int main()
{
	Scheduler scheduler("car.txt", "road.txt", "cross.txt");

	while (!scheduler.isDone)
		scheduler.step();

	return 0;
}