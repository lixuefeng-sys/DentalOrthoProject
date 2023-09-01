
#ifndef PolyDataPointPointPlacer_H
#define PolyDataPointPointPlacer_H

#include <vtkPolyDataPointPlacer.h>
#include <vtkSetGet.h>

class vtkCellPicker;
class vtkPointPicker;
class vtkPolyDataCollection;
class vtkPolyData;
class PolyDataPointPointPlacerInternals;

struct PolyDataPointPointPlacerNode
{
  double WorldPosition[3];
  vtkIdType CellId;
  vtkIdType PointId;
  double ParametricCoords[3]; // parametric coords within cell
  vtkPolyData* PolyData;
};

class PolyDataPointPointPlacer : public vtkPolyDataPointPlacer
{
public:
  static PolyDataPointPointPlacer * New();


  vtkTypeMacro(PolyDataPointPointPlacer, vtkPolyDataPointPlacer);

  void PrintSelf(ostream & os, vtkIndent indent) override;

  // Add /remove a prop, to place points on
  void AddProp(vtkProp*) override;
  void RemoveViewProp(vtkProp* prop) override;
  void RemoveAllProps() override;

  /**
   * Given a renderer and a display position in pixel coordinates,
   * compute the world position and orientation where this point
   * will be placed. This method is typically used by the
   * representation to place the point initially.
   * For the Terrain point placer this computes world points that
   * lie at the specified height above the terrain.
   */
  int ComputeWorldPosition(
    vtkRenderer* ren, double displayPos[2], double worldPos[3], double worldOrient[9]) override;

  /**
   * Given a renderer, a display position, and a reference world
   * position, compute the new world position and orientation
   * of this point. This method is typically used by the
   * representation to move the point.
   */
  int ComputeWorldPosition(vtkRenderer* ren, double displayPos[2], double refWorldPos[3],
    double worldPos[3], double worldOrient[9]) override;

  /**
   * Given a world position check the validity of this
   * position according to the constraints of the placer
   */
  int ValidateWorldPosition(double worldPos[3]) override;

  /**
   * Give the node a chance to update its auxiliary point id.
   */
  int UpdateNodeWorldPosition(double worldPos[3], vtkIdType nodePointId) override;

  /**
   * Given a display position, check the validity of this position.
   */
  int ValidateDisplayPosition(vtkRenderer*, double displayPos[2]) override;

  /**
   * Given a world position and a world orientation,
   * validate it according to the constraints of the placer.
   */
  int ValidateWorldPosition(double worldPos[3], double worldOrient[9]) override;

  ///@{
  /**
   * Get the Prop picker.
   */
  vtkGetObjectMacro(CellPicker, vtkCellPicker);
  ///@}

  ///@{
  /**
   * Be sure to add polydata on which you wish to place points to this list
   * or they will not be considered for placement.
   */
  vtkGetObjectMacro(Polys, vtkPolyDataCollection);
  ///@}

  typedef PolyDataPointPointPlacerNode Node;
  Node* GetNodeAtWorldPosition(double worldPos[3]);

protected:
  PolyDataPointPointPlacer();
  ~PolyDataPointPointPlacer() override;

    // The props that represents the terrain data (one or more) in a rendered
  // scene
  vtkCellPicker * CellPicker;
  vtkPointPicker * PointPicker;
  vtkPolyDataCollection * Polys;
  PolyDataPointPointPlacerInternals * Internals;

  Node * CurrentSelectedNode {nullptr};
private:
  PolyDataPointPointPlacer(const PolyDataPointPointPlacer &) = delete;
  void operator=(const PolyDataPointPointPlacer &) = delete;
};

#endif // PolyDataPointPointPlacer_H
            
