#ifndef UTIL_HPP
#define UTIL_HPP

#include <queue>
#include <vector>
#include <functional>

using namespace std;

// 优先队列
template<typename T, typename priority_t>
class PriorityQueue
{
public:
	typedef pair<priority_t, T> PQElement; // 类型别称

	inline bool empty() const
	{
		return elements.empty();
	}

	inline void push(T item, priority_t priority) 
	{
		elements.emplace(priority, item); // 加到优先队列
	}

	T pop() // 取出优先队列中的top
	{ 
		T best_item = elements.top().second;
		elements.pop();
		return best_item;
	}

private:
	// 内部用了标准库中的优先队列
	priority_queue<PQElement, vector<PQElement>, greater<PQElement> > elements;


};





#endif