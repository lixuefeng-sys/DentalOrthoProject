
#include "ErodeFilter.h"

#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDataObject.h>
#include <vtkPolyData.h>
#include <vtkLine.h>
#include <vtkVertex.h>

#include "Util.h"

vtkStandardNewMacro(ErodeFilter);

int ErodeFilter::FillInputPortInformation( int port, vtkInformation* info )
{
  return PolyDataPointComplexityFilter::FillInputPortInformation(port, info);
}

int ErodeFilter::RequestData(vtkInformation *vtkNotUsed(request),
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

  output->SetPoints(input1->GetPoints());

  vDSP(vtkIdList, featurePointIdList) = this->getFeaturePointIds(input0);

  if(input1->NeedToBuildCells())
  {
    input1->BuildCells();
  }
  input1->BuildLinks();

  this->UpdateProgress(0.1);

  vDNSP(vtkIdList, centerIdList);
  vDNSP(vtkIdList, diskIdList);
  vDNSP(vtkIdList, removeIdList);

//  std::cout << "feature points: " << featurePointIdList->GetNumberOfIds() << std::endl;

  bool finished = false;

  int iteration = 1;
  while(!finished)
  {
//    std::cout << "erosion iteration = " << iteration++ << std::endl;

    centerIdList->SetNumberOfIds(0);
    diskIdList->SetNumberOfIds(0);
    removeIdList->SetNumberOfIds(0);

    finished = true;
    // try to mark each point type
    vEachIdList(i, featurePointIdList)
    {
      vtkIdType id = featurePointIdList->GetId(i);
      vSP<vtkIdList> sortedPIds = getSortedNeighbourPointIds(input1, id);

      bool isDiskCenter = true;
      // for each neighbour
      vEachIdList(j, sortedPIds)
      {
        if(featurePointIdList->IsId(sortedPIds->GetId(j)) == -1)
        {
          // found a non feature neighbour point
          // not a center point
          isDiskCenter = false;
          break;
        }
      }

      if(isDiskCenter) // found center point
      {
        centerIdList->InsertNextId(id);
        // all neighbour points are disk points
        for(int j = 0; j < sortedPIds->GetNumberOfIds(); j++)
        {
          diskIdList->InsertUniqueId(sortedPIds->GetId(j));
        }
      }
    }

    // try to remove points
    vEachIdListR(i, featurePointIdList)
    {
      vtkIdType id = featurePointIdList->GetId(i);
      // not center point but disk point
      if(centerIdList->IsId(id) == -1 && diskIdList->IsId(id) != -1)
      {
        vSP<vtkIdList> sortedPIds = getSortedNeighbourPointIds(input1, id);
        int complexity = calculateComplexity(sortedPIds, featurePointIdList);

        if(complexity < 4)
        {
          featurePointIdList->DeleteId(id);
          finished = false;
        }
      }
    }

    /*
    vEachIdListR(i, featurePointIdList)
    {
      vtkIdType id = featurePointIdList->GetId(i);
      vSP<vtkIdList> sortedPIds = getSortedNeighbourPointIds(input1, id);
      int complexity = calculateComplexity(sortedPIds, featurePointIdList);
      if(centerIdList->IsId(id) == -1   // not center point
         && diskIdList->IsId(id) == -1 // not disk point
         && complexity < 4)             // not complex point
      {
        // removing all outlier points with at least two neighbors belonging to F
        if(countFeaturedNeighbours(sortedPIds, featurePointIdList) >= 2)
        {
          featurePointIdList->DeleteId(id);
        }
      }
    }
    */
  } // while

  this->UpdateProgress(0.8);

//  iteration = 1;
  finished = false;
  while(!finished)
  {
//    std::cout << "prune iteration = " << iteration++ << std::endl;

    finished = true;
    // remove all simple points
    vEachIdListR(i, featurePointIdList)
    {
      vtkIdType id = featurePointIdList->GetId(i);
      vSP<vtkIdList> sortedPIds = getSortedNeighbourPointIds(input1, id);
      int complexity = calculateComplexity(sortedPIds, featurePointIdList);
      if(complexity < 4) {
        featurePointIdList->DeleteId(id);
        finished = false;
      }
    }
  }

//  std::cout << "feature points: " << featurePointIdList->GetNumberOfIds() << std::endl;
  this->UpdateProgress(0.9);

  vDNSP(vtkCellArray, newLines);
  vtkCellArray *lines = input0->GetLines();
  for(int i = 0; i < lines->GetNumberOfCells(); i++)
  {
    vDNSP(vtkIdList, pointIds);
    lines->GetCellAtId(i, pointIds);

    bool keep = true;
    vEachIdList(j, pointIds)
    {
      vtkIdType pId = pointIds->GetId(j);
      if(featurePointIdList->IsId(pId) == -1) // point is removed
      {
        keep = false;
        break;
      }
    }
    if(keep == true)
    {
      vDNSP(vtkLine, line);
      line->GetPointIds()->InsertId(0, pointIds->GetId(0));
      line->GetPointIds()->InsertId(1, pointIds->GetId(1));
      newLines->InsertNextCell(line);
    }
  }
  output->SetLines(newLines);

  this->UpdateProgress(1.0);

  /*
  vDNSP(vtkCellArray, vertices);
  vEachIdList(i, featurePointIdList)
  {
    vDNSP(vtkVertex, vertex);
    vertex->GetPointIds()->InsertId(0, featurePointIdList->GetId(i));
    vertices->InsertNextCell(vertex);
  }

  output->SetVerts(vertices);
  */

  return 1;
}

int ErodeFilter::countFeaturedNeighbours(vSP<vtkIdList> sortedPointIds,
                                         vSP<vtkIdList> featureList)
{
  int count = 0;
  vEachIdList(i, sortedPointIds)
  {
    vtkIdType id = sortedPointIds->GetId(i);
    count += featureList->IsId(id) == -1 ? 0 : 1;
  }
  return count;
}

ErodeFilter::ErodeFilter()
  :PolyDataPointComplexityFilter()
{
}

ErodeFilter::~ErodeFilter()
{
}

void ErodeFilter::PrintSelf(ostream & os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
            
