#include "vtkUtil.h"

#include <vtkColor.h>
#include <vtkCoordinate.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkRenderer.h>

namespace vtkUtil
{

bool checkPointsCounterClockwise(vtkSmartPointer<vtkPoints> points)
{
  int sum = 0;
  for(int i = 1; i < points->GetNumberOfPoints(); i++)
  {
    double pos1[3];
    points->GetPoint(i - 1, pos1);
    double pos2[3];
    points->GetPoint(i, pos2);

    sum += (pos2[0] - pos1[0]) * (pos2[2] + pos1[2]);
  }

  return sum < 0;
}


void printIds(vSP<vtkIdList> list)
{
  for(int i = 0; i < list->GetNumberOfIds(); i++)
  {
    std::cout << list->GetId(i) << ", ";
  }
  std::cout << std::endl;
}

void convertColor(vtkColor3ub color, double * nColor)
{
  nColor[0] = color.GetRed() / 255.;
  nColor[1] = color.GetGreen() / 255.;
  nColor[2] = color.GetBlue() / 255.;
}

void convertColor(int r, int g, int b, double * nColor)
{
  nColor[0] = r / 255.;
  nColor[1] = g / 255.;
  nColor[2] = b / 255.;
}

void displayPositionToLocalDisplayPosition(vtkRenderer * renderer, int * pos, int * result)
{
  vtkNew<vtkCoordinate> coord;
  coord->SetCoordinateSystemToDisplay();
  coord->SetValue(pos[0], pos[1]);

  int * local = coord->GetComputedLocalDisplayValue(renderer);
  result[0] = local[0];
  result[1] = local[1];
}

} // namespace vtkUtil
