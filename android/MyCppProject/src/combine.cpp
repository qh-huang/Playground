/*
注意: 本代码仅仅能支持2~3个地块的合并
*/

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream> // std::cout
#include <vae/barrier.h>
#include <vae/geometry.h>
#include <vae/polygon.h>
#include <vae/route_plan_check.h>

#define CHANNEL_WIDE 7
using namespace std;

void getExtensionLinePoint(double width, MyPoint pt0, MyPoint pt1, MyPoint &p1,
                           MyPoint &p2);
void printFlyWayPoint2(vector<PointFlyAttr> dqFlyWp);

int findDoubleChannelIndex(Field *fieldGroup, int fieldNum) {
  int indexDouble = 0;
  for (int i = 0; i < fieldNum; i++) {
    if (fieldGroup[i].mChannelNum == 1) {
      continue;
    } else {
      indexDouble = i;
    }
  }
  cout << "indexDouble=" << indexDouble << endl;
  return indexDouble;
}

/*
PolygonArea:计算多边形的面积函数
*/
double PolygonArea(MyPoint *polygon, int N) {
  int i, j;
  double area = 0;

  for (i = 0; i < N; i++) {
    j = (i + 1) % N;
    area += polygon[i].x * polygon[j].y;
    area -= polygon[i].y * polygon[j].x;
  }

  area /= 2;
  return (area < 0 ? -area : area);
}

//获取多边形的质心
MyPoint getPolygonCenter(MyPoint *p, int n) {
  double area = PolygonArea(p, n);
  double cx = 0, cy = 0;
  double factor = 0;
  int i = 0, j = 0;
  for (i = 0; i < n; i++) {
    j = (i + 1) % n;
    factor = (p[i].x * p[j].y - p[j].x * p[i].y);
    cx += (p[i].x + p[j].x) * factor;
    cy += (p[i].y + p[j].y) * factor;
  }
  area *= 6.0f;
  factor = 1 / area;
  cx *= factor;
  cy *= factor;

  return MyPoint(cx, cy);
}

/*
getVfield: 获取vField向量组
为了简化设计，代码中首先保证双通道的地块放在第二个位置
另一个原则是保证飞机从离起飞点近的地块开始起飞
*/
void getVfield(FlyBasicArgs Args, Field *fieldGroup, int fieldNum,
               vector<Field> &vField) {
  if (fieldNum == 2) {
    MyPoint center1 = getPolygonCenter(fieldGroup[0].p, fieldGroup[0].n);
    MyPoint center2 = getPolygonCenter(fieldGroup[1].p, fieldGroup[1].n);
    double dist1 = dist_Point_to_Point(center1, Args.TakeOffPoint);
    double dist2 = dist_Point_to_Point(center2, Args.TakeOffPoint);
    if (dist1 < dist2) {
      vField.push_back(fieldGroup[0]);
      vField.push_back(fieldGroup[1]);
    } else {
      vField.push_back(fieldGroup[1]);
      vField.push_back(fieldGroup[0]);
    }
  }

  if (fieldNum == 3) {
    /*
             为简化处理，通道数为2的地块只能放在数组中间，另外，飞机需要从
             离通道数为1的地块中距离近的地块开始起飞
             */
    int indexDouble = findDoubleChannelIndex(fieldGroup, fieldNum);
    if (0 == indexDouble) {
      MyPoint center1 = getPolygonCenter(fieldGroup[1].p, fieldGroup[1].n);
      MyPoint center2 = getPolygonCenter(fieldGroup[2].p, fieldGroup[2].n);
      double dist1 = dist_Point_to_Point(center1, Args.TakeOffPoint);
      double dist2 = dist_Point_to_Point(center2, Args.TakeOffPoint);
      if (dist1 < dist2) {
        vField.push_back(fieldGroup[1]);
        vField.push_back(fieldGroup[0]);
        vField.push_back(fieldGroup[2]);
      } else {
        vField.push_back(fieldGroup[2]);
        vField.push_back(fieldGroup[0]);
        vField.push_back(fieldGroup[1]);
      }
    } else if (1 == indexDouble) {
      MyPoint center1 = getPolygonCenter(fieldGroup[0].p, fieldGroup[0].n);
      MyPoint center2 = getPolygonCenter(fieldGroup[2].p, fieldGroup[2].n);
      double dist1 = dist_Point_to_Point(center1, Args.TakeOffPoint);
      double dist2 = dist_Point_to_Point(center2, Args.TakeOffPoint);
      if (dist1 < dist2) {
        vField.push_back(fieldGroup[0]);
        vField.push_back(fieldGroup[1]);
        vField.push_back(fieldGroup[2]);
      } else {
        vField.push_back(fieldGroup[2]);
        vField.push_back(fieldGroup[1]);
        vField.push_back(fieldGroup[0]);
      }
    } else {
      MyPoint center1 = getPolygonCenter(fieldGroup[0].p, fieldGroup[0].n);
      MyPoint center2 = getPolygonCenter(fieldGroup[1].p, fieldGroup[1].n);
      double dist1 = dist_Point_to_Point(center1, Args.TakeOffPoint);
      double dist2 = dist_Point_to_Point(center2, Args.TakeOffPoint);
      if (dist1 < dist2) {
        vField.push_back(fieldGroup[0]);
        vField.push_back(fieldGroup[2]);
        vField.push_back(fieldGroup[1]);
      } else {
        vField.push_back(fieldGroup[1]);
        vField.push_back(fieldGroup[2]);
        vField.push_back(fieldGroup[0]);
      }
    }
  }
}

/*
getDistWpToSegment: 计算点到直线的距离
算法实现步骤
1.	计算waypoint到直线(边p1p2)的距离d，计算cross_pt(waypoint到直线的投影)
2.	计算dist_p1p2(p1到p2的距离)，dist_p1cpt(p1到cross_pt之间的距离)
dist_p2cpt(p2到cross_pt之间的距离)
3.	如果If (dist_p1cpt < dist_p1p2) && (dist_p2cpt <
dist_p1p2)：表明cross_pt处于p1p2上，那么waypoint和border的距离是d。 4.
否则waypoint到border的距离是waypoint到p1的距离，waypoint到p2的距离中的较小者。
*/
double getDistWpToSegment(MyPoint pt, Segment seg) {
  double dist = dist_Point_to_Line(pt, seg);
  MyPoint proj = getProjectedPointOnLine(pt, seg.p0, seg.p1);
  bool ret = onSegment(seg.p0, seg.p1, proj);
  if (!ret) /* 投影点不在线段上 */
  {
    double dist_p1cpt = dist_Point_to_Point(pt, seg.p0);
    double dist_p2cpt = dist_Point_to_Point(pt, seg.p1);
    dist = (dist_p1cpt < dist_p2cpt) ? dist_p1cpt : dist_p2cpt;
  }
  return dist;
}

/*
setBaseIndexNormal: 正常情况下的设置BaseIndex的函数
*/
void setBaseIndexNormal(unsigned int i, const MyPoint *p, int n, double d,
                        Line referBorder, deque<PointFlyAttr> &ptQueue) {
  MyPoint pt = ptQueue.at(i).pt;
  point2LineDequeType dqDistance;
  Point2LineDis ptDis;
  Line seg;

  for (int j = 0; j < n; j++) {
    seg.p0 = p[j];
    seg.p1 = p[(j + 1) % n];
    ptDis.d1 = getDistWpToSegment(pt, seg);
    ptDis.index = j;
    ptDis.pt = pt;
    dqDistance.push_back(ptDis);
  }
  std::sort(dqDistance.begin(), dqDistance.end());
  int index = dqDistance.at(0).index;
  ptQueue.at(i).BaseIndex = index;
}

/*
设置合并地块后地块的waypoint baseIndex
*/
void setCombineBaseIndex(const MyPoint *p, int n, double d, Line referBorder,
                         deque<PointFlyAttr> &ptQueue,
                         vector<MyPoint> vptChannel) {
  unsigned int i = 0;
  unsigned int j = 0;
  Line l;
  for (i = 0; i < ptQueue.size(); i++) {
    if ((ptQueue.at(i).BaseIndex == 65534) ||
        (ptQueue.at(i).BaseIndex == 65533))
      continue;
    /* 通道点需要特殊处理 */
    for (j = 0; j < vptChannel.size(); j++) {
      if (ptQueue.at(i).pt == vptChannel.at(j)) {
        ptQueue.at(i).BaseIndex = 65532;
        continue;
      }
    }
    setBaseIndexNormal(i, p, n, d, referBorder, ptQueue);
  }
}

/*
在多边形上找一个离pt最近的顶点
*/
MyPoint findNearestPoint(MyPoint *p, int n, MyPoint pt) {
  double dMin = dist_Point_to_Point(pt, p[0]);
  double dist;
  MyPoint ptNearest = p[0];
  for (int i = 1; i < n; i++) {
    dist = dist_Point_to_Point(pt, p[i]);
    if (dist < dMin) {
      dMin = dist;
      ptNearest = p[i];
    }
  }
  return ptNearest;
}

/*
当地块的出口点或入口点和该地块的第一个或最后一个waypoint形成的
线段。与地块本身或地块内的障碍物相交时，需要入队过渡点以到达
通道出口点
*/
void enqueueFieldTransitionPoint(MyPoint pt1, MyPoint pt2, int ptChannelIndex,
                                 vector<PointFlyAttr> &wpQue,
                                 const pointDequeType &dqTmp) {
  PointFlyAttr tmp;
  for (unsigned int i = 0; i < dqTmp.size(); i++) {
    memset(&tmp, 0, sizeof(PointFlyAttr));
    tmp.BaseIndex = 65533;
    tmp.SideIndex = 65535;
    tmp.pt = dqTmp[i];
    /* 终点不入队，终点在该函数完成后再入队 */
    if ((ptChannelIndex == 1) && (IsEqual2(tmp.pt.x, pt2.x)) &&
        (IsEqual2(tmp.pt.y, pt2.y))) {
      continue;
    }
    if ((ptChannelIndex == 2) && (IsEqual2(tmp.pt.x, pt1.x)) &&
        (IsEqual2(tmp.pt.y, pt1.y))) {
      continue;
    }
    wpQue.push_back(tmp);
  }
}

/*
该函数的功能是入队每个地块的入口点或出口点，在此过程中，出入口点与
该地块的第一个或最后一个waypoint形成的线段可能与地块本身或地块内部的
障碍物相交，为此需要相应处理。
index: 是当前访问的地块序号
ptChannelIndex: 值为1或2，表示pt1和pt2中哪个为通道点
*/
void enqueueFieldInOutPoint(int index, double d, MyPoint pt1, MyPoint pt2,
                            int ptChannelIndex, vector<PointFlyAttr> &wpQue,
                            vector<Field> &vField) {
  bool ret;
  PointFlyAttr tmp;
  MyPoint *qTmp = NULL;
  int t = 0;
  bool intersectFlag = false;
  /* 如果地块包含障碍物，则首先需要判断该地块的最后一个waypoint
  与地块通道出口点构成的连线与障碍物是否相交，如果相交则需要做
  多边形绕边处理*/
  if (wpQue.size() > 0) {
    int k = 0;
    ret = intersect2D_SegPoly2(vField[index].p, vField[index].n, pt1, pt2, k);
    if (ret && (k > 1))
      intersectFlag = true;
    /* 如果航线与多边形外边界不相交，则需要进一步判断航线
    是否与障碍物相交*/
    if (false == intersectFlag) {
      if (vField[index].bNum > 0) {
        for (int j = 0; j < vField[index].bNum; j++) {
          qTmp = new MyPoint[vField[index].pB[j].n];
          get_envelope_of_polygen(vField[index].pB[j].p, vField[index].pB[j].n,
                                  qTmp, &t, 0.5 * d, 1);
          ret = intersect2D_SegPoly2(qTmp, t, pt1, pt2, k);
          if (qTmp) {
            delete qTmp;
            qTmp = NULL;
          }
          if (ret && (k > 1))
            intersectFlag = true;
          if (intersectFlag) {
            break;
          }
        }
      }
    }
    if (intersectFlag) {
      pointDequeType dqTmp1;      //顺时针前进
      pointDequeType dqTmp2;      //逆时针前进
      MyPoint InterEnvelope[200]; //多边形的内包络
      int t = 0;
      get_envelope_of_polygen(vField[index].p, vField[index].n, InterEnvelope,
                              &t, 0.5 * d, 0);
      if (ptChannelIndex == 1) {
        MyPoint ptNearest = findNearestPoint(InterEnvelope, t, pt1);
        FindShortestPathSub(InterEnvelope, t, ptNearest, pt2, dqTmp1, dqTmp2);
      } else /* (ptChannelIndex == 2) */
      {
        MyPoint ptNearest = findNearestPoint(InterEnvelope, t, pt2);
        FindShortestPathSub(InterEnvelope, t, pt1, ptNearest, dqTmp1, dqTmp2);
      }
      deque<MyPoint>::iterator iter1 = unique(dqTmp1.begin(), dqTmp1.end());
      dqTmp1.erase(iter1, dqTmp1.end());
      deque<MyPoint>::iterator iter2 = unique(dqTmp2.begin(), dqTmp2.end());
      dqTmp2.erase(iter2, dqTmp2.end());
      double d1 = 0.0, d2 = 0.0;
      unsigned int i = 0;
      /* 选择距离短的路线为最终路径 */
      for (i = 0; i < dqTmp1.size() - 1; i++) {
        d1 += dist_Point_to_Point(dqTmp1[i], dqTmp1[i + 1]);
      }
      for (i = 0; i < dqTmp2.size() - 1; i++) {
        d2 += dist_Point_to_Point(dqTmp2[i], dqTmp2[i + 1]);
      }

      if (d1 <= d2) {
        enqueueFieldTransitionPoint(pt1, pt2, ptChannelIndex, wpQue, dqTmp1);
      } else {
        enqueueFieldTransitionPoint(pt1, pt2, ptChannelIndex, wpQue, dqTmp2);
      }
    }
  }
}

void printFlyWayPoint2(vector<PointFlyAttr> dqFlyWp) {
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
getExtensionLinePoint: 定比分点法求连线延长线上的对应点
width: 延长的距离
*/
void getExtensionLinePoint(double width, MyPoint pt0, MyPoint pt1, MyPoint &p1,
                           MyPoint &p2) {
  double lambda1;
  double x1;
  double y1;
  double x2;
  double y2;
  double pt0pt1Length = dist_Point_to_Point(pt0, pt1);

  lambda1 = -(pt0pt1Length + width) / width;
  x2 = (pt0.x + lambda1 * pt1.x) / (1 + lambda1);
  y2 = (pt0.y + lambda1 * pt1.y) / (1 + lambda1);
  x1 = (pt1.x + lambda1 * pt0.x) / (1 + lambda1);
  y1 = (pt1.y + lambda1 * pt0.y) / (1 + lambda1);
  p1.x = x1;
  p1.y = y1;
  p2.x = x2;
  p2.y = y2;
}

/*
getWpQueCom: 合并地块获取waypoint队列的函数
*/
void getWpQueCom(FlyBasicArgs Args, Field *fieldGroup, int fieldNum,
                 vector<Field> &vField, vector<PointFlyAttr> &wpQue,
                 SimplePolygon combinePoly) {
  int i = 0;
  int j = 0;
  int k = 0;
  FlyWpInfo *FlyWpInfoTmp = NULL;
  deque<PointFlyAttr> wpQueTmp;
  vector<MyPoint> vptChannel;
  for (j = 0; j < fieldNum; j++) {
    for (k = 0; k < fieldGroup[j].mChannelNum; k++) {
      vptChannel.push_back(fieldGroup[j].ptChannel[k].pt);
    }
  }

  for (i = 0; i < fieldNum; i++) {
    Args.refer_border = vField[i].refer_border;
    if (i > 0) {
      MyPoint pt = wpQue.back().pt;
      Args.TakeOffPoint = pt;
    }
    FlyWpInfoTmp = RoutePlanForAndroid(Args, vField[i].p, vField[i].n,
                                       vField[i].pB, vField[i].bNum);
    /* 入队地块的通道入口点 */
    if (i == 1 || i == 2) {
      PointFlyAttr tmp;
      MyPoint lastPoint = wpQue.back().pt;
      memset(&tmp, 0, sizeof(PointFlyAttr));
      tmp.BaseIndex = 65532;
      if (vField[i].mChannelNum == 2) {
        if (onSegment(vField[i].ch[0].chLine.p0, vField[i].ch[0].chLine.p1,
                      lastPoint)) {
          tmp.pt = vField[i].ptChannel[0].pt;
          tmp.SideIndex = vField[i].ptChannel[0].channelIndex;
        } else {
          tmp.pt = vField[i].ptChannel[1].pt;
          tmp.SideIndex = vField[i].ptChannel[1].channelIndex;
        }
      } else {
        tmp.pt = vField[i].ptChannel[0].pt;
        tmp.SideIndex = vField[i].ptChannel[0].channelIndex;
      }
      wpQue.push_back(tmp);
      MyPoint pt1 = tmp.pt;
      MyPoint pt2 = FlyWpInfoTmp->wp[0].pt;
      enqueueFieldInOutPoint(i, Args.d, pt1, pt2, 1, wpQue, vField);
    }
    wpQueTmp.clear();
    /* 入队地块的waypoint */
    for (int j = 0; j < FlyWpInfoTmp->m; j++) {
      wpQueTmp.push_back(FlyWpInfoTmp->wp[j]);
    }
    setCombineBaseIndex(combinePoly.p, combinePoly.n, Args.d, Args.refer_border,
                        wpQueTmp, vptChannel);
    for (unsigned int j = 0; j < wpQueTmp.size(); j++) {
      wpQue.push_back(wpQueTmp.at(j));
    }

    /* 入队地块的通道出口点 */
    if ((i == 0) || ((i == 1) && (fieldNum == 3))) {
      PointFlyAttr tmp;
      memset(&tmp, 0, sizeof(PointFlyAttr));
      tmp.BaseIndex = 65532;
      if (vField[i].mChannelNum == 2) {
        if (onSegment(vField[i].ch[0].chLine.p0, vField[i].ch[0].chLine.p1,
                      vField[2].ptChannel[0].pt)) {
          tmp.pt = vField[i].ptChannel[0].pt;
          tmp.SideIndex = vField[i].ptChannel[0].channelIndex;
        } else {
          tmp.pt = vField[i].ptChannel[1].pt;
          tmp.SideIndex = vField[i].ptChannel[1].channelIndex;
        }
      } else {
        tmp.pt = vField[i].ptChannel[0].pt;
        tmp.SideIndex = vField[i].ptChannel[0].channelIndex;
      }
      MyPoint pt1 = wpQue.back().pt;
      MyPoint pt2 = tmp.pt;
      enqueueFieldInOutPoint(i, Args.d, pt1, pt2, 2, wpQue, vField);
      wpQue.push_back(tmp);
    }

    if (FlyWpInfoTmp->wp) {
      delete FlyWpInfoTmp->wp;
      FlyWpInfoTmp->wp = NULL;
    }
    /* 地块合并后自地块的base边不再需要 */
    if (FlyWpInfoTmp->baseBorder != NULL) {
      delete FlyWpInfoTmp->baseBorder;
      FlyWpInfoTmp->baseBorder = NULL;
    }
    if (FlyWpInfoTmp) {
      delete FlyWpInfoTmp;
      FlyWpInfoTmp = NULL;
    }
  }
}

/*
RoutePlanForAndroidCom是用于地块合并航线规划的总接口
combinePoly 是地块合并后的多边形
barGroup 是所有地块的障碍物数组
*/
extern "C" FlyWpInfo *RoutePlanForAndroidCom(FlyBasicArgs Args,
                                             Field *fieldGroup, int fieldNum,
                                             SimplePolygon combinePoly,
                                             SimplePolygon *barGroup,
                                             int barNum) {
  Line l;
  vector<PointFlyAttr> wpQue;
  vector<Field> vField;
  MyPoint qTmp[200];
  int t = 0;
  FlyWpInfo *FlyWpInfoTotal = new FlyWpInfo;
  combineArgs::instance().barrierNum = barNum;
  combineArgs::instance().barrier = barGroup;
  combineArgs::instance().ppSacledBarrier =
      new Polygon *[combineArgs::instance().barrierNum];
  /* 必须先缩边然后才能判断waypoint在哪个障碍物上面 */
  for (int i = 0; i < combineArgs::instance().barrierNum; i++) {
    memset(qTmp, 0, sizeof(qTmp));
    get_envelope_of_polygen(combineArgs::instance().barrier[i].p,
                            combineArgs::instance().barrier[i].n, qTmp, &t,
                            0.6 * OriginArgs::instance().d, 1);
    combineArgs::instance().ppSacledBarrier[i] =
        new Polygon(combineArgs::instance().barrierNum, qTmp);
  }

  if (fieldNum > 3 || fieldNum < 1) {
    FlyWpInfoTotal->ErrCode = ARGS_ERR;
    return FlyWpInfoTotal;
  }
  FlyWpInfoTotal->ErrCode = 0;
  FlyWpInfoTotal->k = 0;
  getVfield(Args, fieldGroup, fieldNum, vField);
  getWpQueCom(Args, fieldGroup, fieldNum, vField, wpQue, combinePoly);
  printFlyWayPoint2(wpQue);
  /* FlyWpInfoTotal 由调用者使用完后释放内存 */
  FlyWpInfoTotal->m = wpQue.size();
  FlyWpInfoTotal->wp = new PointFlyAttr[FlyWpInfoTotal->m];
  for (unsigned int k = 0; k < wpQue.size(); k++) {
    FlyWpInfoTotal->wp[k] = wpQue.at(k);
  }
  wpQue.clear();
  int bNum = 0; /* 总的障碍物个数 */
  for (int i = 0; i < fieldNum; i++) {
    bNum += vField[i].bNum;
  }
  setBaseBorder(combinePoly.p, combinePoly.n, FlyWpInfoTotal);
  CheckWaypointValid(Args, combinePoly.p, combinePoly.n, barGroup, barNum,
                     FlyWpInfoTotal);
  //  vector<Field>().swap(vField);
  checkRoutePlanAferPlan(FlyWpInfoTotal, Args.TakeOffPoint);
  combineArgs::instance().barrierNum = 0;
  delBarScale(combineArgs::instance().ppSacledBarrier,
              combineArgs::instance().barrierNum);
  combineArgs::instance().ppSacledBarrier = NULL;
  return FlyWpInfoTotal;
}
