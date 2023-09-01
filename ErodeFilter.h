#ifndef ErodeFilter_H
#define ErodeFilter_H

#include <vtkPolyDataAlgorithm.h>
#include <vtkSetGet.h>

#include "PolyDataPointComplexityFilter.h"

class ErodeFilter : public PolyDataPointComplexityFilter
{
public:
  static ErodeFilter * New();

  vtkTypeMacro(ErodeFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream & os, vtkIndent indent) override;

protected:
  ErodeFilter();
  virtual ~ErodeFilter() override;

  int FillInputPortInformation(int port, vtkInformation * info) override;
  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *) override;

  int countFeaturedNeighbours(vSP<vtkIdList> sortedPointIds,
                                 vSP<vtkIdList> featureList);


private:
  ErodeFilter(const ErodeFilter &) = delete;
  void operator=(const ErodeFilter &) = delete;
};

#endif  // ErodeFilter_H
            
