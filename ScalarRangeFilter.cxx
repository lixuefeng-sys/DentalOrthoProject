
#include "ScalarRangeFilter.h"

#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDataObject.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>

vtkStandardNewMacro(ScalarRangeFilter);

int ScalarRangeFilter::FillInputPortInformation( int port, vtkInformation* info )
{
  if(port == 0)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData" );
    return 1;
  }

  vtkErrorMacro("This filter does not have more than 1 input port!");
  return 0;
}

int ScalarRangeFilter::RequestData(vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  if(this->m_filterZType != NotFilterZ && this->m_filterZValue == NAN)
  {
    vtkErrorMacro("filter Z value is not correct!");
    return 0;
  }

  // get the info objects
  vtkInformation *inInfo0 = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPolyData *input0 = vtkPolyData::SafeDownCast(
    inInfo0->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkDataSetAttributes> sourceAttributes;
  vtkSmartPointer<vtkAbstractArray> sourceArray;
  if(this->m_dataType == PointData)
  {
    sourceAttributes = input0->GetPointData();
  }
  else if(this->m_dataType == CellData)
  {
    sourceAttributes = input0->GetCellData();
  }
  else
  {
    vtkErrorMacro("Only PointData and CellData can be specified as dataType!");
    return 0;
  }

  if(this->m_sourceScalarName.empty())
  {
    sourceArray = sourceAttributes->GetScalars();
  }
  else
  {
    sourceArray = sourceAttributes->GetAbstractArray(this->m_sourceScalarName.c_str());
  }

  if(sourceArray != nullptr)
  {
    vtkSmartPointer<vtkDoubleArray> oArray = vtkDoubleArray::SafeDownCast(sourceArray);

    vtkSmartPointer<vtkDoubleArray> fArray = vtkSmartPointer<vtkDoubleArray>::New();
    fArray->SetNumberOfValues(oArray->GetNumberOfValues());
    if(!this->m_destScalarName.empty())
    {
      fArray->SetName(this->m_destScalarName.c_str());
    }

    double pos[3];
    for(int i = 0; i < oArray->GetNumberOfValues(); i++)
    {
      if(oArray->GetValue(i) > 0) {
          fArray->SetValue(i, 0);
      } else if(oArray->GetValue(i) < 0) {
        bool filter = false;
        if(this->m_filterZType != NotFilterZ)
        {
          input0->GetPoint(i, pos);
          if(this->m_filterZType == FilterZBelow) // filter out all point below Z
          {
            if(pos[2] <= this->m_filterZValue)
            {
              filter = true;
            }
          }
          else if(this->m_filterZType == FilterZAbove)
          {
            if(pos[2] >= this->m_filterZValue)
            {
              filter = true;
            }
          }
        }

        if(!filter && oArray->GetValue(i) < this->m_maxValue && oArray->GetValue(i) > this->m_minValue) {
          fArray->SetValue(i, -1);
        } else {
          fArray->SetValue(i, 0);
        }

      }
    }

    output->DeepCopy(input0);

    if(this->m_dataType == PointData)
    {
      output->GetPointData()->SetScalars(fArray);
    }
    else if(this->m_dataType == CellData)
    {
      output->GetCellData()->SetScalars(fArray);
    }
  }
  else
  {
    vtkErrorMacro("Do not found data array named " << m_sourceScalarName);
    return 0;
  }

  return 1;
}

ScalarRangeFilter::ScalarRangeFilter()
  :vtkPolyDataAlgorithm()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

ScalarRangeFilter::~ScalarRangeFilter()
{
}

void ScalarRangeFilter::PrintSelf(ostream & os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

double ScalarRangeFilter::getMaxValue() const
{
  return m_maxValue;
}

void ScalarRangeFilter::setMaxValue(double newMaxValue)
{
  m_maxValue = newMaxValue;
  this->Modified();
}

double ScalarRangeFilter::getMinValue() const
{
  return m_minValue;
}

void ScalarRangeFilter::setMinValue(double newMinValue)
{
  m_minValue = newMinValue;
  this->Modified();
}

const std::string &ScalarRangeFilter::getSourceScalarName() const
{
  return m_sourceScalarName;
}

void ScalarRangeFilter::setSourceScalarName(const std::string &newSourceScalarName)
{
  m_sourceScalarName = newSourceScalarName;
  this->Modified();
}

const std::string &ScalarRangeFilter::getDestScalarName() const
{
  return m_destScalarName;
}

void ScalarRangeFilter::setDestScalarName(const std::string &newDestScalarName)
{
  m_destScalarName = newDestScalarName;
  this->Modified();
}

int ScalarRangeFilter::getDataType()
{
  return m_dataType;
}

void ScalarRangeFilter::setDataType(enum DataType newDataType)
{
  m_dataType = newDataType;
  this->Modified();
}

double ScalarRangeFilter::getFilterZValue() const
{
  return m_filterZValue;
}

void ScalarRangeFilter::setFilterZValue(double newFilterZ)
{
  m_filterZValue = newFilterZ;
  this->Modified();
}

int ScalarRangeFilter::getFilterZType()
{
  return m_filterZType;
}

void ScalarRangeFilter::setFilterZType(enum FilterZType newFilterZType)
{
  m_filterZType = newFilterZType;
  this->Modified();
}
            
