
#ifndef ScalarPointToEdgeExtractor_H
#define ScalarPointToEdgeExtractor_H

#include <vtkPolyDataAlgorithm.h>
#include <vtkSetGet.h>

class ScalarPointToEdgeExtractor : public vtkPolyDataAlgorithm
{
public:
  static ScalarPointToEdgeExtractor * New();

  vtkTypeMacro(ScalarPointToEdgeExtractor, vtkPolyDataAlgorithm);
  void PrintSelf(ostream & os, vtkIndent indent) override;

protected:
  ScalarPointToEdgeExtractor();
  virtual ~ScalarPointToEdgeExtractor() override;

  int FillInputPortInformation(int port, vtkInformation * info) override;
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

private:
  ScalarPointToEdgeExtractor(const ScalarPointToEdgeExtractor &) = delete;
  void operator=(const ScalarPointToEdgeExtractor &) = delete;
};

#endif  // ScalarPointToEdgeExtractor_H
            
