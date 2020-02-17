#include "split.h"
#include <algorithm> // std::for_each
#include <cmath>
#include <deque>
#include <iostream> // std::cout
#include <iterator> // std::next
#include <list>     // std::list
#include <vae/barrier.h>
#include <vector>
//#include <vae/mygraph.h>

static vector<planCell> cellList; // for build cell graph

bool compare(const PointAttr &a, const PointAttr &b) {
  if (fabs(a.pt.x - b.pt.x) > 0.0001)
    return a.pt.x < b.pt.x;
  else
    return a.pt.y < b.pt.y;
}

/* 转换多边形顶点的时钟顺序 */
void clockConvert(MyPoint *p, int n) {
  int i;
  vector<MyPoint> que;
  for (i = 1; i < n; i++)
    que.push_back(p[i]);
  reverse(que.begin(), que.end());
  for (i = 1; i < n; i++) {
    p[i] = que[i - 1];
  }
}

//将多边形全部转成逆时针顺序
void dataPreprocess(MyPoint *p, int n, SimplePolygon *pB, int bNum) {
  int i = 0;
  bool isClockwise = get_clockwise(p, n);
  if (isClockwise) {
    clockConvert(p, n);
  }
  for (i = 0; i < bNum; i++) {
    isClockwise = get_clockwise(pB[i].p, pB[i].n);
    if (isClockwise) {
      clockConvert(pB[i].p, pB[i].n);
    }
  }
}

/* 获取顶点在多边形中的索引 */
int getPointIndex(MyPoint *p, int n, MyPoint pt) {
  for (int i = 0; i < n; i++) {
    if (pt == p[i]) {
      return i;
    }
  }
  return 0;
}

/*
Event分为IN/OUT/FLOOR/CEILING/SPECIAL五种类型，如何用代码确定event的类型?
临近当前event的两个event的x坐标，如果一个大于current event的x坐标，
另一个小于current event的x坐标，那么此event是FLOOR或CEILING类型，
外边界（地块本身）的middle event不用关注。
如果临近当前event的两个event的x坐标都大于current event的x坐标则为IN event，
如果临近当前event的两个event的x坐标都小于current event的x坐标则为OUT event
*/
void setEventType(planEvent &event, MyPoint *p, int n, SimplePolygon *pB,
                  int bNum) {
  MyPoint curr = event.criticalPoint;
  MyPoint prev, next;
  int index;
  int barrierIndex = event.PolygonIndex - 1;
  if (event.PolygonIndex == 0) {
    index = getPointIndex(p, n, curr);
    prev = p[(index - 1 + n) % n];
    next = p[(index + 1) % n];
  } else {
    index = getPointIndex(pB[barrierIndex].p, pB[barrierIndex].n, curr);
    prev = pB[barrierIndex].p[(index - 1 + n) % n];
    next = pB[barrierIndex].p[(index + 1) % n];
  }
  if (fabs(prev.x - curr.x) <
      0.0001) // 6646t190328090317(1_2):x值近似相等时不能分解
  {
    event.eventType = FLOOR;
  } else if ((prev.x < curr.x) && (next.x < curr.x)) {
    event.eventType = OUT;
    int ret = getOutEventType(event, p, n);
    if (ret == OUTSPECIAL)
      event.eventType = OUTSPECIAL;
  } else if ((prev.x > curr.x) && (next.x > curr.x)) {
    event.eventType = IN;
    int ret = getInEventType(event, p, n);
    if (ret == INSPECIAL)
      event.eventType = INSPECIAL;
  } else if ((next.x > curr.x) && (prev.x < curr.x)) {
    event.eventType = FLOOR;
  } else {
    event.eventType = CEILING;
  }
}

/*
Event分为IN/OUT/FLOOR/CEILING/SPECIAL五种类型，如何用代码确定event的类型?
临近当前event的两个event的x坐标，如果一个大于current event的x坐标，
另一个小于current event的x坐标，那么此event是FLOOR或CEILING类型，
外边界（地块本身）的middle event不用关注。
如果临近当前event的两个event的x坐标都大于current event的x坐标则为IN event，
如果临近当前event的两个event的x坐标都小于current event的x坐标则为OUT event
*/
void setEventTypeWithBarrier(planEvent &event, MyPoint *p, int n,
                             SimplePolygon *pB, int bNum) {
  MyPoint curr = event.criticalPoint;
  MyPoint prev, next;
  int index;
  int pBn = 0;
  int barrierIndex = event.PolygonIndex - 1;
  if (event.PolygonIndex == 0) {
    index = getPointIndex(p, n, curr);
    prev = p[(index - 1 + n) % n];
    next = p[(index + 1) % n];
  } else {
    pBn = pB[barrierIndex].n;
    index = getPointIndex(pB[barrierIndex].p, pB[barrierIndex].n, curr);
    prev = pB[barrierIndex].p[(index - 1 + pBn) % pBn];
    next = pB[barrierIndex].p[(index + 1) % pBn];
  }
  if (fabs(prev.x - curr.x) <
      0.0001) // 6646t190328090317(1_2):x值近似相等时不能分解
  {
    event.eventType = FLOOR;
  } else if ((prev.x < curr.x) && (next.x < curr.x)) {
    event.eventType = OUT;
    if (event.PolygonIndex != 0) {
      int index = event.PolygonIndex;
      MyPoint extendPt1(event.criticalPoint.x, event.criticalPoint.y + 0.1);
      MyPoint extendPt2(event.criticalPoint.x, event.criticalPoint.y - 0.1);
      int ret1 = cn_PnPoly(extendPt1, pB[index - 1].p, pB[index - 1].n);
      int ret2 = cn_PnPoly(extendPt2, pB[index - 1].p, pB[index - 1].n);
      if (ret1 && ret2) /* 只封闭一个cell */
      {
        event.eventType = OUTSPECIAL;
      }
    } else {
      int ret = getOutEventType(event, p, n);
      if (ret == OUTSPECIAL)
        event.eventType = OUTSPECIAL;
    }
  } else if ((prev.x > curr.x) && (next.x > curr.x)) {
    event.eventType = IN;
    if (event.PolygonIndex != 0) {
      int index = event.PolygonIndex;
      MyPoint extendPt1(event.criticalPoint.x, event.criticalPoint.y + 0.1);
      MyPoint extendPt2(event.criticalPoint.x, event.criticalPoint.y - 0.1);
      int ret1 = cn_PnPoly(extendPt1, pB[index - 1].p, pB[index - 1].n);
      int ret2 = cn_PnPoly(extendPt2, pB[index - 1].p, pB[index - 1].n);
      if (ret1 && ret2) /* 只封闭一个cell */
      {
        event.eventType = INSPECIAL;
      }
    } else {
      int ret = getInEventType(event, p, n);
      if (ret == INSPECIAL)
        event.eventType = INSPECIAL;
    }
  } else if ((next.x > curr.x) && (prev.x < curr.x)) {
    event.eventType = FLOOR;
  } else {
    event.eventType = CEILING;
  }
}

bool filledEdges(int edgesType, int polygonIndex, MyPoint pt,
                 pointDequeType dqTmp, vector<PointAttr> &filledObject) {
  PointAttr tmp;
  if (edgesType == 0) {
    if (pt == dqTmp.at(1)) {
      for (unsigned int i = 0; i < dqTmp.size(); i++) {
        tmp.PolygonIndex = polygonIndex;
        tmp.pt = dqTmp.at(i);
        filledObject.push_back(tmp);
      }
      return true;
    }
  }
  if (edgesType == 1) {
    if (pt == dqTmp.at(1)) {
      for (unsigned int i = 0; i < dqTmp.size(); i++) {
        tmp.PolygonIndex = polygonIndex;
        tmp.pt = dqTmp.at(i);
        filledObject.push_back(tmp);
      }
      return true;
    }
  }
  return false;
}

void setCellEdgesSub2(pointDequeType &dqTmp1, pointDequeType &dqTmp2,
                      planCell &cell, PointAttr start, PointAttr end,
                      int edgesType) {
  PointAttr tmp;
  if (edgesType == 0) //填充ceiling edges
  {
    std::sort(dqTmp1.begin(), dqTmp1.end());
    if (((start.pt.x - dqTmp1.front().x) < 0.01) &&
        ((dqTmp1.back().x - end.pt.x) < 0.01)) {
      for (unsigned int i = 0; i < dqTmp1.size(); i++) {
        tmp.PolygonIndex = start.PolygonIndex;
        tmp.pt = dqTmp1.at(i);
        cell.ceiling.push_back(tmp);
      }
    } else {
      for (unsigned int i = 0; i < dqTmp2.size(); i++) {
        tmp.PolygonIndex = start.PolygonIndex;
        tmp.pt = dqTmp2.at(i);
        cell.ceiling.push_back(tmp);
      }
    }
  } else {
    std::sort(dqTmp1.begin(), dqTmp1.end());
    if (((start.pt.x - dqTmp1.front().x) < 0.01) &&
        ((dqTmp1.back().x - end.pt.x) < 0.01)) {
      for (unsigned int i = 0; i < dqTmp1.size(); i++) {
        tmp.PolygonIndex = start.PolygonIndex;
        tmp.pt = dqTmp1.at(i);
        cell.floor.push_back(tmp);
      }
    } else {
      for (unsigned int i = 0; i < dqTmp2.size(); i++) {
        tmp.PolygonIndex = start.PolygonIndex;
        tmp.pt = dqTmp2.at(i);
        cell.floor.push_back(tmp);
      }
    }
  }
}

// edgesType: 决定填充的edge类型，为0时填充ceiling，为1时填充floor
void setCellEdges(MyPoint *p, int n, SimplePolygon *pB, int bNum,
                  planCell &cell, PointAttr start, PointAttr end,
                  int edgesType) {
  int isVertexCount = 0;
  for (int i = 0; i < n; i++) {
    if (p[i] == start.pt) {
      isVertexCount++;
    }
    if (p[i] == end.pt) {
      isVertexCount++;
    }
  }

  if (isVertexCount == 2) {
    pointDequeType dqTmp1, dqTmp2;
    PointAttr tmp;
    getTwoPathWhenVertex(p, n, start.pt, end.pt, dqTmp1, dqTmp2);
    MyPoint nextOfStartPoint;
    MyPoint prevOfStartPoint;
    int index = getPointIndex(p, n, start.pt);
    nextOfStartPoint.x = p[(index + 1) % n].x;
    nextOfStartPoint.y = p[(index + 1) % n].y;
    prevOfStartPoint.x = p[(index - 1 + n) % n].x;
    prevOfStartPoint.y = p[(index - 1 + n) % n].y;
    setCellEdgesSub2(dqTmp1, dqTmp2, cell, start, end, edgesType);
  } else {
    pointDequeType dqTmp1, dqTmp2;
    PointAttr tmp;
    FindShortestPathSub(p, n, start.pt, end.pt, dqTmp1, dqTmp2);
    setCellEdgesSub2(dqTmp1, dqTmp2, cell, start, end, edgesType);
  }
}

// edgesType: 决定填充的edge类型，为0时填充ceiling，为1时填充floor
void setCellEdgesWithBarrier(planEvent event, MyPoint *p, int n,
                             SimplePolygon *pB, int bNum, planCell &cell,
                             PointAttr start, PointAttr end, int edgesType) {
  if (start.PolygonIndex == 0)
    return setCellEdges(p, n, pB, bNum, cell, start, end, edgesType);
  int isVertexCount = 0;
  if ((start.PolygonIndex == end.PolygonIndex) && (start.PolygonIndex != 0)) {
    for (int i = 0; i < pB[start.PolygonIndex - 1].n; i++) {
      if (pB[start.PolygonIndex - 1].p[i] == start.pt) {
        isVertexCount++;
      }
      if (pB[start.PolygonIndex - 1].p[i] == end.pt) {
        isVertexCount++;
      }
    }

    if (isVertexCount == 2) {
      pointDequeType dqTmp1, dqTmp2;
      PointAttr tmp;
      getTwoPathWhenVertex(pB[start.PolygonIndex - 1].p,
                           pB[start.PolygonIndex - 1].n, start.pt, end.pt,
                           dqTmp1, dqTmp2);
      MyPoint nextOfStartPoint;
      MyPoint prevOfStartPoint;
      int index = getPointIndex(pB[start.PolygonIndex - 1].p,
                                pB[start.PolygonIndex - 1].n, start.pt);
      int pBn = pB[start.PolygonIndex - 1].n;
      nextOfStartPoint.x = pB[start.PolygonIndex - 1].p[(index + 1) % pBn].x;
      nextOfStartPoint.y = pB[start.PolygonIndex - 1].p[(index + 1) % pBn].y;
      prevOfStartPoint.x =
          pB[start.PolygonIndex - 1].p[(index - 1 + pBn) % pBn].x;
      prevOfStartPoint.y =
          pB[start.PolygonIndex - 1].p[(index - 1 + pBn) % pBn].y;
      bool ret = false;
      if (edgesType == 0) //填充ceiling edges
      {
        ret = filledEdges(edgesType, start.PolygonIndex, nextOfStartPoint,
                          dqTmp1, cell.ceiling);
        if (!ret) {
          ret = filledEdges(edgesType, start.PolygonIndex, nextOfStartPoint,
                            dqTmp2, cell.ceiling);
        }
      } else if (edgesType == 1) //填充floor edges
      {
        ret = filledEdges(edgesType, start.PolygonIndex, prevOfStartPoint,
                          dqTmp1, cell.floor);
        if (!ret) {
          ret = filledEdges(edgesType, start.PolygonIndex, prevOfStartPoint,
                            dqTmp2, cell.floor);
        }
      }
    } else {
      pointDequeType dqTmp1, dqTmp2;
      PointAttr tmp;
      FindShortestPathSub(pB[start.PolygonIndex - 1].p,
                          pB[start.PolygonIndex - 1].n, start.pt, end.pt,
                          dqTmp1, dqTmp2);
      setCellEdgesSub2(dqTmp1, dqTmp2, cell, start, end, edgesType);
    }
  }
#if 0
	else
	{
		setCellEdges(p, n, pB, bNum, cell, start, end, edgesType);
	}
#endif
}

/*
经过event做垂线与多边形相较于两点，顶部的点称为c，底部的点称为f
*/
void getEventCFWithBarrier(vector<PointAttr> &eventQue, planEvent &event,
                           MyPoint *p, int n, SimplePolygon *pB, int bNum,
                           PointAttr &c, PointAttr &f) {
  int index = 0;
  MyPoint pt1(event.criticalPoint.x, 0);
  MyPoint pt2(event.criticalPoint.x, 100);
  vector<PointAttr> vPointAttr; // event与原始多边形的交点
  vector<MyPoint> vPoint;
  PointAttr tmp;
  get_line_polygon_intersetion2(p, n, pt1, pt2, vPoint);
  for (unsigned int i = 0; i < vPoint.size(); i++) {
    tmp.PolygonIndex = 0;
    tmp.pt = vPoint.at(i);
    vPointAttr.push_back(tmp);
  }
  for (int i = 0; i < bNum; i++) {
    vPoint.clear();
    get_line_polygon_intersetion2(pB[i].p, pB[i].n, pt1, pt2, vPoint);
    for (unsigned int j = 0; j < vPoint.size(); j++) {
      tmp.PolygonIndex = i + 1;
      tmp.pt = vPoint.at(j);
      vPointAttr.push_back(tmp);
    }
  }
  std::sort(vPointAttr.begin(), vPointAttr.end(), compare);
  vector<PointAttr>::iterator iter1 =
      unique(vPointAttr.begin(), vPointAttr.end());
  vPointAttr.erase(iter1, vPointAttr.end());
  for (unsigned int i = 0; i < vPointAttr.size(); i++) {
    if (vPointAttr.at(i).pt == event.criticalPoint) {
      index = i;
      break;
    }
  }
  c = vPointAttr[index + 1];
  f = vPointAttr[index - 1];
}

bool isOutSpecialCell(deque<MyPoint> dqTmp, planEvent &event, MyPoint start,
                      MyPoint end) {
  if (dqTmp.size() < 3)
    return false;
  vector<MyPoint> vTmp;
  MyPoint ptTmp;
  unsigned int i = 0;
  for (i = 0; i < dqTmp.size(); i++) {
    ptTmp = dqTmp.at(i);
    if (fabs(ptTmp.x - start.x) < 0.00001) {
      continue;
    } else {
      vTmp.push_back(ptTmp);
    }
  }
  std::sort(vTmp.begin(), vTmp.end());
  /* vTmp 中所有点必须全部在start end连线左边或全部在连线右边才能形成special
   * cell */
  if (vTmp.front().x < start.x) {
    return false;
  } else {
    return true;
  }
}

/*
function: 求cellPoly
使用start和end的连线将多边形切割为2个，
如果存在完全在切割线右边的多边形则符合要求
*/
int setOutSpecialCell(planCell &currCell, planEvent &event, MyPoint *p, int n,
                      MyPoint start, MyPoint end) {
  pointDequeType dqTmp1;
  pointDequeType dqTmp2;
  FindShortestPathSub(p, n, start, end, dqTmp1, dqTmp2);

  bool ret = isOutSpecialCell(dqTmp1, event, start, end);
  if (ret) {
    for (unsigned int i = 0; i < dqTmp1.size(); i++) {
      currCell.cellPoly.push_back(dqTmp1.at(i));
    }
    return 1;
  }
  ret = isOutSpecialCell(dqTmp2, event, start, end);
  if (ret) {
    for (unsigned int i = 0; i < dqTmp2.size(); i++) {
      currCell.cellPoly.push_back(dqTmp2.at(i));
    }
    return 1;
  }
  return 0;
}

void tryOutSpecialEventCloseCellWithBarrier(planCell &currCell,
                                            planEvent &event, MyPoint *p, int n,
                                            SimplePolygon *pB, int bNum,
                                            PointAttr start, PointAttr end) {
  if (start.PolygonIndex != end.PolygonIndex)
    return;
  int index = start.PolygonIndex;
  int ret = setOutSpecialCell(currCell, event, pB[index - 1].p, pB[index - 1].n,
                              start.pt, end.pt);
  if (ret == 0)
    return;
  ret = isPolygon(currCell.cellPoly);
  if (ret) {
    currCell.cellState = CLOSED;
  } else {
    currCell.cellState = INIT;
    currCell.floor.clear();
    currCell.ceiling.clear();
    currCell.cellPoly.clear();
  }
}

/* 尝试围成一个cell，如果这个cell不存在inEvent或outEvent
 * 且该cell能围成多边形，那么该cell就需要关闭 */
void tryOutSpecialEventCloseCell(planCell &currCell, planEvent &event,
                                 MyPoint *p, int n, SimplePolygon *pB, int bNum,
                                 PointAttr start, PointAttr end) {
  bool isFound = false;
  if (start.PolygonIndex != 0) {
    tryOutSpecialEventCloseCellWithBarrier(currCell, event, p, n, pB, bNum,
                                           start, end);
    return;
  }
  int ret = setOutSpecialCell(currCell, event, p, n, start.pt, end.pt);
  if (ret == 0)
    return;
  for (unsigned int i = 0; i < currCell.cellPoly.size(); i++) {
    if (currCell.cellPoly.at(i) == event.criticalPoint) {
      isFound = true;
      break;
    }
  }
  if (!isFound) {
    currCell.cellState = INIT;
    currCell.floor.clear();
    currCell.ceiling.clear();
    currCell.cellPoly.clear();
    return;
  }
#if 1
  vector<MyPoint> vTmp;
  MyPoint ptTmp;
  for (unsigned int i = 0; i < currCell.cellPoly.size(); i++) {
    ptTmp = currCell.cellPoly.at(i);
    if ((ptTmp == currCell.bottomLeft.pt) || (ptTmp == currCell.topLeft.pt)) {
      continue;
    } else {
      vTmp.push_back(ptTmp);
    }
  }
  planEvent myPlanEventTmp;
  /* 判断currCell 是否可以自我封闭 */
  bool ret2 = true;
  for (unsigned int i = 0; i < vTmp.size(); i++) {
    myPlanEventTmp.criticalPoint = vTmp.at(i);
    myPlanEventTmp.PolygonIndex = 0;
    setEventType(myPlanEventTmp, p, n, 0, 0);
    if ((myPlanEventTmp.eventType == IN) || (myPlanEventTmp.eventType == OUT)) {
      ret2 = false;
      break;
    }
  }
#endif
  ret = isPolygon(currCell.cellPoly);
  if (ret && ret2) {
    currCell.cellState = CLOSED;
  } else {
    currCell.cellState = INIT;
    currCell.floor.clear();
    currCell.ceiling.clear();
    currCell.cellPoly.clear();
  }
}

void inEventOpenCellWithBarrier(vector<PointAttr> &eventQue, planEvent &event,
                                MyPoint *p, int n, SimplePolygon *pB, int bNum,
                                int &closeCellIndex1, int &closeCellIndex2) {
  PointAttr eventPoint;
  eventPoint.pt = event.criticalPoint,
  eventPoint.PolygonIndex = event.PolygonIndex;
  planCell topCell, bottomCell;
  PointAttr c; // ceiling point
  PointAttr f; // floor point
  getEventCFWithBarrier(eventQue, event, p, n, pB, bNum, c, f);
  /* 先入队BOTTOM CELL再入队TOP CELL */
  bottomCell.cellIndex = cellList.size();
  cellList.push_back(bottomCell);
  cellList.back().bottomLeft = f;
  cellList.back().topLeft = eventPoint;
  cellList.back().cellState = BOTTOM;
  cellList.back().in = event;
  closeCellIndex1 = bottomCell.cellIndex;

  topCell.cellIndex = cellList.size();
  cellList.push_back(topCell);
  cellList.back().bottomLeft = eventPoint;
  cellList.back().topLeft = c;
  cellList.back().cellState = TOP;
  cellList.back().in = event;
  closeCellIndex2 = topCell.cellIndex;
}

const double eps = 1e-4;
int dcmp(double x) {
  if (fabs(x) < eps)
    return 0;
  return x < 0 ? -1 : 1;
}

double Dot(Vector A, Vector B) { return A.x * B.x + A.y * B.y; }
double Length(Vector A) { return sqrt(Dot(A, A)); }
double Cross(Vector A, Vector B) { return A.x * B.y - A.y * B.x; }

bool InSegment(MyPoint P, MyPoint a1,
               MyPoint a2) //判断点P是否在线段a1a2上(在端点上也算)
{
  return dcmp(Cross(a1 - P, a2 - P)) == 0 &&
         dcmp(Dot(a1 - P, a2 - P)) <= 0; //注意这里的<=0
}

bool SegmentIntersection(MyPoint a1, MyPoint a2, MyPoint b1,
                         MyPoint b2) //判断线段a1a2与线段b1b2是否存在任何公共点
{
  double c1 = Cross(a2 - a1, b1 - a1), c2 = Cross(a2 - a1, b2 - a1);
  double c3 = Cross(b2 - b1, a1 - b1), c4 = Cross(b2 - b1, a2 - b1);
  if (dcmp(c1) * dcmp(c2) < 0 && dcmp(c3) * dcmp(c4) < 0)
    return true;
  if (dcmp(c1) == 0 && InSegment(b1, a1, a2))
    return true;
  if (dcmp(c2) == 0 && InSegment(b2, a1, a2))
    return true;
  if (dcmp(c3) == 0 && InSegment(a1, b1, b2))
    return true;
  if (dcmp(c4) == 0 && InSegment(a2, b1, b2))
    return true;
  return false;
}

bool isPolygon(MyPoint *p, int n) //判定是否能构成多边形
{
  p[n] = p[0];
  for (int i = 0; i < n; ++i)       //枚举第一条线段
    for (int j = i + 1; j < n; ++j) //枚举第二条线段
    {
      if (j == i + 1 || (j + 1) % n == i)
        continue; //两线段本来就相邻
      if (SegmentIntersection(p[i], p[i + 1], p[j], p[j + 1]))
        return false;
    }
  return true;
}

bool isPolygon(vector<MyPoint> p) //判定是否能构成多边形
{
  int n = p.size();
  p.push_back(p[0]);
  for (int i = 0; i < n; ++i)       //枚举第一条线段
    for (int j = i + 1; j < n; ++j) //枚举第二条线段
    {
      if (j == i + 1 || (j + 1) % n == i)
        continue; //两线段本来就相邻
      if (SegmentIntersection(p[i], p[i + 1], p[j], p[j + 1]))
        return false;
    }
  return true;
}

bool isPolygonNeeded(planCell currCell) //判定是否是我们需要的多边形
{
  vector<MyPoint> tmp;
  if (currCell.cellPoly.size() == 0)
    return false;
  bool ret = isPolygon(currCell.cellPoly);
  if (!ret)
    return false;
  for (unsigned int i = 0; i < currCell.cellPoly.size(); i++) {
    tmp.push_back(currCell.cellPoly.at(i));
  }
  std::sort(tmp.begin(), tmp.end());
  if (((currCell.topLeft.pt.x - tmp.front().x) > 0.001) &&
      ((currCell.bottomLeft.pt.x - tmp.front().x) > 0.001))
    return false;
  if (((tmp.back().x - currCell.topRight.pt.x) > 0.001) &&
      ((tmp.back().x - currCell.bottomRight.pt.x) > 0.001))
    return false;
  return true;
}

bool eventFindMatchCell(planEvent event, PointAttr topRight,
                        PointAttr bottomRight, MyPoint *p, int n,
                        SimplePolygon *pB, int bNum, int &cellIndex) {
  bool ret = false;
  for (unsigned int i = 0; i < cellList.size(); i++) {
    if (cellList.at(i).cellState != CLOSED) {
      if ((topRight.PolygonIndex == cellList.at(i).topLeft.PolygonIndex) &&
          (bottomRight.PolygonIndex ==
           cellList.at(i).bottomLeft.PolygonIndex)) {
        cellIndex = i;
        cellList.at(cellIndex).topRight = topRight;
        cellList.at(cellIndex).bottomRight = bottomRight;
        closeCellWithBarrier(event, cellList.at(i), p, n, pB, bNum);
        ret = isPolygonNeeded(cellList.at(i));
        if (ret) {
          cellList.at(i).cellState = CLOSED;
          return true;
        } else {
          cellList.at(i).cellState = INIT;
          cellList.at(i).floor.clear();
          cellList.at(i).ceiling.clear();
          cellList.at(i).cellPoly.clear();
        }
      }
    }
  }
  return false;
}

void inEventCloseCellWithBarrier(vector<PointAttr> &eventQue, planEvent &event,
                                 MyPoint *p, int n, SimplePolygon *pB, int bNum,
                                 int &closeCellIndex) {
  PointAttr c; // ceiling point
  PointAttr f; // floor point
  getEventCFWithBarrier(eventQue, event, p, n, pB, bNum, c, f);
  if (0 == cellList.size()) {
    planCell firstCell;
    firstCell.cellIndex = 0;
    if (eventQue.at(0).pt.y > eventQue.at(1).pt.y) {
      firstCell.topLeft = eventQue.at(0);
      firstCell.bottomLeft = eventQue.at(1);
    } else {
      firstCell.topLeft = eventQue.at(1);
      firstCell.bottomLeft = eventQue.at(0);
    }
    cellList.push_back(firstCell);
    cellList.at(0).topRight = c;
    cellList.at(0).bottomRight = f;
    closeCellWithBarrier(event, cellList.at(0), p, n, pB, bNum);
    firstCell.cellState = CLOSED;
    closeCellIndex = 0;
  } else {
    int cellIndex = -1;
    eventFindMatchCell(event, c, f, p, n, pB, bNum, cellIndex);
  }
}

int getOutEventType(planEvent &event, MyPoint *p, int n) {
  MyPoint extendPt1(event.criticalPoint.x, event.criticalPoint.y + 0.2);
  MyPoint extendPt2(event.criticalPoint.x, event.criticalPoint.y - 0.2);
  int ret1 = cn_PnPoly(extendPt1, p, n);
  int ret2 = cn_PnPoly(extendPt2, p, n);
  if (!ret1 && !ret2) /* 只封闭一个cell */
  {
    return OUTSPECIAL;
  }
  return OUT;
}

/*
判断in event是in 还是in special类型
判断的方法: 将 in
event沿着y轴做延长线，在延长线上取得2个点，如果这两个点都不在原始地块
多边形内，那么该点是in special
*/
int getInEventType(planEvent &event, MyPoint *p, int n) {
  MyPoint extendPt1(event.criticalPoint.x, event.criticalPoint.y + 0.1);
  MyPoint extendPt2(event.criticalPoint.x, event.criticalPoint.y - 0.1);
  int ret1 = cn_PnPoly(extendPt1, p, n);
  int ret2 = cn_PnPoly(extendPt2, p, n);
  if (!ret1 && !ret2) /* 只封闭一个cell */
  {
    return INSPECIAL;
  }
  return IN;
}

void closeCellWithBarrier(planEvent event, planCell &currCell, MyPoint *p,
                          int n, SimplePolygon *pB, int bNum) {
  PointAttr start = currCell.topLeft;
  PointAttr end = currCell.topRight;
  setCellEdgesWithBarrier(event, p, n, pB, bNum, currCell, start, end, 0);
  start = currCell.bottomLeft;
  end = currCell.bottomRight;
  setCellEdgesWithBarrier(event, p, n, pB, bNum, currCell, start, end, 1);
  currCell.cellState = CLOSED;
  if (currCell.topRight == currCell.bottomRight) {
    for (vector<PointAttr>::iterator it = currCell.floor.begin();
         it != currCell.floor.end(); it++) {
      currCell.cellPoly.push_back((*it).pt);
    }
    currCell.cellPoly.pop_back();
    for (vector<PointAttr>::reverse_iterator it = currCell.ceiling.rbegin();
         it != currCell.ceiling.rend(); it++) {
      currCell.cellPoly.push_back((*it).pt);
    }
  } else {
    for (vector<PointAttr>::iterator it = currCell.floor.begin();
         it != currCell.floor.end(); it++) {
      currCell.cellPoly.push_back((*it).pt);
    }
    for (vector<PointAttr>::reverse_iterator it = currCell.ceiling.rbegin();
         it != currCell.ceiling.rend(); it++) {
      currCell.cellPoly.push_back((*it).pt);
    }
    if (currCell.topLeft == currCell.bottomLeft) {
      currCell.cellPoly.pop_back();
    }
  }
}

int getCurrOpenCellNum(vector<int> &vIndex) {
  int num = 0;
  for (unsigned int i = 0; i < cellList.size(); i++) {
    if (cellList.at(i).cellState != CLOSED) {
      vIndex.push_back(i);
      num++;
    }
  }
  return num;
}

/* 查找某个点是否位于多边形上，如果是，则返回他所处边的序号 */
void getBorderIndex(MyPoint *p, int n, MyPoint pt, vector<int> &borderIndex) {
  Segment s;
  int ret = 0;
  for (int i = 0; i < n; i++) {
    s.p0 = p[i];
    s.p1 = p[(i + 1) % n];
    ret = onSegment(s.p0, s.p1, pt);
    if (ret == 1) {
      borderIndex.push_back(i);
    }
  }
}

void setInSpecialCell(planCell &currCell, planEvent &event, MyPoint *p, int n,
                      MyPoint start, MyPoint end) {
  pointDequeType dqTmp1;
  pointDequeType dqTmp2;
  bool chooseTmp1 = true;
  FindShortestPathSub(p, n, start, end, dqTmp1, dqTmp2);
  /* 去掉重复的点，当航线与多边形顶点相交时会重复出现 */
  deque<MyPoint>::iterator iter1 = unique(dqTmp1.begin(), dqTmp1.end());
  dqTmp1.erase(iter1, dqTmp1.end());
  deque<MyPoint>::iterator iter2 = unique(dqTmp2.begin(), dqTmp2.end());
  dqTmp2.erase(iter2, dqTmp2.end());
  for (unsigned int i = 0; i < dqTmp1.size(); i++) {
    if (dqTmp1.at(i).x - event.criticalPoint.x > 0.01) {
      chooseTmp1 = false;
      break;
    }
  }
  if (chooseTmp1) {
    for (unsigned int i = 0; i < dqTmp1.size(); i++) {
      currCell.cellPoly.push_back(dqTmp1.at(i));
    }
  } else {
    for (unsigned int i = 0; i < dqTmp2.size(); i++) {
      currCell.cellPoly.push_back(dqTmp2.at(i));
    }
  }
}

void outEventCloseCellWithBarrier(vector<PointAttr> &eventQue, planEvent &event,
                                  MyPoint *p, int n, SimplePolygon *pB,
                                  int bNum, int &closeCellIndex1,
                                  int &closeCellIndex2) {
  PointAttr c; // ceiling point
  PointAttr f; // floor point
  PointAttr eventTmp;
  int cellIndex = -1;
  eventTmp.PolygonIndex = event.PolygonIndex;
  eventTmp.pt = event.criticalPoint;

  getEventCFWithBarrier(eventQue, event, p, n, pB, bNum, c, f);
  eventFindMatchCell(event, c, eventTmp, p, n, pB, bNum, cellIndex);
  eventFindMatchCell(event, eventTmp, f, p, n, pB, bNum, cellIndex);
}

void outEventOpenCellWithBarrier(vector<PointAttr> &eventQue, planEvent &event,
                                 MyPoint *p, int n, SimplePolygon *pB, int bNum,
                                 int &openCellIndex) {
  PointAttr eventPoint;
  eventPoint.pt = event.criticalPoint,
  eventPoint.PolygonIndex = event.PolygonIndex;
  PointAttr c; // ceiling point
  PointAttr f; // floor point
  getEventCFWithBarrier(eventQue, event, p, n, pB, bNum, c, f);
  planCell currCell;
  currCell.cellIndex = cellList.size();
  cellList.push_back(currCell);
  cellList.back().bottomLeft = f;
  cellList.back().topLeft = c;
  cellList.back().cellState = INIT;
  cellList.back().in = event;
  openCellIndex = currCell.cellIndex;
}

void makeCellConnected(int index1, int index2, int index3) {
  if ((index1 < 0) || (index2 < 0) || (index3 < 0))
    return;
  cellList.at(index1).neighborCellIndex.push_back(index2);
  cellList.at(index1).neighborCellIndex.push_back(index3);
  cellList.at(index2).neighborCellIndex.push_back(index1);
  cellList.at(index3).neighborCellIndex.push_back(index1);
}

void eventHandlerWithBarrier(vector<PointAttr> &eventQue, planEvent &event,
                             MyPoint *p, int n, SimplePolygon *pB, int bNum) {
  if (event.eventType == IN) {
    int closeCellIndex = -1;
    int openCellIndex1 = -1;
    int openCellIndex2 = -1;
    inEventCloseCellWithBarrier(eventQue, event, p, n, pB, bNum,
                                closeCellIndex);
    inEventOpenCellWithBarrier(eventQue, event, p, n, pB, bNum, openCellIndex1,
                               openCellIndex2);
  } else if (event.eventType == OUT) {
    int closeCellIndex1 = -1;
    int closeCellIndex2 = -1;
    int openCellIndex = -1;
    outEventCloseCellWithBarrier(eventQue, event, p, n, pB, bNum,
                                 closeCellIndex1, closeCellIndex2);
    outEventOpenCellWithBarrier(eventQue, event, p, n, pB, bNum, openCellIndex);
  } else if (event.eventType == OUTSPECIAL) {
    vector<int> vIndex;
    int openNum = getCurrOpenCellNum(vIndex);
    for (int i = 0; i < openNum; i++) {
      tryOutSpecialEventCloseCell(cellList.at(vIndex.at(i)), event, p, n, pB,
                                  bNum, cellList.at(vIndex.at(i)).bottomLeft,
                                  cellList.at(vIndex.at(i)).topLeft);
    }
  } else if (event.eventType == INSPECIAL) /* open a cell */
  {
    planCell currCell;
    currCell.cellIndex = cellList.size();
    currCell.cellState = INIT;
    currCell.topLeft.pt = event.criticalPoint;
    currCell.topLeft.PolygonIndex = event.PolygonIndex;
    currCell.bottomLeft.pt = event.criticalPoint;
    currCell.bottomLeft.PolygonIndex = event.PolygonIndex;
    cellList.push_back(currCell);
  }
}

extern vector<int> traverseOrder;

void delCellNode(int **cellNode, int size) {
  if (cellNode != NULL) {
    for (int i = 0; i < size; i++) {
      delete cellNode[i];
      cellNode[i] = NULL;
    }
    delete[] cellNode;
    cellNode = NULL;
  }
}

SimplePolygon *subPoly = NULL;
int subPolyNum = 0;

/* final中存放地块的遍历顺序编号 */
bool buildMap(std::vector<int> &final) {
  int size = cellList.size();

  subPoly = new SimplePolygon[size];
  subPolyNum = cellList.size();
  for (int i = 0; i < size; ++i) {
    subPoly[i].n = cellList.at(i).cellPoly.size();
    subPoly[i].p = new MyPoint[subPoly[i].n];
#if DEBUG
    cout << "z" << i << "="
         << "(POINT * " << subPoly[i].n << ")()" << endl;
#endif

    if (cellList.at(i).cellPoly.size() < 3) {
      cout << ".............index=" << i << endl;
      return true; // is wrong
    }

    for (unsigned int j = 0; j < cellList.at(i).cellPoly.size(); j++) {
      subPoly[i].p[j] = cellList.at(i).cellPoly.at(j);
#if DEBUG
      cout << "z" << i << "[" << j << "]"
           << "= POINT(" << subPoly[i].p[j].x << "," << subPoly[i].p[j].y << ")"
           << endl;
#endif
    }
    bool ret = isPolygon(cellList.at(i).cellPoly);
    if (ret == false) {
      cout << ".............index=" << i << endl;
      return true; // is wrong
    }

    cout << endl;
  }
  delete[] subPoly;

  // TODO: 调试专用
  //	return ;

  for (unsigned int i = 0; i < cellList.size(); i++) {
    final.push_back(i);
#if DEBUG
    cout << final.at(i) << " ";
#endif
  }
  return false;
}

bool isNeedPlanning(FlyBasicArgs Args, int i) {
  MyPoint cellP[2000];
  vector<MyPoint> vPoint;
  int cellN = cellList.at(i).cellPoly.size();
  for (int j = 0; j < cellN; j++) {
    cellP[j] = cellList.at(i).cellPoly.at(j);
    vPoint.push_back(cellP[j]);
  }
  int size = my_area2D_Polygon(cellN, cellP);
  std::sort(vPoint.begin(), vPoint.end());
  double width = vPoint.back().x - vPoint.front().x;
  if (size < 10) /* 面积过小的地块不做规划 */
  {
    return false;
  } else if (width < 0.5 * Args.d) /* 过于狭窄的地块不做规划 */
  {
    return false;
  } else if ((cellN == 3) && (width < 0.6 * Args.d)) {
    return false;
  }
  return true;
}

void sweepLineWithBarrier(vector<PointAttr> &eventQue, MyPoint *p, int n,
                          SimplePolygon *pB, int bNum) {
  planEvent myPlanEventTmp;
  for (vector<PointAttr>::iterator it = eventQue.begin(); it != eventQue.end();
       it++) {
    myPlanEventTmp.criticalPoint = (*it).pt;
    myPlanEventTmp.PolygonIndex = (*it).PolygonIndex;
    setEventTypeWithBarrier(myPlanEventTmp, p, n, pB, bNum);
    eventHandlerWithBarrier(eventQue, myPlanEventTmp, p, n, pB, bNum);
  }
}

double alaph = 0.0;

void getTransformPoly(FlyBasicArgs Args, const MyPoint *p, int n, MyPoint *q) {
  MyPoint pt(Args.refer_border.p0.x, Args.refer_border.p0.y - 10);
  double t =
      getAngelOfTwoVector2(Args.refer_border.p1, pt, Args.refer_border.p0);
  t = t + 0.05; /*  必须偏移，否则无法使用竖直分解算法 */
  alaph = t;
  // cout<<"t="<<t<<endl;
  t = (t * 3.1415926) / 180;
  // cout<<"t="<<t<<endl;
  for (int i = 0; i < n; i++) {
    q[i].x = p[i].x * cos(t) - p[i].y * sin(t);
    q[i].y = p[i].x * sin(t) + p[i].y * cos(t);
  }
}

void getOriginPoly(PointFlyAttr *q, int n, MyPoint *r) {
  double t = -alaph;
  t = (t * 3.1415926) / 180;

  for (int i = 0; i < n; i++) {
    r[i].x = q[i].pt.x * cos(t) - q[i].pt.y * sin(t);
    r[i].y = q[i].pt.x * sin(t) + q[i].pt.y * cos(t);
  }
}

void getTransformPoint(PointFlyAttr *wp, int m) {
  double t = -alaph;
  t = (t * 3.1415926) / 180;
  MyPoint tmp;

  for (int i = 0; i < m; i++) {
    tmp.x = wp[i].pt.x * cos(t) - wp[i].pt.y * sin(t);
    tmp.y = wp[i].pt.x * sin(t) + wp[i].pt.y * cos(t);
    wp[i].pt.x = tmp.x;
    wp[i].pt.y = tmp.y;
  }
}

MyPoint getTransformPoint(MyPoint mypt) {
  double t = -alaph;
  t = (t * 3.1415926) / 180;
  MyPoint tmp;
  tmp.x = mypt.x * cos(t) - mypt.y * sin(t);
  tmp.y = mypt.x * sin(t) + mypt.y * cos(t);
  mypt.x = tmp.x;
  mypt.y = tmp.y;
  return mypt;
}

//看能不能通过调用MultiLinePointWithBarrierHandlerSub来解决
void getSubFieldTranstionPoint(MyPoint *p, int n, SimplePolygon *pB, int bNum,
                               PointFlyAttr *wpArr2, int &k, Graph const &graph,
                               PointAttr pT0, PointAttr pT1,
                               deque<MyPoint> &dqWpEveryTime) {
  if (bNum) {
    return getSubFieldTranstionPointWithBarrier(p, n, pB, bNum, wpArr2, k,
                                                graph, pT0, pT1, dqWpEveryTime);
  }
  vector<MyPoint> vPoint;
  vector<MyPoint> carePoint; /* 存放过渡点 */
  deque<MyPoint> dqTmp;
  bool ret = false;
  get_line_polygon_intersetion2(p, n, pT0.pt, pT1.pt, vPoint);
  carePoint.push_back(pT0.pt);
  for (unsigned int i = 0; i < vPoint.size(); i++) {
    ret = onSegment2(pT0.pt, pT1.pt, vPoint.at(i));
    if (ret) {
      carePoint.push_back(vPoint.at(i));
    }
  }
  carePoint.push_back(pT1.pt);
  std::sort(carePoint.begin(), carePoint.end());
  if (pT0.pt.x > pT1.pt.x) {
    std::reverse(carePoint.begin(), carePoint.end());
  }
  if (carePoint.size() < 2) {
    return;
  }
  dqWpEveryTime.push_back(carePoint.at(0));
  for (unsigned int i = 1; i < carePoint.size(); i++) {
    dqTmp.clear();
    ret = intersect2D_SegPoly3(p, n, carePoint.at(i - 1), carePoint.at(i), 0);
    if (ret) {
      FindShortestPathOfTwoPoint(p, n, carePoint.at(i - 1), carePoint.at(i),
                                 dqTmp);
    } else {
      dqTmp.push_back(carePoint.at(i));
    }
    for (unsigned int i = 0; i < dqTmp.size(); i++) {
      dqWpEveryTime.push_back(dqTmp.at(i));
    }
  }
}

/*
功能: 解决单次航线相邻两个航线点构成的线段与多边形相交的问题。
pt0和pt1是航线上相邻的两个航线点
*/
void FindMiddlePointOfNeighboringPoint3(MyPoint *p, int n, SimplePolygon *pB,
                                        int bNum, PointAttr pT0, PointAttr pT1,
                                        pointDequeType &dqWpEveryTime) {
  /* 如果pt0和pt1属于不同多边形则直接入队 */
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
  } else /* 无论是障碍物还是地块都需要绕边 */
  {
    MyPoint pt0 = pT0.pt;
    MyPoint pt1 = pT1.pt;

    if (0 == pT0.PolygonIndex) {
      /* 如果pt0或pt1处于外边界则不需要绕边 */
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
      p = pB[pT0.PolygonIndex - 1].p;
      n = pB[pT0.PolygonIndex - 1].n;
    }
    FindShortestPathOfTwoPoint(p, n, pt0, pt1, dqWpEveryTime);
  }
}

void MultiLinePointWithBarrierHandlerSub2(MyPoint *p, int n, SimplePolygon *pB,
                                          int bNum,
                                          pointAttrDequeType dqLinePoint,
                                          pointDequeType &dqWpEveryTime) {
  unsigned int i = 0;
  PointAttr pT0, pT1;
  for (i = 0; i < dqLinePoint.size() - 1; i++) {
    pT0 = dqLinePoint.at(i);
    pT1 = dqLinePoint.at(i + 1);
    FindMiddlePointOfNeighboringPoint3(p, n, pB, bNum, pT0, pT1, dqWpEveryTime);
  }
}

void getSubFieldTranstionPointWithBarrier(MyPoint *p, int n, SimplePolygon *pB,
                                          int bNum, PointFlyAttr *wpArr2,
                                          int &k, Graph const &graph,
                                          PointAttr pT0, PointAttr pT1,
                                          deque<MyPoint> &dqWpEveryTime) {
  vector<MyPoint> vPoint;
  PointAttr paTmp;
  deque<PointAttr> insectPoint; /* 存放线段与多边形交点 */
  unsigned int line_point_num = 0; //直线与多边形交点个数(即航线点个数)
  MyPoint pIntersection[200]; //航线点数组
  bool ret = false;
  get_line_polygon_intersetion2(p, n, pT0.pt, pT1.pt, vPoint);
  insectPoint.push_back(pT0);
  /*  只需要考虑线段pt0-pt1与多边形的交点 */
  for (unsigned int i = 0; i < vPoint.size(); i++) {
    ret = onSegment2(pT0.pt, pT1.pt, vPoint.at(i));
    if (ret) {
      paTmp.pt = vPoint.at(i);
      paTmp.PolygonIndex = getPointPolygonIndex(paTmp.pt, p, n, pB, bNum);
      ;
      insectPoint.push_back(paTmp);
    }
  }
  insectPoint.push_back(pT1);
  for (int i = 0; i < bNum; i++) {
    get_line_polygon_intersetion(pB[i].p, pB[i].n, pT0.pt, pT1.pt,
                                 pIntersection, &line_point_num);
    for (unsigned int m = 0; m < line_point_num; m++) {
      ret = onSegment2(pT0.pt, pT1.pt, pIntersection[m]);
      if (ret) {
        paTmp.pt = pIntersection[m];
        paTmp.PolygonIndex = i + 1;
        insectPoint.push_back(paTmp);
      }
    }
  }
  if (insectPoint.size() == 2) {
    dqWpEveryTime.push_back(pT0.pt);
    dqWpEveryTime.push_back(pT1.pt);
    return;
  }
  std::sort(insectPoint.begin(), insectPoint.end(), compare);
  deque<PointAttr>::iterator iter1 =
      unique(insectPoint.begin(), insectPoint.end());
  insectPoint.erase(iter1, insectPoint.end());
  if (pT0.pt.x > pT1.pt.x) {
    std::reverse(insectPoint.begin(), insectPoint.end());
  }

  MultiLinePointWithBarrierHandlerSub2(p, n, pB, bNum, insectPoint,
                                       dqWpEveryTime);
}

/* 通过引入过渡点来连接相邻子地块
k:  wpArr2中元素个数
*/
void connectSubField(FlyBasicArgs Args, const MyPoint *p, int n,
                     SimplePolygon *pB, int bNum, PointFlyAttr *wpArr,
                     PointFlyAttr *wpArr2, int &k, MyPoint in,
                     int currCellIndex, int lastCellIndex, Graph g) {
  pointDequeType dqWpEveryTime;
  PointAttr pT0, pT1;
  pT0.pt = wpArr2[k - 1].pt;
  pT0.PolygonIndex = getPointPolygonIndex(pT0.pt, p, n, pB, bNum);
  pT1.pt = wpArr[0].pt;
  pT1.PolygonIndex = getPointPolygonIndex(pT1.pt, p, n, pB, bNum);
  if ((unsigned int)pT1.PolygonIndex < cellList.size()) {
    pT1.pt = wpArr[0].pt;
    getSubFieldTranstionPoint((MyPoint *)p, n, pB, bNum, wpArr2, k, g, pT0, pT1,
                              dqWpEveryTime);
  }
  for (unsigned int i = 0; i < dqWpEveryTime.size(); i++) {
    MyPoint curr = dqWpEveryTime.at(i);
    if ((curr == wpArr2[k - 1].pt) || (curr == wpArr[0].pt))
      continue;
    else {
      wpArr2[k].pt = dqWpEveryTime.at(i);
      wpArr2[k].BaseIndex = 65533;
      wpArr2[k].SideIndex = 65535;
      wpArr2[k].SegmentType = 0;
      wpArr2[k].SprayCtrl = 0;
      k++;
    }
  }
}

void getPlanningParams(MyPoint *cellP, int &cellN, FlyBasicArgs &Args,
                       int loopTimes, int &k, MyPoint lastWp) {
  vector<planCell> cellListShadow;
  vector<MyPoint> vPoint;
  for (unsigned int i = 0; i < cellList.size(); i++) {
    cellListShadow.push_back(cellList.at(i));
  }
  cellN = cellListShadow.at(loopTimes).cellPoly.size();
  for (int j = 0; j < cellN; j++) {
    cellP[j] = cellListShadow.at(loopTimes).cellPoly.at(j);
    vPoint.push_back(cellP[j]);
  }
  std::sort(vPoint.begin(), vPoint.end());
  /* 切分的小地块以过临界点的垂线为参考边 */
  for (int i = 0; i < cellN; i++) {
    if (fabs(cellP[i].x - cellP[(i + 1) % cellN].x) < 0.0001) {
      Args.refer_border.p0 = cellP[i];
      Args.refer_border.p1 = cellP[(i + 1) % cellN];
      if ((vPoint.front() == Args.refer_border.p0) ||
          (vPoint.front() == Args.refer_border.p1))
        break;
    }
  }
  if (k - 1 > 0) {
    Args.TakeOffPoint = lastWp;
  }
}

Graph makeGraph(vector<planCell> cellList) {
  // initialize edges as per above diagram
  vector<Edge> edges;
  Edge tmp;
  for (unsigned int i = 0; i < cellList.size(); ++i) {
    for (unsigned int j = 0; j < cellList.at(i).neighborCellIndex.size(); j++) {
      tmp.weight = 1;
      tmp.source = cellList.at(i).cellIndex;
      tmp.dest = cellList.at(i).neighborCellIndex.at(j);
      edges.push_back(tmp);
    }
  }

  // Number of nodes in the graph
  int N = cellList.size();

  // construct graph
  Graph graph(edges, N);
  return graph;
}

/*
 */
int getPointPolygonIndex(MyPoint pt, const MyPoint *p, int n, SimplePolygon *pB,
                         int bNum) {
  int i, j;
  for (i = 0; i < n; i++) {
    if (onSegment(p[i], p[(i + 1) % n], pt)) {
      return 0;
    }
  }
  for (i = 0; i < bNum; i++) {
    for (j = 0; j < pB[i].n; j++) {
      if (onSegment(pB[i].p[j], pB[i].p[(j + 1) % pB[i].n], pt)) {
        return i + 1;
      }
    }
  }
  return -1;
}

/* 分别对各个子地块进行航线规划
wpArr: 保存当前子地块的规划结果
m: wpArr中元素个数
wpArr2: 保存上一个子地块的规划结果
k: wpArr2中元素个数
*/
FlyWpInfo *subFieldPlanning(FlyBasicArgs Args, const MyPoint *p, int n,
                            SimplePolygon *pB, int bNum,
                            std::vector<int> &final, int &k, int &m,
                            PointFlyAttr *wpArr, PointFlyAttr *wpArr2,
                            FlyWpInfo *pFlyWpInfoInstance,
                            PointFlyAttr *SweepPointArr) {
  bool isNeed = false;
  MyPoint cellP[2000];
  int cellN = 0;
  MyPoint in;
  Graph g = makeGraph(cellList);
  int lastCellIndex = 0;
  int k1 = 0;

  for (int i = 0; i < (int)final.size(); i++) {
    isNeed = isNeedPlanning(Args, i);
    if ((unsigned int)(i + 1) < final.size()) {
      in = cellList.at(i).in.criticalPoint;
    }
    if (isNeed) {
      if (k > 0) {
        getPlanningParams(cellP, cellN, Args, i, k, wpArr2[k - 1].pt);
      } else {
        MyPoint lastWp = Args.TakeOffPoint;
        getPlanningParams(cellP, cellN, Args, i, k, lastWp);
      }
      m = 0;
      int ret = route_plan2(Args, cellP, cellN, pB, bNum, wpArr, &m,
                            SweepPointArr, &k1);
      if (ret != 0) {
        cout << "ErrCode=" << pFlyWpInfoInstance->ErrCode << endl;
        pFlyWpInfoInstance->ErrCode = ret;
        return pFlyWpInfoInstance;
      }
      if (m > 0 && k > 0) {
        connectSubField(Args, p, n, pB, bNum, wpArr, wpArr2, k, in, i,
                        lastCellIndex, g);
        lastCellIndex = i;
      }
      for (int j = 0; j < m; j++) {
        wpArr2[k] = wpArr[j];
        k++;
      }
    } else {
    }
  }
  m = k;
  if (m > 0)
    pFlyWpInfoInstance->wp = new PointFlyAttr[m];
  else
    pFlyWpInfoInstance->wp = NULL;
  for (int i = 0; i < m; i++) {
    pFlyWpInfoInstance->wp[i] = wpArr2[i];
  }
  return pFlyWpInfoInstance;
}

extern LeakSprayInfo gLeakSprayInfo;
extern "C" FlyWpInfo *RoutePlanForAndroid2(FlyBasicArgs Args, const MyPoint *p,
                                           int n, SimplePolygon *pB, int bNum) {
  FlyWpInfo *pFlyWpInfoInstance = new FlyWpInfo;
  MyPoint *pShadow = new MyPoint[n]; //存放地块多边形旋转后的结果
  MyPoint *pTransform = new MyPoint[n]; //存放外多边形进行时针顺序转换后的结果
  memcpy(pTransform, p, n * sizeof(MyPoint));
  setOriginArgs(p, n, Args.d, Args.refer_border);
  setBaseBorder(p, n, pFlyWpInfoInstance);
  vector<PointAttr> eventQue;
  PointAttr tmp;
  PointFlyAttr wpArr[MAX_WP_NUM];
  PointFlyAttr wpArr2[MAX_WP_NUM];
  PointFlyAttr SweepPointArr[500];
  int k = 0;
  int m = 0;
  int t1 = 0;
  int t2 = 0;
  Polygon *pGround;
  pFlyWpInfoInstance->k = 0;
  bool ClockWise = false;
  pointDequeType dqPitsArrTemp;
  bool IsConvex = isConvex((MyPoint *)p, n, &ClockWise, dqPitsArrTemp);
  if (IsConvex)
    return RoutePlanForAndroid(Args, (MyPoint *)p, n, pB, bNum);
  dataPreprocess(pTransform, n, pB, bNum);
  if (bNum > 0) {
    pGround = new Polygon(n, (MyPoint *)p); //地块
    initBarrierFieldObject(pGround, (MyPoint *)p, n, pB, bNum,
                           OriginArgs::instance().d);
  }

  MyPoint InterEnvelope[MAX_WP_NUM];
  MyPoint qTmp[MAX_WP_NUM];
  get_envelope_of_polygen(pTransform, n, InterEnvelope, &t1, 0.4 * Args.d, 0);
  cellList.clear();
  traverseOrder.clear();
  getTransformPoly(Args, InterEnvelope, t1, pShadow);

  for (int i = 0; i < bNum; i++) {
    getTransformPoly(Args, OriginArgs::instance().pTransformBar[i].p,
                     OriginArgs::instance().pTransformBar[i].n, qTmp);
    for (int j = 0; j < OriginArgs::instance().pTransformBar[i].n; j++) {
      OriginArgs::instance().pTransformBar[i].p[j] = qTmp[j];
    }
    get_envelope_of_polygen(OriginArgs::instance().pTransformBar[i].p,
                            OriginArgs::instance().pTransformBar[i].n, qTmp,
                            &t2, 0.5 * Args.d, 1);
    for (int j = 0; j < OriginArgs::instance().pTransformBar[i].n; j++) {
      OriginArgs::instance().pTransformBar[i].p[j] = qTmp[j];
    }
  }
#if DEBUG
  cout << "display polygon InterEnvelope:" << endl;
  for (int j = 0; j < t1; j++) {
    cout << "a"
         << "[" << j << "]"
         << "= POINT(" << pShadow[j].x << "," << pShadow[j].y << ")" << endl;
  }
#endif

  for (int i = 0; i < bNum; i++) {
#if DEBUG
    cout << "b" << i << "="
         << "(POINT * " << OriginArgs::instance().pTransformBar[i].n << ")()"
         << endl;
#endif
    for (int j = 0; j < OriginArgs::instance().pTransformBar[i].n; j++) {
#if DEBUG
      cout << "b" << i << "[" << j << "]"
           << "= POINT(" << OriginArgs::instance().pTransformBar[i].p[j].x
           << "," << OriginArgs::instance().pTransformBar[i].p[j].y << ")"
           << endl;
#endif
    }
    cout << endl;
  }
  for (int i = 0; i < t1; i++) {
    tmp.pt = pShadow[i];
    tmp.PolygonIndex = 0;
    eventQue.push_back(tmp);
  }
  for (int i = 0; i < bNum; i++) {
    for (int j = 0; j < OriginArgs::instance().pTransformBar[i].n; j++) {
      tmp.pt = OriginArgs::instance().pTransformBar[i].p[j];
      tmp.PolygonIndex = i + 1;
      eventQue.push_back(tmp);
    }
  }

  std::sort(eventQue.begin(), eventQue.end(), compare);
  sweepLineWithBarrier(eventQue, pShadow, t1,
                       OriginArgs::instance().pTransformBar, bNum);
  std::vector<int> final;
  bool isSplitWrong = buildMap(final);
  if (isSplitWrong) {
    // TODO: 由于缩边错误导致新版航线规划无法规划就使用老版本航线规划
    FlyWpInfo *temp = RoutePlanForAndroid(Args, (MyPoint *)p, n, pB, bNum);
    // temp->ErrCode = SPLIT_ERR;
    return temp;
  } else {
    OriginArgs::instance().isSplitOK = true;
  }

  if (final.size() == 0) {
    if (pShadow) {
      delete[] pShadow;
      pShadow = NULL;
    }
    destroyBarrierFiledObject(pGround);
    return RoutePlanForAndroid(Args, (MyPoint *)p, n, pB, bNum);
  } else {
    bool isNeed = false;
    int needPlanCount = 0;
    for (int i = 0; i < (int)final.size(); i++) {
      isNeed = isNeedPlanning(Args, i);
      if (isNeed) {
        needPlanCount++;
      }
    }
    if (needPlanCount < 2) {
      if (pShadow) {
        delete[] pShadow;
        pShadow = NULL;
      }
      return RoutePlanForAndroid(Args, (MyPoint *)p, n, pB, bNum);
    } else {
      subFieldPlanning(Args, (const MyPoint *)pShadow, t1,
                       OriginArgs::instance().pTransformBar, bNum, final, k, m,
                       wpArr, wpArr2, pFlyWpInfoInstance, SweepPointArr);
      getTransformPoint(pFlyWpInfoInstance->wp, m);
      if (pShadow) {
        delete[] pShadow;
        pShadow = NULL;
      }
    }
  }
  pFlyWpInfoInstance->m = m;
  pointFlyAttrDequeType dqFlyWp;
  dqFlyWp.clear();
  for (int i = 0; i < m; i++) {
    dqFlyWp.push_back(pFlyWpInfoInstance->wp[i]);
  }

  if (final.size() != 0) {
    vector<MyPoint> vptChannel;
    vptChannel.clear();
    setCombineBaseIndex(p, n, Args.d, Args.refer_border, dqFlyWp, vptChannel);
    for (int i = 0; i < m; i++) {
      pFlyWpInfoInstance->wp[i].BaseIndex = dqFlyWp.at(i).BaseIndex;
    }
    gLeakSprayInfo.fieldArea = my_area2D_Polygon(n, p);
    CalSprayAreaInfo(gLeakSprayInfo, Args.d, pFlyWpInfoInstance->wp, m);
  }
#if DEBUG
  printFlyWayPoint(dqFlyWp);
#endif

  // int status = checkSoInterface(Args, (MyPoint *)p, n, pB, bNum,
  // pFlyWpInfoInstance);
  pFlyWpInfoInstance->ErrCode = 0;
  memcpy(&pFlyWpInfoInstance->leakSprayInfo, &gLeakSprayInfo,
         sizeof(gLeakSprayInfo));

  if (bNum > 0) {
    destroyBarrierFiledObject(pGround);
  }

  return pFlyWpInfoInstance;
}
