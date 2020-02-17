#include <cmath>
#include <vae/geometry.h>
using namespace std;

/*
因浮点数运算容易发生精度丢失问题，在平面坐标情况下
小数点后三位能精确到毫米，所以精确到小数点后五位能
满足实际需求
*/
static const double absError = 0.00001;
bool IsEqual(double a, double b) {
  if (fabs(a - b) < absError)
    return true;
  return false;
}

static bool LessEqual(double a, double b) {
  if (fabs(a - b) < absError)
    return true;
  if (a < b)
    return true;
  return false;
}

static const double absError2 = 0.001;
bool IsEqual2(double a, double b) {
  if (fabs(a - b) < absError2)
    return true;
  return false;
}

// Function used to display X and Y coordinates
// of a point
void DisplayPoint(MyPoint P) {
  cout << "(" << P.x << ", " << P.y << ")" << endl;
}

bool onSegment(MyPoint Pi, MyPoint Pj, MyPoint Q) {
  if (IsEqual((Q.x - Pi.x) * (Pj.y - Pi.y), (Pj.x - Pi.x) * (Q.y - Pi.y)) &&
      LessEqual(min(Pi.x, Pj.x), Q.x) && LessEqual(Q.x, max(Pi.x, Pj.x)) &&
      LessEqual(min(Pi.y, Pj.y), Q.y) && LessEqual(Q.y, max(Pi.y, Pj.y)))
    return true;
  else
    return false;
}

/* ios系统中精度丢失可能造成判断不准，为此降低精度要求 */
bool onSegment2(MyPoint Pi, MyPoint Pj, MyPoint Q) {
  if (IsEqual2(Pi.x, Q.x) &&
      IsEqual2(Pi.y, Q.y)) // double有精度问题，必须做这种处理
    return true;
  if (IsEqual2(Pj.x, Q.x) && IsEqual2(Pj.y, Q.y))
    return true;
  if (IsEqual((Q.x - Pi.x) * (Pj.y - Pi.y), (Pj.x - Pi.x) * (Q.y - Pi.y)) &&
      LessEqual(min(Pi.x, Pj.x), Q.x) && LessEqual(Q.x, max(Pi.x, Pj.x)) &&
      LessEqual(min(Pi.y, Pj.y), Q.y) && LessEqual(Q.y, max(Pi.y, Pj.y)))
    return true;
  else
    return false;
}

// C++ Implementation. To find the point of
// intersection of two lines
int lineLineIntersection(pdd A, pdd B, pdd C, pdd D, MyPoint &Intesection) {
  // Line AB represented as a1x + b1y = c1
  double a1 = B.second - A.second;
  double b1 = A.first - B.first;
  double c1 = a1 * (A.first) + b1 * (A.second);

  // Line CD represented as a2x + b2y = c2
  double a2 = D.second - C.second;
  double b2 = C.first - D.first;
  double c2 = a2 * (C.first) + b2 * (C.second);

  double determinant = a1 * b2 - a2 * b1;
  if (IsEqual(determinant, 0)) {
    // The lines are parallel. This is simplified
    return 0;
  } else {
    double x = (b2 * c1 - b1 * c2) / determinant;
    double y = (a1 * c2 - a2 * c1) / determinant;
    Intesection.x = x;
    Intesection.y = y;
    return 1;
  }
}

//判断两条直线是否平行
bool IsLineLineParallel(MyPoint A1, MyPoint A2, MyPoint B1, MyPoint B2) {
  pdd Ct = make_pair(B1.x, B1.y);
  pdd Dt = make_pair(B2.x, B2.y);
  pdd At = make_pair(A1.x, A1.y);
  pdd Bt = make_pair(A2.x, A2.y);
  MyPoint Intersection;
  int ret = 0;
  ret = lineLineIntersection(At, Bt, Ct, Dt, Intersection);
  if (ret == 0) {
    return true;
  }
  return false;
}

bool LineLineIntersect(Line l1, Line l2, MyPoint &Intersection) {
  pdd Ct = make_pair(l2.p0.x, l2.p0.y);
  pdd Dt = make_pair(l2.p1.x, l2.p1.y);
  pdd At = make_pair(l1.p0.x, l1.p0.y);
  pdd Bt = make_pair(l1.p1.x, l1.p1.y);
  int ret = 0;
  ret = lineLineIntersection(At, Bt, Ct, Dt, Intersection);
  if (ret == 0) {
    return false;
  }
  return true;
}

//近似平行
bool IsLineLineNearParallel(MyPoint A1, MyPoint A2, MyPoint B1, MyPoint B2) {
  int ret;
  double VectorAngle = 0.0;

  ret = IsLineLineParallel(A1, A2, B1, B2);
  if (ret == 1) {
    return true;
  }

  VectorAngle =
      CalculateVectorAngle(A2.x - A1.x, A2.y - A1.y, B2.x - B1.x, B2.y - B1.y);
  //   cout<<"VectorAngle="<<VectorAngle<<endl;
  if (VectorAngle < 2 || VectorAngle > 178)
    return true;
  return false;
}

//近似平行
bool IsLineLineNearParallel2(MyPoint A1, MyPoint A2, MyPoint B1, MyPoint B2) {
  int ret;
  double VectorAngle = 0.0;

  ret = IsLineLineParallel(A1, A2, B1, B2);
  if (ret == 1) {
    return true;
  }

  VectorAngle =
      CalculateVectorAngle(A2.x - A1.x, A2.y - A1.y, B2.x - B1.x, B2.y - B1.y);
  //   cout<<"VectorAngle="<<VectorAngle<<endl;
  if (VectorAngle < 5 || VectorAngle > 175)
    return true;
  return false;
}

//判断两条直线是否重合
bool IsLineLineCoincidence(MyPoint A1, MyPoint A2, MyPoint B1, MyPoint B2) {
  pdd Ct = make_pair(B1.x, B1.y);
  pdd Dt = make_pair(B2.x, B2.y);
  pdd At = make_pair(A1.x, A1.y);
  pdd Bt = make_pair(A2.x, A2.y);
  MyPoint Intersection;
  Line B1B2 = {B1, B2};
  double d = 0.0;
  int ret = 0;
  ret = lineLineIntersection(At, Bt, Ct, Dt, Intersection);

  if (ret == 0) {
    d = dist_Point_to_Line(A1, B1B2);
    if (d < SMALL_NUM)
      return true;
  }
  return false;
}

// dist_Point_to_Line(): get the distance of a point to a line
//     Input:  a MyPoint P and a Line L (in any dimension)
//     Return: the shortest distance from P to L
double dist_Point_to_Line(MyPoint P, Line L) {
  Vector v = L.p1 - L.p0;
  Vector w = P - L.p0;

  double c1 = dot(w, v);
  double c2 = dot(v, v);
  double b = c1 / c2;

  MyPoint Pb = L.p0 + b * v;
  //#define norm(v)     sqrt(dot(v,v))     // norm = length of  vector
  //#define d(u,v)      norm(u-v)          // distance = norm of difference
  double d;
  d = sqrt(dot(P - Pb, P - Pb));
  return d;
  // return d(P, Pb);
}

// isLeft(): test if a point is Left|On|Right of an infinite 2D line.
//    Input:  three points p0, p1, and P2
//    Return: >0 for P2 left of the line through p0 to p1
//          =0 for P2 on the line
//          <0 for P2 right of the line
double isLeft(MyPoint p0, MyPoint p1, MyPoint P2) {
  return ((p1.x - p0.x) * (P2.y - p0.y) - (P2.x - p0.x) * (p1.y - p0.y));
}

/**
 * Get projected point P' of P on line e1.
 * @return projected point p.
 * 计算pt点在直线v1，v2上的投影点
 */
MyPoint getProjectedPointOnLine(MyPoint pt, MyPoint v1, MyPoint v2) {
  // get dot product of e1, e2
  MyPoint e1;
  MyPoint e2;

  e1.x = v2.x - v1.x;
  e1.y = v2.y - v1.y;
  e2.x = pt.x - v1.x;
  e2.y = pt.y - v1.y;
  double valDp = dot(e1, e2);
  // get length of vectors
  double lenLineE1 = sqrt(e1.x * e1.x + e1.y * e1.y);
  double lenLineE2 = sqrt(e2.x * e2.x + e2.y * e2.y);
  if (lenLineE1 < SMALL_NUM || lenLineE2 < SMALL_NUM)
    return pt;
  double cos = valDp / (lenLineE1 * lenLineE2);
  // length of v1P'
  double projLenOfLine = cos * lenLineE2;
  MyPoint p;
  p.x = (v1.x + (projLenOfLine * e1.x) / lenLineE1);
  p.y = (v1.y + (projLenOfLine * e1.y) / lenLineE1);
  return p;
}

/*
1. (y-y0)*(y-y0) + (x-x0)*(x-x0) = d*d;
2. y = k0*x + b0;
给定一条直线，求过该直线上某个端点直线方程上距离该端点为dist的两点坐标
*/
int calTwoPointOnPrepLine(MyPoint pt1, MyPoint pt2, MyPoint &ptNeeded1,
                          MyPoint &ptNeeded2, double dist) {
  MyPoint mid = pt1;
  double k1 = (pt2.y - pt1.y) / (pt2.x - pt1.x);
  double k0 = -1 / k1;
  double b0 = mid.y - k0 * mid.x;
  double a = k0 * k0 + 1;
  double b = 2 * (k0 * b0 - k0 * mid.y - mid.x);
  double c = (b0 - mid.y) * (b0 - mid.y) + mid.x * mid.x - dist * dist;
  double tmp = sqrt(b * b - 4 * a * c);
  ptNeeded1.x = (-b + tmp) / (2 * a);
  ptNeeded2.x = (-b - tmp) / (2 * a);
  ptNeeded1.y = k0 * ptNeeded1.x + b0;
  ptNeeded2.y = k0 * ptNeeded2.x + b0;
  return 0;
}
//===================================================================
