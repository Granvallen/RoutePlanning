#ifndef UTIL_HPP
#define UTIL_HPP

#include <queue>
#include <vector>
#include <functional>

using namespace std;

// ���ȶ���
template<typename T, typename priority_t>
class PriorityQueue
{
public:
	typedef pair<priority_t, T> PQElement; // ���ͱ��

	inline bool empty() const
	{
		return elements.empty();
	}

	inline void push(T item, priority_t priority) 
	{
		elements.emplace(priority, item); // �ӵ����ȶ���
	}

	T pop() // ȡ�����ȶ����е�top
	{ 
		T best_item = elements.top().second;
		elements.pop();
		return best_item;
	}

private:
	// �ڲ����˱�׼���е����ȶ���
	priority_queue<PQElement, vector<PQElement>, greater<PQElement> > elements;


};





#endif