# 19华为软件挑战赛初赛题解
细节可参考[博文](https://granvallen.github.io/codecraft2019/)

## 实现
- 判题器(因为要求给出车辆实际出发时间, 没有与官方的实现对比过)
- 路径搜索算法(在Dijkstra算法的基础上改进)

## 模块
- car.hpp 实现Car与CarList两个类
- road.hpp 实现Road与RoadList两个类
- cross.hpp 实现Cross与CrossList两个类
- RoadGraph.hpp 实现RoadGraph类, 负责路径搜索
- Scheduler.hpp 实现Scheduler类, 调度器/判题器
- util.hpp 实现一个修改的优先队列

## 性能
没做过多的调试与测试, 也没赶上用官方的程序测过, 所以结果的正确性暂未知. 本地测试在官方的一张训练地图(车10240, 路105, 路口64)上的调度时间在700以内.

## 改进
- 没做死锁判定 但运行了几次都没碰到死锁的情况
- 车辆实际上路时间没有进行规划 能上则上
- 没有充分利用实时路况信息 只考虑了道路上车的数量对路径搜索时路权重的影响