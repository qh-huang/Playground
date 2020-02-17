#ifndef BARRIER_H
#define BARRIER_H
#include <vae/geometry.h>

class OriginArgs {
public:
  const MyPoint *p;
  int n;
  double d;
  Line refer_border;
  Polygon **pBarrier;
  MyPoint *pTransform;
  SimplePolygon *pTransformBar;
  bool isSplitOK;
  int BarrierNum;
  pointDequeType dqPitsArr;
  static OriginArgs &instance() {
    static OriginArgs *instance = new OriginArgs();
    return *instance;
  }

private:
  OriginArgs() {}
};

void FindMiddlePointOfNeighboringPoint2(PointAttr pT0, PointAttr pT1,
                                        pointDequeType &dqWpEveryTime,
                                        Polygon *pGround);
bool isBarrierEnvelopeVertexPoint(MyPoint pt);
void getIntersetion2(Polygon **ppBarrier, int bnum,
                     pointAttrDequeType &dqLinePoint, MyPoint *pt);
void initBarrierFieldObject(Polygon *pGround, MyPoint *p, int n,
                            SimplePolygon *pB, int bnum, double d);
void destroyBarrierFiledObject(Polygon *pGround);
void getLinePoint(unsigned int line_point_num, pointAttrDequeType &dqLinePoint,
                  MyPoint *pIntersection, int bnum, MyPoint *pt,
                  MyPoint **OutBorderTmp);
void SimpleLinePointHandler(MyPoint *p, int n, pointAttrDequeType dqLinePoint,
                            pointDequeType dqWp, pointDequeType &dqWpEveryTime);
void MultiLinePointWithBarrierHandler(MyPoint *p, int n,
                                      pointAttrDequeType &dqLinePoint,
                                      pointDequeType &dqWp,
                                      pointDequeType &dqWpEveryTime, int bnum,
                                      MyPoint **OutBorderTmp, Polygon *pGround);
void MultiLinePointWithBarrierHandlerSub(pointAttrDequeType dqLinePoint,
                                         pointDequeType &dqWpEveryTime,
                                         Polygon *pGround);
void getTwoPathWhenVertex(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                          pointDequeType &dqTmp1, pointDequeType &dqTmp2);
bool operator<(const PointAttr &a, const PointAttr &b);
bool operator==(const PointAttr &a, const PointAttr &b);

#endif
