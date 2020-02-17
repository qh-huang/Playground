// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

// Assume that classes are already given for the objects:
//    MyPoint and Vector with
//        coordinates {float x, y, z;}
//        operators for:
//            == to test  equality
//            != to test  inequality
//            MyPoint   = MyPoint 卤 Vector
//            Vector =  MyPoint - myPoint
//            Vector =  Scalar * Vector    (scalar product)
//            Vector =  Vector * Vector    (3D cross product)
//    Line and Ray and Segment with defining  points {MyPoint p0, p1;}
//        (a Line is infinite, Rays and  Segments start at p0)
//        (a Ray extends beyond p1, but a  Segment ends at p1)
//    Plane with a point and a normal {MyPoint V0; Vector  n;}
//===================================================================
#include <math.h>
#include <stdio.h>
#include <vae/geometry.h>

//===================================================================

// inSegment(): determine if a point is inside a segment
//    Input:  a point P, and a collinear segment S
//    Return: 1 = P is inside S
//            0 = P is not inside S
int inSegment(MyPoint P, Segment S) {
  if ((P == S.p0) || (P == S.p1)) // double有精度问题，必须做这种处理
    return 1;
  if (S.p0.x != S.p1.x) { // S is not  vertical
    if (S.p0.x <= P.x && P.x <= S.p1.x)
      return 1;
    if (S.p0.x >= P.x && P.x >= S.p1.x)
      return 1;
  } else { // S is vertical, so test y  coordinate
    if (S.p0.y <= P.y && P.y <= S.p1.y)
      return 1;
    if (S.p0.y >= P.y && P.y >= S.p1.y)
      return 1;
  }
  return 0;
}

MyPoint getMidPoint(MyPoint pt0, MyPoint pt1) {
  return MyPoint((pt0.x + pt1.x) / 2, (pt0.y + pt1.y) / 2);
}

//===================================================================
