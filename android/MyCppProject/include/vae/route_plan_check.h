#ifndef CHECK_H
int CheckPolygon(MyPoint *p, int n);
int CheckInput(MyPoint *p, int n, SimplePolygon *pB, int bNum);
int CheckWaypointValid(FlyBasicArgs Args, MyPoint *p, int n, SimplePolygon *pB,
                       int bNum, FlyWpInfo *pFlyWpInfoInstance);
int CheckWaypointDistanceValid(FlyWpInfo *pFlyWpInfoInstance);
int CheckBaseBorderValid(FlyWpInfo *pFlyWpInfoInstance);
int checkDistanceFromHome(FlyWpInfo *pFlyWpInfoInstance, MyPoint takeOffPoint);
int CheckWaypointSegmentValid(FlyWpInfo *pFlyWpInfoInstance);
void checkRoutePlanAferPlan(FlyWpInfo *pFlyWpInfoInstance,
                            MyPoint takeoffPoint);
void delBarScale(Polygon **ppBarScale, int bNum);

#endif
