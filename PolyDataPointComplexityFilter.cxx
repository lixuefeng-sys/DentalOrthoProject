
#include "PolyDataPointComplexityFilter.h"

#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDataObject.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkCellArrayIterator.h>

#include <set>
#include <cmath>

#include "vtkCommon.h"

vtkStandardNewMacro(PolyDataPointComplexityFilter);

int PolyDataPointComplexityFilter::FillInputPortInformation( int port, vtkInformation* info )
{
  if(port == 0) // first port is the source
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData" );
    return 1;
  }
  if(port == 1) // second port is the connection map
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData" );
    return 1;
  }

  vtkErrorMacro("This filter does not have more than 2 input port!");
  return 0;
}

int PolyDataPointComplexityFilter::RequestData(vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo0 = inputVector[0]->GetInformationObject(0);
  vtkInformation *inInfo1 = inputVector[1]->GetInformationObject(0);

  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPolyData *input0 = vtkPolyData::SafeDownCast(
    inInfo0->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *input1 = vtkPolyData::SafeDownCast(
    inInfo1->Get(vtkDataObject::DATA_OBJECT()));

  output->ShallowCopy(input0);
  vDNSP(vtkIntArray, complexityArray);
  complexityArray->SetName("PointComplexity");
  complexityArray->SetNumberOfValues(input1->GetNumberOfPoints());

  vDSP(vtkIdList, featurePointIdList) = this->getFeaturePointIds(input0);

  std::cout << "feature points extracted" << std::endl;

  if(input1->NeedToBuildCells())
  {
    input1->BuildCells();
  }
  input1->BuildLinks();

  std::cout << "cell and links built" << std::endl;

  vtkIdType numberOfPoints = input1->GetNumberOfPoints();
  for(int i = 0; i < numberOfPoints; i++)
  {
//    std::cout << "processing point " << i << "/" << numberOfPoints << std::endl;

    if(featurePointIdList->IsId(i) == -1) // not feature point
    {
      complexityArray->SetValue(i, 0);
    }
    else // feature point, calculate complexity
    {
      vSP<vtkIdList> sortedPIds = getSortedNeighbourPointIds(input1, i);
//      std::cout << "neighbour points extracted " << std::endl;
      int complexity = this->calculateComplexity(sortedPIds, featurePointIdList);
//      std::cout << "point id is " << i << ", neighbours is " << sortedPIds->GetNumberOfIds()
//                << "," << ", complexity is " << complexity << std::endl;
      complexityArray->SetValue(i, complexity);
    }
  }

  output->GetPointData()->AddArray(complexityArray);

  return 1;
}

vtkSmartPointer<vtkIdList> PolyDataPointComplexityFilter::getFeaturePointIds(vtkPolyData *input0)
{
  // extract all the point ids in lines
  vDNSP(vtkIdList, featurePointIdList);
  vtkCellArray *lines = input0->GetLines();
  for(int i = 0; i < lines->GetNumberOfCells(); i++)
  {
//    std::cout << "line:" << i << std::endl;
    vDNSP(vtkIdList, pList);
    lines->GetCellAtId(i, pList); // a line has two points;

    featurePointIdList->InsertUniqueId(pList->GetId(0));
    featurePointIdList->InsertUniqueId(pList->GetId(1));
  }

  return featurePointIdList;
}

vSP<vtkIdList> PolyDataPointComplexityFilter::getSortedNeighbourPointIds(vtkPolyData *polydata,
    vtkIdType centerPointId)
{
  if(m_neighbourMap.find(centerPointId) == m_neighbourMap.end()) // not cached
  {
    // get all neighbour cell ids using that point
    vtkNew<vtkIdList> neighbourCellIds;
    polydata->GetPointCells(centerPointId, neighbourCellIds);

    vtkNew<vtkIdList> sortedIds;
    vtkIdType lastCellId = neighbourCellIds->GetId(0);

    // the first cell
    vtkCell *cell = polydata->GetCell(lastCellId);
    vtkIdList *ids = cell->GetPointIds(); // points in that cell

    // initialize the sortedIds
  //  std::cout << "Initial sortedIds: ";
    for(int i = 0; i < ids->GetNumberOfIds(); ++i)
    {
      vtkIdType id = ids->GetId(i);
      if(id != centerPointId) {
        sortedIds->InsertNextId(id);
  //      std::cout << id << " ";
      }
    }
  //  std::cout << std::endl;

    // initialize the total number of points
    vDNSP(vtkIdList, neighbourPoints);
    neighbourPoints->DeepCopy(sortedIds);
    for(int i = 1; i < neighbourCellIds->GetNumberOfIds(); i++)
    {
      vtkIdType cellId = neighbourCellIds->GetId(i);
      vtkCell *cell = polydata->GetCell(cellId);
      vtkIdList *pointIds = cell->GetPointIds(); // points in that cell

      // for each point
      for(int j = 0; j < pointIds->GetNumberOfIds(); j++)
      {
        vtkIdType id = pointIds->GetId(j);
        if(id != centerPointId) // skip the center point id
        {
          neighbourPoints->InsertUniqueId(id);
  //        std::cout << "append point id: " << id << " ";
        }
      }
    }

    while (sortedIds->GetNumberOfIds() < neighbourPoints->GetNumberOfIds())
    {
      vtkNew<vtkIdList> cellIds;
      int index = sortedIds->GetNumberOfIds() - 1;
      vtkIdType lastPointId = sortedIds->GetId(index);

      polydata->GetCellEdgeNeighbors(lastCellId, centerPointId, lastPointId, cellIds);
      if(cellIds->GetNumberOfIds() > 0) {
        vtkIdType cellId = cellIds->GetId(0);
        vtkCell *cell = polydata->GetCell(cellId);
        vtkIdList *pointIds = cell->GetPointIds();
        for(int i = 0; i < pointIds->GetNumberOfIds(); i++)
        {
          vtkIdType id = pointIds->GetId(i);
          if(id != centerPointId && sortedIds->IsId(id) == -1)
          {
            sortedIds->InsertNextId(id);
            lastCellId = cellId;
          }
        }
      }
      else
      {
        std::cerr << "do not found any cell edge neighbors" << std::endl;
      }
    }

    this->m_neighbourMap[centerPointId] = sortedIds;
    return sortedIds;
  }
  else  // found cache
  {
    return this->m_neighbourMap[centerPointId];
  }
}

int PolyDataPointComplexityFilter::calculateComplexity(vtkSmartPointer<vtkIdList> sortedPointIds,
    vtkIdList *featurePointIdList)
{
  int complexity = 0;
  int points = sortedPointIds->GetNumberOfIds();
  for(int i = 0; i < points; i++)
  {
    int p1 = featurePointIdList->IsId(sortedPointIds->GetId(i)) == -1 ? 0 : 1;
    int p2 = featurePointIdList->IsId(sortedPointIds->GetId((i + 1) % points)) == -1 ? 0 : 1;
    complexity += std::abs(p1 - p2);
  }
  return complexity;
}

PolyDataPointComplexityFilter::PolyDataPointComplexityFilter()
  :vtkPolyDataAlgorithm()
{
  // accepts two ports
  this->SetNumberOfInputPorts(2);
}

PolyDataPointComplexityFilter::~PolyDataPointComplexityFilter()
{
}

void PolyDataPointComplexityFilter::PrintSelf(ostream & os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
