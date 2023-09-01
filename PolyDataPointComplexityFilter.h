#ifndef PolyDataPointComplexityFilter_H
#define PolyDataPointComplexityFilter_H

#include <vtkPolyDataAlgorithm.h>
#include <vtkSetGet.h>

#include <map>
#include <vector>

#include "vtkCommon.h"

class vtkPolyData;
class vtkIdList;

/**
 * @brief The PolyDataPointComplexityFilter class has two input ports
 *  the first input port has the points to process
 *  the second input port has the connection map
 *
 *  the output is the same topology as the first input port, but with point field data array named "PointComplexity"
 */
class PolyDataPointComplexityFilter : public vtkPolyDataAlgorithm
{
public:
  static PolyDataPointComplexityFilter * New();

  typedef std::map<vtkIdType, vSP<vtkIdList>> NeighbourMap;

  vtkTypeMacro(PolyDataPointComplexityFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream & os, vtkIndent indent) override;

protected:
  PolyDataPointComplexityFilter();
  virtual ~PolyDataPointComplexityFilter() override;

  int FillInputPortInformation(int port, vtkInformation * info) override;
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  vSP<vtkIdList> getFeaturePointIds(vtkPolyData * input0);
  vSP<vtkIdList> getSortedNeighbourPointIds(vtkPolyData * polydata, vtkIdType centerPointId);
  int calculateComplexity(vSP<vtkIdList> sortedPointIds, vtkIdList * featurePointIdList);

  NeighbourMap m_neighbourMap;

private:
  PolyDataPointComplexityFilter(const PolyDataPointComplexityFilter &) = delete;
  void operator=(const PolyDataPointComplexityFilter &) = delete;
};

#endif  // PolyDataPointComplexityFilter_H
            
