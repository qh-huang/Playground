#include <algorithm> // std::for_each
#include <cmath>
#include <iostream> // std::cout
#include <vae/barrier.h>
#include <vae/geometry.h>
#include <vae/polygon.h>
#include <vae/route_plan_check.h>

int CheckPolygon(MyPoint *p, int n) {
  double d = 0.0;
  for (int i = 0; i < n; i++) {
    d = dist_Point_to_Point(p[i], p[(i + 1) % n]);
    if (d < VERTEX_SMALLEST_DISTANCE) {
      cout << "BORDER_POINT_TOO_CLOSE" << endl;
      return BORDER_POINT_TOO_CLOSE;
    }
  }
  return 0;
}

/*
���ؿ�͵ؿ��ڵĶ�����Ƿ�Ϸ�
*/
int CheckInput(MyPoint *p, int n, SimplePolygon *pB, int bNum) {
  int ret = 0;
  ret = CheckPolygon(p, n);
  if (ret != 0)
    return ret;
  for (int i = 0; i < bNum; i++) {
    ret = CheckPolygon(pB[i].p, pB[i].n);
    if (ret != 0)
      return ret;
  }
  return 0;
}

void delBarScale(Polygon **ppBarScale, int bNum) {
  if (ppBarScale != NULL) {
    for (int i = 0; i < bNum; i++) {
      delete ppBarScale[i];
      ppBarScale[i] = NULL;
    }
    delete[] ppBarScale;
    ppBarScale = NULL;
  }
}

/*
������ο���ƽ�е�ֱ�������εĽ��㣬���Ǻ��ߵ�
pIntersection: ���������εĽ�������
*/
void get_line_polygon_intersetion2(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
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
  vector<MyPoint> insec;
  for (unsigned int j = 0; j < k; j++) {
    insec.push_back(pIntersection[j]);
  }
  std::sort(insec.begin(), insec.end());
  vector<MyPoint>::iterator it = unique(insec.begin(), insec.end());
  insec.erase(it, insec.end());
  *pm = insec.size();
  for (unsigned int j = 0; j < insec.size(); j++) {
    pIntersection[j] = insec.at(j);
  }
}

/*�ж��߶��Ƿ��������ཻ�������ж��߶�����ֱ���Ƿ�������
�ཻ, ������ཻ���߶������β��ཻ������ཻ����Ҫ��һ���ж�
�߶ε������˵��Լ����ǵ��е��Ƿ��ڶ�����ڣ��������ôҲ
���������ཻ
����ֵΪtrue��ʾ�ཻ�������ཻ
*/
bool intersect2D_SegPoly3(MyPoint *p, int n, MyPoint pt1, MyPoint pt2) {
  MyPoint ptArr[200];
  unsigned int m = 0;
  Segment S;
  S.p0 = pt1;
  S.p1 = pt2;
  int ret = 0;
  vector<MyPoint> concernPt; //ֻ�����߶�pt1-pt2�ϵĵ����Ҫ��ע
  get_line_polygon_intersetion2(p, n, pt1, pt2, ptArr, &m);
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
bool intersect2D_SegPoly3(MyPoint *p, int n, MyPoint pt1, MyPoint pt2, int k) {
  MyPoint ptArr[200];
  vector<MyPoint> intersection;
  unsigned int m = 0;
  Segment S;
  S.p0 = pt1;
  S.p1 = pt2;
  k = 0;
  int ret = 0;
  get_line_polygon_intersetion2(p, n, pt1, pt2, ptArr, &m);

  for (unsigned int i = 0; i < m; i++) {
    ret = inSegment(ptArr[i], S);
    if (ret == 1) {
      k++;
      intersection.push_back(ptArr[i]);
    }
  }
  bool ret2 = intersect2D_SegPoly3(p, n, pt1, pt2);
  /* ע��: ���������Ϊ1ʱ�������������ǡ����pt1��pt2����ô����
  �����Ӧ����Ϊ�߶��������ཻ*/
  if (k == 1) {
    if ((intersection.front() == pt1) || (intersection.front() == pt2)) {
      ret2 = false;
    }
  }
  return ret2;
}

/*
Check if waypoint is valid, firstly, the waypoint cannot be outside of ground,
secondly, if it has barrier, the line composed by neighboring waypoint cannot
cross the outer envelope of any barrier
*/
int CheckWaypointValid(FlyBasicArgs Args, MyPoint *p, int n, SimplePolygon *pB,
                       int bNum, FlyWpInfo *pFlyWpInfoInstance) {
  PointFlyAttr pt1, pt2;
  volatile int ret = 0;
  int t = 0;
  MyPoint qTmp[500];
  int num = pFlyWpInfoInstance->m;
  Polygon **ppBarScale = new Polygon *[bNum];

  for (int i = 0; i < bNum; i++) {
    get_envelope_of_polygen(pB[i].p, pB[i].n, qTmp, &t, 0.4 * (Args.d), 1);
    ppBarScale[i] = new Polygon(t, qTmp);
  }

  for (int i = 0; i < num - 1; i++) {
    pt1 = pFlyWpInfoInstance->wp[i];
    pt2 = pFlyWpInfoInstance->wp[i + 1];
    ret = intersect2D_SegPoly(p, n, pt1.pt, pt2.pt);
    if (ret == true) {
      delBarScale(ppBarScale, bNum);
      pFlyWpInfoInstance->ErrCode += ROUTELINE_OUT_OF_BORDER;
      return ROUTELINE_OUT_OF_BORDER;
    }
  }

  for (int i = 0; i < bNum; i++) {
    for (int j = 0; j < num - 1; j++) {
      pt1 = pFlyWpInfoInstance->wp[j];
      pt2 = pFlyWpInfoInstance->wp[j + 1];
      ret = intersect2D_SegPoly3(ppBarScale[i]->p, ppBarScale[i]->npts, pt1.pt,
                                 pt2.pt, 0);
      if (ret) {
        cout << "ROUTELINE_OUT_OF_BORDER:barrier index=" << i
             << ", WayPoint index=" << j << endl;
        delBarScale(ppBarScale, bNum);
        pFlyWpInfoInstance->ErrCode += ROUTELINE_OUT_OF_BORDER;
        return ROUTELINE_OUT_OF_BORDER;
      }
    }
  }
  return 0;
}

/* �˺�����ʱ�������� */
int CheckWaypointDistanceValid(FlyWpInfo *pFlyWpInfoInstance) {
  PointFlyAttr *wp = pFlyWpInfoInstance->wp;
  int m = pFlyWpInfoInstance->m;
  double d = 0.0;
  for (int i = 0; i < m - 1; i++) {
    d = dist_Point_to_Point(wp[i].pt, wp[i + 1].pt);
    if (d < WP_SMALLEST_DISTANCE) {
      return WAYPOINT_TOO_CLOSE;
    }
  }
  return 0;
}

/*
���base���Ƿ���Ч
*/
int CheckBaseBorderValid(FlyWpInfo *pFlyWpInfoInstance) {
  BaseBorderInfo *baseBorder = pFlyWpInfoInstance->baseBorder;
  int baseBorderNum = pFlyWpInfoInstance->baseBorderNum;
  double d = 0.0;
  for (int i = 0; i < baseBorderNum; i++) {
    d = dist_Point_to_Point(baseBorder[i].p0, baseBorder[i].p1);
    if (d < 1) {
      return BASE_BORDER_POINT_TOO_CLOSE;
    }
  }
  return 0;
}

int checkDistanceFromHome(FlyWpInfo *pFlyWpInfoInstance, MyPoint takeOffPoint) {
  PointFlyAttr *wp = pFlyWpInfoInstance->wp;
  int m = pFlyWpInfoInstance->m;
  double d = 0.0;
  for (int i = 0; i < m; i++) {
    d = dist_Point_to_Point(wp[i].pt, takeOffPoint);
    if (d > MAX_DISTANCE_FROM_HOME) {
      return MAX_DISTANCE_FROM_HOME_ERR;
    }
  }
  d = dist_Point_to_Point(wp[0].pt, takeOffPoint);
  if (d > DISTANCE_FIRST_WP_AND_HOME) {
    return DISTANCE_FIRST_WP_AND_HOME_ERR;
  }
  return 0;
}

int CheckWaypointSegmentValid(FlyWpInfo *pFlyWpInfoInstance) {
  PointFlyAttr *wp = pFlyWpInfoInstance->wp;
  int m = pFlyWpInfoInstance->m;
  for (int i = 1; i < m; i++) {
    if ((wp[i].SegmentType == 1) && (wp[i - 1].SegmentType == 1)) {
      return SEGMENT_ATTR_ERR;
    }
  }
  return 0;
}

void checkRoutePlanAferPlan(FlyWpInfo *pFlyWpInfoInstance,
                            MyPoint takeoffPoint) {
  int ret = 0;
#if 0
    ret = CheckWaypointDistanceValid(pFlyWpInfoInstance);
    if (ret != 0)
        pFlyWpInfoInstance->ErrCode += ret;
#endif
  ret = CheckBaseBorderValid(pFlyWpInfoInstance);
  if (ret != 0)
    pFlyWpInfoInstance->ErrCode += ret;
  ret = CheckWaypointSegmentValid(pFlyWpInfoInstance);
  if (ret != 0)
    pFlyWpInfoInstance->ErrCode += ret;
#if 0 // TODO: ��ʱ����Ҫ�����У��
    if ((fabs(takeoffPoint.x) < ABNORMAL_COORDINATE) && (fabs(takeoffPoint.y) < ABNORMAL_COORDINATE))
    {
        ret = checkDistanceFromHome(pFlyWpInfoInstance, takeoffPoint);
        if (ret != 0)
            pFlyWpInfoInstance->ErrCode += ret;
    }
#endif
  //  cout<<"ErrCode="<<pFlyWpInfoInstance->ErrCode<<endl;
}
