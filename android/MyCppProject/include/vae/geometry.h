#ifndef GEOMETRY_H
#define GEOMETRY_H
#include <cmath>
#include <deque>
#include <iostream> // std::cout
#include <string>
#include <vector>

using namespace std;

// This pair is used to store the X and Y coordinates of a point respectively
#define pdd pair<double, double>
#define DEBUG 0

struct MyPoint {
  MyPoint(double x1, double y1) : x(x1), y(y1) {}
  MyPoint() {}
  double x;
  double y;
};

struct PointAttr {
  MyPoint pt;
  int PolygonIndex;
  void operator=(const PointAttr &b) {
    pt = b.pt;
    PolygonIndex = b.PolygonIndex;
  }
};

typedef struct myChannel {
  MyPoint pt;
  int channelIndex;
} myChannel;

/* waypoint with flying attribute */
typedef struct mytagPointFlyAttr {
  MyPoint pt;
  unsigned int SprayCtrl;
  unsigned int SegmentType;
  unsigned int BaseIndex; /* 65533: concave, 65534: internal obstacle */
  unsigned int SideIndex;
} PointFlyAttr;

struct SimplePolygon {
  int n;
  MyPoint *p;
};

typedef struct tagPoint2LineDis {
  int index;
  MyPoint pt;
  double d1; // distance
} Point2LineDis;

typedef struct tagP2PDistance {
  MyPoint pt;
  double d1; // distance
} P2PDistance;
typedef deque<Point2LineDis> point2LineDequeType;

typedef MyPoint Vector;

struct Segment {
  MyPoint p0;
  MyPoint p1;
};
typedef deque<MyPoint> pointDequeType;
typedef deque<PointAttr> pointAttrDequeType;
typedef deque<PointFlyAttr> pointFlyAttrDequeType;
typedef deque<Segment> segmentDequeType;
typedef struct closepointpair {
  MyPoint p0;
  MyPoint p1;
} ClosePointPair;
typedef deque<ClosePointPair> ClosePointPairDequeType;
typedef Segment Line;
typedef struct line2 {
  MyPoint q[8];
  unsigned int TotalPointNum;
} Line2;

typedef deque<Line2> line2DequeType;

//   public static final int SPRAY_ALL_OFF = 1;//??????
//   public static final int SPRAY_TURN_ON = 2;//??????
//   public static final int SPRAY_TURN_OFF = 3;//??????

typedef struct mytagFlyBasicArgs {
  int sweep;
  int isSprayRepeat;
  int uavType;
  int SprayType;
  double d;
  double SweepDelta;
  Line refer_border;
  MyPoint TakeOffPoint;
} FlyBasicArgs;

class channel {
public:
  Line chLine;
  int PolyIndex1; /* channel相关的第一个多边形index */
  int PolyIndex2; /* channel相关的第二个多边形index */
};

typedef struct tagField {
  MyPoint *p;        /* 地块多边形 */
  int n;             /* 地块的顶点个数 */
  SimplePolygon *pB; /* 障碍物 多边形*/
  int bNum;          /* 障碍物的个数 */
  myChannel *ptChannel;
  channel *ch; /* 地块的通道，最多2个，如果只有一个则存放在 ch[0]中 */
  int mChannelNum; /* 该多边形通道点个数也等于通道个数, 范围是1~2 */
  Line refer_border;
} Field;

typedef struct tagRouteLineArgs {
  double d;  /* Spary Width */
  double d1; /* Spary Width */
  double d2; /* Spary Width */
  int k;
  int k1;
  int k2;
  Line refer_border;
  double A;
  double B;
  double dmax;
  MyPoint MaxD;
  MyPoint ReferNearestPoint;
  MyPoint takeOffPoint;
} T_stRouteLineArgs;

typedef struct tagBaseBorderInfo {
  MyPoint p0;
  MyPoint p1;
  int index;
} BaseBorderInfo;

typedef struct LeakSprayInfo {
  double leakArea;
  double fieldArea;
  double sprayArea;
  double leakPercent;
  double flyDistance;
} LeakSprayInfo;

typedef struct mytagFlyWpInfo {
  PointFlyAttr *wp;
  int m;
  PointFlyAttr *SweepPoint;
  int k;
  BaseBorderInfo *baseBorder;
  int baseBorderNum;
  int ErrCode; // 0: OK
  LeakSprayInfo leakSprayInfo;
} FlyWpInfo;

class Polygon {
private:
  bool is_convex;
  bool is_clockwise;
  pointDequeType dqPitsArr;

public:
  int npts;
  MyPoint *p;
  MyPoint OutBorderPoint[2];
  Polygon();
  Polygon(int n, MyPoint *vert);
  ~Polygon();
  void setConvex(bool input_para);
  bool getConvex(void);
  void setClockwise(bool input_para);
  bool getClockwise(void);
  void isConvex(void);
  void printPloygon(void);
};

class combineArgs {
public:
  SimplePolygon *barrier = NULL;
  Polygon **ppSacledBarrier = NULL;
  int barrierNum = 0;
  static combineArgs &instance() {
    static combineArgs *instance = new combineArgs();
    return *instance;
  }

private:
  combineArgs() {}
};

#define MAX_DISTANCE_FROM_HOME (1500)
#define DISTANCE_FIRST_WP_AND_HOME (800)
#define DISTANCE_OUT_OF_RANGE (800)
#define DISTANCE_OUT_OF_RANGE_ADJUST (600)

#define ARGS_ERR 0x1
#define LEAKAGE_ERR 0x2
#define TOO_MANY_WAYPOINT_ERR 0x4
#define BORDER_POINT_TOO_CLOSE 0x8
#define ROUTELINE_OUT_OF_BORDER 0x10
#define WAYPOINT_TOO_CLOSE 0x20
#define BASE_BORDER_POINT_TOO_CLOSE 0x40
#define SEGMENT_ATTR_ERR 0x80
#define MAX_DISTANCE_FROM_HOME_ERR 0x100
#define DISTANCE_FIRST_WP_AND_HOME_ERR 0x200
#define SPLIT_ERR 0x300

#define SMALL_NUM 0.00000001 // anything that avoids division overflow
// dot product (3D) which allows vector operations in arguments
#define dot(u, v) ((u).x * (v).x + (u).y * (v).y)
#define perp(u, v) ((u).x * (v).y - (u).y * (v).x) // perp product  (2D)

//#define norm(v)     sqrt(dot(v,v))     // norm = length of  vector
//#define d(u,v)      norm(u-v)          // distance = norm of difference

Vector operator+(const Vector &vecIn1, const Vector &vecIn2);
Vector operator-(const Vector &vecIn1, const Vector &vecIn2);
Vector operator*(const double t, const Vector &vecIn1);
bool operator==(const Vector &vecIn1, const Vector &vecIn2);
bool operator!=(const Vector &vecIn1, const Vector &vecIn2);
ostream &operator<<(ostream &out, MyPoint a);

bool intersect2D_2Segments(Segment S1, Segment S2, MyPoint *I0, MyPoint *I1);
void DisplayPoint(MyPoint P);
extern "C" int route_plan(FlyBasicArgs Args, MyPoint *p, int n,
                          SimplePolygon *pB, int bNum, PointFlyAttr *wp,
                          int *pm, PointFlyAttr *SweepPoint, int *pk);
/* 单个地块调用的航线规划接口 */
extern "C" FlyWpInfo *RoutePlanForAndroid(FlyBasicArgs Args, MyPoint *p, int n,
                                          SimplePolygon *pB, int bNum);
/* 多个地块合并时调用的航线规划接口
group 是通道多边形，个数是fieldNum -1
*/
extern "C" FlyWpInfo *RoutePlanForAndroidVersion(FlyBasicArgs Args, MyPoint *p,
                                                 int n, SimplePolygon *pB,
                                                 int bNum, int planVersion);
extern "C" FlyWpInfo *RoutePlanForAndroidCom(FlyBasicArgs Args,
                                             Field *fieldGroup, int fieldNum,
                                             SimplePolygon combinePoly,
                                             SimplePolygon *barGroup,
                                             int barNum);

int inSegment(MyPoint P, Segment S);
int lineLineIntersection(pdd A, pdd B, pdd C, pdd D, MyPoint &Intesection);

extern double dist_Point_to_Line(MyPoint P, Line L);
extern double dist_Point_to_Point(MyPoint p1, MyPoint p2);
extern bool IsLineLineCoincidence(MyPoint A1, MyPoint A2, MyPoint B1,
                                  MyPoint B2);

extern bool onSegment(MyPoint Pi, MyPoint Pj, MyPoint Q);
MyPoint getProjectedPointOnLine(MyPoint pt, MyPoint v1, MyPoint v2);
double isLeft(MyPoint p0, MyPoint p1, MyPoint P2);
void FindMiddlePointOfNeighboringPoint(MyPoint *p, int n, MyPoint pt0,
                                       MyPoint pt1,
                                       pointDequeType &dqWpEveryTime);
double cross(MyPoint p0, MyPoint p1, MyPoint p2);
void get_scale_parallel_line(MyPoint *pInput, int n, int start_index,
                             double delta, int direction, Line *l);
extern "C" void get_envelope_of_polygen(MyPoint *pInput, int n, MyPoint *q,
                                        int *m, double delta, int direction);

void get_parallel_line(double A, double B, double Cm, MyPoint *pt);
void get_parallel_line3(double A, double B, double Cm, MyPoint *pt,
                        Line refer_border);

int RoutePlanWithBarrier(double d, int isRepeat, int uavType, bool isMiddle,
                         int SprayType, Line refer_border, MyPoint *p, int n,
                         SimplePolygon *pB, int bnum, PointFlyAttr *wp,
                         int *pm);

#define MAX_WP_NUM 2000 // max waypoint num

extern void get_useful_args(MyPoint *p, int n, int index_max, Line refer_border,
                            double *pA, double *pB, MyPoint *pMaxD);
extern void get_line_polygon_intersetion(MyPoint *p, int n, MyPoint pt0,
                                         MyPoint pt1, MyPoint *pIntersection,
                                         unsigned int *pm);
bool getIntersectionSegPoly(MyPoint *p, int n, MyPoint pt1, MyPoint pt2,
                            MyPoint *intersection, Line &l, int &m);
extern bool NextLineNeedReverse(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                                MyPoint lastWp);
extern bool FindNextLineStart(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                              MyPoint curWp, MyPoint prevWp, bool isPrevExist);
extern bool FindNextLineStart2(MyPoint pt0, MyPoint pt1, MyPoint LastWp);
extern bool get_clockwise(MyPoint *p, int n);
extern bool IsLineLineParallel(MyPoint A1, MyPoint A2, MyPoint B1, MyPoint B2);
extern bool LineLineIntersect(Line l1, Line l2, MyPoint &Intersection);
extern bool IsLineLineNearParallel(MyPoint A1, MyPoint A2, MyPoint B1,
                                   MyPoint B2);
extern "C" void getAttr(PointFlyAttr *pA, int *m);
extern void setWpAttr(MyPoint *p, int n, int isRepeat, int uavType,
                      Line refer_border, int SprayType, bool IsBarrier,
                      pointDequeType &dqWp, pointFlyAttrDequeType &dqFlyWp,
                      const ClosePointPairDequeType &dqClosePointPair);

extern bool operator<(const MyPoint &a, const MyPoint &b);
int cn_PnPoly(MyPoint P, MyPoint *V, int n);
int wn_PnPoly(MyPoint P, MyPoint *V, int n);
void FindShortestPathOfTwoPoint(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                                pointDequeType &dqWpEveryTime);
double CalculateVectorAngle(double x1, double y1, double x2, double y2);

void getCm(double &Cm, int k, int loopTimes, double A, double B, MyPoint MaxD,
           MyPoint ReferNearestPoint, double alaph, double d,
           Line refer_border);

void getCmSpecial(double &Cm, unsigned int loopTimes, double A, double B,
                  MyPoint MaxD, MyPoint ReferNearestPoint, double d1, double d2,
                  unsigned int k1, unsigned int k2, Line refer_border);
void getTrueDmaxNormal(MyPoint *p, int n, unsigned int &k, double &d,
                       MyPoint &MaxD, MyPoint &ReferNearestPoint, double &dmax,
                       Line refer_border);
void getTrueDmaxSpecial(MyPoint *p, int n, unsigned int &k1, unsigned int &k2,
                        double &d, double &d1, double &d2, MyPoint &MaxD,
                        MyPoint &ReferNearestPoint, double &dmax,
                        Line refer_border);
void get_index_max(Line refer_border, MyPoint *p, int n, int &index_max,
                   double &dmax);

extern bool isConvex(MyPoint *p, int n, bool *pclockwise,
                     pointDequeType &dqPitsArrTemp);

bool IsPointInBarrier(MyPoint pt);
bool is_pit_point(MyPoint pt);
bool is_pit_point2(MyPoint pt, pointDequeType dqPitsArrTemp);
void setOriginArgs(const MyPoint *p, int n, double d, Line refer_border);
bool operator<(const Point2LineDis &a, const Point2LineDis &b);
void getIntersetion(double &Cm, double A, double B, MyPoint MaxD,
                    MyPoint ReferNearestPoint, double d1, double d2, double d,
                    unsigned int k1, unsigned int k2, Line refer_border,
                    bool isMiddle, bool isScaled, int j, MyPoint *pt,
                    MyPoint *pIntersection, unsigned int *num, MyPoint *p,
                    int n, int k);
double getAngelOfTwoVector(MyPoint pt1, MyPoint pt2, MyPoint c);
bool isVertexPoint(MyPoint *p, int n, MyPoint pt);
int FindReferBorderIndex(MyPoint *p, int n, Line refer_border);
double area2D_Polygon(int n, const MyPoint *V);
int intersect2D_SegPoly_Origin(Segment S, MyPoint *p, int n, Segment *IS);
//线段是否在多边形外
bool IsSegmentOutOfBorder(MyPoint *p, int n, MyPoint pt1, MyPoint pt2);
void CalSprayAreaInfo(LeakSprayInfo &tLeakSprayInfo, double d, PointFlyAttr *wp,
                      int m);
int waypointHandler(MyPoint *p, int n, double d, int isRepeat, int uavType,
                    bool isBarrier, int SprayType, Line refer_border,
                    PointFlyAttr *wp, int *pm, pointDequeType &dqWp);

double my_area2D_Polygon(int n, const MyPoint *p);
void sprayRepeat(pointDequeType &dqWp);
bool intersect2D_SegPoly(MyPoint *p, int n, MyPoint pt1, MyPoint pt2);
bool intersect2D_SegPoly2(MyPoint *p, int n, MyPoint pt1, MyPoint pt2, int &k);
bool intersect2D_SegPoly3(MyPoint *p, int n, MyPoint pt1, MyPoint pt2, int k);

void FindShortestPathSub(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                         pointDequeType &dqTmp1, pointDequeType &dqTmp2);
MyPoint getMidPoint(MyPoint pt0, MyPoint pt1);
bool onSegment2(MyPoint Pi, MyPoint Pj, MyPoint Q);
bool IsEqual2(double a, double b);
void setBaseBorder(const MyPoint *p, int n, FlyWpInfo *pFlyWpInfoInstance);
int findBaseIndex(MyPoint ptFind, MyPoint *p, int n, double d,
                  vector<int> &indexQue);

void setBaseIndexNormal(unsigned int i, const MyPoint *p, int n, double d,
                        Line referBorder, deque<PointFlyAttr> &ptQueue);
bool IsLineLineNearParallel2(MyPoint A1, MyPoint A2, MyPoint B1, MyPoint B2);
void getPrevPoint(const pointDequeType &dqWp, bool &isPrevExist, MyPoint &curWp,
                  MyPoint &prevWp);
bool isNShapePoint(MyPoint ptPrev, MyPoint ptCurr, MyPoint ptNext,
                   MyPoint ptNextNext);
extern "C" FlyWpInfo *RoutePlanForAndroid2(FlyBasicArgs Args, const MyPoint *p,
                                           int n, SimplePolygon *pB, int bNum);
extern "C" FlyWpInfo *RoutePlanForAndroid3(FlyBasicArgs Args, const MyPoint *p,
                                           int n, SimplePolygon *pB, int bNum);
MyPoint findNearestPoint(MyPoint *p, int n, MyPoint pt);
// bool operator==(const PointAttr& a, const PointAttr& b);
void get_line_polygon_intersetion2(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                                   vector<MyPoint> &vPoint);
void get_line_polygon_intersetion2(MyPoint *p, int n, MyPoint pt0, MyPoint pt1,
                                   MyPoint *pIntersection, unsigned int *pm);
int tsp(int n, int **a);
bool IsEqual(double a, double b);
void printFlyWayPoint(const pointFlyAttrDequeType &dqFlyWp);
void enqueueTwoPointByTakeoffPoint(MyPoint takeOffPoint,
                                   pointDequeType &dqWpEveryTime,
                                   MyPoint *pIntersection);
extern "C" int route_plan2(FlyBasicArgs Args, const MyPoint *p, int n,
                           SimplePolygon *pB, int bNum, PointFlyAttr *wp,
                           int *pm, PointFlyAttr *SweepPoint, int *pk);
double getAngelOfTwoVector2(MyPoint pt1, MyPoint pt2, MyPoint c);
void setCombineBaseIndex(const MyPoint *p, int n, double d, Line referBorder,
                         deque<PointFlyAttr> &ptQueue,
                         vector<MyPoint> vptChannel);
int checkSoInterface(FlyBasicArgs Args, MyPoint *p, int n, SimplePolygon *pB,
                     int bNum, FlyWpInfo *pFlyWpInfoInstance);
int calTwoPointOnPrepLine(MyPoint pt1, MyPoint pt2, MyPoint &ptNeeded1,
                          MyPoint &ptNeeded2, double dist);

#endif
