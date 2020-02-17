/*
===========================================================================
Project:   UAV Route Planning  Algorithm

File:      polygon.cpp
Author:    xukai (email: germanxk@163.com)
Version:   1.0
Date:       2018/04/12
*/
#include <algorithm> // std::for_each
#include <cmath>
#include <cstring>
#include <deque>
#include <iostream> // std::cout
#include <iterator> // std::next
#include <list>     // std::list
#include <vae/barrier.h>
#include <vae/polygon.h>
#include <vae/route_plan_check.h>
#include <vector>

using namespace std;
LeakSprayInfo gLeakSprayInfo;
static Line NewReferBorder;

bool gIsConcaveSpray = false; /* ���������Ƿ������� */

// this code is used to find concave point of concave polygon
bool isConvex(MyPoint *p, int n, bool *pclockwise,
              pointDequeType &dqPitsArrTemp) {
  int i = 0, j = 0;
  int flag = true;
  MyPoint pit;
  bool is_clockwise = get_clockwise(p, n);
  dqPitsArrTemp.clear();
  MyPoint u, v;
  // ���жϸտ�ʼ��˳ʱ�뻹����ʱ��
  if (is_clockwise) {
    *pclockwise = true;
    //������������,
    //������һ����͹�����
    if (n < 4) {
    } else {
      for (i = 0; i < n; ++i) {
        u.x = p[i].x - p[(i + n - 1) % n].x;
        u.y = p[i].y - p[(i + n - 1) % n].y;
        v.x = p[(i + 1) % n].x - p[i].x;
        v.y = p[(i + 1) % n].y - p[i].y;
        if (perp(u, v) > 0) {
          pit.x = p[i].x;
          pit.y = p[i].y;
          dqPitsArrTemp.push_back(pit);
          j++;
          flag = false; //�������
        }
      }
    }
  } else {
    *pclockwise = false;
    for (i = 0; i < n; ++i) {
      if (n < 4) {
      } else {
        u.x = p[i].x - p[(i + n - 1) % n].x;
        u.y = p[i].y - p[(i + n - 1) % n].y;
        v.x = p[(i + 1) % n].x - p[i].x;
        v.y = p[(i + 1) % n].y - p[i].y;
        if (perp(u, v) < 0) {
          pit.x = p[i].x;
          pit.y = p[i].y;
          dqPitsArrTemp.push_back(pit);
          j++;
          flag = false; //�������
        }
      }
    }
  }
  if (!flag) {
#if DEBUG
    cout << "print pits point n=" << dqPitsArrTemp.size() << endl;
    for (pointDequeType::iterator it = dqPitsArrTemp.begin();
         it != dqPitsArrTemp.end(); it++) {
      cout << *it << endl;
    }
#endif
  }

  return flag;
}

/* �жϵ��Ƿ��ǰ���, ע�����������֮�����εİ���*/
bool is_pit_point(MyPoint pt) {
  for (pointDequeType::iterator it = OriginArgs::instance().dqPitsArr.begin();
       it != OriginArgs::instance().dqPitsArr.end(); it++) {
    if (*it == pt) {
      return true;
    }
  }
  return false;
}

/* �жϵ��Ƿ��ǰ���*/
bool is_pit_point2(MyPoint pt, pointDequeType dqPitsArrTemp) {
  for (pointDequeType::iterator it = dqPitsArrTemp.begin();
       it != dqPitsArrTemp.end(); it++) {
    if (*it == pt) {
      return true;
    }
  }
  return false;
}

bool isVertexPoint(MyPoint *p, int n, MyPoint pt) {
  for (int i = 0; i < n; i++) {
    if (p[i] == pt)
      return true;
  }
  return false;
}

void reverse_array(MyPoint *p, int n) {
  int i;
  vector<MyPoint> que;
  for (i = 0; i < n; i++)
    que.push_back(p[i]);
  reverse(que.begin(), que.end());
  for (i = 0; i < n; i++) {
    p[i] = que[i];
  }
}

/*
������ο���ƽ�е�ֱ�������εĽ��㣬���Ǻ��ߵ�
pIntersection: ���������εĽ�������
*/
void get_line_polygon_intersetion(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                                  MyPoint *pIntersection, unsigned int *pm) {
  unsigned int k = 0;
  int i = 0;
  int t = 0;
  Segment S; // S�Ƕ���ζ����γɵ��߶Σ�һ��n��
  MyPoint Intersection;
  int ret = 0;
  for (i = 0; i < n; i++) {
    pdd B0 = make_pair(p[i].x, p[i].y);
    pdd B1 = make_pair(p[(i + 1) % n].x, p[(i + 1) % n].y);
    pdd A0 = make_pair(pt0.x, pt0.y); //ƽ�����ϵĵ�
    pdd A1 = make_pair(pt1.x, pt1.y); //ƽ�����ϵĵ�
    ret = lineLineIntersection(A0, A1, B0, B1, Intersection);
    if (0 == ret) //ƽ�л��غ�
    {
      ;
    } else {
      MyPoint P;
      P.x = Intersection.x;
      P.y = Intersection.y;
      t = (i + 1) % n;
      S.p0 = p[i];
      S.p1 = p[t];
      bool result = inSegment(P, S); //����ֱ�������߶��ཻ
      if (result) {
        pIntersection[k].x = Intersection.x;
        pIntersection[k].y = Intersection.y;
        k++;
      }
    }
  }
  if (2 == k) {
    if (pIntersection[0] == pIntersection[1]) {
      k = 1;
    }
  }
  *pm = k;
}

// vPoint: ��������
void get_line_polygon_intersetion2(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                                   vector<MyPoint> &vPoint) {
  unsigned int line_point_num = 0; //ֱ�������ν������(�����ߵ����)
  MyPoint pIntersection[200];
  get_line_polygon_intersetion(p, n, pt0, pt1, pIntersection, &line_point_num);
  for (unsigned int i = 0; i < line_point_num; i++) {
    vPoint.push_back(pIntersection[i]);
  }
  std::sort(vPoint.begin(), vPoint.end());
  vector<MyPoint>::iterator it = unique(vPoint.begin(), vPoint.end());
  vPoint.erase(it, vPoint.end());
}

/*
����: ������n�κ��ߺ͵�n+1�κ����������ཻ�ĺ�����
*/
void segment_polygon_intersetion_handler(
    MyPoint *p, int n, MyPoint *pIntersection, int line_point_num,
    pointDequeType &dqWp, pointDequeType &dqWpEveryTime, MyPoint &nextPoint) {
  MyPoint pt0, pt1;
  pt0 = dqWp.back(); //��һ�ֺ��ߵ��յ�
  bool isPrevExist = false;
  MyPoint prevWp;
  getPrevPoint(dqWp, isPrevExist, pt0, prevWp);
  bool flag = FindNextLineStart(p, n, pIntersection[0],
                                pIntersection[line_point_num - 1], pt0, prevWp,
                                isPrevExist);
  if (flag) {
    pt1 = pIntersection[0];
    nextPoint = pIntersection[0];
  } else {
    pt1 = pIntersection[line_point_num - 1];
    nextPoint = pIntersection[line_point_num - 1];
    std::reverse(pIntersection, pIntersection + line_point_num);
  }
  FindMiddlePointOfNeighboringPoint(p, n, pt0, pt1, dqWpEveryTime);
}

/*
��ȡ��Ҫ���waypoint���е��м�㣬��������Ǹ��ѵ�
*/
void get_middle_waypoint(MyPoint *p, int n, Segment S1, Segment S2, MyPoint pt0,
                         MyPoint pt1, pointDequeType &dqMidPointOfSingleRoute) {
  MyPoint I0, I1;
  int ret = 0;
  pointDequeType dqPitsArrTemp;
  pointDequeType dqMid; //�����м�waypoint����ʱ����

  ret = intersect2D_2Segments(S1, S2, &I0, &I1);
  if (1 == ret) {
    int ret2 = cn_PnPoly(I0, p, n);
    if (ret2 != 0)
      dqMid.push_back(I0);
  } else /* S1, S2���ཻ */
  {
    FindShortestPathOfTwoPoint(p, n, pt0, pt1, dqMid);
  }

  if (dqMid.size() > 0) {
    /* ���waypoint�������ٵĶ������������ */
    if (0 == dqMidPointOfSingleRoute.size()) {
      for (pointDequeType::iterator it = dqMid.begin(); it != dqMid.end();
           it++) {
        dqMidPointOfSingleRoute.push_back(*it);
      }
    } else {
      if (dqMid.size() < dqMidPointOfSingleRoute.size()) {
        dqMidPointOfSingleRoute.clear();
        for (pointDequeType::iterator it = dqMid.begin(); it != dqMid.end();
             it++) {
          dqMidPointOfSingleRoute.push_back(*it);
        }
      }
    }
  }
}

void MiddleWaypointHandler(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                           segmentDequeType dqCrossBorder,
                           pointDequeType dqIntersectionTmp,
                           pointDequeType &dqMidPointOfSingleRoute) {
  unsigned int j;
  int ret1, ret2;
  Segment St;
  Segment S1, S2; // S�Ƕ���ζ����γɵ��߶Σ�һ��n��
  segmentDequeType dqCrossBorderOfPt0; //����pt0�ı�
  segmentDequeType dqCrossBorderOfPt1; //����pt1�ı�

  for (j = 0; j < dqCrossBorder.size(); j++) {
    St = dqCrossBorder.at(j);
    ret1 = onSegment(St.p0, St.p1, pt0);
    ret2 = onSegment(St.p0, St.p1, pt1);
    if (ret1) {
      dqCrossBorderOfPt0.push_back(St);
    }
    if (ret2) {
      dqCrossBorderOfPt1.push_back(St);
    }
  }

  if (dqIntersectionTmp.size() == 2) {
    S1 = dqCrossBorderOfPt0.front();
    S2 = dqCrossBorderOfPt1.front();
  }

  if ((1 == dqCrossBorderOfPt0.size()) && (1 == dqCrossBorderOfPt1.size())) {
    S1 = dqCrossBorderOfPt0.front();
    S2 = dqCrossBorderOfPt1.front();
    get_middle_waypoint(p, n, S1, S2, pt0, pt1, dqMidPointOfSingleRoute);
  } else {
    /*
    ���ߵ���������ζ����ཻ�����������Ҫ���⴦��
    ȡpt0��pt1���������Ϊ����
    */
    if (dqCrossBorderOfPt0.size() > 1) {
      S1 = dqCrossBorderOfPt0.front();
      S2 = dqCrossBorderOfPt1.front();
      get_middle_waypoint(p, n, S1, S2, pt0, pt1, dqMidPointOfSingleRoute);
      S1 = dqCrossBorderOfPt0.back();
      get_middle_waypoint(p, n, S1, S2, pt0, pt1, dqMidPointOfSingleRoute);
    }
    if (dqCrossBorderOfPt1.size() > 1) {
      S1 = dqCrossBorderOfPt0.front();
      S2 = dqCrossBorderOfPt1.front();
      get_middle_waypoint(p, n, S1, S2, pt0, pt1, dqMidPointOfSingleRoute);
      S2 = dqCrossBorderOfPt1.back();
      get_middle_waypoint(p, n, S1, S2, pt0, pt1, dqMidPointOfSingleRoute);
    }
  }
}

/*
����: ������κ��������������ߵ㹹�ɵ��߶��������ཻ
�����⡣pt0��pt1�Ǻ��������ڵ��������ߵ�,
��FindShortestPathOfTwoPoint()����
*/
void FindMiddlePointOfNeighboringPoint(MyPoint *p, int n, MyPoint pt0,
                                       MyPoint pt1,
                                       pointDequeType &dqWpEveryTime) {
  int i = 0;
  int t = 0;
  segmentDequeType dqCrossBorder; //�뺽���ཻ�Ķ���εı�
  pointDequeType dqIntersectionTmp;
  Segment S;
  Segment Sinput = {pt0, pt1}; // pt0��pt1��ɵ��߶�
  int ret = 0;
  bool is_intersect = false;
  bool is_new = true;
  pointDequeType
      dqMidPointOfSingleRoute; //���κ�����Ҫ���waypoint���е��м����У����κ���Ψһ
  /* �����ж��ܷ���pt0ֱ��pt1 */
  bool isIntersect1 = intersect2D_SegPoly(p, n, pt0, pt1);
  bool isIntersect2 = false;
  for (int i = 0; i < OriginArgs::instance().BarrierNum; i++) {
    isIntersect2 =
        intersect2D_SegPoly(OriginArgs::instance().pBarrier[i]->p,
                            OriginArgs::instance().pBarrier[i]->npts, pt0, pt1);
    if (isIntersect2) {
      break;
    }
  }
  if (!isIntersect1 && !isIntersect2) {
    return;
  }

  for (i = 0; i < n; i++) {
    is_new = true;
    pdd B0 = make_pair(p[i].x, p[i].y);
    pdd B1 = make_pair(p[(i + 1) % n].x, p[(i + 1) % n].y);
    pdd A0 = make_pair(pt0.x, pt0.y); //�����ϵĵ�
    pdd A1 = make_pair(pt1.x, pt1.y); //�����ϵĵ�
    MyPoint Intersection;
    ret = lineLineIntersection(A0, A1, B0, B1, Intersection);
    if (0 == ret) //ƽ�л��غ�
    {
      ;
    } else {
      MyPoint P; //����
      P.x = Intersection.x;
      P.y = Intersection.y;
      t = (i + 1) % n;
      S.p0 = p[i];
      S.p1 = p[t];
      bool result1 = inSegment(P, S); //����ֱ�������߶��ཻ
      bool result2 = inSegment(P, Sinput); //����ֱ�������߶��ཻ
      if (result1 && result2) {
        is_intersect = true;
        /*
        ���ߵ���������ζ����ཻ�����������Ҫ���⴦��
        �ظ��ĵ㲻���
        */
        for (pointDequeType::iterator it1 = dqIntersectionTmp.begin();
             it1 != dqIntersectionTmp.end(); it1++) {
          if (*it1 == Intersection) {
            is_new = false;
            break;
          }
        }
        if (is_new) {
          dqIntersectionTmp.push_back(Intersection);
        }
        dqCrossBorder.push_back(S); //�õ��뺽���ཻ�ı�
      }
    }
  }
  if (!is_intersect)
    return;
  /* �������Ϊ2��������������ߵ��ڶ������ʱ����ֱ�ӷɲ������� */
  if (dqIntersectionTmp.size() == 2) {
    MyPoint MiddlePoint = getMidPoint(pt0, pt1);
    if (cn_PnPoly(MiddlePoint, p, n))
      return;
  }
  MiddleWaypointHandler(p, n, pt0, pt1, dqCrossBorder, dqIntersectionTmp,
                        dqMidPointOfSingleRoute);
  if (dqMidPointOfSingleRoute.size() > 0) {
    for (pointDequeType::iterator it = dqMidPointOfSingleRoute.begin();
         it != dqMidPointOfSingleRoute.end(); it++) {
      if (dqWpEveryTime.size() > 0) {
        if (dist_Point_to_Point(dqWpEveryTime.back(), *it) >=
            WP_SMALLEST_DISTANCE)
          dqWpEveryTime.push_back(*it);
      } else {
        dqWpEveryTime.push_back(*it);
      }
    }
  }
}

void enqueueTwoPoint(bool flag, pointDequeType &dqWpEveryTime,
                     MyPoint *pIntersection) {
  /* �Ҿ�����ĵ���Ϊ��һ������ */
  if (flag) {
    dqWpEveryTime.push_back(pIntersection[0]);
    dqWpEveryTime.push_back(pIntersection[1]);
  } else {
    dqWpEveryTime.push_back(pIntersection[1]);
    dqWpEveryTime.push_back(pIntersection[0]);
  }
}

void enqueueTwoPointByTakeoffPoint(MyPoint takeOffPoint,
                                   pointDequeType &dqWpEveryTime,
                                   MyPoint *pIntersection) {
  /* �Ҿ�����ɵ���ĵ���Ϊ��һ������ */
  double d1 = dist_Point_to_Point(takeOffPoint, pIntersection[0]);
  double d2 = dist_Point_to_Point(takeOffPoint, pIntersection[1]);
  bool flag = (d1 <= d2) ? true : false;
  enqueueTwoPoint(flag, dqWpEveryTime, pIntersection);
}

void get_useful_args(MyPoint *p, int n, int index_max, Line refer_border,
                     double *pA, double *pB, MyPoint *pMaxD) {
  *pA = refer_border.p1.y - refer_border.p0.y;
  *pB = refer_border.p0.x - refer_border.p1.x;
  pMaxD->x = p[index_max].x;
  pMaxD->y = p[index_max].y;
}

/*�ж��߶��Ƿ��������ཻ�������ж��߶�����ֱ���Ƿ�������
�ཻ, ������ཻ���߶������β��ཻ������ཻ����Ҫ��һ���ж�
�߶ε������˵��Լ����ǵ��е��Ƿ��ڶ�����ڣ��������ôҲ
���������ཻ
����ֵΪtrue��ʾ�ཻ�������ཻ
*/
bool intersect2D_SegPoly(MyPoint *p, int n, MyPoint pt1, MyPoint pt2) {
  MyPoint ptArr[200];
  unsigned int m = 0;
  Segment S;
  S.p0 = pt1;
  S.p1 = pt2;
  int ret = 0;
  vector<MyPoint> concernPt; //ֻ�����߶�pt1-pt2�ϵĵ����Ҫ��ע
  get_line_polygon_intersetion(p, n, pt1, pt2, ptArr, &m);
  for (unsigned int i = 0; i < m; i++) {
    ret = inSegment(ptArr[i], S);
    if (ret == 1) {
      concernPt.push_back(ptArr[i]);
    }
  }
  concernPt.push_back(pt1);
  concernPt.push_back(pt2);
  std::sort(concernPt.begin(), concernPt.end());

  for (unsigned int i = 0; i < m; i++) {
    ret = inSegment(ptArr[i], S);
    if (ret == 1)
      break;
  }
  if (ret == 1) {
    MyPoint MiddlePoint;
    int OutSide;
    for (unsigned int i = 0; i < concernPt.size() - 1; i++) {
      MiddlePoint = getMidPoint(concernPt.at(i), concernPt.at(i + 1));
      OutSide = cn_PnPoly(MiddlePoint, p, n);
      if (0 == OutSide) {
        return true;
      }
    }
  }
  return false;
}

/*
�ú����Ĺ���: �ж��߶��Ƿ��������ཻ������ཻ��Ҫ���
�������
@return: true(�ཻ)��false(���ཻ)
*/
bool intersect2D_SegPoly2(MyPoint *p, int n, MyPoint pt1, MyPoint pt2, int &k) {
  MyPoint ptArr[200];
  unsigned int m = 0;
  Segment S;
  S.p0 = pt1;
  S.p1 = pt2;
  k = 0;
  int ret = 0;
  get_line_polygon_intersetion(p, n, pt1, pt2, ptArr, &m);

  for (unsigned int i = 0; i < m; i++) {
    ret = inSegment(ptArr[i], S);
    if (ret == 1) {
      k++;
    }
  }
  bool ret2 = intersect2D_SegPoly(p, n, pt1, pt2);
  return ret2;
}

void getPrevPoint(const pointDequeType &dqWp, bool &isPrevExist, MyPoint &curWp,
                  MyPoint &prevWp) {
  unsigned int size = dqWp.size();
  if (size > 1) {
    isPrevExist = true;
    curWp = dqWp.back();
    prevWp = dqWp.at(size - 2);
  } else {
    isPrevExist = false;
    curWp = dqWp.back();
    prevWp = dqWp.back();
  }
}

/* �������ߵ�ʱ�Ĵ������� */
void TwoLinePointHandler2(MyPoint *p, int n, MyPoint *pIntersection,
                          MyPoint takeOffPoint, pointDequeType &dqWp,
                          pointDequeType &dqWpEveryTime) {
  if (dqWp.size() == 0) {
    MyPoint pt0, pt1;
    if (dqWpEveryTime.size() == 0) {
      enqueueTwoPointByTakeoffPoint(takeOffPoint, dqWpEveryTime, pIntersection);
    } else {
      pt0 = dqWpEveryTime.back(); //��һ�ֺ��ߵ��յ�
      bool flag = IsSegmentOutOfBorder(p, n, pt0, pIntersection[1]);
      enqueueTwoPoint(flag, dqWpEveryTime, pIntersection);
    }
  } else {
    MyPoint pt0, pt1;
    MyPoint prevWp;
    bool isPrevExist = false;
    if (dqWpEveryTime.size() > 0) {
      pt0 = dqWpEveryTime.back(); //��һ�ֺ��ߵ��յ�
      getPrevPoint(dqWpEveryTime, isPrevExist, pt0, prevWp);
    } else {
      pt0 = dqWp.back(); //��һ�ֺ��ߵ��յ�
      getPrevPoint(dqWp, isPrevExist, pt0, prevWp);
    }

    bool flag = FindNextLineStart(p, n, pIntersection[0], pIntersection[1], pt0,
                                  prevWp, isPrevExist);
    if (flag) {
      pt1 = pIntersection[0];
    } else {
      pt1 = pIntersection[1];
    }
    bool ret = intersect2D_SegPoly3((MyPoint *)OriginArgs::instance().p,
                                    OriginArgs::instance().n, pt0, pt1, 0);
    if (ret)
      FindShortestPathOfTwoPoint(p, n, pt0, pt1, dqWpEveryTime);
    if (dqWp.size() > 0) {
      enqueueTwoPoint(flag, dqWpEveryTime, pIntersection);
    }
  }
}

/*
�����������ν����������2ʱ�Ĵ�������
*/
void MultiLinePointHandler(MyPoint *p, int n, MyPoint *pIntersection,
                           int line_point_num, pointDequeType &dqWp,
                           pointDequeType &dqWpEveryTime) {
  int i = 0;
  MyPoint pt0, pt1;
  MyPoint nextPoint(pIntersection[0]);
  if (dqWp.size() > 0)
    segment_polygon_intersetion_handler(p, n, pIntersection, line_point_num,
                                        dqWp, dqWpEveryTime, nextPoint);

  dqWpEveryTime.push_back(nextPoint);
  for (i = 1; i < line_point_num; i++) {
    pt0 = pIntersection[i - 1];
    pt1 = pIntersection[i];
    FindMiddlePointOfNeighboringPoint(p, n, pt0, pt1, dqWpEveryTime);
    dqWpEveryTime.push_back(pt1);
  }
}

/*
�µ������Ż������������ڵ����ݵؿ飬MultiLinePointHandler2��
MultiLinePointHandler ���ṩ���ŵ�����
*/
void MultiLinePointHandler2(MyPoint *p, int n, line2DequeType &dqLine2,
                            pointDequeType &dqWp,
                            pointDequeType &dqWpEveryTime) {
  unsigned int MaxLineNum = 0; //��ཻ�������ÿ��ȡ2�������
  unsigned int i = 0, j = 0;
  MyPoint pt0, pt1;
  unsigned int size = dqLine2.size();

  for (i = 0; i < size; i++) {
    if (dqLine2.at(i).TotalPointNum > MaxLineNum) {
      MaxLineNum = dqLine2.at(i).TotalPointNum;
    }
  }

  for (j = 0; j < MaxLineNum; j += 2) {
    for (i = 0; i < size; i++) {
      MyPoint LastWp;
      MyPoint NextWp;
      bool flag;
      pt0 = dqLine2.at(i).q[j];
      pt1 = dqLine2.at(i).q[j + 1];

      if ((0 == dqWpEveryTime.size()) && (0 == dqWp.size())) {
        flag = true;
      } else {
        if (dqWpEveryTime.size() > 1) {
          LastWp = dqWpEveryTime.back();
        } else {
          LastWp = dqWp.back();
        }
        if ((j == 0) && (i == size - 1) && ((j + 2) < MaxLineNum)) {
          MyPoint tmp1;
          tmp1 = dqLine2.at(0).q[j + 1];
          NextWp = tmp1;
          flag = FindNextLineStart2(pt0, pt1, NextWp);
        } else {
          bool isPrevExist = false;
          MyPoint prevWp;
          if (dqWpEveryTime.size() > 1) {
            getPrevPoint(dqWpEveryTime, isPrevExist, LastWp, prevWp);
          } else {
            getPrevPoint(dqWp, isPrevExist, LastWp, prevWp);
          }
          flag = FindNextLineStart(p, n, pt0, pt1, LastWp, prevWp, isPrevExist);
        }
        if (flag) {
          FindMiddlePointOfNeighboringPoint(p, n, LastWp, pt0, dqWpEveryTime);
        } else {
          FindMiddlePointOfNeighboringPoint(p, n, LastWp, pt1, dqWpEveryTime);
        }
      }
      if (flag) {
        dqWpEveryTime.push_back(pt0);
        dqWpEveryTime.push_back(pt1);
      } else {
        dqWpEveryTime.push_back(pt1);
        dqWpEveryTime.push_back(pt0);
      }
    }
  }
}

/* ��ȡ����ֱ�߷����е�C */
void getCm(double &Cm, int k, int loopTimes, double A, double B, MyPoint MaxD,
           MyPoint ReferNearestPoint, double alaph, double d,
           Line refer_border) {
  double sqrtAB = sqrt(A * A + B * B);
  double CmL, CmR;

  CmL = -A * refer_border.p0.x - B * refer_border.p0.y +
        (alaph * d + loopTimes * d) * sqrtAB;
  CmR = -A * refer_border.p0.x - B * refer_border.p0.y -
        (alaph * d + loopTimes * d) * sqrtAB;
  //����ReferNearestPointΪ0.5d�ĵط���ƽ���ߣ���ô����MaxD �Ͻ���ֱ��Ϊ����
  if (fabs(A * MaxD.x + B * MaxD.y + CmL) <=
      fabs(A * MaxD.x + B * MaxD.y + CmR)) {
    Cm = CmL;
  } else {
    Cm = CmR;
  }
}

/* ��ȡ����ֱ�߷����е�C */
void getLastCm(double &Cm, int k, int loopTimes, double A, double B,
               MyPoint MaxD, MyPoint ReferNearestPoint, double alpha, double d,
               Line refer_border) {
  double sqrtAB = sqrt(A * A + B * B);
  double CmL, CmR;

  CmL = -A * refer_border.p0.x - B * refer_border.p0.y +
        (alpha * d + loopTimes * d) * sqrtAB;
  CmR = -A * refer_border.p0.x - B * refer_border.p0.y -
        (alpha * d + loopTimes * d) * sqrtAB;
  //����ReferNearestPointΪ0.5d�ĵط���ƽ���ߣ���ô����MaxD �Ͻ���ֱ��Ϊ����
  if (fabs(A * MaxD.x + B * MaxD.y + CmL) <=
      fabs(A * MaxD.x + B * MaxD.y + CmR)) {
    Cm = CmL;
  } else {
    Cm = CmR;
  }
}

/* ��ȡ����ֱ�߷����е�C */
void getLastCm(double &Cm, double A, double B, MyPoint MaxD) {
  Cm = -A * MaxD.x - B * MaxD.y;
}

/* ��ȡ����ֱ�߷����е�C */
void getFirstCm(double &Cm, unsigned int loopTimes, double A, double B,
                MyPoint MaxD, MyPoint ReferNearestPoint, double d1, double d2,
                unsigned int k1, unsigned int k2, Line refer_border) {
  double sqrtAB = sqrt(A * A + B * B);
  double CmL;
  double CmR;
  if (loopTimes < k1) {
    CmL = -A * ReferNearestPoint.x - B * ReferNearestPoint.y +
          (0.55 * d1 + loopTimes * d1) * sqrtAB;
    CmR = -A * ReferNearestPoint.x - B * ReferNearestPoint.y -
          (0.55 * d1 + loopTimes * d1) * sqrtAB;
  } else {
    CmL = -A * refer_border.p0.x - B * refer_border.p0.y +
          (0.55 * d2 + (loopTimes - k1) * d2) * sqrtAB;
    CmR = -A * refer_border.p0.x - B * refer_border.p0.y -
          (0.55 * d2 + (loopTimes - k1) * d2) * sqrtAB;
  }
  if (fabs(A * MaxD.x + B * MaxD.y + CmL) <=
      fabs(A * MaxD.x + B * MaxD.y + CmR)) {
    Cm = CmL;
  } else {
    Cm = CmR;
  }
}

/* ��ȡ����ֱ�߷����е�C */
void getCmSpecial(double &Cm, unsigned int loopTimes, double A, double B,
                  MyPoint MaxD, MyPoint ReferNearestPoint, double d1, double d2,
                  unsigned int k1, unsigned int k2, Line refer_border) {
  double sqrtAB = sqrt(A * A + B * B);
  double CmL;
  double CmR;
  if (k1 == 0 && loopTimes == 0) {
    CmL = -A * refer_border.p0.x - B * refer_border.p0.y +
          (0.52 * d1 + loopTimes * d1) * sqrtAB;
    CmR = -A * refer_border.p0.x - B * refer_border.p0.y -
          (0.52 * d1 + loopTimes * d1) * sqrtAB;

  } else {
    if (loopTimes < k1) {
      //����ReferNearestPointΪ0.5d�ĵط���ƽ���ߣ���ô����MaxD �Ͻ���ֱ��Ϊ����
      CmL = -A * ReferNearestPoint.x - B * ReferNearestPoint.y +
            (0.5 * d1 + loopTimes * d1) * sqrtAB;
      CmR = -A * ReferNearestPoint.x - B * ReferNearestPoint.y -
            (0.5 * d1 + loopTimes * d1) * sqrtAB;
    } else {
      //ʹ��0.55��ԭ���Ǳ��⺽�������ߺ�ı��غ϶����º��߹滮����
      CmL = -A * refer_border.p0.x - B * refer_border.p0.y +
            (0.55 * d2 + (loopTimes - k1) * d2) * sqrtAB;
      CmR = -A * refer_border.p0.x - B * refer_border.p0.y -
            (0.55 * d2 + (loopTimes - k1) * d2) * sqrtAB;
    }
  }
  if (fabs(A * MaxD.x + B * MaxD.y + CmL) <=
      fabs(A * MaxD.x + B * MaxD.y + CmR)) {
    Cm = CmL;
  } else {
    Cm = CmR;
  }
}

/*
Ϊ�˽����һ���ߴ���©�����������˴���
*/
void getIntersetion(double &Cm, double A, double B, MyPoint MaxD,
                    MyPoint ReferNearestPoint, double d1, double d2, double d,
                    unsigned int k1, unsigned int k2, Line refer_border,
                    bool isMiddle, bool isScaled, int j, MyPoint *pt,
                    MyPoint *pIntersection, unsigned int *num, MyPoint *p,
                    int n, int k) {
  if (isMiddle) {
    getCmSpecial(Cm, j, A, B, MaxD, ReferNearestPoint, d1, d2, k1, k2,
                 refer_border);
    get_parallel_line3(A, B, Cm, pt, refer_border);
    get_line_polygon_intersetion(p, n, pt[0], pt[1], pIntersection, num);
    if ((0 == j) && (*num == 2)) {
      double dist1 = dist_Point_to_Point(refer_border.p0, refer_border.p1);
      double dist2 = dist_Point_to_Point(pIntersection[0], pIntersection[1]);
      if (dist2 < 0.7 * dist1) {
        getFirstCm(Cm, j, A, B, MaxD, ReferNearestPoint, d1, d2, k1, k2,
                   refer_border);
        get_parallel_line3(A, B, Cm, pt, refer_border);
        get_line_polygon_intersetion(p, n, pt[0], pt[1], pIntersection, num);
      }
    }
  } else {
    getCm(Cm, k, j, A, B, MaxD, refer_border.p0, 0.4, d, refer_border);
    get_parallel_line3(A, B, Cm, pt, refer_border);
    if (j == 0 && isScaled) {
      pIntersection[0] = NewReferBorder.p0;
      pIntersection[1] = NewReferBorder.p1;
      *num = 2;
    } else if (j == (k - 1)) {
      get_line_polygon_intersetion(p, n, pt[0], pt[1], pIntersection, num);
      if (*num < 2) {
        getLastCm(Cm, k, j, A, B, MaxD, refer_border.p0, 0.4, d, refer_border);
        get_parallel_line3(A, B, Cm, pt, refer_border);
        get_line_polygon_intersetion(p, n, pt[0], pt[1], pIntersection, num);
      }
    } else
      get_line_polygon_intersetion(p, n, pt[0], pt[1], pIntersection, num);
  }
}

/*
Ϊ�˽����һ���ߴ���©�����������˴���
*/
void getIntersetionNew(double &Cm, double A, double B, MyPoint MaxD, double d,
                       Line refer_border, int j, MyPoint *pt,
                       MyPoint *pIntersection, unsigned int *num, MyPoint *p,
                       int n, int k) {
  get_parallel_line3(A, B, Cm, pt, refer_border);
  get_line_polygon_intersetion(p, n, pt[0], pt[1], pIntersection, num);
  if ((*num < 2) && (j == k - 1)) /* ���һ���߱Ƚ����� */
  {
    getLastCm(Cm, A, B, MaxD);
    get_parallel_line3(A, B, Cm, pt, refer_border);
    get_line_polygon_intersetion(p, n, pt[0], pt[1], pIntersection, num);
  }
}

/*
�����������㷨�������㷨������ֵΪ0�����㷨(���������еؿ�)��
����ֵΪ1�����㷨(�����ڵ����ݵؿ飬��������±����㷨����)
*/
int concave_handler_determine(T_stRouteLineArgs Args, MyPoint *p, int n,
                              bool isMiddle, bool isScaled) {
  MyPoint pt[2];                     //���ߵ�ֱ�߷���
  MyPoint pIntersection[MAX_WP_NUM]; //���ߵ�����
  int j = 0;
  int k = Args.k;
  int k1 = Args.k1;
  int k2 = Args.k2;
  unsigned int line_point_num = 0; //ֱ�������ν������(�����ߵ����)
  double Cm;
  double A = Args.A;
  double B = Args.B;
  double d = Args.d;
  double d1 = Args.d1;
  double d2 = Args.d2;
  Line refer_border = Args.refer_border;
  MyPoint MaxD = Args.MaxD;
  MyPoint ReferNearestPoint = Args.ReferNearestPoint;
  while (j < k - 1) {
    getIntersetion(Cm, A, B, MaxD, ReferNearestPoint, d1, d2, d, k1, k2,
                   refer_border, isMiddle, isScaled, j, pt, pIntersection,
                   &line_point_num, p, n, k);
    if (line_point_num > 4) {
      return 0;
    }
    j++;
  }
  return 1;
}

void TwoLinePointHandler(T_stRouteLineArgs Args, MyPoint *p, int n,
                         pointDequeType &dqWp, pointDequeType &dqWpEveryTime,
                         MyPoint *pIntersection) {
  double Cm;
  bool IsSpecial;
  MyPoint ptLine[2];
  MyPoint SpecialPoint; //Ϊ������������ʶ����ӵ�waypoint���ο�aoxian5.py ��������
  Line refer_border = Args.refer_border;

  if (2 == dqWp.size()) {
    Line NearBorder;
    IsSpecial = SecondLineSpecialProcess(p, n, refer_border, NearBorder);
    if (IsSpecial) {
      double A = refer_border.p1.y - refer_border.p0.y;
      double B = refer_border.p0.x - refer_border.p1.x;
      double Cmr = 0.5 * Args.d * sqrt(A * A + B * B) - A * refer_border.p0.x -
                   B * refer_border.p0.y;
      double Cml = -0.5 * Args.d * sqrt(A * A + B * B) - A * refer_border.p0.x -
                   B * refer_border.p0.y;
      double d1 = fabs(A * Args.MaxD.x + B * Args.MaxD.y + Cml);
      double d2 = fabs(A * Args.MaxD.x + B * Args.MaxD.y + Cmr);
      if (d1 < d2) {
        Cm = Cml;
      } else {
        Cm = Cmr;
      }
      get_parallel_line(A, B, Cm, ptLine);
      d1 = dist_Point_to_Point(NearBorder.p0, dqWp.back());
      d2 = dist_Point_to_Point(NearBorder.p1, dqWp.back());
      if (d1 > d2)
        SpecialPoint = NearBorder.p0;
      else
        SpecialPoint = NearBorder.p1;
      double VectorAngle =
          getAngelOfTwoVector(dqWp.at(0), SpecialPoint, dqWp.at(1));
      MyPoint MiddlePoint = getMidPoint(dqWp.at(1), SpecialPoint);
      if (cn_PnPoly(MiddlePoint, p, n)) {
        /* �ο�2018050202.py ��aoxian12.py ���� */
        if (VectorAngle > 170) {
          DisplayPoint(SpecialPoint);
        }
      }
    }
  }

  if (dqWp.size() == 0) {
    enqueueTwoPointByTakeoffPoint(Args.takeOffPoint, dqWpEveryTime,
                                  pIntersection);
  } else {
    MyPoint pt0, pt1;
    pt0 = dqWp.back(); //��һ�ֺ��ߵ��յ�
    bool isPrevExist = false;
    MyPoint prevWp;
    getPrevPoint(dqWp, isPrevExist, pt0, prevWp);
    bool flag = FindNextLineStart(p, n, pIntersection[0], pIntersection[1], pt0,
                                  prevWp, isPrevExist);
    if (flag) {
      pt1 = pIntersection[0];
    } else {
      pt1 = pIntersection[1];
    }

    /* �ж���һ�κ��ߵ��յ��뱾�κ�����㹹�ɵ��߶�
    �Ƿ��밼������ཻ������ཻ����ô����ӱ��κ���
    ֮ǰ��Ҫ����Ӱ���*/
    FindMiddlePointOfNeighboringPoint(p, n, pt0, pt1, dqWpEveryTime);
    if (dqWp.size() > 0) {
      enqueueTwoPoint(flag, dqWpEveryTime, pIntersection);
    }
  }
}

//�Ƚ�����Point��x����
bool operator<(const P2PDistance &a, const P2PDistance &b) {
  return a.d1 < b.d1;
}

void addLastWayPointWhenNeeded(MyPoint *p, int n, PointFlyAttr *wp, int m,
                               pointDequeType &dqWp) {
  MyPoint last;
  LeakSprayInfo tLeakSprayInfo;
  tLeakSprayInfo.fieldArea = gLeakSprayInfo.fieldArea;
  CalSprayAreaInfo(tLeakSprayInfo, OriginArgs::instance().d, wp, m);
  if (tLeakSprayInfo.leakPercent > 8) {
    if (dqWp.size() < 4)
      return;
    if (dqWp.size() == 4) {
      P2PDistance ptDis;
      vector<P2PDistance> vDistance;
      for (int i = 0; i < n; i++) {
        ptDis.d1 = dist_Point_to_Point(dqWp.at(m - 4), p[i]);
        ptDis.pt = p[i];
        vDistance.push_back(ptDis);
      }
      std::sort(vDistance.begin(), vDistance.end());
      last = vDistance.at(1).pt;
      dqWp.push_back(last);
    } else {
      MyPoint ptNeeded1, ptNeeded2;
      Line l;
      l.p0 = dqWp.at(m - 1);
      l.p1 = dqWp.at(m - 2);
      calTwoPointOnPrepLine(dqWp.at(m - 4), dqWp.at(m - 3), ptNeeded1,
                            ptNeeded2, 0.9 * OriginArgs::instance().d);
      double d1 = dist_Point_to_Line(ptNeeded1, l);
      double d2 = dist_Point_to_Line(ptNeeded2, l);
      if (d1 < d2)
        last = ptNeeded1;
      else
        last = ptNeeded2;
      int tmp = cn_PnPoly(last, (MyPoint *)OriginArgs::instance().p,
                          OriginArgs::instance().n);
      if (tmp == 1)
        dqWp.push_back(last);
    }
  }
}

/*
concave_handler_old: ͨ�õİ�����εؿ鴦�����������߲�һ�����ţ�
�����ܽ�����а�����εؿ�ĺ��߹滮
MaxD: ��ο�����Զ�Ķ��㵽�ο��ߵľ���
*/
void concave_handler_old(T_stRouteLineArgs Args, MyPoint *p, int n,
                         bool isMiddle, bool isScaled, pointDequeType &dqWp,
                         pointDequeType &dqWpEveryTime) {
  MyPoint pt[2];                     //���ߵ�ֱ�߷���
  MyPoint pIntersection[MAX_WP_NUM]; //���ߵ�����
  int j = 0;
  int k = Args.k;
  int k1 = Args.k1;
  int k2 = Args.k2;
  unsigned int line_point_num = 0; //ֱ�������ν������(�����ߵ����)
  double Cm;
  double A = Args.A;
  double B = Args.B;
  double d = Args.d;
  double d1 = Args.d1;
  double d2 = Args.d2;
  Line refer_border = Args.refer_border;
  MyPoint ReferNearestPoint = Args.ReferNearestPoint;

#if DEBUG
  cout << "concave_handler_old" << endl;
#endif
  while (j < k) {
    if (dqWpEveryTime.size()) {
      dqWpEveryTime.clear();
    }
    getIntersetion(Cm, A, B, Args.MaxD, ReferNearestPoint, d1, d2, d, k1, k2,
                   refer_border, isMiddle, isScaled, j, pt, pIntersection,
                   &line_point_num, p, n, k);
#if DEBUG
    cout << "line_point_num=" << line_point_num << endl;
#endif
    std::sort(pIntersection, pIntersection + line_point_num);

    if (0 == line_point_num) {
      j++; //����2718t180607104748�����3.5m�����Ӵ���������ĳ���ο��ߵ�����ѭ��
      continue;
    } else if (2 == line_point_num) {
      TwoLinePointHandler(Args, p, n, dqWp, dqWpEveryTime, pIntersection);
    } else {
      if (dqWp.size() > 0) {
        bool flag =
            NextLineNeedReverse(p, n, pIntersection[0],
                                pIntersection[line_point_num - 1], dqWp.back());
        if (!flag) {
          std::reverse(pIntersection, pIntersection + line_point_num);
        }
      }
      MultiLinePointHandler(p, n, pIntersection, line_point_num, dqWp,
                            dqWpEveryTime);
    }

    if (dqWpEveryTime.size() > 0) {
      for (pointDequeType::iterator it = dqWpEveryTime.begin();
           it != dqWpEveryTime.end(); it++) {
        dqWp.push_back(*it);
      }
    }
    j++;
  }
  // addLastWayPointWhenNeeded(p, n, dqWp);
}

/*
MaxD: ��ο�����Զ�Ķ��㵽�ο��ߵľ���
isScaled : �ؿ������Ƿ񾭹�����
*/
void concave_handler(T_stRouteLineArgs Args, MyPoint *p, int n, bool isMiddle,
                     bool isScaled, pointDequeType &dqWp,
                     pointDequeType &dqWpEveryTime) {
  MyPoint pt[2];                     //���ߵ�ֱ�߷���
  MyPoint pIntersection[MAX_WP_NUM]; //���ߵ�����
  int j = 0;
  int k = Args.k;
  int k1 = Args.k1;
  int k2 = Args.k2;
  unsigned int line_point_num = 0; //ֱ�������ν������(�����ߵ����)
  unsigned int last_line_point_num = 0; //ֱ�������ν������(�����ߵ����)
  double Cm;
  double A = Args.A;
  double B = Args.B;
  double d = Args.d;
  double d1 = Args.d1;
  double d2 = Args.d2;
  Line refer_border = Args.refer_border;
  MyPoint MaxD = Args.MaxD;
  MyPoint ReferNearestPoint = Args.ReferNearestPoint;
  bool NeedReverse = false;
  bool NeedReverse2 = false;
  line2DequeType dqLine2; //��ʱ���ֱ���������ཻ����2�������waypoint

  while (j < k) {
    last_line_point_num = line_point_num;
    /* ���ο���λ�ڵؿ�����ʱ�ĵ�һ�����߿��ܱȽ����⣬��Ҫר��
     * ����*/
    if (j == 0 && isMiddle == true) {
      getIntersetion(Cm, A, B, MaxD, ReferNearestPoint, d1, d2, d, k1, k2,
                     refer_border, isMiddle, isScaled, j, pt, pIntersection,
                     &line_point_num, p, n, k);
      std::sort(
          pIntersection,
          pIntersection +
              line_point_num); //�Ե����򣬱��ں�������
      dqWpEveryTime.push_back(pIntersection[0]);
      if (line_point_num > 1) {
        FindShortestPathOfTwoPoint(p, n, pIntersection[0],
                                   pIntersection[line_point_num - 1],
                                   dqWpEveryTime);
        dqWpEveryTime.push_back(pIntersection[line_point_num - 1]);
      }
      if (dqWpEveryTime.size() > 0) {
        double d1 =
            dist_Point_to_Point(dqWpEveryTime.front(), Args.takeOffPoint);
        double d2 =
            dist_Point_to_Point(dqWpEveryTime.back(), Args.takeOffPoint);
        if (d1 > d2) {
          std::reverse(dqWpEveryTime.begin(), dqWpEveryTime.end());
          dqWpEveryTime.pop_front();
        }
      }
      goto END;
    } else {
      getIntersetion(Cm, A, B, MaxD, ReferNearestPoint, d1, d2, d, k1, k2,
                     refer_border, isMiddle, isScaled, j, pt, pIntersection,
                     &line_point_num, p, n, k);
      std::sort(
          pIntersection,
          pIntersection +
              line_point_num); //�Ե����򣬱��ں�������
    }
    if (j == 0 && line_point_num > 0) {
      double d1 = dist_Point_to_Point(pIntersection[0], Args.takeOffPoint);
      double d2 = dist_Point_to_Point(pIntersection[line_point_num - 1],
                                      Args.takeOffPoint);
      if (d2 < d1) {
        reverse_array(pIntersection, line_point_num);
      }
    }

    if (0 == line_point_num) {
      j++;
      continue;
    } else if (2 == line_point_num) {
      if (dqLine2.size() > 0) {
        MultiLinePointHandler2(p, n, dqLine2, dqWp, dqWpEveryTime);
        dqLine2.clear();
      }
      TwoLinePointHandler2(p, n, pIntersection, Args.takeOffPoint, dqWp,
                           dqWpEveryTime);
    } else {
      if (j == (k - 1)) {
        if (dqLine2.size() > 0) {
          MultiLinePointHandler2(p, n, dqLine2, dqWp, dqWpEveryTime);
          dqLine2.clear();
          TwoLinePointHandler2(p, n, pIntersection, Args.takeOffPoint, dqWp,
                               dqWpEveryTime);
        } else {
          if (dqWp.size() > 0) {
            double d1 = dist_Point_to_Point(pIntersection[0], dqWp.back());
            double d2 = dist_Point_to_Point(pIntersection[line_point_num - 1],
                                            dqWp.back());
            if (d2 < d1) {
              NeedReverse = true;
            }
            if (NeedReverse) {
              reverse_array(pIntersection, line_point_num);
            }
          }
          MultiLinePointHandler(p, n, pIntersection, line_point_num, dqWp,
                                dqWpEveryTime);
        }
      } else {
        Line2 Line2Tmp;
        Line2 *pLine2Tmp = (Line2 *)&Line2Tmp;
        /* �� line_point_num ��С���ʱ����Ҫ����line_point_num˳��
        ����·�߲���ȷ*/
        if ((last_line_point_num < line_point_num) && (dqWp.size() > 0)) {
          int MinDistanceIndex = 0;
          double dt = 0.0;
          double MinDistance =
              dist_Point_to_Point(pIntersection[0], dqWp.back());
          for (int i = 1; i < 4; i++) {
            dt = dist_Point_to_Point(pIntersection[i], dqWp.back());
            if (dt < MinDistance) {
              MinDistance = dt;
              MinDistanceIndex = i;
            }
          }
          if (MinDistanceIndex > 1) {
            NeedReverse2 = true;
          } else {
            NeedReverse2 = false;
          }
        }
        if (NeedReverse2) {
          reverse_array(pIntersection, line_point_num);
        }
        memcpy(&pLine2Tmp->q[0], pIntersection,
               line_point_num * sizeof(MyPoint));
        pLine2Tmp->TotalPointNum = line_point_num;
        dqLine2.push_back(Line2Tmp);
      }
    }
  END:
    if (dqWpEveryTime.size() > 0) {
      for (pointDequeType::iterator it = dqWpEveryTime.begin();
           it != dqWpEveryTime.end(); it++) {
        dqWp.push_back(*it);
      }
      dqWpEveryTime.clear();
    }
    j++;
  }
  if (dqWpEveryTime.size() > 0) {
    for (pointDequeType::iterator it = dqWpEveryTime.begin();
         it != dqWpEveryTime.end(); it++) {
      dqWp.push_back(*it);
    }
  }
}

/*
MaxD: ��ο�����Զ�Ķ��㵽�ο��ߵľ���
*/
void subBlockHandler(T_stRouteLineArgs Args, const MyPoint *pOrigin,
                     const int nOrigin, MyPoint *p, int n, pointDequeType &dqWp,
                     pointDequeType &dqWpEveryTime) {
  MyPoint pt[2]; //����
  MyPoint pIntersection[20];
  unsigned int num = 0;
  int j = 0;
  double Cm;

  while (j < Args.k) {
    /* ���������ؿ�*/
    if (Args.k == 1) {
      double Cml, Cmr;
      Cml = -Args.A * Args.refer_border.p0.x - Args.B * Args.refer_border.p0.y -
            0.5 * Args.dmax * sqrt(Args.A * Args.A + Args.B * Args.B);
      Cmr = -Args.A * Args.refer_border.p0.x - Args.B * Args.refer_border.p0.y +
            0.5 * Args.dmax * sqrt(Args.A * Args.A + Args.B * Args.B);
      double d1 = fabs(Args.A * Args.MaxD.x + Args.B * Args.MaxD.y + Cml);
      double d2 = fabs(Args.A * Args.MaxD.x + Args.B * Args.MaxD.y + Cmr);
      if (d1 < d2) {
        Cm = Cml;
      } else {
        Cm = Cmr;
      }
      get_parallel_line(Args.A, Args.B, Cm, pt);
      get_line_polygon_intersetion(p, n, pt[0], pt[1], pIntersection, &num);
    } else {
      if (j == 0) {
        getCm(Cm, Args.k, j, Args.A, Args.B, Args.MaxD, Args.refer_border.p0,
              0.5, Args.d, Args.refer_border);
      } else {
        getCm(Cm, Args.k, j, Args.A, Args.B, Args.MaxD, Args.refer_border.p0,
              0.5, Args.d, Args.refer_border);
      }
      getIntersetionNew(Cm, Args.A, Args.B, Args.MaxD, Args.d,
                        Args.refer_border, j, pt, pIntersection, &num, p, n,
                        Args.k);
    }
    if (0 == dqWp.size()) {
      if (num == 0) {
        getCm(Cm, Args.k, j, Args.A, Args.B, Args.MaxD, Args.refer_border.p0,
              0.5, Args.d, Args.refer_border);
        getIntersetionNew(Cm, Args.A, Args.B, Args.MaxD, Args.d,
                          Args.refer_border, j, pt, pIntersection, &num, p, n,
                          Args.k);
      }
      enqueueTwoPointByTakeoffPoint(Args.takeOffPoint, dqWp, pIntersection);
    } else {
      /* ��Last waypoint����ͬһ���ߵĵ������ */
      bool isPrevExist = false;
      MyPoint prevWp;
      pointDequeType dqWpEveryTime;
      getPrevPoint(dqWp, isPrevExist, dqWp.back(), prevWp);
      bool ret = false;
      bool flag = FindNextLineStart(p, n, pIntersection[0], pIntersection[1],
                                    dqWp.back(), prevWp, isPrevExist);
      if (num && flag) {
        ret = intersect2D_SegPoly3((MyPoint *)pOrigin, nOrigin, dqWp.back(),
                                   pIntersection[0], 0);
        if (ret) {
          FindShortestPathOfTwoPoint(p, n, dqWp.back(), pIntersection[0],
                                     dqWpEveryTime);
          for (unsigned int i = 0; i < dqWpEveryTime.size(); i++) {
            dqWp.push_back(dqWpEveryTime.at(i));
          }
        }
        dqWp.push_back(pIntersection[0]);
        dqWp.push_back(pIntersection[1]);
      } else if (num == 0) {
        // do nothing
      } else {
        ret = intersect2D_SegPoly3((MyPoint *)pOrigin, nOrigin, dqWp.back(),
                                   pIntersection[1], 0);
        if (ret) {
          FindShortestPathOfTwoPoint(p, n, dqWp.back(), pIntersection[1],
                                     dqWpEveryTime);
          for (unsigned int i = 0; i < dqWpEveryTime.size(); i++) {
            dqWp.push_back(dqWpEveryTime.at(i));
          }
        }
        dqWp.push_back(pIntersection[1]);
        dqWp.push_back(pIntersection[0]);
      }
    }
    j++;
  }
}

/*
�жϵ�ǰ���Ƿ���N���κ��ߵĹյ�
*/
bool isNShapePoint(MyPoint ptPrev, MyPoint ptCurr, MyPoint ptNext,
                   MyPoint ptNextNext) {
  double VectorAngle1 = getAngelOfTwoVector(ptNext, ptPrev, ptCurr);
  double VectorAngle2 = getAngelOfTwoVector(ptCurr, ptNextNext, ptNext);
  if ((VectorAngle1 < 45) && (VectorAngle2 < 45)) //С��45�ſ��ܳ���N����
  {
    if (fabs(VectorAngle1 - VectorAngle2) < 0.2) {
      return true;
    }
  }
  return false;
}

/*ѡ����һ�����ߵ���ʼ��*/
bool FindNextLineStart(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                       MyPoint curWp, MyPoint prevWp, bool isPrevExist) {
  int i = 0;
  Segment S;
  bool flag = true;
  bool OnBorder = false;

  for (i = 0; i < n; i++) {
    S.p0 = p[i];
    S.p1 = p[(i + 1) % n];
    if (onSegment(S.p0, S.p1, curWp)) {
      OnBorder = true;
      break;
    }
  }
  /* ����ѡ����last waypoint����ͬһ���ߵĵ㣬���û��
  ��ѡ����last waypoint���ĵ���Ϊ��һ��waypoint*/
  if (OnBorder && onSegment(S.p0, S.p1, pt1)) {
    flag = false; //ѡ��pt1��Ϊ��һ��waypoint
  } else if (OnBorder && onSegment(S.p0, S.p1, pt0)) {
    flag = true; //ѡ��pt0��Ϊ��һ��waypoint
  } else {
    bool ret = FindNextLineStart2(pt0, pt1, curWp);
    flag = !ret;
    if (isPrevExist && flag) {
      /* N���κ���Ӧ�þ�����������ԭ���ǻ�����©����
       */
      bool isN1 = isNShapePoint(prevWp, curWp, pt0, pt1);
      if (isN1) {
        bool isIntersect1 = intersect2D_SegPoly(p, n, prevWp, pt1);
        if (!isIntersect1) {
          flag = false;
        }
      }
    }
  }
  return flag;
}

/*ѡ����һ�����ߵ���ʼ��*/
bool FindNextLineStart2(MyPoint pt0, MyPoint pt1, MyPoint NextWp) {
  bool flag = true;
  double d1 = dist_Point_to_Point(pt0, NextWp);
  double d2 = dist_Point_to_Point(pt1, NextWp);
  if (d1 >= d2) {
    flag = true;
  } else {
    flag = false;
  }
  return flag;
}

/* �ж���һ�����ߵĺ����Ƿ���Ҫ���� */
bool NextLineNeedReverse(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                         MyPoint lastWp) {
  int i = 0;
  Segment S;

  for (i = 0; i < n; i++) {
    S.p0 = p[i];
    S.p1 = p[(i + 1) % n];
    if (onSegment(S.p0, S.p1, lastWp))
      break;
  }
  if (onSegment(S.p0, S.p1, pt0)) {
    return true;
  } else if (onSegment(S.p0, S.p1, pt1)) {
    return false;
  } else {
    double d1 = dist_Point_to_Point(pt0, lastWp);
    double d2 = dist_Point_to_Point(pt1, lastWp);
    if (d1 <= d2) {
      return true;
    } else {
      return false;
    }
  }
}

/*
dmax�Ƕ���ζ����о���ο��ߵ���Զ����
*/
void get_index_max(Line refer_border, MyPoint *p, int n, int &index_max,
                   double &dmax) {
  int i;
  double tmp = 0.0;

  for (i = 0; i < n; i++) {
    if (p[i] == refer_border.p0 || p[i] == refer_border.p1) {
      continue;
    }

    tmp = dist_Point_to_Line(p[i], refer_border);
    if (dmax < tmp) {
      dmax = tmp;
      index_max = i;
    }
  }
}

//�ο��߲��ǰ���ʱ��������㺯��
// MaxD : ����ο�����Զ�Ķ���
void getTrueDmaxNormal(MyPoint *p, int n, unsigned int &k, double &d,
                       MyPoint &MaxD, MyPoint &ReferNearestPoint, double &dmax,
                       Line refer_border) {
  int i = 0;
  double dmaxt = 0.0;
  double originD = d;
  // cout<<"origin d ="<<d<<endl;

  /* �ȼ��� MaxD��refer_border�ϵ�ͶӰ�㣬�ɸ�ͶӰ����MaxD���
  һ��ֱ�ߣ�Ȼ�������������ڸ�ֱ���ϵ�ͶӰ�㣬����
  MaxD
  ��Զ��ͶӰ��Ϊ������������ԭ���Ƿ�ֹ�ο����ڵؿ�
  �м�ʱֱ�Ӽ���MaxD���ο��ߵľ���ᵼ�µؿ���һ��©��*/
  MyPoint MaxDProj; /* MaxD��refer_border�ϵ�ͶӰ��*/
  MyPoint TmpProj;
  MyPoint TheFarthestPoint; /*����MaxD ��Զ�ĵ� */
  MyPoint TheFarthestPointProj; /*����MaxD ��Զ�ĵ� ��ͶӰ��*/
  MaxDProj = getProjectedPointOnLine(MaxD, refer_border.p0, refer_border.p1);
  for (i = 0; i < n; i++) {
    TmpProj = getProjectedPointOnLine(p[i], MaxD, MaxDProj);
    if (dist_Point_to_Point(MaxD, TmpProj) > dmaxt) {
      dmaxt = dist_Point_to_Point(MaxD, TmpProj);
      TheFarthestPoint = p[i];
      TheFarthestPointProj = TmpProj;
    }
  }
  dmax = dmaxt + 0.8 * d;
  k = dmax / d;
  double diff = dmax - k * d;
  if (diff > 0.5 * d)
    k = k + 1;
  if (k == 0)
    k = 1;
  d = dmax / k;
  if ((k < 5) && (originD - d) > 0.3 * d) {
    k = k - 1;
    d = dmax / k;
  }
  if ((k < 5) && (d - originD) > 0.1) {
    d = originD;
  }

  ReferNearestPoint = TheFarthestPoint;
#if DEBUG
  cout << "[getTrueDmaxNormal]k = " << k << endl;
  cout << "dmax = " << dmax << endl;
  cout << "d = " << d << endl;
  cout << "TheFarthestPoint=" << TheFarthestPoint << endl;
  cout << "TheFarthestPointProj=" << TheFarthestPointProj << endl;
  cout << "ReferNearestPoint=" << ReferNearestPoint << endl;
  cout << "adjust d =" << d << endl;
#endif
}

//�ο��߲��ǰ���ʱ��������㺯��
// MaxD : ����ο�����Զ�Ķ���
void getTrueDmaxNormal2(MyPoint *p, int n, unsigned int &k, double &d,
                        MyPoint &MaxD, double &dmax, Line refer_border) {
  deque<MyPoint> dqTmp;
  for (int i = 0; i < n; i++) {
    dqTmp.push_back(p[i]);
  }
  std::sort(dqTmp.begin(), dqTmp.end());
  dmax = dqTmp.back().x - dqTmp.front().x + 0.2 * d;
  k = ceil(dmax / d);
  if (k > 1) {
    if ((dmax - (k - 1) * d) <
        1.0) // for example , 24.06 (d is 3)only need 8 lines
    {
      k = k - 1;
    }
  }
  if (k == 0)
    k = 1;
  d = dmax / k;
#if DEBUG
  cout << "[getTrueDmaxNormal2]k = " << k << endl;
  cout << "dmax = " << dmax << endl;
  cout << "d = " << d << endl;
  cout << "adjust d =" << d << endl;
#endif
}

//�ο����ǰ���ʱ��������㺯��
// MaxD: ����ο�����Զ�Ķ���
void getTrueDmaxSpecial(MyPoint *p, int n, unsigned int &k1, unsigned int &k2,
                        double &d, double &d1, double &d2, MyPoint &MaxD,
                        MyPoint &ReferNearestPoint, double &dmax,
                        Line refer_border) {
  int i = 0;
  double dmaxt = 0.0;
  double originD = d;
  double dt;
  /* �ȼ��� MaxD��refer_border�ϵ�ͶӰ�㣬�ɸ�ͶӰ����MaxD���
          һ��ֱ�ߣ�Ȼ�������������ڸ�ֱ���ϵ�ͶӰ�㣬����
          MaxD ��Զ��ͶӰ��Ϊ����*/
  MyPoint MaxDProj; /* MaxD��refer_border�ϵ�ͶӰ��*/
  MyPoint TmpProj;
  MyPoint TheFarthestPoint; /*����MaxD ��Զ�ĵ� */
  MyPoint TheFarthestPointProj; /*����MaxD ��Զ�ĵ� ��ͶӰ��*/

  //  cout<<"[getTrueDmaxSpecial]origin d="<<d<<endl;
  MaxDProj = getProjectedPointOnLine(MaxD, refer_border.p0, refer_border.p1);
  for (i = 0; i < n; i++) {
    TmpProj = getProjectedPointOnLine(p[i], MaxD, MaxDProj);
    if (dist_Point_to_Point(MaxD, TmpProj) > dmaxt) {
      dmaxt = dist_Point_to_Point(MaxD, TmpProj);
      TheFarthestPoint = p[i];
      TheFarthestPointProj = TmpProj;
    }
  }
  dmax = dmaxt;
#if DEBUG
  cout << "[getTrueDmaxSpecial]dmax = " << dmax << endl;
  cout << "TheFarthestPoint=" << TheFarthestPoint << endl;
  cout << "TheFarthestPointProj=" << TheFarthestPointProj << endl;
#endif
  if (dmax < d) {
    k1 = 1;
    k2 = 0;
    d1 = originD;
    d2 = originD;
    return;
  }

  ReferNearestPoint = TheFarthestPoint;
  dt = dist_Point_to_Line(ReferNearestPoint, refer_border);
  if (dt < 0.5 * d) {
    k1 = 0;
  } else if (dt >= 0.5 * d && dt < d) {
    k1 = 1;
    d1 = originD;
  } else {
    k1 = ceil(dt / originD);
    if (k1 > 2) {
      double diff = fabs(dt - (k1 - 1) * originD);
      if (diff < 0.5 * originD) {
        k1 = k1 - 1;
      }
    }
    d1 = dt / k1;
  }
#if DEBUG
  cout << "k1 = " << k1 << endl;
  cout << "[getTrueDmaxSpecial]adjust wide d1 = " << d1 << endl;
#endif
  dt = dist_Point_to_Line(MaxD, refer_border);
  if (dt < d) {
    k2 = 1;
    d2 = originD;
  } else {
    k2 = ceil(dt / originD);
    d2 = dt / k2;
  }
  if (k1 > k2) {
    d = d1;
  } else {
    d = d2;
  }
#if DEBUG
  cout << "k2 = " << k2 << endl;
  cout << "[getTrueDmaxSpecial] adjust wide d2 = " << d2 << endl;
#endif
}

/*
��ȡ�����������ο��߶�Ӧ�ıߣ�����µĲο��߿���Ϊ
��һ������
*/
void getNewReferBorder(const MyPoint *p, int n, double d, Line refer_border) {
  MyPoint InterEnvelope[MAX_WP_NUM]; //����ε��ڰ��磬����ɨ��
  int t;
  int index1 = 0, index2 = 0;
  get_envelope_of_polygen((MyPoint *)p, n, InterEnvelope, &t, 0.4 * d, 0);
  for (int i = 0; i < n; i++) {
    if (refer_border.p0 == p[i]) {
      index1 = i;
    }
    if (refer_border.p1 == p[i]) {
      index2 = i;
    }
  }
  NewReferBorder.p0 = InterEnvelope[index1];
  NewReferBorder.p1 = InterEnvelope[index2];
#if DEBUG
  cout << "NewReferBorder:" << endl;
  DisplayPoint(NewReferBorder.p0);
  DisplayPoint(NewReferBorder.p1);
#endif
}

/*
���������Ƿ�����
*/
// TODO:
// ���ڰ����Ƿ������������⣬�����������׼ȷ�ж��������
bool ConcaveSprayDetermine(T_stRouteLineArgs Args, MyPoint *p, int n,
                           bool isMiddle, bool isScaled) {
  MyPoint pt[2];                     //���ߵ�ֱ�߷���
  MyPoint pIntersection[MAX_WP_NUM]; //���ߵ�����
  int j = 0;
  int k = Args.k;
  int k1 = Args.k1;
  int k2 = Args.k2;
  unsigned int line_point_num = 0; //ֱ�������ν������(�����ߵ����)
  double Cm;
  double A = Args.A;
  double B = Args.B;
  double d = Args.d;
  double d1 = Args.d1;
  double d2 = Args.d2;
  Line refer_border = Args.refer_border;
  MyPoint MaxD = Args.MaxD;
  MyPoint ReferNearestPoint = Args.ReferNearestPoint;
  int ContinuousCount =
      0; //�������ߵ��������2ʱ��������°��߲��ܿ����������������������
  while (j < k - 1) {
    getIntersetion(Cm, A, B, MaxD, ReferNearestPoint, d1, d2, d, k1, k2,
                   refer_border, isMiddle, isScaled, j, pt, pIntersection,
                   &line_point_num, p, n, k);
    if (line_point_num > 2) {
      ContinuousCount++;
    } else {
      ContinuousCount = 0;
    }
    if (ContinuousCount > 1) {
      break;
    } else {
      j++;
    }
  }
  if (ContinuousCount > 1) {
    gIsConcaveSpray = false;
  } else {
    gIsConcaveSpray = true;
  }
  return gIsConcaveSpray;
}

void CalSprayAreaInfo(LeakSprayInfo &tLeakSprayInfo, double d, PointFlyAttr *wp,
                      int m) {
  double totalSprayDistance = 0.0;
  double totalFlyDistance = 0.0;

  for (int i = 0; i < m; i++) {
    if (i > 0) {
      totalFlyDistance += dist_Point_to_Point(wp[i - 1].pt, wp[i].pt);
    }
    if ((i > 0) && (wp[i - 1].SprayCtrl == 1))
      totalSprayDistance += dist_Point_to_Point(wp[i - 1].pt, wp[i].pt);
  }

  tLeakSprayInfo.flyDistance = totalFlyDistance;
  tLeakSprayInfo.sprayArea = d * totalSprayDistance;
  tLeakSprayInfo.leakArea = tLeakSprayInfo.fieldArea - tLeakSprayInfo.sprayArea;
  tLeakSprayInfo.leakPercent =
      (tLeakSprayInfo.leakArea / tLeakSprayInfo.fieldArea) * 100;
#if DEBUG
  cout << "[CalSprayAreaInfo]totalSprayDistance=" << totalSprayDistance << endl;
  cout << "d=" << d << endl;
  cout << "m=" << m << endl;
  cout << "flyDistance=" << tLeakSprayInfo.flyDistance << endl;
  cout << "sprayArea=" << tLeakSprayInfo.sprayArea << endl;
  cout << "fieldArea=" << tLeakSprayInfo.fieldArea << endl;
  cout << "leakArea=" << tLeakSprayInfo.leakArea << endl;
  cout << "leakPercent=" << tLeakSprayInfo.leakPercent << endl;
#endif
}

//�õ�waypoint����Ҫ������������
int waypointHandler(MyPoint *p, int n, double d, int isRepeat, int uavType,
                    bool isBarrier, int SprayType, Line refer_border,
                    PointFlyAttr *wp, int *pm, pointDequeType &dqWp) {
  unsigned int k = 0;
  pointFlyAttrDequeType dqFlyWp;            //ȫ��Ψһ
  ClosePointPairDequeType dqClosePointPair; //�ٽ����
  if (dqWp.size() > 0) //ֱ�������β��ཻ���Ѿ����waypoint����ʱɨ�����
  {
    RemoveWaypointTooClose(dqWp, dqClosePointPair);
    //	waypointTooFarHandler(dqWp);
    if (isRepeat)
      sprayRepeat(dqWp);
    setWpAttr(p, n, isRepeat, uavType, refer_border, SprayType, isBarrier, dqWp,
              dqFlyWp, dqClosePointPair);
    if (dqFlyWp.size() > MAX_WP_NUM)
      return TOO_MANY_WAYPOINT_ERR;
    for (k = 0; k < dqFlyWp.size(); k++) {
      wp[k] = dqFlyWp.at(k);
    }
    *pm = k;
  }
  return 0;
}

/* ͹����εؿ鴦������ */
void convexHandler(T_stRouteLineArgs Args, MyPoint *p, int n, bool isMiddle,
                   bool isScaled, pointDequeType &dqWp) {
  MyPoint pt[2]; //����
  MyPoint pIntersection[20];
  unsigned int num = 0;
  int j = 0;
  double Cm;

  while (j < Args.k) {
    /* ���������ؿ�*/
    if (Args.k == 1) {
      double Cml, Cmr;
      Cml = -Args.A * Args.refer_border.p0.x - Args.B * Args.refer_border.p0.y -
            0.5 * Args.dmax * sqrt(Args.A * Args.A + Args.B * Args.B);
      Cmr = -Args.A * Args.refer_border.p0.x - Args.B * Args.refer_border.p0.y +
            0.5 * Args.dmax * sqrt(Args.A * Args.A + Args.B * Args.B);
      double d1 = fabs(Args.A * Args.MaxD.x + Args.B * Args.MaxD.y + Cml);
      double d2 = fabs(Args.A * Args.MaxD.x + Args.B * Args.MaxD.y + Cmr);
      if (d1 < d2) {
        Cm = Cml;
      } else {
        Cm = Cmr;
      }
      get_parallel_line(Args.A, Args.B, Cm, pt);
      get_line_polygon_intersetion(p, n, pt[0], pt[1], pIntersection, &num);
    } else {
      getIntersetion(Cm, Args.A, Args.B, Args.MaxD, Args.ReferNearestPoint,
                     Args.d1, Args.d2, Args.d, Args.k1, Args.k2,
                     Args.refer_border, isMiddle, isScaled, j, pt,
                     pIntersection, &num, p, n, Args.k);
    }
    if (0 == dqWp.size()) {
      enqueueTwoPointByTakeoffPoint(Args.takeOffPoint, dqWp, pIntersection);
    } else {
      /* ��Last waypoint����ͬһ���ߵĵ������ */
      bool isPrevExist = false;
      MyPoint prevWp;
      getPrevPoint(dqWp, isPrevExist, dqWp.back(), prevWp);
      bool flag = FindNextLineStart(p, n, pIntersection[0], pIntersection[1],
                                    dqWp.back(), prevWp, isPrevExist);
      if (num == 0) {
      } else {
        if (flag) {
          dqWp.push_back(pIntersection[0]);
          dqWp.push_back(pIntersection[1]);
        } else {
          dqWp.push_back(pIntersection[1]);
          dqWp.push_back(pIntersection[0]);
        }
      }
    }
    j++;
  }
}

//�����ϰ���ĺ��߹滮
int RoutePlanWithoutBarrier(double d, int isRepeat, bool isScaled, int uavType,
                            bool isMiddle, int SprayType, Line refer_border,
                            MyPoint *p, int n, PointFlyAttr *wp, int *pm,
                            MyPoint takeOffPoint) {
  unsigned int k = 0; //�ܹ�k���������
  double dmax = 0.0; //��ο�����Զ�Ķ��㵽�ο��ߵľ���
  unsigned int k1, k2;
  double d1 = d;
  double d2 = d;
  int index_max = 0; //��ο�����Զ�Ķ����������е��±�
  bool is_clockwise = false;
  MyPoint MaxD; //����ο�����Զ�Ķ���
  MyPoint ReferNearestPoint;
  double A, B;
  T_stRouteLineArgs Args = {0};
  pointDequeType dqWp;
  pointDequeType dqWpEveryTime;
  get_index_max(refer_border, p, n, index_max, dmax);
  get_useful_args(p, n, index_max, refer_border, &A, &B, &MaxD);
  if (isScaled) {
    if (isMiddle) {
      getTrueDmaxSpecial(p, n, k1, k2, d, d1, d2, MaxD, ReferNearestPoint, dmax,
                         refer_border);
      k = k1 + k2;
    } else {
      getTrueDmaxNormal(p, n, k, d, MaxD, ReferNearestPoint, dmax,
                        refer_border);
    }
  } else {
    k = ceil(dmax / d);
    d = dmax / k;
    ReferNearestPoint = refer_border.p0;
  }
  Args.A = A;
  Args.B = B;
  Args.d = d;
  Args.d1 = d1;
  Args.d2 = d2;
  Args.k = k;
  Args.k1 = k1;
  Args.k2 = k2;
  Args.MaxD.x = MaxD.x;
  Args.MaxD.y = MaxD.y;
  Args.ReferNearestPoint = ReferNearestPoint;
  Args.dmax = dmax;
  Args.refer_border = refer_border;
  Args.takeOffPoint = takeOffPoint;
  bool isConv = isConvex(p, n, &is_clockwise, OriginArgs::instance().dqPitsArr);
  if (isConv) {
    convexHandler(Args, p, n, isMiddle, isScaled, dqWp);
    // addLastWayPointWhenNeeded(p, n, dqWp);
  } else {
    ConcaveSprayDetermine(Args, p, n, isMiddle, isScaled);
    int ret = concave_handler_determine(Args, p, n, isMiddle, isScaled);
    ret = 0;
    if (ret == 1) {
      concave_handler(Args, p, n, isMiddle, isScaled, dqWp, dqWpEveryTime);
    } else {
      concave_handler_old(Args, p, n, isMiddle, isScaled, dqWp, dqWpEveryTime);
    }
  }

  int ret2 = waypointHandler(p, n, d, isRepeat, uavType, false, SprayType,
                             refer_border, wp, pm, dqWp);
  //	addLastWayPointWhenNeeded(p, n, wp, *pm, dqWp);
  //	ret2 = waypointHandler(p, n, d, isRepeat, uavType, false, SprayType,
  // refer_border, wp, pm, dqWp);
  return ret2;
}

int RoutePlanNew(double d, int isRepeat, bool isScaled, int uavType,
                 bool isBarrier, int SprayType, Line refer_border,
                 const MyPoint *pOrigin, const int nOrigin, MyPoint *p, int n,
                 PointFlyAttr *wp, int *pm, MyPoint takeOffPoint) {
  unsigned int k = 0; //�ܹ�k���������
  double dmax = 0.0; //��ο�����Զ�Ķ��㵽�ο��ߵľ���
  int index_max = 0; //��ο�����Զ�Ķ����������е��±�
  MyPoint MaxD; //����ο�����Զ�Ķ���
  double A, B;
  MyPoint ReferNearestPoint;
  T_stRouteLineArgs Args = {0};
  pointDequeType dqWp;
  pointDequeType dqWpEveryTime;
  get_index_max(refer_border, p, n, index_max, dmax);
  get_useful_args(p, n, index_max, refer_border, &A, &B, &MaxD);
  getTrueDmaxNormal2(p, n, k, d, MaxD, dmax, refer_border);
  Args.A = A;
  Args.B = B;
  Args.d = d;
  Args.k = k;
  Args.MaxD.x = MaxD.x;
  Args.MaxD.y = MaxD.y;
  Args.dmax = dmax;
  Args.refer_border = refer_border;
  Args.takeOffPoint = takeOffPoint;
  subBlockHandler(Args, pOrigin, nOrigin, p, n, dqWp, dqWpEveryTime);
  if (dqWp.size() > 1) {
    if (dqWp.front().x > dqWp.back().x) {
      std::reverse(dqWp.begin(), dqWp.end());
    }
  }
  int ret2 = waypointHandler(p, n, d, isRepeat, uavType, isBarrier, SprayType,
                             refer_border, wp, pm, dqWp);
  return ret2;
}

void getSweepPoint(MyPoint *p, int n, double delta, int direction,
                   int SprayType, PointFlyAttr *SweepPoint, int *pk) {
  MyPoint InterEnvelope[MAX_WP_NUM]; //����ε��ڰ��磬����ɨ��
  int t;
  int i = 0;

  get_envelope_of_polygen(p, n, InterEnvelope, &t, delta, 0);
  cout << "sweep point" << endl;
  for (i = 0; i < t; i++) {
    SweepPoint[i].pt = InterEnvelope[i];
    SweepPoint[i].SegmentType = 0;
    if (SprayType == 1)
      SweepPoint[i].SprayCtrl = 0;
    else if (SprayType == 2)
      SweepPoint[i].SprayCtrl = 1;
    else if (SprayType == 3)
      SweepPoint[i].SprayCtrl = 1;
    else {
      cout << "[getSweepPoint] [ERROR]SprayType =" << SprayType << endl;
      return;
    }
    SweepPoint[i].BaseIndex = 65535;
    SweepPoint[i].SideIndex = i;
#if DEBUG
    DisplayPoint(InterEnvelope[i]);
#endif
  }
  SweepPoint[t].pt = InterEnvelope[0];
  SweepPoint[t].SegmentType = 0;
  SweepPoint[t].SprayCtrl = 0;
  SweepPoint[t].BaseIndex = 65535;
  SweepPoint[t].SideIndex = 65535;
  *pk = (t + 1);
}

/*
�ؿ��ظ���������Ҫ���ڹ�������
*/
void sprayRepeat(pointDequeType &dqWp) {
  pointDequeType dqWayPointTmp;
  for (pointDequeType::reverse_iterator it = dqWp.rbegin(); it != dqWp.rend();
       it++) {
    dqWayPointTmp.push_back(*it);
  }
  dqWp.pop_back();
  for (pointDequeType::iterator it = dqWayPointTmp.begin();
       it != dqWayPointTmp.end(); it++) {
    dqWp.push_back(*it);
  }
}

// �����ľ���С��0.5m ����Ҫ�����⴦��
void RemoveWaypointTooClose(pointDequeType &dqWp,
                            ClosePointPairDequeType dqClosePointPair) {
  if (dqWp.size() < 2)
    return;
  if (dqWp.size() == 2) {
    double d = dist_Point_to_Point(dqWp.at(0), dqWp.at(1));
    if (d < WP_SMALLEST_DISTANCE) {
      dqWp.clear();
      return;
    }
  }
  std::vector<MyPoint> vecOfPoint; //�ݴ�waypoint
  for (pointDequeType::iterator it = dqWp.begin(); it != dqWp.end(); it++) {
    vecOfPoint.push_back(*it);
  }
  vector<MyPoint>::iterator it =
      unique(vecOfPoint.begin(),
             vecOfPoint.end()); // it ����Ϊ vector<MyPoint>::iterator
  vecOfPoint.erase(it, vecOfPoint.end());
  dqWp.clear();
  dqWp.push_back(vecOfPoint.front());
  for (vector<MyPoint>::iterator it = vecOfPoint.begin();
       it != vecOfPoint.end() - 1; it++) {
    MyPoint p1 = *it;
    MyPoint p2 = *(it + 1);
    ClosePointPair closePointPairTmp;
    int ret1, ret2;
    /* �����ľ���С��0.5m
     * ����Ҫ�����⴦��*/
    double d = dist_Point_to_Point(p1, p2);
    if (d < WP_SMALLEST_DISTANCE) {
#if DEBUG
      cout << "abnormal point:";
      cout << "p1=" << p1 << endl;
      cout << "p2=" << p2 << endl;
#endif
      closePointPairTmp.p0 = p1;
      closePointPairTmp.p1 = p2;
      dqClosePointPair.push_back(closePointPairTmp);
      ret1 = isBarrierEnvelopeVertexPoint(p1);
      ret2 = isBarrierEnvelopeVertexPoint(p2);
      /* �������С��0.5���е�һ����Ϊ������ɾ���ð���
                  ���⵼���ж�������ο����Ƿ�ƽ��ʱ������*/
      if (is_pit_point(p1)) {
        dqWp.pop_back();
        dqWp.push_back(p2);
      }
      /*
                      �������С��0.5���������е�һ����Ϊ���ߵ����һ��Ϊ�ϰ���
                      �Ķ��㣬��ôӦ��ɾ�����ߵ㣬��Ȼ�ᵼ�º��ߴ�Խ�ϰ������
                      �ߵ����
                    */
      else if (!ret1 && ret2) {
        dqWp.pop_back();
        dqWp.push_back(p2);
      } else {
      }
    } else {
      dqWp.push_back(p2);
    }
  }
}

void waypointTooFarHandler(pointDequeType &dqWp) {
  double d = 0.0;
  double lambda = 0.0;
  int j = 1;
  pointDequeType dqWpTmp;
  if (dqWp.size() < 2)
    return;
  dqWpTmp.push_back(dqWp.front());
  for (deque<MyPoint>::iterator it = dqWp.begin(); it != dqWp.end() - 1; it++) {
    MyPoint p1 = *it;
    MyPoint p2 = *(it + 1);
    d = dist_Point_to_Point(p1, p2);
    if (d > DISTANCE_OUT_OF_RANGE) {
      j = 1;
      while ((d - DISTANCE_OUT_OF_RANGE_ADJUST * j) > 0) {
        lambda = DISTANCE_OUT_OF_RANGE_ADJUST * j /
                 (d - DISTANCE_OUT_OF_RANGE_ADJUST * j);
        MyPoint t;
        t.x = (p1.x + lambda * p2.x) / (1 + lambda);
        t.y = (p1.y + lambda * p2.y) / (1 + lambda);
        dqWpTmp.push_back(t);
        j++;
      }
    }
    dqWpTmp.push_back(p2);
  }
  dqWp.clear();
  for (deque<MyPoint>::iterator it = dqWpTmp.begin(); it != dqWpTmp.end();
       it++) {
    dqWp.push_back(*it);
  }
}

/*
dmaxС����������ĵؿ鱻����Ϊ��խ�ؿ飬��խ�ؿ鲻�����ߡ�
dmax<2*d��ԭ������Щ�ؿ鲻�Ǳ�׼�ľ��Σ�С��2d�ĵؿ鶼������
ɨ�ߵķ�ʽ�������Ӷ��򻯴����߼�Ҳ����������������Ҫ��
*/
int IsNarrowGround(MyPoint *p, int n, double d, Line refer_border) {
  double A, B;
  double dmax = 0.0; //��ο�����Զ�Ķ��㵽�ο��ߵľ���
  MyPoint MaxD; //����ο�����Զ�Ķ���
  int index_max = 0; //��ο�����Զ�Ķ����������е��±�

  get_index_max(refer_border, p, n, index_max, dmax);
  get_useful_args(p, n, index_max, refer_border, &A, &B, &MaxD);
  if (dmax <= d + 0.4) {
    return 1;
  } else if ((dmax < 2 * d) && (dmax > d + 0.4)) {
    return 2;
  } else {
    return 0;
  }
}

/* ���߶ε������� */
void getInnerLinePoint(double width, MyPoint pt0, MyPoint pt1, MyPoint &p1,
                       MyPoint &p2) {
  double lambda1;
  double lambda2;
  double x1;
  double y1;
  double x2;
  double y2;
  double pt0pt1Length = dist_Point_to_Point(pt0, pt1);

  lambda1 = width / (pt0pt1Length - width);
  lambda2 = (pt0pt1Length - width) / width;
  x1 = (pt0.x + lambda1 * pt1.x) / (1 + lambda1);
  y1 = (pt0.y + lambda1 * pt1.y) / (1 + lambda1);
  x2 = (pt0.x + lambda2 * pt1.x) / (1 + lambda2);
  y2 = (pt0.y + lambda2 * pt1.y) / (1 + lambda2);
  p1.x = x1;
  p1.y = y1;
  p2.x = x2;
  p2.y = y2;
}

int findShortestBorder(MyPoint *p, int n) {
  int index = 0;
  double min = dist_Point_to_Point(p[0], p[1]);
  double l = 0.0;
  for (int i = 1; i < n; i++) {
    l = dist_Point_to_Point(p[i], p[(i + 1) % n]);
    if (l < min) {
      min = l;
      index = i;
    }
  }
  return index;
}

/* dmaxС��dʱֻ����һ�����ߣ��ú������ڴ���dmaxС��dʱ����խ�ؿ� */
void narrowGroundHandler1(FlyBasicArgs Args, MyPoint *p, int n,
                          PointFlyAttr *wp, int *pm, double alpha) {
  MyPoint middlePoint1;
  MyPoint middlePoint2;
  MyPoint pt1, pt2;
  pointDequeType dqWp;

  if (n == 4) {
    int minIndex = findShortestBorder(p, n);
    middlePoint1 = getMidPoint(p[minIndex], p[(minIndex + 1) % n]);
    middlePoint2 = getMidPoint(p[(minIndex + 2) % n], p[(minIndex + 3) % n]);
    /* �����Ŷ�Ҫ�����˻����߾���ؿ�߽�0.5�����
     */
    getInnerLinePoint(0.5 * Args.d, middlePoint1, middlePoint2, pt1, pt2);
  } else if (n == 3) {
    double d1 = dist_Point_to_Point(p[0], p[1]);
    double d2 = dist_Point_to_Point(p[1], p[2]);
    double d3 = dist_Point_to_Point(p[0], p[2]);
    if ((d1 < d2) && (d1 < d3)) {
      middlePoint1 = getMidPoint(p[0], p[1]);
      middlePoint2 = p[2];
    } else if ((d2 < d1) && (d2 < d3)) {
      middlePoint1 = getMidPoint(p[1], p[2]);
      middlePoint2 = p[0];
    } else if ((d3 < d1) && (d3 < d2)) {
      middlePoint1 = getMidPoint(p[0], p[2]);
      middlePoint2 = p[1];
    }
    getInnerLinePoint(alpha * Args.d, middlePoint1, middlePoint2, pt1, pt2);
  } else {
    double Cml, Cmr;
    MyPoint pt[2]; //����
    MyPoint pIntersection[20];
    unsigned int num = 0;
    double Cm;
    int index_max = 0;
    MyPoint MaxD; //����ο�����Զ�Ķ���
    double A, B;
    double dmax = 0.0;
    get_index_max(Args.refer_border, p, n, index_max, dmax);
    get_useful_args(p, n, index_max, Args.refer_border, &A, &B, &MaxD);
    Cml = -A * Args.refer_border.p0.x - B * Args.refer_border.p0.y -
          0.5 * dmax * sqrt(A * A + B * B);
    Cmr = -A * Args.refer_border.p0.x - B * Args.refer_border.p0.y +
          0.5 * dmax * sqrt(A * A + B * B);
    double d1 = fabs(A * MaxD.x + B * MaxD.y + Cml);
    double d2 = fabs(A * MaxD.x + B * MaxD.y + Cmr);
    if (d1 < d2) {
      Cm = Cml;
    } else {
      Cm = Cmr;
    }
    get_parallel_line(A, B, Cm, pt);
    get_line_polygon_intersetion(p, n, pt[0], pt[1], pIntersection, &num);
    getInnerLinePoint(alpha * Args.d, pIntersection[0], pIntersection[1], pt1,
                      pt2);
  }
  if (dist_Point_to_Point(Args.TakeOffPoint, pt1) <
      dist_Point_to_Point(Args.TakeOffPoint, pt2)) {
    dqWp.push_back(pt1);
    dqWp.push_back(pt2);
  } else {
    dqWp.push_back(pt2);
    dqWp.push_back(pt1);
  }
  waypointHandler(p, n, Args.d, Args.isSprayRepeat, Args.uavType, false,
                  Args.SprayType, Args.refer_border, wp, pm, dqWp);
}

/* �ú������ڴ���dmax����d��С��2d����խ�ؿ� */
void narrowGroundHandler2(FlyBasicArgs Args, MyPoint *p, int n,
                          PointFlyAttr *wp, int *pm) {
  int i = 0;
  int minIndex = 0;
  int t = 0;
  pointDequeType dqWp;
  MyPoint *InterEnvelope = new MyPoint[n]; //����ε��ڰ��磬��Ϊת����ҪԤ��������
  //��խ�ؿ����ɨ�ߵķ�ʽ���������ַ�ʽ���
  get_envelope_of_polygen(p, n, InterEnvelope, &t, 0.4 * Args.d, 0);
  double minD = dist_Point_to_Point(Args.TakeOffPoint, InterEnvelope[0]);
  double dt = 0.0;
  for (i = 1; i < t; i++) {
    dt = dist_Point_to_Point(Args.TakeOffPoint, InterEnvelope[i]);
    if (dt < minD) {
      minIndex = i;
    }
  }
  for (i = minIndex; i < t; i++) {
    dqWp.push_back(InterEnvelope[i]);
  }
  for (i = 0; i < minIndex; i++) {
    dqWp.push_back(InterEnvelope[i]);
  }
  dqWp.push_back(InterEnvelope[minIndex]);
  waypointHandler(p, n, Args.d, Args.isSprayRepeat, Args.uavType, false,
                  Args.SprayType, Args.refer_border, wp, pm, dqWp);
  if (InterEnvelope) {
    delete[] InterEnvelope;
    InterEnvelope = NULL;
  }
}

//�жϲο����ǲ��ǰ���
bool IsReferBorderConcave(MyPoint *p, int n, Line refer_border) {
  bool ClockWise = false;
  pointDequeType dqPitsArrTemp;
  bool ret1, ret2;
  bool IsConcave;

  isConvex(p, n, &ClockWise, dqPitsArrTemp);
  ret1 = is_pit_point2(refer_border.p0, dqPitsArrTemp);
  ret2 = is_pit_point2(refer_border.p1, dqPitsArrTemp);
  if (ret1 || ret2)
    IsConcave = true;
  else
    IsConcave = false;
  // cout<<"IsReferBorderConcave="<<IsConcave<<endl;
  return IsConcave;
}

//�жϲο����ǲ����ڵؿ�����
bool IsReferBorderMiddleSub(MyPoint *p, int n, Line refer_border) {
  bool IsMiddle = false;
  pointDequeType dqProjected; //ͶӰ�����
  pointDequeType dqPitsArrTemp;
  bool ClockWise = false;
  MyPoint Intersection;
  bool IsConvexPolygon = isConvex(p, n, &ClockWise, dqPitsArrTemp);
  Line l2;
  if (IsConvexPolygon)
    return false;

  //�����ο����ϵ�һ���˵���ֱ�ο��ߵ�ֱ�߷���
  double A = refer_border.p1.y - refer_border.p0.y;
  double B = refer_border.p0.x - refer_border.p1.x;
  MyPoint pt0 = refer_border.p0;
  double C1 = -B * pt0.x + A * pt0.y;
  MyPoint pt1;
  MyPoint ptTmp;
  pt1.x = 100;
  pt1.y = B * pt1.x / A + C1 / A;

  for (int i = 0; i < n; i++) {
    if (p[i] == refer_border.p0)
      continue;
    if (p[i] == refer_border.p1)
      continue;
    ptTmp = getProjectedPointOnLine(p[i], pt0, pt1);
    dqProjected.push_back(ptTmp);
  }
  l2.p0 = pt0;
  l2.p1 = pt1;
  //����
  LineLineIntersect(refer_border, l2, Intersection);
#if 0
    cout<<"cuizu="<<endl;
    DisplayPoint(Intersection);
#endif
  std::sort(dqProjected.begin(), dqProjected.end());
  if (Intersection.x > dqProjected.front().x &&
      Intersection.x < dqProjected.back().x) {
    IsMiddle = true;
  }
  return IsMiddle;
}

/*
�жϲο����Ƿ�λ�ڵؿ����룬����ο����ǰ��ߣ���ô��һ��
λ�ڵؿ����룬������ǰ�������Ҫ��һ���ж��Ƿ�λ�ڵؿ�����
*/
bool IsReferBorderMiddle(MyPoint *p, int n, Line refer_border) {
  bool IsConcave = false;
  bool IsMiddle = false;

  IsConcave = IsReferBorderConcave(p, n, refer_border);
  if (IsConcave)
    return true;
  IsMiddle = IsReferBorderMiddleSub(p, n, refer_border);

  if (IsMiddle)
    return true;
  return false;
}

/*
p: polygon vertex point array(input para)
n: number of point
wp: output waypoint array
(*pm): number of waypoint
sweep: sweep or not
*/
extern "C" int route_plan2(FlyBasicArgs Args, const MyPoint *p, int n,
                           SimplePolygon *pB, int bNum, PointFlyAttr *wp,
                           int *pm, PointFlyAttr *SweepPoint, int *pk) {
  MyPoint InterEnvelope[MAX_WP_NUM]; //����ε��ڰ��磬����ɨ��
  MyPoint InterEnvelope2[MAX_WP_NUM]; //����ε��ڰ��磬��Ϊת����ҪԤ��������
  int t;
  int ret = 0;

  OriginArgs::instance().BarrierNum = bNum;
  gLeakSprayInfo.fieldArea = my_area2D_Polygon(n, p);
#if DEBUG
  cout << "orginfieldArea= " << gLeakSprayInfo.fieldArea << endl;
#endif
  if (bNum > 0) {
    for (int i = 0; i < bNum; i++) {
      gLeakSprayInfo.fieldArea -= my_area2D_Polygon(pB[i].n, pB[i].p);
    }
  }
  Line ReferBorder = Args.refer_border;
  double d = Args.d;
  double delta = Args.SweepDelta;
  int sweep = Args.sweep;
  int SprayType = Args.SprayType;

  if (n > 200) {
    cout << "[route_plan] "
            "TOO_MANY_VETEX_ERR................................................"
            "...."
         << endl;
    return ARGS_ERR;
  }
  bool isBarrier = (bNum > 0) ? true : false;
  if (sweep) {
    get_envelope_of_polygen((MyPoint *)p, n, InterEnvelope, &t, 2 * delta, 0);
    getSweepPoint((MyPoint *)p, n, delta, 0, SprayType, SweepPoint, pk);
    get_envelope_of_polygen(InterEnvelope, n, InterEnvelope2, &t, 0.5 * d, 0);
    ret = RoutePlanNew(d, Args.isSprayRepeat, false, Args.uavType, isBarrier,
                       SprayType, ReferBorder, p, n, InterEnvelope2, t, wp, pm,
                       Args.TakeOffPoint);
  } else {
    ret = RoutePlanNew(d, Args.isSprayRepeat, true, Args.uavType, isBarrier,
                       SprayType, ReferBorder, p, n, (MyPoint *)p, n, wp, pm,
                       Args.TakeOffPoint);
  }
  if ((ret == TOO_MANY_WAYPOINT_ERR) || (*pm > MAX_WP_NUM)) {
    cout << "[route_plan] "
            "TOO_MANY_WAYPOINT_ERR............................................."
            "......."
         << endl;
    return TOO_MANY_WAYPOINT_ERR;
  }
  return 0;
}

/*
p: polygon vertex point array(input para)
n: number of point
wp: output waypoint array
(*pm): number of waypoint
sweep: sweep or not
*/
extern "C" int route_plan(FlyBasicArgs Args, MyPoint *p, int n,
                          SimplePolygon *pB, int bNum, PointFlyAttr *wp,
                          int *pm, PointFlyAttr *SweepPoint, int *pk) {
  MyPoint InterEnvelope[MAX_WP_NUM]; //����ε��ڰ��磬����ɨ��
  MyPoint InterEnvelope2[MAX_WP_NUM]; //����ε��ڰ��磬��Ϊת����ҪԤ��������
  int t;
  int ret = 0;

  OriginArgs::instance().BarrierNum = bNum;
  gLeakSprayInfo.fieldArea = my_area2D_Polygon(n, p);
#if DEBUG
  cout << "orginfieldArea= " << gLeakSprayInfo.fieldArea << endl;
#endif
  if (bNum > 0) {
    for (int i = 0; i < bNum; i++) {
      gLeakSprayInfo.fieldArea -= my_area2D_Polygon(pB[i].n, pB[i].p);
    }
  }
  Line ReferBorder = Args.refer_border;
  double d = Args.d;
  double delta = Args.SweepDelta;
  int sweep = Args.sweep;
  int SprayType = Args.SprayType;
  int IsNarrow = 0;
  setOriginArgs(p, n, Args.d, Args.refer_border);
  /* ��խ�ؿ���Ҫ���⴦�� */
  IsNarrow = IsNarrowGround(p, n, d, ReferBorder);

  if (n > 200) {
    cout << "[route_plan] "
            "TOO_MANY_VETEX_ERR................................................"
            "...."
         << endl;
    return ARGS_ERR;
  }
  bool isMiddle = IsReferBorderMiddle(p, n, ReferBorder);
  if (sweep) {
    get_envelope_of_polygen(p, n, InterEnvelope, &t, 2 * delta, 0);
    getSweepPoint(p, n, delta, 0, SprayType, SweepPoint, pk);
    get_envelope_of_polygen(InterEnvelope, n, InterEnvelope2, &t, 0.5 * d, 0);
    if (0 == bNum) {
      ret = RoutePlanWithoutBarrier(
          d, Args.isSprayRepeat, false, Args.uavType, isMiddle, SprayType,
          ReferBorder, InterEnvelope2, t, wp, pm, Args.TakeOffPoint);
    } else {
      ret = RoutePlanWithBarrier(d, Args.isSprayRepeat, Args.uavType, isMiddle,
                                 SprayType, ReferBorder, InterEnvelope2, t, pB,
                                 bNum, wp, pm);
    }
  } else {
    if (IsNarrow) {
      cout << "the ground is narrow, IsNarrow=" << IsNarrow << endl;
      if (IsNarrow == 1) {
        narrowGroundHandler1(Args, p, n, wp, pm, 0.5);
      }
      if (IsNarrow == 2) {
        if (n == 3)
          narrowGroundHandler1(Args, p, n, wp, pm, 0.5);
        else
          narrowGroundHandler2(Args, p, n, wp, pm);
      }
    } else {
      getNewReferBorder(p, n, d, ReferBorder);
      //��ͳһ���߻ᵼ�¶������״�ı䣬�Ӷ��������λ��Ҳ���ܸı�
      get_envelope_of_polygen(p, n, InterEnvelope2, &t, 0.4 * d, 0);
      if (0 == bNum) {
        ret = RoutePlanWithoutBarrier(
            d, Args.isSprayRepeat, true, Args.uavType, isMiddle, SprayType,
            ReferBorder, InterEnvelope2, t, wp, pm, Args.TakeOffPoint);
      } else {
        ret = RoutePlanWithBarrier(d, Args.isSprayRepeat, Args.uavType,
                                   isMiddle, SprayType, ReferBorder,
                                   InterEnvelope2, t, pB, bNum, wp, pm);
      }
    }
  }
  if ((ret == TOO_MANY_WAYPOINT_ERR) || (*pm > MAX_WP_NUM)) {
    cout << "[route_plan] "
            "TOO_MANY_WAYPOINT_ERR............................................."
            "......."
         << endl;
    return TOO_MANY_WAYPOINT_ERR;
  }

  return 0;
}

void clear(void) {
  gIsConcaveSpray = 0;
  memset(&gLeakSprayInfo, 0, sizeof(gLeakSprayInfo));
}

void setOriginArgs(const MyPoint *p, int n, double d, Line refer_border) {
  OriginArgs::instance().d = d;
  OriginArgs::instance().p = p;
  OriginArgs::instance().n = n;
  OriginArgs::instance().refer_border = refer_border;
}

/*
���ο������ڽ���ֱ�߼нǼ�Сʱ����Ҫ���⴦��
����ᵼ��©��
*/
bool SecondLineSpecialProcess(MyPoint *p, int &n, Line &refer_border,
                              Line &NearBorder) {
  int ReferIndex = FindReferBorderIndex(p, n, refer_border);
  bool IsSpecial = false;

  double VectorAngle1 = getAngelOfTwoVector(
      p[(ReferIndex + n - 1) % n], p[(ReferIndex + 1) % n], p[ReferIndex]);
  if (VectorAngle1 > 170) {
    IsSpecial = true;
    NearBorder.p0 = p[(ReferIndex + n - 1) % n];
    NearBorder.p1 = p[ReferIndex];
  }
  double VectorAngle2 = getAngelOfTwoVector(
      p[(ReferIndex + 2) % n], p[ReferIndex], p[(ReferIndex + 1) % n]);
  if (VectorAngle2 > 170) {
    IsSpecial = true;
    NearBorder.p0 = p[(ReferIndex + 1) % n];
    NearBorder.p1 = p[(ReferIndex + 2) % n];
  }
  return IsSpecial;
}

void setBaseBorder(const MyPoint *p, int n, FlyWpInfo *pFlyWpInfoInstance) {
  pFlyWpInfoInstance->baseBorder = new BaseBorderInfo[n];
  for (int i = 0; i < n; i++) {
    pFlyWpInfoInstance->baseBorder[i].index = i;
    pFlyWpInfoInstance->baseBorder[i].p0 = p[i];
    pFlyWpInfoInstance->baseBorder[i].p1 = p[(i + 1) % n];
  }
  pFlyWpInfoInstance->baseBorderNum = n;
}

// extern pointFlyAttrDequeType dqFlyWayPoint; //ȫ��Ψһ

/*
p : ָ��ؿ�ĸ�������
n : �ؿ鶥�����
Barrier : ָ���ϰ���
bNum : �ϰ���ĸ���
*/
extern "C" FlyWpInfo *RoutePlanForAndroid(FlyBasicArgs Args, MyPoint *p, int n,
                                          SimplePolygon *pB, int bNum) {
  FlyWpInfo *pFlyWpInfoInstance = new FlyWpInfo;
  PointFlyAttr wpArr[MAX_WP_NUM];
  PointFlyAttr SweepPointArr[500];
  int k = 0;
  int m = 0;
  int i = 0;
  int ret = 0;

  if (Args.SprayType > 3 || NULL == p) {
    pFlyWpInfoInstance->ErrCode = ARGS_ERR;
    return pFlyWpInfoInstance;
  }

#if DEBUG
  cout << "bNum=" << bNum << endl;
  cout << "print barrier......................" << endl;
  for (int j = 0; j < bNum; j++) {
    for (int i = 0; i < pB[j].n; i++) {
      DisplayPoint(pB[j].p[i]);
    }
  }
  cout << "print barrier......................" << endl;
#endif
  setBaseBorder(p, n, pFlyWpInfoInstance);
  MyPoint *q = new MyPoint[n];
  memcpy(q, p, n * sizeof(MyPoint));
  ret = CheckInput(p, n, pB, bNum);
  if (ret == BORDER_POINT_TOO_CLOSE) {
    pFlyWpInfoInstance->ErrCode = ret;
    return pFlyWpInfoInstance;
  }
  clear();
  ret = route_plan(Args, p, n, pB, bNum, wpArr, &m, SweepPointArr, &k);
  if (ret != 0) {
    cout << "ErrCode=" << pFlyWpInfoInstance->ErrCode << endl;
    pFlyWpInfoInstance->ErrCode = ret;
    return pFlyWpInfoInstance;
  }

  pFlyWpInfoInstance->k = k;
  pFlyWpInfoInstance->m = m;
  if (m > 0)
    pFlyWpInfoInstance->wp = new PointFlyAttr[m];
  else
    pFlyWpInfoInstance->wp = NULL;
  if (k > 0)
    pFlyWpInfoInstance->SweepPoint = new PointFlyAttr[k];
  else
    pFlyWpInfoInstance->SweepPoint = NULL;
  for (i = 0; i < m; i++) {
    pFlyWpInfoInstance->wp[i] = wpArr[i];
  }
  for (i = 0; i < k; i++) {
    pFlyWpInfoInstance->SweepPoint[i] = SweepPointArr[i];
  }

  //	pFlyWpInfoInstance->ErrCode = checkSoInterface(Args, p, n, pB, bNum,
  // pFlyWpInfoInstance);
  pFlyWpInfoInstance->ErrCode = 0;
  if (pFlyWpInfoInstance->ErrCode == 0) {
    pointFlyAttrDequeType dqFlyWp;
    dqFlyWp.clear();
    for (int i = 0; i < m; i++) {
      dqFlyWp.push_back(pFlyWpInfoInstance->wp[i]);
    }
#if DEBUG
    printFlyWayPoint(dqFlyWp);
#endif
    CalSprayAreaInfo(gLeakSprayInfo, Args.d, pFlyWpInfoInstance->wp, m);
    memcpy(&pFlyWpInfoInstance->leakSprayInfo, &gLeakSprayInfo,
           sizeof(gLeakSprayInfo));
  }
  clear();

  return pFlyWpInfoInstance;
}

extern "C" FlyWpInfo *RoutePlanForAndroidVersion(FlyBasicArgs Args, MyPoint *p,
                                                 int n, SimplePolygon *pB,
                                                 int bNum, int planVersion) {
  if (planVersion == 1) {
    return RoutePlanForAndroid2(Args, (const MyPoint *)p, n, pB, bNum);
  } else {
    return RoutePlanForAndroid(Args, p, n, pB, bNum);
  }
}
