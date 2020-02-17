#include <vae/geometry.h>

#include <algorithm> // std::for_each
#include <cmath>
#include <cstring>
#include <deque>
#include <iostream> // std::cout
#include <iterator> // std::next
#include <list>     // std::list

Vector operator+(const Vector &vecIn1, const Vector &vecIn2) {
  Vector vecOut;
  vecOut.x = vecIn1.x + vecIn2.x;
  vecOut.y = vecIn1.y + vecIn2.y;
  return vecOut;
}

// overload - operator to provide a vector subtraction
Vector operator-(const Vector &vecIn1, const Vector &vecIn2) {
  Vector vecOut;
  vecOut.x = vecIn1.x - vecIn2.x;
  vecOut.y = vecIn1.y - vecIn2.y;
  return vecOut;
}

Vector operator*(const double t, const Vector &vecIn1) {
  Vector vecOut;
  vecOut.x = t * vecIn1.x;
  vecOut.y = t * vecIn1.y;
  return vecOut;
}

bool operator==(const Vector &vecIn1, const Vector &vecIn2) {
  return IsEqual(vecIn1.x, vecIn2.x) && IsEqual(vecIn1.y, vecIn2.y);
}

bool operator!=(const Vector &vecIn1, const Vector &vecIn2) {
  return !IsEqual(vecIn1.x, vecIn2.x) || !IsEqual(vecIn1.y, vecIn2.y);
}

bool operator==(const Segment &s1, const Segment &s2) {
  if ((s1.p0 == s2.p0) && (s1.p1 == s2.p1))
    return true;
  if ((s1.p0 == s2.p1) && (s1.p0 == s2.p1))
    return true;
  return false;
}

bool operator!=(const Segment &s1, const Segment &s2) { return !(s1 == s2); }

ostream &operator<<(ostream &out, MyPoint a) {
  return out << "(" << a.x << ", " << a.y << ")";
}

// p0为起始点，求p1点在p0,p2所在直线的哪一侧
// 也可以说，p0->p1构成的向量v在p0->p2构成的向量w的哪一边
// 叉乘
double cross(MyPoint p0, MyPoint p1, MyPoint p2) {
  return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
}

//比较两个Point的x坐标
bool operator<(const Point2LineDis &a, const Point2LineDis &b) {
  return a.d1 < b.d1;
}

// cn_PnPoly(): crossing number test for a point in a polygon
//      Input:   P = a point,
//               V[] = vertex points of a polygon V[n-1]
//      Return:  0 = outside, 1 = inside, 2 =  on
// This code is patterned after [Franklin, 2000]

int cn_PnPoly(MyPoint P, MyPoint *V, int n) {
  int nCross = 0;
  for (int i = 0; i < n; i++) {
    if (P == V[i]) {
      return 2;
    }
  }
  for (int i = 0; i < n; i++) {
    MyPoint p1 = V[i];
    MyPoint p2 = V[(i + 1) % n];
    if (onSegment(p1, p2, P)) {
      return 2;
    }
    // 求解 y=p.y 与 p1p2 的交点
    if (p1.y == p2.y) // p1p2 与 y=p0.y平行
      continue;
    if (P.y < min(p1.y, p2.y)) // 交点在p1p2延长线上
      continue;
    if (P.y >= max(p1.y, p2.y)) // 交点在p1p2延长线上
      continue;

    // 求交点的 X 坐标
    // --------------------------------------------------------------
    double x =
        (double)(P.y - p1.y) * (double)(p2.x - p1.x) / (double)(p2.y - p1.y) +
        p1.x;

    if (x > P.x)
      nCross++; // 只统计单边交点
  }
  // 单边交点为偶数，点在多边形之外 ---
  return (nCross % 2 == 1);
}

// wn_PnPoly(): winding number test for a point in a polygon
//      Input:   P = a point,
//               V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
//      Return:  wn = the winding number (=0 only if P is outside V[])
int wn_PnPoly(MyPoint P, MyPoint *V, int n) {
  int wn = 0; // the winding number counter

  // loop through all edges of the polygon
  for (int i = 0; i < n; i++) { // edge from V[i] to V[i+1]
    if (V[i].y <=
        P.y) { // start y <= P.y if (V[i+1].y > P.y)      // an upward crossing
      if (isLeft(V[i], V[i + 1], P) > 0)   // P left of edge
        ++wn;                              // have a valid up intersect
    } else {                               // start y > P.y (no test needed)
      if (V[i + 1].y <= P.y)               // a downward crossing
        if (isLeft(V[i], V[i + 1], P) < 0) // P right of edge
          --wn;                            // have a valid down intersect
    }
  }
  return wn;
}

int cn_PnPoly_Origin(MyPoint P, MyPoint *V, int n) {
  MyPoint *q = new MyPoint[n + 1];
  int ret = false;
  memcpy(q, V, sizeof(MyPoint) * n);
  q[n] = V[0];
  ret = wn_PnPoly(P, q, n);
  if (q) {
    delete[] q;
    q = NULL;
  }
  return ret;
}

// intersect2D_SegPoly(): intersect a 2D segment with a convex polygon
//    Input:  S = 2D segment to intersect with the convex polygon V[]
//            n = number of 2D points in the polygon
//            V[] = array of n+1 vertex points with V[n] = V[0]
//      Note: The polygon MUST be convex and
//                have vertices oriented counterclockwise (ccw).
//            This code does not check for and verify these conditions.
//    Output: *IS = the intersection segment (when it exists)
//    Return: FALSE = no intersection
//            TRUE  = a valid intersection segment exists

int intersect2D_SegPoly_Inner(Segment seg, MyPoint *V, int n, Segment *IS) {
  if (seg.p0 == seg.p1) { // the segment S is a single point
    // test for inclusion of seg.p0 in the polygon
    *IS = seg; // same point if inside polygon
    return wn_PnPoly(seg.p0, V, n);
  }

  float tE = 0;                // the maximum entering segment parameter
  float tL = 1;                // the minimum leaving segment parameter
  float t, N, D;               // intersect parameter t = N / D
  Vector dS = seg.p1 - seg.p0; // the  segment direction vector
  Vector e;                    // edge vector
  // Vector ne;               // edge outward normal (not explicit in code)

  for (int i = 0; i < n; i++) // process polygon edge V[i]V[i+1]
  {
    e = V[i + 1] - V[i];
    N = perp(e, seg.p0 - V[i]); // = -dot(ne, seg.p0 - V[i])
    D = -perp(e, dS);           // = dot(ne, dS)
    if (fabs(D) < SMALL_NUM) {  // S is nearly parallel to this edge
      if (N < 0)                // p0 is outside this edge, so
        return false;           // S is outside the polygon
      else                      // S cannot cross this edge, so
        continue;               // ignore this edge
    }

    t = N / D;
    if (D < 0) {    // segment S is entering across this edge
      if (t > tE) { // new max tE
        tE = t;
        if (tE > tL) // S enters after leaving polygon
          return false;
      }
    } else {        // segment S is leaving across this edge
      if (t < tL) { // new min tL
        tL = t;
        if (tL < tE) // S leaves before entering polygon
          return false;
      }
    }
  }

  // tE <= tL implies that there is a valid intersection subsegment
  IS->p0 = seg.p0 + tE * dS; // = P(tE) = point where seg enters polygon
  IS->p1 = seg.p0 + tL * dS; // = P(tL) = point where seg leaves polygon
  return true;
}

int intersect2D_SegPoly_Origin(Segment S, MyPoint *p, int n, Segment *IS) {
  MyPoint *q = new MyPoint[n + 1];
  int ret = false;
  memcpy(q, p, sizeof(MyPoint) * n);
  q[n] = p[0];
  ret = intersect2D_SegPoly_Inner(S, q, n, IS);
  if (q) {
    delete[] q;
    q = NULL;
  }
  return ret;
}

//线段是否在多边形外
bool IsSegmentOutOfBorder(MyPoint *p, int n, MyPoint pt1, MyPoint pt2) {
  bool ret = false;
  Segment seg;
  seg.p0 = pt1;
  seg.p1 = pt2;
  Segment IS;
  ret = intersect2D_SegPoly_Origin(seg, p, n, &IS);
  if (ret == true) {
    MyPoint MiddlePoint = getMidPoint(pt1, pt2);
    int OutSide = cn_PnPoly_Origin(MiddlePoint, p, n);
    if (0 == OutSide) {
      cout << "ROUTELINE_OUT_OF_BORDER:" << endl;
      return true;
    }
    OutSide = cn_PnPoly_Origin(pt1, p, n);
    if (0 == OutSide) {
      cout << "ROUTELINE_OUT_OF_BORDER:" << endl;
      return true;
    }
    OutSide = cn_PnPoly_Origin(pt2, p, n);
    if (0 == OutSide) {
      cout << "ROUTELINE_OUT_OF_BORDER:" << endl;
      return true;
    }
  }
  return false;
}

/// <summary>
/// 判断多边形是顺时针还是逆时针.
bool get_clockwise(MyPoint *p, int n) {
  int k = 0;
  int ConvexPointIndex = 0; //极值点的index，极值点必定是凸点
  double MaxX = p[0].x;
  MyPoint u, v;
  for (k = 1; k < n; k++) {
    if (p[k].x > MaxX) {
      MaxX = p[k].x;
      ConvexPointIndex = k;
    }
  }

  u.x = p[ConvexPointIndex].x - p[(ConvexPointIndex + n - 1) % n].x;
  u.y = p[ConvexPointIndex].y - p[(ConvexPointIndex + n - 1) % n].y;
  v.x = p[(ConvexPointIndex + 1) % n].x - p[ConvexPointIndex].x;
  v.y = p[(ConvexPointIndex + 1) % n].y - p[ConvexPointIndex].y;
  if (perp(u, v) > 0) {
    return false;
  } else {
    return true;
  }
}

double dist_Point_to_Point(MyPoint p1, MyPoint p2) {
  return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

// intersect2D_2Segments(): find the 2D intersection of 2 finite segments
//    Input:  two finite segments S1 and S2
//    Output: *I0 = intersect point (when it exists)
//            *I1 =  endpoint of intersect segment [I0,I1] (when it exists)
//    Return: 0=disjoint (no intersect)
//            1=intersect  in unique point I0
//            2=overlap  in segment from I0 toa I1
//该函数用于判断线段是否相交
bool intersect2D_2Segments(Segment S1, Segment S2, MyPoint *I0, MyPoint *I1) {
  MyPoint Intersection;
  int ret = 0;
  pdd B0 = make_pair(S1.p0.x, S1.p0.y);
  pdd B1 = make_pair(S1.p1.x, S1.p1.y);
  pdd A0 = make_pair(S2.p0.x, S2.p0.y);
  pdd A1 = make_pair(S2.p1.x, S2.p1.y); //平行线上的点
  ret = lineLineIntersection(A0, A1, B0, B1, Intersection);
  if (0 == ret) //平行或重合
  {
    return false;
  } else {
    MyPoint P;
    P.x = Intersection.x;
    P.y = Intersection.y;
    bool result1 = onSegment(S1.p0, S1.p1, P); //点在直线上则线段相交
    bool result2 = onSegment(S2.p0, S2.p1, P); //点在直线上则线段相交
    if (result1 && result2) {
      I0->x = Intersection.x;
      I0->y = Intersection.y;
      return true;
    }
  }
  return false;
}

//将直线的一般方程转换为两点方程
void get_parallel_line(double A, double B, double Cm, MyPoint *pt) {
  if (IsEqual(A, 0.0)) {
    pt[0].x = 0;
    pt[0].y = -Cm / B;
    pt[1].x = 100;
    pt[1].y = -Cm / B;
  } else if (IsEqual(B, 0.0)) {
    pt[0].x = -Cm / A;
    pt[0].y = 0;
    pt[1].x = -Cm / A;
    pt[1].y = 100;
  } else {
    pt[0].x = 100; // pt: temp point
    pt[0].y = -(Cm + 100 * A) / B;
    pt[1].x = -(100 * B + Cm) / A;
    pt[1].y = 100;
  }
}

void get_parallel_line3(double A, double B, double Cm, MyPoint *pt,
                        Line refer_border) {
  if (IsEqual(A, 0.0)) {
    pt[0].x = 0;
    pt[0].y = -Cm / B;
    pt[1].x = 100;
    pt[1].y = -Cm / B;
  } else if (IsEqual(B, 0.0)) {
    pt[0].x = -Cm / A;
    pt[0].y = 0;
    pt[1].x = -Cm / A;
    pt[1].y = 100;
  } else {
    pt[0].x = -(refer_border.p0.y * B + Cm) / A; // pt: temp point
    pt[0].y = refer_border.p0.y;
    pt[1].x = -(refer_border.p1.y * B + Cm) / A;
    pt[1].y = refer_border.p1.y;
  }
}

//计算平行线的任意两点, 得到平行于参考边的直线方程
void get_parallel_line2(double A, double B, double Cm, double x0, double y0,
                        double x1, double y1, MyPoint &pt0, MyPoint &pt1) {
  if (IsEqual(A, 0.0)) {
    pt0.x = x0;
    pt0.y = -Cm / B;
    pt1.x = x1;
    pt1.y = -Cm / B;
  } else if (IsEqual(B, 0.0)) {
    pt0.x = -Cm / A;
    pt0.y = y0;
    pt1.x = -Cm / A;
    pt1.y = y1;
  } else {
    pt0.x = x0; // pt: temp point
    pt0.y = -(Cm + x0 * A) / B;
    pt1.x = x1;
    pt1.y = -(Cm + x1 * A) / B;
  }
}

/*
pInput: 多边形数组,
start_index: 参考直线的起点index
direction: direction为0是内包络，为1是外包络
l: l是传出参数，是与边平行的内包络或外包络上的直线
*/
void get_scale_parallel_line(MyPoint *pInput, int n, int start_index,
                             double delta, int direction, Line *l) {
  bool is_clockwise = false;
  double A, B;
  double C1, C2; //表示平行于多边形边的两条不同直线，他们距离多边形的边为delta
  MyPoint prj1,
      prj2; //过多边形顶点做垂直于边的直线，得到落在内外平行于边的直线的投影点坐标
  MyPoint start_vertex = pInput[(start_index - 1 + n) % n];
  MyPoint end_vertex = pInput[(start_index) % n];
  Line l1, l2;
  MyPoint t1, t2;
  double ret;

  is_clockwise = get_clockwise(pInput, n);
  /* 得到平行线的直线方程 */
  A = end_vertex.y - start_vertex.y;
  B = start_vertex.x - end_vertex.x;
  C1 = delta * sqrt(A * A + B * B) - A * start_vertex.x - B * start_vertex.y;
  C2 = -delta * sqrt(A * A + B * B) - A * start_vertex.x - B * start_vertex.y;
  get_parallel_line2(A, B, C1, start_vertex.x, start_vertex.y, end_vertex.x,
                     end_vertex.y, t1, t2);
  l1.p0 = t1;
  l1.p1 = t2;
  get_parallel_line2(A, B, C2, start_vertex.x, start_vertex.y, end_vertex.x,
                     end_vertex.y, t1, t2);
  l2.p0 = t1;
  l2.p1 = t2;
  prj1 = getProjectedPointOnLine(end_vertex, l1.p0, l1.p1);
  prj2 = getProjectedPointOnLine(end_vertex, l2.p0, l2.p1);
  ret = cross(start_vertex, end_vertex, prj1);

  if (is_clockwise) {
    if (ret > 0 && direction == 0) //内边
    {
      l->p0 = l2.p0;
      l->p1 = l2.p1;
    } else if (ret > 0 && direction == 1) //外边
    {
      l->p0 = l1.p0;
      l->p1 = l1.p1;
    } else if (ret < 0 && direction == 0) {
      l->p0 = l2.p0;
      l->p1 = l2.p1;
    } else // ret < 0 && direction == true
    {
      l->p0 = l1.p0;
      l->p1 = l1.p1;
    }
  } else {
    if (ret > 0 && direction == 0) //内边
    {
      l->p0 = l1.p0;
      l->p1 = l1.p1;
    } else if (ret > 0 && direction == 1) //外边
    {
      l->p0 = l2.p0;
      l->p1 = l2.p1;
    } else if (ret < 0 && direction == 0) //内边
    {
      l->p0 = l1.p0;
      l->p1 = l1.p1;
    } else // ret < 0 && direction == true
    {
      l->p0 = l2.p0;
      l->p1 = l2.p1;
    }
  }
}

//测试用例task_id = '10084t190403171335'
void get_envelope_of_polygen(MyPoint *pInput, int n, MyPoint *q, int *m,
                             double delta, int direction) {
  segmentDequeType dqNewBorder; //新多边形的边
  int i = 0;
  Line l;
  MyPoint t1, t2;
  MyPoint Intersection;
  pdd B0, B1, A0, A1;
  //	bool isPolySelfIntersect = false;
  vector<MyPoint> poly;
  for (i = 0; i < n; i++) {
    get_scale_parallel_line(pInput, n, i, delta, direction, &l);
    dqNewBorder.push_back(l);
  }

  for (i = 0; i < n; i++) {
    l = dqNewBorder.at(i);
    t1 = l.p0;
    t2 = l.p1;
    B0 = make_pair(t1.x, t1.y);
    B1 = make_pair(t2.x, t2.y);
    l = dqNewBorder.at((i + 1) % n);
    t1 = l.p0;
    t2 = l.p1;
    A0 = make_pair(t1.x, t1.y); //平行线上的点
    A1 = make_pair(t2.x, t2.y); //平行线上的点
    lineLineIntersection(A0, A1, B0, B1, Intersection);
    q[i] = Intersection;
  }
#if DEBUG
  cout << "display polygon origin InterEnvelope:" << endl;
  for (int j = 0; j < n; j++) {
    cout << "s"
         << "[" << j << "]"
         << "= POINT(" << q[j].x << "," << q[j].y << ")" << endl;
  }
#endif
  *m = n;
#if 0
	if ((direction == 0) && (n>3))
	{
		bool ret = false;
		Segment S1, S2;
		MyPoint I0, I1;
		int j=0;
		for(j=0; j<n; j++)
		{
			S1 = {q[j], q[(j+1)%n]};
			S2 = {q[(j+2)%n], q[(j+3)%n]};
			ret = intersect2D_2Segments(S1, S2, &I0, &I1);
			if (ret)
			{
				isPolySelfIntersect = true;
				break;
			}
		}
		if (isPolySelfIntersect)
		{
			for(int k=0; k<n; k++)
			{
				if ((k==(j+1)%n) || (k==(j+2)%n))
				{
					if (k==(j+1)%n)
					{
						poly.push_back(I0);
					}
					continue;
				}
				else
				{
					poly.push_back(q[k]);
				}
			}
		}
	}
	if (isPolySelfIntersect)
	{
		*m = n-1;
		for(int i=0; i<n-1; i++)
		{
			q[i] = poly.at(i);
		}
	}
	else
	{
    	*m = n;
	}

#endif
}

//计算向量夹角
double CalculateVectorAngle(double x1, double y1, double x2, double y2) {
  // acos return radian,we should transform it into degree
  double t =
      (x1 * x2 + y1 * y2) / sqrt((x1 * x1 + y1 * y1) * (x2 * x2 + y2 * y2));
  return acos(t) * 180.0 / M_PI;
}

// 以c为基准  ，计算向量夹角
double getAngelOfTwoVector(MyPoint pt1, MyPoint pt2, MyPoint c) {
  double theta =
      atan2(pt1.x - c.x, pt1.y - c.y) - atan2(pt2.x - c.x, pt2.y - c.y);
  if (theta > M_PI)
    theta -= 2 * M_PI;
  if (theta < -M_PI)
    theta += 2 * M_PI;

  theta = theta * 180.0 / M_PI;
  return fabs(theta);
}

double getAngelOfTwoVector2(MyPoint pt1, MyPoint pt2, MyPoint c) {
  double theta =
      atan2(pt1.x - c.x, pt1.y - c.y) - atan2(pt2.x - c.x, pt2.y - c.y);
  if (theta > M_PI)
    theta -= 2 * M_PI;
  if (theta < -M_PI)
    theta += 2 * M_PI;

  theta = theta * 180.0 / M_PI;
  return (theta);
}

// area2D_Polygon(): compute the area of a 2D polygon
//  Input:  int n = the number of vertices in the polygon
//          myPoint* V = an array of n+1 vertex points with V[n]=V[0]
//  Return: the (float) area of the polygon
double area2D_Polygon(int n, const MyPoint *V) {
  double area = 0;
  int i, j, k; // indices

  if (n < 3)
    return 0; // a degenerate polygon

  for (i = 1, j = 2, k = 0; i < n; i++, j++, k++) {
    area += V[i].x * (V[j].y - V[k].y);
  }
  area += V[n].x * (V[1].y - V[n - 1].y); // wrap-around term
  return fabs(area / 2.0);
}

double my_area2D_Polygon(int n, const MyPoint *p) {
  double area = 0;
  MyPoint *q = new MyPoint[n + 1];
  memcpy(q, p, sizeof(MyPoint) * n);
  q[n] = p[0];
  area = area2D_Polygon(n, q);
  if (q) {
    delete[] q;
    q = NULL;
  }
  return area;
}
