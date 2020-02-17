#ifndef POLYGON_H
#define POLYGON_H
#define WP_SMALLEST_DISTANCE 0.1 //两个waypoint之间的最小距离不能小于0.5m
#define VERTEX_SMALLEST_DISTANCE                                               \
  1 //多边形两个相邻边界点之间的最小距离不能小于1m
#define FAR_FROM_BORDER 1.51 //?????k????1.51m?
#define ABNORMAL_COORDINATE (10000)

bool SecondLineSpecialProcess(MyPoint *p, int &n, Line &refer_border,
                              Line &NearBorder);
void RemoveWaypointTooClose(pointDequeType &dqWp,
                            ClosePointPairDequeType dqClosePointPair);
void waypointTooFarHandler(pointDequeType &dqWp);

#endif
