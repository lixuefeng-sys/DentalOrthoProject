#ifndef PointDataFilter_H
#define PointDataFilter_H

#include <vtkPolyDataAlgorithm.h>
#include <vtkSetGet.h>

#include "vtkCommon.h"

class PointDataFilter : public vtkPolyDataAlgorithm
{
public:
  static PointDataFilter * New();

  vtkTypeMacro(PointDataFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream & os, vtkIndent indent) override;

protected:
  PointDataFilter();
  virtual ~PointDataFilter() override;

  int FillInputPortInformation(int port, vtkInformation * info) override;
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

private:
  PointDataFilter(const PointDataFilter &) = delete;
  void operator=(const PointDataFilter &) = delete;
};

#endif  // PointDataFilter_H
            
