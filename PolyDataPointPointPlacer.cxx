
#include "PolyDataPointPointPlacer.h"

#include <vtkObjectFactory.h>
#include <vtkCellPicker.h>
#include <vtkPointPicker.h>
#include <vtkPropCollection.h>
#include <vtkCellPicker.h>
#include <vtkPointPicker.h>
#include <vtkMapper.h>
#include <vtkPolyData.h>
#include <vtkAssemblyNode.h>
#include <vtkAssemblyPath.h>

class PolyDataPointPointPlacerInternals
{
public:
  typedef std::vector<PolyDataPointPointPlacerNode*> NodesContainerType;

  NodesContainerType Nodes;

  ~PolyDataPointPointPlacerInternals()
  {
    for (unsigned int i = 0; i < this->Nodes.size(); i++)
    {
      delete this->Nodes[i];
    }
    this->Nodes.clear();
  }

  PolyDataPointPointPlacerNode* GetNodeAtWorldPosition(double worldPos[3])
  {
    const double tolerance = 0;
    for (unsigned int i = 0; i < this->Nodes.size(); i++)
    {
      if (vtkMath::Distance2BetweenPoints(this->Nodes[i]->WorldPosition, worldPos) <= tolerance)
      {
        return this->Nodes[i];
      }
    }
    return nullptr;
  }

  PolyDataPointPointPlacerNode* InsertNodeAtCurrentPickPosition(
    vtkCellPicker * cellPicker, vtkPointPicker * pointPicker)
  {
    double worldPos[3];
    pointPicker->GetPickPosition(worldPos);

    // Get a node at this position if one exists and overwrite it
    // with the current pick position. If one doesn't exist, add
    // a new node.
    PolyDataPointPointPlacerNode* node = this->GetNodeAtWorldPosition(worldPos);
    if (!node)
    {
      node = new PolyDataPointPointPlacerNode;
      this->Nodes.push_back(node);
    }

    vtkMapper* mapper = vtkMapper::SafeDownCast(cellPicker->GetMapper());
    if (!mapper)
    {
      return nullptr;
    }

    // Get the underlying dataset
    vtkPolyData* pd = vtkPolyData::SafeDownCast(mapper->GetInput());
    if (!pd)
    {
      return nullptr;
    }

    node->CellId = cellPicker->GetCellId();
    cellPicker->GetPCoords(node->ParametricCoords);

    node->PointId = pointPicker->GetPointId();
    double pos[3];
    pd->GetPoints()->GetPoint(node->PointId, pos);
    node->WorldPosition[0] = pos[0];
    node->WorldPosition[1] = pos[1];
    node->WorldPosition[2] = pos[2];

    node->PolyData = pd;

    return node;
  }

  PolyDataPointPointPlacerNode* InsertNodeAtCurrentPickPosition(vtkPolyData* pd,
    double worldPos[3], vtkIdType cellId, vtkIdType pointId,
    const double vtkNotUsed(distanceOffset), int vtkNotUsed(snapToClosestPoint))
  {

    // Get a node at this position if one exists and overwrite it
    // with the current pick position. If one doesn't exist, add
    // a new node.
    PolyDataPointPointPlacerNode* node = this->GetNodeAtWorldPosition(worldPos);
    if (!node)
    {
      node = new PolyDataPointPointPlacerNode;
      this->Nodes.push_back(node);
    }

    node->CellId = cellId;
    node->PointId = pointId;

    node->WorldPosition[0] = worldPos[0];
    node->WorldPosition[1] = worldPos[1];
    node->WorldPosition[2] = worldPos[2];
    node->PolyData = pd;

    return node;
  }
  // ashish
};  // PolyDataPointPointPlacerInternals

//------------------------------------------------------------------------------
vtkStandardNewMacro(PolyDataPointPointPlacer)

PolyDataPointPointPlacer::PolyDataPointPointPlacer()
  :vtkPolyDataPointPlacer()
{
  this->CellPicker = vtkCellPicker::New();
  this->CellPicker->PickFromListOn();
  this->CellPicker->SetTolerance(0.005); // need some fluff

  this->PointPicker = vtkPointPicker::New();
  this->PointPicker->PickFromListOn();
  this->PointPicker->SetTolerance(0.005);

  this->Internals = new PolyDataPointPointPlacerInternals;
}

PolyDataPointPointPlacer::~PolyDataPointPointPlacer()
{
  this->CellPicker->Delete();
  this->PointPicker->Delete();
  delete this->Internals;

  if(CurrentSelectedNode != nullptr)
  {
    delete CurrentSelectedNode;
  }
}

void PolyDataPointPointPlacer::PrintSelf(std::ostream & os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

void PolyDataPointPointPlacer::AddProp(vtkProp * prop)
{
  this->SurfaceProps->AddItem(prop);
  this->CellPicker->AddPickList(prop);
  this->PointPicker->AddPickList(prop);
}

void PolyDataPointPointPlacer::RemoveViewProp(vtkProp *prop)
{
  this->Superclass::RemoveViewProp(prop);
  this->CellPicker->DeletePickList(prop);
  this->PointPicker->DeletePickList(prop);
}

void PolyDataPointPointPlacer::RemoveAllProps()
{
  this->Superclass::RemoveAllProps();
  this->CellPicker->InitializePickList();
  this->PointPicker->InitializePickList();
}

int PolyDataPointPointPlacer::ComputeWorldPosition(vtkRenderer *ren,
    double displayPos[2], double worldPos[3], double vtkNotUsed(worldOrient)[9])
{
  if (this->CellPicker->Pick(displayPos[0], displayPos[1], 0.0, ren) &&
      this->PointPicker->Pick(displayPos[0], displayPos[1], 0.0, ren))
  {
//    std::cout << "cell and point selected" << std::endl;
    vtkMapper* mapper = vtkMapper::SafeDownCast(this->CellPicker->GetMapper());
    if (!mapper)
    {
      return 0;
    }

    // Get the underlying dataset
    vtkPolyData* pd = vtkPolyData::SafeDownCast(mapper->GetInput());
    if (!pd)
    {
      return 0;
    }

    if (vtkAssemblyPath* path = this->CellPicker->GetPath())
    {
      // We are checking if the prop present in the path is present
      // in the list supplied to us.. If it is, that prop will be picked.
      // If not, no prop will be picked.

      bool found = false;
      vtkAssemblyNode* node = nullptr;
      vtkCollectionSimpleIterator sit;
      this->SurfaceProps->InitTraversal(sit);

      while (vtkProp* p = this->SurfaceProps->GetNextProp(sit))
      {
        vtkCollectionSimpleIterator psit;
        path->InitTraversal(psit);

        for (int i = 0; i < path->GetNumberOfItems() && !found; ++i)
        {
          node = path->GetNextNode(psit);
          found = (node->GetViewProp() == p);
        }

        if (found) // selected prop is in the list
        {
//          std::cout << "prop found in the list" << std::endl;
          vtkPolyData * pd = vtkPolyData::SafeDownCast(
                this->PointPicker->GetDataSet());

          vtkIdType cellId = this->CellPicker->GetCellId();
          vtkIdType pointId = this->PointPicker->GetPointId();

//          std::cout << "cellId: " << cellId << std::endl;
//          std::cout << "pointId: " << pointId << std::endl;

          vtkCell * cell = pd->GetCell(cellId);
          vtkIdList *pointIds = cell->GetPointIds();

          // make sure point is on the surface
          if(pointIds->IsId(pointId) == -1)
          {
            return 0;
          }

//          std::cout << "point found in cell" << std::endl;

          Node * node = this->Internals->InsertNodeAtCurrentPickPosition(
                this->CellPicker, this->PointPicker);

          worldPos[0] = node->WorldPosition[0];
          worldPos[1] = node->WorldPosition[1];
          worldPos[2] = node->WorldPosition[2];

          return 1;
        }
      }
    }
  }

  return 0;
}

int PolyDataPointPointPlacer::ComputeWorldPosition(vtkRenderer *ren,
    double displayPos[], double * vtkNotUsed(refWorldPos), double worldPos[],
    double worldOrient[])
{
  return this->ComputeWorldPosition(ren, displayPos, worldPos, worldOrient);
}

int PolyDataPointPointPlacer::ValidateWorldPosition(double worldPos[])
{
  return 1;
}

int PolyDataPointPointPlacer::UpdateNodeWorldPosition(double worldPos[], vtkIdType nodePointId)
{
  return 1;
}

int PolyDataPointPointPlacer::ValidateDisplayPosition(vtkRenderer *, double displayPos[])
{
  // We could check here to ensure that the display point picks one of the
  // terrain props, but the contour representation always calls
  // ComputeWorldPosition followed by
  // ValidateDisplayPosition/ValidateWorldPosition when it needs to
  // update a node...
  //
  // So that would be wasting CPU cycles to perform
  // the same check twice..  Just return 1 here.

  return 1;
}

int PolyDataPointPointPlacer::ValidateWorldPosition(double worldPos[], double * vtkNotUsed(worldOrient))
{
  return this->ValidateWorldPosition(worldPos);
}

PolyDataPointPointPlacer::Node *PolyDataPointPointPlacer::GetNodeAtWorldPosition(double worldPos[3])
{
  return this->Internals->GetNodeAtWorldPosition(worldPos);
}
            
