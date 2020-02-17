#include <algorithm> //std:unique
#include <deque>
#include <iostream> // std::cout
#include <vae/barrier.h>
#include <vae/polygon.h>
#include <vae/route_plan_check.h>
#include <vector>
using namespace std;

static int gBorderIndex = 65535; //未获得时设置为65535
extern bool gIsConcaveSpray;

void setSegmentTypeMiWuJi(MyPoint *p, int n, int m, Line refer_border,
                          int CurIndex, PointFlyAttr &ptFlyPrev,
                          PointFlyAttr &ptFly, MyPoint ptFlyNext,
                          MyPoint ptFlyNextNext);
void setSegmentTypeNormal(MyPoint *p, int n, int m, Line refer_border,
                          int CurIndex, PointFlyAttr &ptFlyPrev,
                          PointFlyAttr &ptFly, MyPoint ptFlyNext,
                          MyPoint ptFlyNextNext);

int FindReferBorderIndex(MyPoint *p, int n, Line refer_border) {
  int i = 0;
  int ReferBorderIndex = 0;

  if (65535 != gBorderIndex)
    return gBorderIndex;
  for (i = 0; i < n; i++) {
    if ((p[i] == refer_border.p0) && (p[(i + 1) % n] == refer_border.p1)) {
      ReferBorderIndex = i;
      break;
    }
    if ((p[i] == refer_border.p1) && (p[(i + 1) % n] == refer_border.p0)) {
      ReferBorderIndex = i;
      break;
    }
  }
  gBorderIndex = ReferBorderIndex;
  return ReferBorderIndex;
}

/*
必须是获得所有waypoint后才能调用此函数
*/
bool isPointUnique(MyPoint pt, const pointDequeType &dqWp) {
  int k = 0;
  unsigned int i = 0;
  for (i = 0; i < dqWp.size(); i++) {
    if (pt == dqWp.at(i)) {
      k++;
    }
    if (k > 1) {
      return false;
    }
  }
  return true;
}

/*
isPointOnBarrier: 判断点是否在障碍物包络上
*/
bool isPointOnBarrier(MyPoint pt) {
  int m = 0;
  int k = 0;
  int j = 0;
  MyPoint *p2;
  for (m = 0; m < OriginArgs::instance().BarrierNum; m++) {
    if (OriginArgs::instance().isSplitOK) {
      p2 = OriginArgs::instance().pTransformBar[m].p;
      k = OriginArgs::instance().pTransformBar[m].n;
    } else {
      p2 = OriginArgs::instance().pBarrier[m]->p;
      k = OriginArgs::instance().pBarrier[m]->npts;
    }
    for (j = 0; j < k; j++) {
      if (onSegment(p2[j], p2[(j + 1) % k], pt)) {
        return true;
      }
    }
  }
  return false;
}

/*
需要设置baseIndex的原因是方便无人机获取地块边界轮廓，如果设置
错误严重情况下会导致炸机
ptQueue: waypoint队列
*/
void setBaseIndex(const MyPoint *p, int n, double d, Line referBorder,
                  deque<PointFlyAttr> &ptQueue) {
  unsigned int i = 0;
  MyPoint pt;
  for (i = 0; i < ptQueue.size(); i++) {
    pt = ptQueue.at(i).pt;
    bool ret = is_pit_point(pt);
    if (ret) {
      ptQueue.at(i).BaseIndex = 65533;
      continue;
    }
    ret = isPointOnBarrier(ptQueue.at(i).pt);
    if (ret) {
      ptQueue.at(i).BaseIndex = 65534;
      continue;
    }
    setBaseIndexNormal(i, p, n, d, referBorder, ptQueue);
  }
}

void setSideIndex(PointFlyAttr &ptAttr) {
  char high = 0;
  char low = 0;
  MyPoint *p2;
  unsigned int m = 0;
  unsigned int k = 0;
  unsigned int j = 0;
  ptAttr.SideIndex = 65535;
  if (OriginArgs::instance().BarrierNum == 0) {
    ptAttr.SideIndex = 65535;
    return;
  }

  if (0 ==
      combineArgs::instance().barrierNum) /* 设置单个地块waypoint的side index */
  {
    for (m = 0; m < (unsigned int)OriginArgs::instance().BarrierNum; m++) {
      p2 = OriginArgs::instance().pBarrier[m]->p;
      k = (unsigned int)OriginArgs::instance().pBarrier[m]->npts;
      for (j = 0; j < k; j++) {
        if (onSegment(p2[j], p2[(j + 1) % k], ptAttr.pt)) {
          high = m;
          low = j;
          ptAttr.SideIndex = (low << 8) + high;
          return;
        }
      }
    }
  } else /* 设置地块合并waypoint的side index */
  {
    /* 必须先缩边然后才能判断waypoint在哪个障碍物上面 */
    Polygon **ppBarrier = combineArgs::instance().ppSacledBarrier;
    if (ppBarrier == NULL)
      return;

    for (m = 0; m < (unsigned int)combineArgs::instance().barrierNum; m++) {
      p2 = ppBarrier[m]->p;
      k = ppBarrier[m]->npts;
      for (j = 0; j < k; j++) {
        if (onSegment(p2[j], p2[(j + 1) % k], ptAttr.pt)) {
          high = m;
          low = j;
          ptAttr.SideIndex = (low << 8) + high;
          return;
        }
      }
    }
  }
}

void setSegmentType(MyPoint *p, int n, int m, int uavType, Line refer_border,
                    int CurIndex, PointFlyAttr &ptFlyPrev, PointFlyAttr &ptFly,
                    MyPoint ptFlyNext, MyPoint ptFlyNextNext) {
  if (1 == uavType) {
    setSegmentTypeMiWuJi(p, n, m, refer_border, CurIndex, ptFlyPrev, ptFly,
                         ptFlyNext, ptFlyNextNext);
  } else {
    setSegmentTypeNormal(p, n, m, refer_border, CurIndex, ptFlyPrev, ptFly,
                         ptFlyNext, ptFlyNextNext);
  }
}

void setSegmentTypeNormal(MyPoint *p, int n, int m, Line refer_border,
                          int CurIndex, PointFlyAttr &ptFlyPrev,
                          PointFlyAttr &ptFly, MyPoint ptFlyNext,
                          MyPoint ptFlyNextNext) {
  int ret1, ret2;
  double VectorAngle = 0.0;
  MyPoint *p2;
  int k = 0;

  /* the first two waypoint and the last two waypoint need special care */
  if ((0 == CurIndex) || (CurIndex == (m - 1)) || (CurIndex == (m - 2))) {
    ptFly.SegmentType = 0;
    return;
  }
  if (ptFlyPrev.SegmentType) {
    ptFly.SegmentType = 0; // two continuous point cannot be both turning
                           // segment
    return;
  }
  if (CurIndex == m - 3) {
    if (ptFly.BaseIndex == 65533 || ptFly.BaseIndex == 65534) {
      ptFly.SegmentType = 0;
      return;
    }
    VectorAngle = getAngelOfTwoVector(ptFlyNext, ptFlyPrev.pt, ptFly.pt);
    if (VectorAngle >= 45 && VectorAngle <= 135) {
      ptFly.SegmentType = 1;
    } else {
      ptFly.SegmentType = 0;
    }
    return;
  }

  if (ptFlyPrev.SegmentType) {
    ptFly.SegmentType = 0; // we cannot set two continuous to be segmentType = 1
    return;
  }
  if ((CurIndex > 1) && (CurIndex < m - 3)) {
    if (ptFly.BaseIndex == 65533 || ptFly.BaseIndex == 65534) {
      ptFly.SegmentType = 0;
      return;
    }
  }
  for (int i = 0; i < OriginArgs::instance().BarrierNum; i++) {
    p2 = OriginArgs::instance().pBarrier[i]->p;
    k = OriginArgs::instance().pBarrier[i]->npts;
    for (int j = 0; j < k; j++) {
      if (onSegment(p2[j], p2[(j + 1) % k], ptFly.pt)) {
        ptFly.SegmentType = 0;
        return;
      }
    }
  }

  ret1 = IsLineLineNearParallel(refer_border.p0, refer_border.p1, ptFly.pt,
                                ptFlyPrev.pt);
  ret2 = IsLineLineNearParallel(refer_border.p0, refer_border.p1, ptFlyNext,
                                ptFlyNextNext);
  if (ret1 && ret2) {
    VectorAngle = getAngelOfTwoVector(ptFlyNext, ptFlyPrev.pt, ptFly.pt);
    if (VectorAngle >= 45 && VectorAngle <= 135) {
      ptFly.SegmentType = 1;
    } else {
      ptFly.SegmentType = 0;
    }
  } else {
    ptFly.SegmentType = 0;
  }
}

void setSegmentTypeMiWuJi(MyPoint *p, int n, int m, Line refer_border,
                          int CurIndex, PointFlyAttr &ptFlyPrev,
                          PointFlyAttr &ptFly, MyPoint ptFlyNext,
                          MyPoint ptFlyNextNext) {
  int ret1, ret2;
  MyPoint *p2;
  int k = 0;
  cout << "setSegmentTypeMiWuJi" << endl;
  /* the first two waypoint and the last two waypoint need special care */
  if ((0 == CurIndex) || (CurIndex == (m - 1))) {
    ptFly.SegmentType = 0;
    return;
  }
  if (ptFlyPrev.SegmentType) {
    ptFly.SegmentType = 0; // two continuous point cannot be both turning
                           // segment
    return;
  }

  if ((CurIndex > 1) && (CurIndex < m - 1)) {
    if (ptFly.BaseIndex == 65533 || ptFly.BaseIndex == 65534) {
      ptFly.SegmentType = 0;
      return;
    }
  }
  /* 处于障碍物包络上的点不能转弯 */
  for (int i = 0; i < OriginArgs::instance().BarrierNum; i++) {
    p2 = OriginArgs::instance().pBarrier[i]->p;
    k = OriginArgs::instance().pBarrier[i]->npts;
    for (int j = 0; j < k; j++) {
      if (onSegment(p2[j], p2[(j + 1) % k], ptFly.pt)) {
        ptFly.SegmentType = 0;
        return;
      }
    }
  }

  ret1 = IsLineLineNearParallel(refer_border.p0, refer_border.p1, ptFly.pt,
                                ptFlyPrev.pt);
  ret2 = IsLineLineNearParallel(refer_border.p0, refer_border.p1, ptFlyNext,
                                ptFlyNextNext);
  if (ret1 && ret2) {
    ptFly.SegmentType = 1;
  }
}

//开启过喷洒的waypoint不重复开启喷洒
void setPointSpray(int isRepeat, PointFlyAttr &ptFly,
                   const pointDequeType &dqWp) {
  bool isUnique = false;
  if (isRepeat) {
    ptFly.SprayCtrl = 1;
    return;
  }
  isUnique = isPointUnique(ptFly.pt, dqWp);
  if (isUnique) {
    ptFly.SprayCtrl = 1;
  } else {
    ptFly.SprayCtrl = 0;
  }
}

//获取临近点对中的点
bool getClosePoint(MyPoint &p,
                   const ClosePointPairDequeType &dqClosePointPair) {
  for (unsigned int j = 0; j < dqClosePointPair.size(); j++) {
    if (dqClosePointPair.at(j).p0 == p) {
      p = dqClosePointPair.at(j).p1;
      return true;
    }
    if (dqClosePointPair.at(j).p1 == p) {
      p = dqClosePointPair.at(j).p0;
      return true;
    }
  }
  return false;
}

bool isClosePointPit(MyPoint p,
                     const ClosePointPairDequeType &dqClosePointPair) {
  bool ret = getClosePoint(p, dqClosePointPair);
  if (ret == true) {
    return is_pit_point(p);
  }
  return false;
}

/*
解决相邻两条航线距离过近造成重喷的问题，参见3792t181011144415
*/
void fixSprayTypeCase1(unsigned int CurIndex, PointFlyAttr &ptFly,
                       pointDequeType &dqWp, pointFlyAttrDequeType &dqFlyWp) {
  if (CurIndex < 2)
    return;
  if (CurIndex + 2 > dqWp.size())
    return;
  if (ptFly.SprayCtrl == 0)
    return;
  PointFlyAttr ptFlyPrev = dqFlyWp.at(CurIndex - 1);
  MyPoint ptNext = dqWp.at(CurIndex + 1);
  if (ptFlyPrev.SprayCtrl == 0)
    return;
  double VectorAngle = getAngelOfTwoVector(ptNext, ptFlyPrev.pt, ptFly.pt);
  double dist1 = dist_Point_to_Point(ptFly.pt, ptFlyPrev.pt);
  double dist2 = dist_Point_to_Point(ptNext, ptFly.pt);
  if (VectorAngle < 5) {
    cout << "VectorAngle=" << VectorAngle << endl;
    if (dist1 > dist2) {
      ptFly.SprayCtrl = 0;
    } else {
      dqFlyWp.at(CurIndex - 1).SprayCtrl = 0;
      ptFly.SprayCtrl = 1;
    }
  }
}

/*
解决相邻两条航线距离过近造成重喷的问题，参见3586t181023132119
*/
void fixSprayTypeCase2(unsigned int CurIndex, PointFlyAttr &ptFly,
                       pointDequeType &dqWp) {
  if (CurIndex < 2)
    return;
  if (CurIndex + 2 > dqWp.size())
    return;
  MyPoint ptPrevPrev = dqWp.at(CurIndex - 2);
  MyPoint ptPrev = dqWp.at(CurIndex - 1);
  MyPoint ptNext = dqWp.at(CurIndex + 1);
  Line l = {ptPrevPrev, ptPrev};
  double dist1 = dist_Point_to_Line(ptFly.pt, l);
  double dist2 = dist_Point_to_Line(ptNext, l);
  if (dist1 < 1 && dist2 < 1) {
    MyPoint proj = getProjectedPointOnLine(ptNext, ptPrev, ptPrevPrev);
    bool isMiddle = false;
    isMiddle = onSegment(ptPrev, ptPrevPrev, proj);
    if (isMiddle) {
      ptFly.SprayCtrl = 0;
    }
  }
}

/*
解决相邻两条航线距离过近造成重喷的问题
*/
void fixSprayTypeBecauseReapet(unsigned int CurIndex, PointFlyAttr &ptFly,
                               pointDequeType &dqWp,
                               pointFlyAttrDequeType &dqFlyWp) {
  fixSprayTypeCase1(CurIndex, ptFly, dqWp, dqFlyWp);
  fixSprayTypeCase2(CurIndex, ptFly, dqWp);
}

/*
设置凹点的喷洒属性
*/
void setPitPointSpray(int isRepeat, PointFlyAttr &ptFly,
                      const pointDequeType &dqWp, Line refer_border,
                      PointFlyAttr ptFlyNext) {
  if (gIsConcaveSpray) {
    setPointSpray(isRepeat, ptFly, dqWp);
  } else {
    bool ret1 = IsLineLineNearParallel2(refer_border.p0, refer_border.p1,
                                        ptFly.pt, ptFlyNext.pt);
    if (ret1) {
      setPointSpray(isRepeat, ptFly, dqWp);
    } else {
      ptFly.SprayCtrl = 0;
    }
  }
}

void setSprayTypeWhenParallel(int isRepeat, int SprayType, Line refer_border,
                              MyPoint p1, MyPoint p2, PointFlyAttr &ptFly,
                              PointFlyAttr ptFlyNext, pointDequeType &dqWp,
                              const ClosePointPairDequeType &dqClosePointPair) {
  if (SprayType == 2) {
    for (unsigned int j = 0; j < dqClosePointPair.size(); j++) {
      if (dqClosePointPair.at(j).p0 == p2) {
        p2 = dqClosePointPair.at(j).p1;
        break;
      }
    }
    if (is_pit_point(p1) || is_pit_point(p2)) {
      setPitPointSpray(isRepeat, ptFly, dqWp, refer_border, ptFlyNext);
    }
    // when waypoint on barrier, we need to turn off spray
    else if (IsPointInBarrier(p1) && IsPointInBarrier(p2)) {
      ptFly.SprayCtrl = 0;
    } else {
      setPointSpray(isRepeat, ptFly, dqWp);
    }
  } else if (SprayType == 3) {
    if (is_pit_point(p1) || is_pit_point(p2) ||
        isClosePointPit(p1, dqClosePointPair) ||
        isClosePointPit(p2, dqClosePointPair)) {
      setPitPointSpray(isRepeat, ptFly, dqWp, refer_border, ptFlyNext);
    } else {
      ptFly.SprayCtrl = 0;
    }
  } else {
    ptFly.SprayCtrl = 0;
  }
}

void setSprayType(MyPoint *p, int n, int isRepeat, int SprayType,
                  bool IsBarrier, Line refer_border, unsigned int CurIndex,
                  PointFlyAttr &ptFly, PointFlyAttr ptFlyNext,
                  pointDequeType &dqWp, pointFlyAttrDequeType &dqFlyWp,
                  const ClosePointPairDequeType &dqClosePointPair) {
  /* turn on spray only when the waypoint is on the route line which is
   * paralleled with refer border */
  bool isParallel = IsLineLineNearParallel(refer_border.p0, refer_border.p1,
                                           ptFly.pt, ptFlyNext.pt);
  MyPoint p1 = dqWp.at(CurIndex);
  MyPoint p2 = dqWp.at(CurIndex + 1);
  /* because some waypoint is removed (2 waypoint distance less than 0.5m), so
  we need to use the close point to judge if is on route line */
  if (IsBarrier) {
    if (isBarrierEnvelopeVertexPoint(p2)) {
      unsigned int j = 0;
      for (j = 0; j < dqClosePointPair.size(); j++) {
        if (dqClosePointPair.at(j).p0 == p2) {
          p2 = dqClosePointPair.at(j).p1;
          break;
        }
      }
    }
  }
  if ((CurIndex > 0) && (CurIndex < (dqWp.size() - 2))) {
    MyPoint ptPrev = dqWp.at(CurIndex - 1);
    MyPoint ptNextNext = dqWp.at(CurIndex + 2);
    // N字型航线的斜线应该关闭喷洒, 见suzhou3.py
    if (!isParallel) {
      bool isN = isNShapePoint(ptPrev, ptFly.pt, ptFlyNext.pt, ptNextNext);
      if (isN) {
        ptFly.SprayCtrl = 0;
        return;
      }
    }
  }
  if (isParallel) {
    if (SprayType == 3) {
      if (isVertexPoint(p, n, p1) && isVertexPoint(p, n, p2))
        ptFly.SprayCtrl = 0;
      else {
        setPointSpray(isRepeat, ptFly, dqWp);
      }
    } else if (SprayType == 2) {
      setPointSpray(isRepeat, ptFly, dqWp);
    } else if (SprayType == 1) {
      ptFly.SprayCtrl = 0;
    }
  } else {
    setSprayTypeWhenParallel(isRepeat, SprayType, refer_border, p1, p2, ptFly,
                             ptFlyNext, dqWp, dqClosePointPair);
  }
  if (OriginArgs::instance().d > 2)
    fixSprayTypeBecauseReapet(CurIndex, ptFly, dqWp, dqFlyWp);
}

void printFlyWayPoint(const pointFlyAttrDequeType &dqFlyWp) {
  for (unsigned int i = 0; i < dqFlyWp.size(); i++) {
    cout << "(" << (i) << ")  ";
    DisplayPoint(dqFlyWp[i].pt);
    cout << " Spray=" << dqFlyWp[i].SprayCtrl;
    cout << " SegmentType=" << dqFlyWp[i].SegmentType;
    cout << " BaseIndex=" << dqFlyWp[i].BaseIndex;
    cout << " SideIndex=" << dqFlyWp[i].SideIndex;
    cout << endl;
  }
}

/*
when the waypoint num is 2, set waypoint attr
*/
void setWpAttrOnlyTwo(MyPoint *p, int n, Line refer_border, int SprayType,
                      pointDequeType &dqWp, pointFlyAttrDequeType &dqFlyWp) {
  unsigned int i = 0;
  PointFlyAttr ptFly;
  PointFlyAttr ptFlyNext;
  ptFly.pt = dqWp.front();
  ptFlyNext.pt = dqWp.back();
  /* set the first waypoint attr */
  ptFly.SideIndex = 65535;
  if (SprayType == 1)
    ptFly.SprayCtrl = 0;
  else if (SprayType == 2)
    ptFly.SprayCtrl = 1;
  else if (SprayType == 3)
    ptFly.SprayCtrl = 1;
  ptFly.SegmentType = 0;
  dqFlyWp.push_back(ptFly);
  ptFly.pt = dqWp.at(dqWp.size() - 1);
  ptFly.SideIndex = 65535;
  ptFly.SprayCtrl = 0;
  ptFly.SegmentType = 0;
  for (i = 0; i < (unsigned int)n; i++) {
    if (onSegment(p[i], p[(i + 1) % n], ptFly.pt)) {
      ptFly.BaseIndex = i;
      break;
    }
  }
  dqFlyWp.push_back(ptFly);
  setBaseIndex(OriginArgs::instance().p, OriginArgs::instance().n,
               OriginArgs::instance().d, refer_border, dqFlyWp);

#if DEBUG
  printFlyWayPoint(dqFlyWp);
#endif
}

void setFirstWpAttr(MyPoint *p, int n, Line refer_border, int SprayType,
                    pointDequeType &dqWp, pointFlyAttrDequeType &dqFlyWp) {
  PointFlyAttr ptFly;
  PointFlyAttr ptFlyNext;

  ptFly.pt = dqWp.at(0);
  ptFlyNext.pt = dqWp.at(1);
  /* set the first waypoint attr */
  ptFly.BaseIndex = 0; /* 得到dqFlyWp 的坐标点后统一设置BaseIndex 属性 */
  setSideIndex(ptFly);
  if (SprayType == 1)
    ptFly.SprayCtrl = 0;
  else if (SprayType == 2)
    ptFly.SprayCtrl = 1;
  else if (SprayType == 3)
    ptFly.SprayCtrl = 1;
  ptFly.SegmentType = 0;
  dqFlyWp.push_back(ptFly);
}

void setLastWpAttr(MyPoint *p, int n, Line refer_border, int SprayType,
                   pointDequeType &dqWp, pointFlyAttrDequeType &dqFlyWp) {
  PointFlyAttr ptFly;
  PointFlyAttr ptFlyPrev;
  unsigned int CurIndex = dqWp.size() - 1;
  ptFly.pt = dqWp.at(CurIndex);
  ptFly.BaseIndex = 0; /* 得到dqFlyWp 的坐标点后统一设置BaseIndex 属性 */
  setSideIndex(ptFly);
  ptFly.SprayCtrl = 0;
  ptFly.SegmentType = 0;
  ptFlyPrev.pt = dqWp.at(CurIndex - 1);
  dqFlyWp.push_back(ptFly);
}

void setNextToLastWpAttr(MyPoint *p, int n, Line refer_border, int SprayType,
                         pointDequeType &dqWp, pointFlyAttrDequeType &dqFlyWp) {
  PointFlyAttr ptFly;
  PointFlyAttr ptFlyNext;
  unsigned int CurIndex = dqWp.size() - 2;
  ptFly.pt = dqWp.at(CurIndex);
  setSideIndex(ptFly);
  if (SprayType == 3)
    ptFly.SprayCtrl = 1;
  else if (SprayType == 2)
    ptFly.SprayCtrl = 1;
  else if (SprayType == 1)
    ptFly.SprayCtrl = 0;
  fixSprayTypeBecauseReapet(CurIndex, ptFly, dqWp, dqFlyWp);
  ptFly.SegmentType = 0;
  ptFlyNext.pt = dqWp.at(CurIndex + 1);
  dqFlyWp.push_back(ptFly);
}

/*
set waypoint attr, include base index, side index, segment and spray
*/
void setWpAttr(MyPoint *p, int n, int isRepeat, int uavType, Line refer_border,
               int SprayType, bool IsBarrier, pointDequeType &dqWp,
               pointFlyAttrDequeType &dqFlyWp,
               const ClosePointPairDequeType &dqClosePointPair) {
  PointFlyAttr ptFly;
  PointFlyAttr ptFlyNext;
  PointFlyAttr ptFlyPrev;
  unsigned int CurIndex = 0;
  unsigned int i = 0;
  if (dqWp.size() < 2) {
    return;
  }
  if (dqWp.size() == 2) {
    setWpAttrOnlyTwo(p, n, refer_border, SprayType, dqWp, dqFlyWp);
    return;
  }

  setFirstWpAttr(p, n, refer_border, SprayType, dqWp, dqFlyWp);
  for (i = 1; i < dqWp.size() - 2; i++) {
    /* turn on spray only when the waypoint is on the route line which is
     * paralleled with refer border */
    ptFly.pt = dqWp.at(i);
    ptFlyNext.pt = dqWp.at(i + 1);
    CurIndex = i;
    setSprayType(p, n, isRepeat, SprayType, IsBarrier, refer_border, CurIndex,
                 ptFly, ptFlyNext, dqWp, dqFlyWp, dqClosePointPair);
    setSideIndex(ptFly);
    setSegmentType(p, n, dqWp.size(), uavType, refer_border, CurIndex,
                   dqFlyWp.back(), ptFly, dqWp.at(i + 1), dqWp.at(i + 2));
    dqFlyWp.push_back(ptFly);
  }

  setNextToLastWpAttr(p, n, refer_border, SprayType, dqWp, dqFlyWp);
  setLastWpAttr(p, n, refer_border, SprayType, dqWp, dqFlyWp);
  setBaseIndex(OriginArgs::instance().p, OriginArgs::instance().n,
               OriginArgs::instance().d, refer_border, dqFlyWp);
#if DEBUG
  printFlyWayPoint(dqFlyWp);
#endif
}
