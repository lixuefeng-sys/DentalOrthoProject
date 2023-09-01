#ifndef PolySurfacePointhandleRepresentation_H
#define PolySurfacePointhandleRepresentation_H

#include <vtkPointHandleRepresentation3D.h>

class PolySurfacePointHandleRepresentation : public vtkPointHandleRepresentation3D
{
public:
  static PolySurfacePointHandleRepresentation * New();

  vtkTypeMacro(PolySurfacePointHandleRepresentation, vtkPointHandleRepresentation3D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  virtual int CheckConstraint(vtkRenderer * renderer, double pos[2]) override;
protected:
  PolySurfacePointHandleRepresentation();
  ~PolySurfacePointHandleRepresentation() override;

private:
  PolySurfacePointHandleRepresentation(const PolySurfacePointHandleRepresentation &) = delete;
  void operator=(const vtkPointHandleRepresentation3D &) = delete;
};

#endif // PolySurfacePointhandleRepresentation_H
