#include "vae/barrier.h"
#include <algorithm> // std::for_each
#include <cmath>
#include <cstring>
#include <deque>
#include <iostream> // std::cout
#include <iterator> // std::next
#include <list>     // std::list
#include <vector>

Polygon::Polygon(int n, MyPoint *vert) {
  npts = n;
  p = new MyPoint[npts];
  for (int i = 0; i < npts; i++)
    p[i] = vert[i];
}

Polygon::Polygon(void) {}

Polygon::~Polygon() {
  npts = 0;
  dqPitsArr.clear();
  memset(OutBorderPoint, 0, sizeof(MyPoint) * 2);
  if (p != NULL) {
    delete[] p;
    p = NULL;
  }
}

void Polygon::setConvex(bool input_pram) { is_convex = input_pram; }

bool Polygon::getConvex(void) { return is_convex; }

void Polygon::setClockwise(bool input_parm) { is_clockwise = input_parm; }

bool Polygon::getClockwise(void) { return is_clockwise; }

void Polygon::printPloygon(void) {
  int i = 0;
  cout << "print polygon............................." << endl;
  for (i = 0; i < npts; i++) {
    DisplayPoint(p[i]);
  }
}

// this code is used to find concave point of concave polygon
void Polygon::isConvex(void) {
  int i, j = 0;
  int n = npts;
  MyPoint pit;
  bool is_clockwise = get_clockwise(p, n);
  MyPoint u, v;
  dqPitsArr.clear();
  if (is_clockwise) {
    setClockwise(true);
    if (n < 4) {
      setConvex(true);
    } else {
      for (i = 0; i < n; ++i) {
        u.x = p[i].x - p[(i + n - 1) % n].x;
        u.y = p[i].y - p[(i + n - 1) % n].y;
        v.x = p[(i + 1) % n].x - p[i].x;
        v.y = p[(i + 1) % n].y - p[i].y;
        if (perp(u, v) > 0) {
#if 0
                    cout<<p[i].x<<","<<p[i].y<<endl;
#endif
          pit.x = p[i].x;
          pit.y = p[i].y;
          dqPitsArr.push_back(pit);
          j++;
          setConvex(false);
        }
      }
    }
  } else {
    setClockwise(false);
    for (i = 0; i < npts; ++i) {
      if (n < 4) {
      } else {
        u.x = p[i].x - p[(i + n - 1) % n].x;
        u.y = p[i].y - p[(i + n - 1) % n].y;
        v.x = p[(i + 1) % n].x - p[i].x;
        v.y = p[(i + 1) % n].y - p[i].y;
        if (perp(u, v) < 0) {
          pit.x = p[i].x;
          pit.y = p[i].y;
          dqPitsArr.push_back(pit);
          j++;
          setConvex(false);
        }
      }
    }
  }
  if (!is_convex) {
#if DEBUG
    cout << "[barrier]print pits point n=" << dqPitsArr.size() << endl;
    for (pointDequeType::iterator it = dqPitsArr.begin(); it != dqPitsArr.end();
         it++) {
      cout << *it << endl;
    }
#endif
  }
  //    cout<<"[C++]pit_num="<<dqPitsArr.size()<<endl;
}

void MultiLinePointWithBarrierHandlerSub(pointAttrDequeType dqLinePoint,
                                         pointDequeType &dqWpEveryTime,
                                         Polygon *pGround) {
  unsigned int i = 0;
  PointAttr pT0, pT1;
  for (i = 0; i < dqLinePoint.size() - 1; i++) {
    pT0 = dqLinePoint.at(i);
    pT1 = dqLinePoint.at(i + 1);
    FindMiddlePointOfNeighboringPoint2(pT0, pT1, dqWpEveryTime, pGround);
  }
}

void MultiLinePointWithBarrierHandler(MyPoint *p, int n,
                                      pointAttrDequeType &dqLinePoint,
                                      pointDequeType &dqWp,
                                      pointDequeType &dqWpEveryTime, int bnum,
                                      MyPoint **OutBorderTmp,
                                      Polygon *pGround) {
  if (dqWp.size() > 0) {
    bool flag = NextLineNeedReverse(p, n, dqLinePoint.front().pt,
                                    dqLinePoint.back().pt, dqWp.back());
    if (!flag) {
      std::reverse(dqLinePoint.begin(), dqLinePoint.end());
    }
    MyPoint pt0, pt1;
    MyPoint ptTmp[2];
    PointAttr pa;
    pointAttrDequeType dqLinePointTmp;
    bool isIntersect = false;

    //��һ�ֺ��ߵ����һ��waypoint�ͱ��κ��ߵ���㹹�ɵĺ��߿������ϰ����ཻ
    //��������¾���Ҫ���⴦��
    pt0 = dqWp.back(); //��һ�ֺ��ߵ��յ�
    pt1 = dqLinePoint.front().pt;
    ptTmp[0] = pt0;
    ptTmp[1] = pt1;
    getIntersetion2(OriginArgs::instance().pBarrier, bnum, dqLinePointTmp,
                    ptTmp);
    for (int i = 0; i < bnum; i++) {
      isIntersect = intersect2D_SegPoly(
          OriginArgs::instance().pBarrier[i]->p,
          OriginArgs::instance().pBarrier[i]->npts, pt0, pt1);
      if (isIntersect) {
        break;
      }
    }
    if (isIntersect) {
      pa.pt = pt0;
      pa.PolygonIndex = 0;
      dqLinePointTmp.push_back(pa);
      dqLinePointTmp.push_back(dqLinePoint.front());
      std::sort(dqLinePointTmp.begin(), dqLinePointTmp.end());
      /* ���2978t181116110600 ·���滮����������*/
      if (dist_Point_to_Point(dqLinePointTmp.front().pt, dqWp.back()) >
          dist_Point_to_Point(dqLinePointTmp.back().pt, dqWp.back())) {
        std::reverse(dqLinePointTmp.begin(), dqLinePointTmp.end());
      }
      MultiLinePointWithBarrierHandlerSub(dqLinePointTmp, dqWpEveryTime,
                                          pGround);
    } else
      FindMiddlePointOfNeighboringPoint(p, n, pt0, pt1, dqWpEveryTime);
  }

  for (int i = 0; i < bnum; i++) {
    OriginArgs::instance().pBarrier[i]->OutBorderPoint[0] = OutBorderTmp[i][0];
    OriginArgs::instance().pBarrier[i]->OutBorderPoint[1] = OutBorderTmp[i][1];
  }
  if (dqWpEveryTime.size() > 0) {
    for (pointDequeType::iterator it = dqWpEveryTime.begin();
         it != dqWpEveryTime.end(); it++) {
      dqWp.push_back(*it);
    }
    dqWpEveryTime.clear();
  }
  MultiLinePointWithBarrierHandlerSub(dqLinePoint, dqWpEveryTime, pGround);
}

void SimpleLinePointHandler(MyPoint *p, int n, pointAttrDequeType dqLinePoint,
                            pointDequeType dqWp,
                            pointDequeType &dqWpEveryTime) {
  pointDequeType dqTmp;
  unsigned int i = 0;
  for (i = 0; i < dqLinePoint.size(); i++) {
    dqTmp.push_back(dqLinePoint.at(i).pt);
  }
  if (dqWp.size() == 0) {
    dqWpEveryTime.push_back(dqTmp.front());
    dqWpEveryTime.push_back(dqTmp.back());
  } else {
    MyPoint pt0, pt1;
    bool flag = true;
    pt0 = dqWp.back(); //��һ�ֺ��ߵ��յ�
    Segment seg;
    MyPoint LastWp = dqWp.back();

    for (i = 0; i < (unsigned int)n; i++) {
      seg.p0 = p[i];
      seg.p1 = p[(i + 1) % n];
      if (onSegment(seg.p0, seg.p1, LastWp))
        break;
    }
    if (onSegment(seg.p0, seg.p1, dqTmp.back())) {
      pt1 = dqTmp.back();
      flag = false;
    } else if (onSegment(seg.p0, seg.p1, dqTmp.front())) {
      pt1 = dqTmp.front();
      flag = true;
    } else {
      double d1 = dist_Point_to_Point(dqTmp.front(), dqWp.back());
      double d2 = dist_Point_to_Point(dqTmp.back(), dqWp.back());
      if (d1 <= d2) //ȡ����һ�κ����յ���ĺ��ߵ���Ϊ��һ�κ��ߵ����
      {
        pt1 = dqTmp.front();
        flag = true;
      } else {
        pt1 = dqTmp.back();
        flag = false;
      }
    }
    /* �ж���һ�κ��ߵ��յ��뱾�κ�����㹹�ɵ��߶�
                    �Ƿ��밼������ཻ������ཻ����ô����ӱ��κ���
             ֮ǰ��Ҫ����Ӱ���*/
    FindMiddlePointOfNeighboringPoint(p, n, pt0, pt1, dqWpEveryTime);
    if (dqWp.size() > 0) {
      /* �Ҿ�����ĵ���Ϊ��һ������ */
      if (flag) {
        dqWpEveryTime.push_back(dqTmp.front());
        dqWpEveryTime.push_back(dqTmp.back());
      } else {
        dqWpEveryTime.push_back(dqTmp.back());
        dqWpEveryTime.push_back(dqTmp.front());
      }
    }
  }
}

void FindShortestPathSub(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                         pointDequeType &dqTmp1, pointDequeType &dqTmp2) {
  int i = 0;
  Segment seg; // S�Ƕ���ζ����γɵ��߶Σ�һ��n��
  int StartIndexOfpt0 = -1;
  int EndIndexOfpt0 = -1;
  int StartIndexOfpt1 = -1;
  int EndIndexOfpt1 = -1;
  int StartIndexOfSmall = -1; //С���
  int EndIndexOfSmall = -1;   //С���
  int StartIndexOfBig;        //�����
  int EndIndexOfBig;          //�����
  MyPoint StartPoint;
  MyPoint EndPoint;
  dqTmp1.clear();
  dqTmp2.clear();
  for (i = 0; i < n; i++) {
    seg.p0 = p[i];
    seg.p1 = p[(i + 1) % n];
    if (onSegment(seg.p0, seg.p1, pt0)) {
      StartIndexOfpt0 = i;
      EndIndexOfpt0 = (i + 1) % n;
    }
    if (onSegment(seg.p0, seg.p1, pt1)) {
      StartIndexOfpt1 = i;
      EndIndexOfpt1 = (i + 1) % n;
    }
  }
  /* pt0��pt1λ��ͬһ������ */
  if ((StartIndexOfpt0 == StartIndexOfpt1) &&
      (EndIndexOfpt0 == EndIndexOfpt1)) {
    dqTmp1.push_back(pt0);
    dqTmp1.push_back(pt1);
    dqTmp2.push_back(pt0);
    dqTmp2.push_back(pt1);
    return;
  }
  if (StartIndexOfpt0 < StartIndexOfpt1) {
    StartIndexOfSmall = StartIndexOfpt0;
    EndIndexOfSmall = EndIndexOfpt0;
    StartIndexOfBig = StartIndexOfpt1;
    EndIndexOfBig = EndIndexOfpt1;
    StartPoint = pt0;
    EndPoint = pt1;
  } else {
    StartIndexOfSmall = StartIndexOfpt1;
    EndIndexOfSmall = EndIndexOfpt1;
    StartIndexOfBig = StartIndexOfpt0;
    EndIndexOfBig = EndIndexOfpt0;
    StartPoint = pt1;
    EndPoint = pt0;
  }
  dqTmp1.push_back(StartPoint);
  for (i = EndIndexOfSmall; i <= StartIndexOfBig; i++) {
    dqTmp1.push_back(p[i]);
  }
  dqTmp1.push_back(EndPoint);

  dqTmp2.push_back(StartPoint);
  for (i = StartIndexOfSmall; i >= 0; i--) {
    dqTmp2.push_back(p[i]);
  }
  if (EndIndexOfBig > StartIndexOfBig) {
    for (i = n - 1; i >= EndIndexOfBig; i--) {
      dqTmp2.push_back(p[i]);
    }
  }
  dqTmp2.push_back(EndPoint);
  /* make sure routing direction is correct, so we need compare distance */
  if (dqTmp1.size() > 0) {
    double d1 = dist_Point_to_Point(dqTmp1.front(), pt0);
    double d2 = dist_Point_to_Point(dqTmp1.back(), pt0);
    if (d1 > d2) {
      std::reverse(dqTmp1.begin(), dqTmp1.end());
    }
  }
  if (dqTmp2.size() > 0) {
    double d1 = dist_Point_to_Point(dqTmp2.front(), pt0);
    double d2 = dist_Point_to_Point(dqTmp2.back(), pt0);
    if (d1 > d2) {
      std::reverse(dqTmp2.begin(), dqTmp2.end());
    }
  }
  /* ȥ���ظ��ĵ㣬�����������ζ����ཻʱ���ظ����� */
  deque<MyPoint>::iterator iter1 = unique(dqTmp1.begin(), dqTmp1.end());
  dqTmp1.erase(iter1, dqTmp1.end());
  deque<MyPoint>::iterator iter2 = unique(dqTmp2.begin(), dqTmp2.end());
  dqTmp2.erase(iter2, dqTmp2.end());
}

void ChooseShortestPathOfTwoPoint(MyPoint *p, int n, pointDequeType &dqTmp1,
                                  pointDequeType &dqTmp2,
                                  pointDequeType &dqWpEveryTime) {
  double d1 = 0.0, d2 = 0.0;
  unsigned int i = 0;

  /* ѡ�����̵�·��Ϊ����·�� */
  for (i = 0; i < dqTmp1.size() - 1; i++) {
    d1 += dist_Point_to_Point(dqTmp1[i], dqTmp1[i + 1]);
  }
  for (i = 0; i < dqTmp2.size() - 1; i++) {
    d2 += dist_Point_to_Point(dqTmp2[i], dqTmp2[i + 1]);
  }

  if (d1 <= d2) {
    for (i = 0; i < dqTmp1.size(); i++) {
      dqWpEveryTime.push_back(dqTmp1[i]);
    }
  } else {
    for (i = 0; i < dqTmp2.size(); i++) {
      dqWpEveryTime.push_back(dqTmp2[i]);
    }
  }
}

void getTwoPathWhenVertex(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                          pointDequeType &dqTmp1, pointDequeType &dqTmp2) {
  int i = 0;
  int isVertexCount = 0; // pt0,pt1�Ƕ���ĸ���
  int IndexOfpt0 = -1;   //���pt0�Ƕ��㣬���ֵ����0
  int IndexOfpt1 = -1;   //���pt1�Ƕ��㣬���ֵ����0
  /* �ܹ���˳ʱ�����ʱ������·�� */
  for (i = 0; i < n; i++) {
    if (p[i] == pt0) {
      IndexOfpt0 = i;
      isVertexCount++;
    }
    if (p[i] == pt1) {
      IndexOfpt1 = i;
      isVertexCount++;
    }
  }

  if (2 == isVertexCount) {
    if (IndexOfpt0 < IndexOfpt1) {
      for (i = IndexOfpt0; i <= IndexOfpt1; i++) {
        dqTmp1.push_back(p[i]);
      }
      for (i = IndexOfpt0; i >= 0; i--) {
        dqTmp2.push_back(p[i]);
      }
      for (i = n - 1; i >= IndexOfpt1; i--) {
        dqTmp2.push_back(p[i]);
      }
    } else {
      for (i = IndexOfpt0; i < n; i++) {
        dqTmp1.push_back(p[i]);
      }
      for (i = 0; i <= IndexOfpt1; i++) {
        dqTmp1.push_back(p[i]);
      }
      for (i = IndexOfpt0; i >= IndexOfpt1; i--) {
        dqTmp2.push_back(p[i]);
      }
    }
  }
}

/*
Ѱ��ͬһ�����������������·��
*/
void FindShortestPathOfTwoPoint(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                                pointDequeType &dqWpEveryTime) {
  int i = 0;
  int isVertexCount = 0; // pt0,pt1�Ƕ���ĸ���
  int IndexOfpt0 = -1;   //���pt0�Ƕ��㣬���ֵ����0
  int IndexOfpt1 = -1;   //���pt1�Ƕ��㣬���ֵ����0
  /* �ܹ���˳ʱ�����ʱ������·�� */
  pointDequeType dqTmp1; //˳ʱ��ǰ��
  pointDequeType dqTmp2; //��ʱ��ǰ��
  for (i = 0; i < n; i++) {
    if (p[i] == pt0) {
      IndexOfpt0 = i;
      isVertexCount++;
    }
    if (p[i] == pt1) {
      IndexOfpt1 = i;
      isVertexCount++;
    }
  }

  if (2 == isVertexCount) {
    if (IndexOfpt0 < IndexOfpt1) {
      for (i = IndexOfpt0; i <= IndexOfpt1; i++) {
        dqTmp1.push_back(p[i]);
      }
      for (i = IndexOfpt0; i >= 0; i--) {
        dqTmp2.push_back(p[i]);
      }
      for (i = n - 1; i >= IndexOfpt1; i--) {
        dqTmp2.push_back(p[i]);
      }
    } else {
      for (i = IndexOfpt0; i < n; i++) {
        dqTmp1.push_back(p[i]);
      }
      for (i = 0; i <= IndexOfpt1; i++) {
        dqTmp1.push_back(p[i]);
      }
      for (i = IndexOfpt0; i >= IndexOfpt1; i--) {
        dqTmp2.push_back(p[i]);
      }
    }
  } else if (1 == isVertexCount) {
    FindShortestPathSub(p, n, pt0, pt1, dqTmp1, dqTmp2);
    /* ȥ���ظ��ĵ㣬�����������ζ����ཻʱ���ظ����� */
    deque<MyPoint>::iterator iter1 = unique(dqTmp1.begin(), dqTmp1.end());
    dqTmp1.erase(iter1, dqTmp1.end());
    deque<MyPoint>::iterator iter2 = unique(dqTmp2.begin(), dqTmp2.end());
    dqTmp2.erase(iter2, dqTmp2.end());
  } else if (0 == isVertexCount) {
    FindShortestPathSub(p, n, pt0, pt1, dqTmp1, dqTmp2);
  }
  ChooseShortestPathOfTwoPoint(p, n, dqTmp1, dqTmp2, dqWpEveryTime);
}

/*
����: ������κ��������������ߵ㹹�ɵ��߶��������ཻ�����⡣
pt0��pt1�Ǻ��������ڵ��������ߵ�
*/
void FindMiddlePointOfNeighboringPoint2(PointAttr pT0, PointAttr pT1,
                                        pointDequeType &dqWpEveryTime,
                                        Polygon *pGround) {
  int i = 0;
  /* ���pt0��pt1���ڲ�ͬ�������ֱ����� */
  if (pT0.PolygonIndex != pT1.PolygonIndex) {
    if (dqWpEveryTime.size() > 0) {
      if (dqWpEveryTime.back() == pT0.pt) {
      } else {
        dqWpEveryTime.push_back(pT0.pt);
      }
    } else {
      dqWpEveryTime.push_back(pT0.pt);
    }
    dqWpEveryTime.push_back(pT1.pt);
  } else /* �������ϰ��ﻹ�ǵؿ鶼��Ҫ�Ʊ� */
  {
    MyPoint pt0 = pT0.pt;
    MyPoint pt1 = pT1.pt;
    MyPoint *p;
    int n;

    if (0 == pT0.PolygonIndex) {
      p = pGround->p;
      n = pGround->npts;
      /* ���pt0��pt1������߽�����Ҫ�Ʊ� */
      MyPoint MiddlePoint = getMidPoint(pt0, pt1);
      if (cn_PnPoly(MiddlePoint, p, n)) {
        if (dqWpEveryTime.size() > 0) {
          if (dqWpEveryTime.back() == pT0.pt) {
          } else {
            dqWpEveryTime.push_back(pT0.pt);
          }
        } else {
          dqWpEveryTime.push_back(pT0.pt);
        }
        dqWpEveryTime.push_back(pT1.pt);
        return;
      }
    } else {
      for (i = 0; i < OriginArgs::instance().BarrierNum; i++) {
        if ((i + 1) == pT0.PolygonIndex) {
          p = OriginArgs::instance().pBarrier[i]->p;
          n = OriginArgs::instance().pBarrier[i]->npts;
          /* �ϰ����ϵ���������ߵ����ֱ�� */
          if ((pt0 != OriginArgs::instance().pBarrier[i]->OutBorderPoint[0]) &&
              (pt0 != OriginArgs::instance().pBarrier[i]->OutBorderPoint[1]) &&
              (pt1 != OriginArgs::instance().pBarrier[i]->OutBorderPoint[0]) &&
              (pt1 != OriginArgs::instance().pBarrier[i]->OutBorderPoint[1])) {
            dqWpEveryTime.push_back(pt0);
            dqWpEveryTime.push_back(pt1);
            return;
          } else {
            break;
          }
        }
      }
    }
    FindShortestPathOfTwoPoint(p, n, pt0, pt1, dqWpEveryTime);
  }
}

//�Ƚ�����Point��x����
bool operator<(const PointAttr &a, const PointAttr &b) {
  if (fabs(a.pt.x - b.pt.x) > 0.0001)
    return a.pt.x < b.pt.x;
  else
    return a.pt.y < b.pt.y;
}

bool operator==(const PointAttr &a, const PointAttr &b) {
  return (fabs(a.pt.x - b.pt.x) < 0.0001) && (fabs(a.pt.y - b.pt.y) < 0.0001) &&
         (a.PolygonIndex == b.PolygonIndex);
}

bool operator<(const MyPoint &a, const MyPoint &b) {
  if (IsEqual(a.x, b.x))
    return a.y < b.y;
  else
    return a.x < b.x;
}

bool IsPointInBarrier(MyPoint pt) {
  int i = 0;
  int tmp;
  for (i = 0; i < OriginArgs::instance().BarrierNum; i++) {
    tmp = cn_PnPoly(pt, OriginArgs::instance().pBarrier[i]->p,
                    OriginArgs::instance().pBarrier[i]->npts);
    if (1 == tmp || 2 == tmp) {
      return true;
    }
  }
  return false;
}

void getIntersetion2(Polygon **ppBarrier, int bnum,
                     pointAttrDequeType &dqLinePoint, MyPoint *pt) {
  unsigned int line_point_num = 0; //ֱ�������ν������(�����ߵ����)
  MyPoint pIntersection[200]; //���ߵ�����
  pointDequeType dqTmp;
  PointAttr pa;
  unsigned int i = 0;
  unsigned int m = 0;

  for (i = 0; i < (unsigned int)bnum; i++) {
    memset(pIntersection, 0, sizeof(pIntersection));
    line_point_num = 0;
    dqTmp.clear();
    get_line_polygon_intersetion(ppBarrier[i]->p, ppBarrier[i]->npts, pt[0],
                                 pt[1], pIntersection, &line_point_num);
    for (m = 0; m < line_point_num; m++) {
      pa.pt.x = pIntersection[m].x;
      pa.pt.y = pIntersection[m].y;
      pa.PolygonIndex = i + 1;
      if (line_point_num > 0) {
        dqLinePoint.push_back(pa);
        dqTmp.push_back(pIntersection[m]);
      }
    }
    if (line_point_num > 0) {
      std::sort(dqTmp.begin(), dqTmp.end());
      OriginArgs::instance().pBarrier[i]->OutBorderPoint[0] = dqTmp.front();
      OriginArgs::instance().pBarrier[i]->OutBorderPoint[1] = dqTmp.back();
    }
  }
  if (dqLinePoint.size() > 0)
    std::sort(dqLinePoint.begin(), dqLinePoint.end());
}

void initBarrierFieldObject(Polygon *pGround, MyPoint *p, int n,
                            SimplePolygon *pB, int bnum, double d) {
  unsigned int i = 0;
  int t = 0;
  MyPoint qTmp[MAX_WP_NUM];
  bool ClockWise = false;
  OriginArgs::instance().pBarrier = new Polygon *[bnum];
  OriginArgs::instance().pTransformBar = new SimplePolygon[bnum];
  OriginArgs::instance().isSplitOK = false;
  pGround->isConvex();
  isConvex(p, n, &ClockWise, OriginArgs::instance().dqPitsArr);
#if DEBUG
  pGround->printPloygon();
  cout << "bnum=" << bnum << endl;
#endif
  for (i = 0; i < (unsigned int)bnum; i++) {
    get_envelope_of_polygen(pB[i].p, pB[i].n, qTmp, &t, 0.5 * d, 1);
    OriginArgs::instance().pBarrier[i] = new Polygon(t, qTmp);
    OriginArgs::instance().pTransformBar[i].p = new MyPoint[pB[i].n];
    OriginArgs::instance().pTransformBar[i].n = pB[i].n;
    for (int j = 0; j < pB[i].n; j++) {
      OriginArgs::instance().pTransformBar[i].p[j] = pB[i].p[j];
    }
  }
  for (i = 0; i < (unsigned int)bnum; i++) {
    OriginArgs::instance().pBarrier[i]->isConvex();
#if DEBUG
    OriginArgs::instance().pBarrier[i]->printPloygon();
#endif
  }
}

void destroyBarrierFiledObject(Polygon *pGround) {
  if (pGround) {
    delete pGround;
    pGround = NULL;
  } else {
    return;
  }
  for (int i = 0; i < OriginArgs::instance().BarrierNum; i++) {
    delete OriginArgs::instance().pBarrier[i];
  }
  delete[] OriginArgs::instance().pBarrier;
  OriginArgs::instance().pBarrier = NULL;
  delete[] OriginArgs::instance().pTransformBar;
  OriginArgs::instance().pTransformBar = NULL;
}

void getLinePoint(unsigned int line_point_num, pointAttrDequeType &dqLinePoint,
                  MyPoint *pIntersection, int bnum, MyPoint *pt,
                  MyPoint **OutBorderTmp) {
  unsigned int i = 0;
  unsigned int m = 0;
  PointAttr pa;
  pointDequeType dqTmp;

  for (i = 0; i < line_point_num; i++) {
    pa.pt.x = pIntersection[i].x;
    pa.pt.y = pIntersection[i].y;
    pa.PolygonIndex = 0;
    if (line_point_num > 0) {
      dqLinePoint.push_back(pa);
    }
  }
  for (i = 0; i < (unsigned int)bnum; i++) {
    memset(pIntersection, 0, sizeof(MyPoint) * MAX_WP_NUM);
    line_point_num = 0;
    dqTmp.clear();
    get_line_polygon_intersetion(OriginArgs::instance().pBarrier[i]->p,
                                 OriginArgs::instance().pBarrier[i]->npts,
                                 pt[0], pt[1], pIntersection, &line_point_num);
    for (m = 0; m < line_point_num; m++) {
      pa.pt.x = pIntersection[m].x;
      pa.pt.y = pIntersection[m].y;
      pa.PolygonIndex = i + 1;
      if (line_point_num > 0) {
        dqLinePoint.push_back(pa);
        dqTmp.push_back(pIntersection[m]);
      }
    }
    if (line_point_num > 0) {
      std::sort(dqTmp.begin(), dqTmp.end());
      OutBorderTmp[i][0] = dqTmp.front();
      OutBorderTmp[i][1] = dqTmp.back();
    }
  }
  std::sort(dqLinePoint.begin(), dqLinePoint.end());
}

int RoutePlanWithBarrier(double d, int isRepeat, int uavType, bool isMiddle,
                         int SprayType, Line refer_border, MyPoint *p, int n,
                         SimplePolygon *pB, int bnum, PointFlyAttr *wp,
                         int *pm) {
  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int k = 0; //�ܹ�k���������
  double dmax = 0.0;
  int index_max = 0;
  double A, B, Cm;
  bool isScaled = true;
  MyPoint MaxD;  //����ο�����Զ�Ķ���
  MyPoint pt[2]; //���ߵ�ֱ�߷���
  MyPoint pIntersection[MAX_WP_NUM]; //���ߵ�����
  unsigned int line_point_num = 0; //ֱ�������ν������(�����ߵ����)
  pointAttrDequeType dqLinePoint;
  MyPoint ReferNearestPoint;
  unsigned int k1, k2;
  double d1 = d;
  double d2 = d;
  MyPoint **OutBorderTmp = new MyPoint *[bnum];
  for (i = 0; i < (unsigned int)bnum; i++) {
    OutBorderTmp[i] = new MyPoint[2];
  }
  pointDequeType dqWp;
  pointDequeType dqWpEveryTime; //ÿ�κ��ߵ�waypoint����������ͺ��ߵ�
  Polygon *pGround = new Polygon(n, p); //�ؿ�
  *pm = 0;

  initBarrierFieldObject(pGround, p, n, pB, bnum, d);
  get_index_max(refer_border, p, n, index_max, dmax);
  get_useful_args(p, n, index_max, refer_border, &A, &B, &MaxD);
  if (isMiddle) {
    getTrueDmaxSpecial(p, n, k1, k2, d, d1, d2, MaxD, ReferNearestPoint, dmax,
                       refer_border);
    k = k1 + k2;
  } else {
    getTrueDmaxNormal(p, n, k, d, MaxD, ReferNearestPoint, dmax, refer_border);
  }
  while (j < k) {
    dqLinePoint.clear();
    if (dqWpEveryTime.size()) {
      dqWpEveryTime.clear();
    }
    getIntersetion(Cm, A, B, MaxD, ReferNearestPoint, d1, d2, d, k1, k2,
                   refer_border, isMiddle, isScaled, j, pt, pIntersection,
                   &line_point_num, p, n, k);
    getLinePoint(line_point_num, dqLinePoint, pIntersection, bnum, pt,
                 OutBorderTmp);
    if (0 == dqLinePoint.size()) {
      j++;
      continue;
    } else if (2 == dqLinePoint.size()) {
      SimpleLinePointHandler(p, n, dqLinePoint, dqWp, dqWpEveryTime);
    } else {
      MultiLinePointWithBarrierHandler(p, n, dqLinePoint, dqWp, dqWpEveryTime,
                                       bnum, OutBorderTmp, pGround);
    }
    if (dqWpEveryTime.size() > 0) {
      for (pointDequeType::iterator it = dqWpEveryTime.begin();
           it != dqWpEveryTime.end(); it++) {
        dqWp.push_back(*it);
      }
    }
    j++;
  }
  int ret = waypointHandler(p, n, d, isRepeat, uavType, true, SprayType,
                            refer_border, wp, pm, dqWp);
  destroyBarrierFiledObject(pGround);
  if (OutBorderTmp) {
    for (int i = 0; i < bnum; i++) {
      delete[] OutBorderTmp[i];
      OutBorderTmp[i] = NULL;
    }
    delete[] OutBorderTmp;
    OutBorderTmp = NULL;
  }
  return ret;
}

/* �ж�ĳ�����ǲ����ϰ�������ߵĶ��� */
bool isBarrierEnvelopeVertexPoint(MyPoint pt) {
  int i = 0, j = 0, n = 0;
  MyPoint *p;

  for (i = 0; i < OriginArgs::instance().BarrierNum; i++) {
    if (OriginArgs::instance().isSplitOK) {
      p = OriginArgs::instance().pTransformBar[i].p;
      n = OriginArgs::instance().pTransformBar[i].n;
    } else {
      p = OriginArgs::instance().pBarrier[i]->p;
      n = OriginArgs::instance().pBarrier[i]->npts;
    }
    for (j = 0; j < n; j++) {
      if (p[j] == pt)
        return true;
    }
  }
  return false;
}
