
#ifndef ScalarRangeFilter_H
#define ScalarRangeFilter_H

#include <vtkPolyDataAlgorithm.h>
#include <vtkSetGet.h>

class ScalarRangeFilter : public vtkPolyDataAlgorithm
{
public:
  enum DataType {
    CellData = 0,
    PointData
  };

  enum FilterZType {
    NotFilterZ = 0,
    FilterZBelow,
    FilterZAbove
  };

  static ScalarRangeFilter * New();

  vtkTypeMacro(ScalarRangeFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream & os, vtkIndent indent) override;

  double getMaxValue() const;
  void setMaxValue(double newMaxValue);

  double getMinValue() const;
  void setMinValue(double newMinValue);

  const std::string &getSourceScalarName() const;
  void setSourceScalarName(const std::string &newSourceScalarName);

  const std::string &getDestScalarName() const;
  void setDestScalarName(const std::string &newDestScalarName);

  int getDataType();
  void setDataType(enum DataType newDataType);

  double getFilterZValue() const;
  void setFilterZValue(double newFilterZ);

  int getFilterZType();
  void setFilterZType(enum FilterZType newFilterZType);

protected:
  double m_maxValue;
  double m_minValue;

  enum FilterZType m_filterZType {NotFilterZ};
  double m_filterZValue {NAN};

  std::string m_sourceScalarName;
  std::string m_destScalarName;

  enum DataType m_dataType;

  ScalarRangeFilter();
  virtual ~ScalarRangeFilter() override;

  int FillInputPortInformation(int port, vtkInformation * info) override;
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

private:
  ScalarRangeFilter(const ScalarRangeFilter &) = delete;
  void operator=(const ScalarRangeFilter &) = delete;
};

#endif  // ScalarRangeFilter_H
            
