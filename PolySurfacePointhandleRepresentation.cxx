#include "PolySurfacePointHandleRepresentation.h"

#include <vtkPointPlacer.h>
#include <QString>

vtkStandardNewMacro(PolySurfacePointHandleRepresentation);

PolySurfacePointHandleRepresentation::PolySurfacePointHandleRepresentation()
  :vtkPointHandleRepresentation3D()
{
}

PolySurfacePointHandleRepresentation::~PolySurfacePointHandleRepresentation()
{
}

void PolySurfacePointHandleRepresentation::PrintSelf(std::ostream &os, vtkIndent indent)
{
  vtkPointHandleRepresentation3D::PrintSelf(os, indent);
}

int PolySurfacePointHandleRepresentation::CheckConstraint(vtkRenderer *renderer, double pos[])
{
  double worldPos[3];
  double worldOrient[9];

  return this->PointPlacer->ComputeWorldPosition(renderer, pos, worldPos, worldOrient);
}
