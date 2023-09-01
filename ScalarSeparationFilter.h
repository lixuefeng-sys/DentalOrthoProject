#ifndef ScalarSeparationFilter_H
#define ScalarSeparationFilter_H

#include <vtkPolyDataAlgorithm.h>
#include <vtkSetGet.h>

#include <vector>

#include "vtkCommon.h"

class ScalarSeparationFilter : public vtkPolyDataAlgorithm
{
public:
  static ScalarSeparationFilter * New();

  typedef std::vector<vSP<vtkPolyData>> vtkPolyDataList;

  vtkTypeMacro(ScalarSeparationFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream & os, vtkIndent indent) override;

protected:
  typedef struct SeparationFilterEdge
  {
    vtkIdType cellId;
    vtkIdType edgePt1;
    vtkIdType edgePt2;
  } Edge;

  ScalarSeparationFilter();
  virtual ~ScalarSeparationFilter() override;

  int FillInputPortInformation(int port, vtkInformation * info) override;
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  vtkPolyDataList GetSplitedPdsByConnected(vSP<vtkPolyData> pd,
                                           std::vector< vSP<vtkIdList> > cellLists);

  vSP<vtkIdList> GetConnectedCellIds(vtkPolyData *pd,
                                          vtkIdType seedCellId);

private:
  ScalarSeparationFilter(const ScalarSeparationFilter &) = delete;
  void operator=(const ScalarSeparationFilter &) = delete;
};

#endif  // ScalarSeparationFilter_H
