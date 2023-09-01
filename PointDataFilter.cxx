
#include "PointDataFilter.h"

#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDataObject.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>

vtkStandardNewMacro(PointDataFilter);

int PointDataFilter::FillInputPortInformation( int port, vtkInformation* info )
{
  if(port == 0) // first port is the source
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData" );
    return 1;
  }
  if(port == 1) // second port is the data to generate point data
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData" );
    return 1;
  }

  vtkErrorMacro("This filter does not have more than 1 input port!");
  return 0;
}

int PointDataFilter::RequestData(vtkInformation *vtkNotUsed(request),
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

  output->DeepCopy(input0);

  int points = input0->GetNumberOfPoints();
  vDNSP(vtkDoubleArray, array);
  array->SetNumberOfValues(points);

  for(int i = 0; i < points; i++)
  {
    array->InsertValue(i, 0);
  }

  vtkCellArray * lines = input1->GetLines();

  for(int i = 0; i < lines->GetNumberOfCells(); i++)
  {
    vDNSP(vtkIdList, pointList);
    lines->GetCellAtId(i, pointList);
    vEachIdList(j, pointList)
    {
      array->InsertValue(pointList->GetId(j), 1);
    }
  }

  output->GetPointData()->SetScalars(array);

  return 1;
}

PointDataFilter::PointDataFilter()
  :vtkPolyDataAlgorithm()
{
  // accepts two ports
  this->SetNumberOfInputPorts(2);
}

PointDataFilter::~PointDataFilter()
{
}

void PointDataFilter::PrintSelf(ostream & os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
            
