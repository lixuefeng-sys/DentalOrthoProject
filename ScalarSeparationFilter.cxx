#include "ScalarSeparationFilter.h"

#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDataObject.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>

#include <map>

vtkStandardNewMacro(ScalarSeparationFilter);

int ScalarSeparationFilter::FillInputPortInformation( int port, vtkInformation* info )
{
  if(port == 0)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData" );
    return 1;
  }

  vtkErrorMacro("This filter does not have more than 1 input port!");
  return 0;
}

int ScalarSeparationFilter::RequestData(vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo0 = inputVector[0]->GetInformationObject(0);

  vtkPolyData *input0 = vtkPolyData::SafeDownCast(
      inInfo0->Get(vtkDataObject::DATA_OBJECT()));
  if(input0->NeedToBuildCells()) input0->BuildCells();
  input0->BuildLinks();

  vtkIdType pdCellCount = input0->GetNumberOfCells(); // number of cells in the data set
  vtkIdType visitedCount = 0;

  this->UpdateProgress(0.2);

  vtkIdType visCellCount = 0;
  std::vector<bool> markCellIds;
  std::vector<vSP<vtkIdList>> visCellIdsList;
  for(int i = 0; i < pdCellCount; ++i)
  {
    markCellIds.push_back( false );
  }
  while(visCellCount != pdCellCount)
  {
    vtkIdType seedCellId = -1;
    for(int i = 0; i < pdCellCount; ++i)
    {
      if(!markCellIds[i])
      {
        seedCellId = i;
      }
    }
    if(-1 == seedCellId)
    {
      break;
    }
    vSP<vtkIdList> visitedCells = GetConnectedCellIds(input0, seedCellId );
    visitedCount += visitedCells->GetNumberOfIds();

    this->UpdateProgress(0.2 + 0.75 * visitedCount / pdCellCount);

    visCellCount += visitedCells->GetNumberOfIds();
    visCellIdsList.push_back( visitedCells );
    for( int i = 0; i < visitedCells->GetNumberOfIds(); ++i )
    {
      auto id = visitedCells->GetId( i );
      markCellIds[id] = true;
    }
  }

//  std::cout << visCellIdsList.size() << std::endl;

  ScalarSeparationFilter::vtkPolyDataList newPds =
      GetSplitedPdsByConnected(input0, visCellIdsList );

//  std::cout << newPds.size() << std::endl;

  this->SetNumberOfOutputPorts(newPds.size());
  outputVector->SetNumberOfInformationObjects(newPds.size());

  for(int i = 0; i < newPds.size(); i++)
  {
    vtkInformation * outInfo = outputVector->GetInformationObject(i);
    outInfo->Set(vtkDataObject::DATA_OBJECT(), newPds[i]);
  }

  this->UpdateProgress(1.0);

  return 1;
}

ScalarSeparationFilter::vtkPolyDataList ScalarSeparationFilter::GetSplitedPdsByConnected(vtkSmartPointer<vtkPolyData> pd, std::vector<vtkSmartPointer<vtkIdList> > cellLists)
{
  std::vector<vSP<vtkPolyData>> newPolyDatas;
  for( int i = 0; i < cellLists.size(); ++i )
  {
      auto cellList = cellLists[i];
      vDNSP(vtkPolyData, polyData);
      vDNSP(vtkPoints, points);
      vDNSP(vtkCellArray, cells);
      std::map<vtkIdType, vtkIdType> ptMap;
      for( int j = 0; j < cellList->GetNumberOfIds(); ++j )
      {
          auto cellId = cellList->GetId( j );
          auto cell = pd->GetCell( cellId );
          auto ptIds = cell->GetPointIds();

          vDNSP(vtkIdList, newPtIds);
          for( int k = 0; k < ptIds->GetNumberOfIds(); ++k )
          {
              auto ptId = ptIds->GetId( k );
              if( ptMap.find( ptId ) != ptMap.end() )
              {
                  newPtIds->InsertNextId( ptMap[ptId] );
              }
              else
              {
                  auto newPtId = points->InsertNextPoint( pd->GetPoint( ptId ) );
                  ptMap[ptId] = newPtId;
                  newPtIds->InsertNextId( newPtId );
              }
          }
          cells->InsertNextCell( newPtIds );
      }
      polyData->SetPoints( points );
      polyData->SetPolys( cells );
      polyData->Modified();

      newPolyDatas.push_back( polyData );
  }

  return newPolyDatas;
}

vtkSmartPointer<vtkIdList> ScalarSeparationFilter::GetConnectedCellIds(vtkPolyData * pd, vtkIdType seedCellId)
{
  vtkDoubleArray * scalars = vtkDoubleArray::SafeDownCast(pd->GetPointData()->GetScalars());
  vtkIdType nPts;
  const vtkIdType *pts;
  pd->GetCellPoints(seedCellId, nPts, pts);
  std::vector<Edge> currrentEdges;
  for(int i = 0; i < 3; ++i)
  {
    Edge edge;
    edge.cellId = seedCellId;
    edge.edgePt1 = pts[i];
    edge.edgePt2 = pts[(i+1) % 3];
    currrentEdges.push_back(edge);
  }
  int total = pd->GetNumberOfCells();
  vtkNew<vtkIdList> visitedCellIds;
  visitedCellIds->InsertNextId(seedCellId);
  std::vector<Edge> nextEdges;
  while (currrentEdges.size() > 0)
  {
    for(int i = 0; i < currrentEdges.size(); ++i)
    {
      Edge edge = currrentEdges[i];

      vtkIdType pId1 = edge.edgePt1;
      vtkIdType pId2 = edge.edgePt2;

      // skip the edge marked as not connected
      if(scalars->GetValue(pId1) != 0 && scalars->GetValue(pId2) != 0)
      {
        continue;
      }

      vtkNew<vtkIdList> neighborCellIds;
      pd->GetCellEdgeNeighbors(edge.cellId, pId1, pId2, neighborCellIds);
      for(int j = 0; j < neighborCellIds->GetNumberOfIds(); ++j)
      {
        auto neiCellId = neighborCellIds->GetId(j);
        if(-1 != visitedCellIds->IsId(neiCellId))
        {
          continue;
        }
        pd->GetCellPoints(neiCellId, nPts, pts);
        vtkIdType thirdPt = -1;
        for(int k = 0; k < 3; ++k)
        {
          if(pts[k] != edge.edgePt1 && pts[k] != edge.edgePt2)
          {
            thirdPt = pts[k];
            break;
          }
        }
        if(-1 == thirdPt)
        {
          continue;
        }

        Edge edge1;
        edge1.cellId = neiCellId;
        edge1.edgePt1 = edge.edgePt1;
        edge1.edgePt2 = thirdPt;

        Edge edge2;
        edge2.cellId = neiCellId;
        edge2.edgePt1 = edge.edgePt2;
        edge2.edgePt2 = thirdPt;

        nextEdges.push_back(edge1);
        nextEdges.push_back(edge2);
        visitedCellIds->InsertNextId(neiCellId);

        int size = visitedCellIds->GetNumberOfIds();
      }
    }
    currrentEdges.swap(nextEdges);
    nextEdges.clear();
  }

  return visitedCellIds;
}

ScalarSeparationFilter::ScalarSeparationFilter()
  :vtkPolyDataAlgorithm()
{
  this->SetNumberOfInputPorts(1);
}

ScalarSeparationFilter::~ScalarSeparationFilter()
{
}

void ScalarSeparationFilter::PrintSelf(ostream & os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

