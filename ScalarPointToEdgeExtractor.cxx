
#include "ScalarPointToEdgeExtractor.h"

#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDataObject.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>

vtkStandardNewMacro(ScalarPointToEdgeExtractor);

int ScalarPointToEdgeExtractor::FillInputPortInformation( int port, vtkInformation* info )
{
  if(port == 0)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData" );
    return 1;
  }

  vtkErrorMacro("This filter does not have more than 1 input port!");
  return 0;
}

int ScalarPointToEdgeExtractor::RequestData(vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo0 = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPolyData *input0 = vtkPolyData::SafeDownCast(
    inInfo0->Get(vtkDataObject::DATA_OBJECT()));

  output->SetPoints(input0->GetPoints());

  vtkDataArray * dataArray= input0->GetPointData()->GetScalars("FMean_Curvature");
  vtkDoubleArray * fdata= vtkDoubleArray::SafeDownCast(dataArray);

  vtkCellArray * lines = input0->GetLines();

  vtkSmartPointer<vtkCellArray> keepLines = vtkSmartPointer<vtkCellArray>::New();
  for(int i = 0; i < lines->GetNumberOfCells(); i++)
  {
    vtkSmartPointer<vtkIdList> pList = vtkSmartPointer<vtkIdList>::New();
    lines->GetCellAtId(i, pList);

    bool keep = true;
    for(int j = 0; j < pList->GetNumberOfIds(); j++)
    {
      if(fdata->GetValue(pList->GetId(j)) != -1)
      {
        keep = false;
        break;
      }
    }

    if(keep == true)
    {
      keepLines->InsertNextCell(input0->GetCell(i));
    }
  }

  output->SetLines(keepLines);

  return 1;
}


ScalarPointToEdgeExtractor::ScalarPointToEdgeExtractor()
  :vtkPolyDataAlgorithm()
{
}

ScalarPointToEdgeExtractor::~ScalarPointToEdgeExtractor()
{
}

void ScalarPointToEdgeExtractor::PrintSelf(ostream & os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
            
