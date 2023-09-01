#ifndef VTK_UTIL_H
#define VTK_UTIL_H

#include <vtkSmartPointer.h>

#include "vtkCommon.h"

class vtkPoints;
class vtkIdList;
class vtkColor3ub;


class vtkRenderer;
namespace vtkUtil
{

// https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
// https://www.element84.com/blog/determining-the-winding-of-a-polygon-given-as-a-set-of-ordered-points
extern bool checkPointsCounterClockwise(vtkSmartPointer<vtkPoints> points);

extern void printIds(vSP<vtkIdList> list);

extern void convertColor(vtkColor3ub color, double * nColor);

extern void convertColor(int r, int g, int b, double * nColor);

extern void displayPositionToLocalDisplayPosition(vtkRenderer * renderer, int * pos, int * result);

} // namespace vtkUtil

#endif // VTK_UTIL_H
