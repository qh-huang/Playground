#ifndef SPLIT_H
#define SPLIT_H

#include "dijkstra.h"
#include <vae/geometry.h>

typedef enum {
  IN,
  OUT,
  INSPECIAL,  /* do nothing */
  OUTSPECIAL, /* ???????cell */
  FLOOR,
  CEILING,
  PIT
} eType;

typedef enum { INIT, TOP, BOTTOM, CLOSED } CELLSTATE;

typedef struct planEvent {
  MyPoint criticalPoint;
  eType eventType;
  int PolygonIndex;
} planEvent;

struct planCell {
  vector<PointAttr> floor;
  vector<PointAttr> ceiling;
  vector<int> neighborCellIndex;
  int cellIndex;
  vector<MyPoint> cellPoly;
  CELLSTATE cellState;
  PointAttr topLeft;
  PointAttr topRight;
  PointAttr bottomLeft;
  PointAttr bottomRight;
  planEvent in;
  planEvent out;
};
int getInEventType(planEvent &event, MyPoint *p, int n);
int getOutEventType(planEvent &event, MyPoint *p, int n);

int getCurrOpenCellNum(vector<int> &vIndex);
int isOpenedCellMatchOutEvent(vector<PointAttr> &eventQue, planEvent &event,
                              MyPoint *p, int n, SimplePolygon *pB, int bNum,
                              bool &isClosedDown, int &closeCellIndex1,
                              int &closeCellIndex2);

void closeTwoInSpeical(vector<PointAttr> &eventQue, planEvent &event,
                       MyPoint *p, int n, SimplePolygon *pB, int bNum,
                       int &closeCellIndex1, int &closeCellIndex2);
void setInSpecialCell(planCell &currCell, planEvent &event, MyPoint *p, int n,
                      MyPoint start, MyPoint end);
bool isPolygonNeeded(planCell currCell); //�ж��Ƿ���������Ҫ�Ķ����
bool isPolygon(vector<MyPoint> p); //�ж��Ƿ��ܹ��ɶ����
void getSubFieldTranstionPointWithBarrier(MyPoint *p, int n, SimplePolygon *pB,
                                          int bNum, PointFlyAttr *wpArr2,
                                          int &k, Graph const &graph,
                                          PointAttr pT0, PointAttr pT1,
                                          deque<MyPoint> &dqWpEveryTime);
int getPointPolygonIndex(MyPoint pt, const MyPoint *p, int n, SimplePolygon *pB,
                         int bNum);
void closeCellWithBarrier(planEvent event, planCell &currCell, MyPoint *p,
                          int n, SimplePolygon *pB, int bNum);

#endif
