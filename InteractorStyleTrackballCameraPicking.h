
#ifndef InteractorStyleTrackballCameraPicking_H
#define InteractorStyleTrackballCameraPicking_H

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSetGet.h>
#include <vtkRenderWindowInteractor.h>

class vtkPoints;
class SceneWidget;
class ModelEditorWindow;
class vtkUnsignedCharArray;
class vtkPropPicker;
class vtkAreaPicker;
class vtkActor;
class vtkProperty;
class vtkHardwareSelector;

#define PICKING_STYLE_NONE 0
#define PICKING_STYLE_PROP 1
#define PICKING_STYLE_RECT 2

class InteractorStyleTrackballCameraPicking : public vtkInteractorStyleTrackballCamera
{
public:
  static InteractorStyleTrackballCameraPicking * New();

  vtkTypeMacro(InteractorStyleTrackballCameraPicking, vtkInteractorStyleTrackballCamera);

  void PrintSelf(ostream & os, vtkIndent indent) override;

  virtual void OnLeftButtonDown() override;
  virtual void OnMouseMove() override;
  virtual void OnLeftButtonUp() override;

  void OnChar() override;

  void SetPickingStyleToNone()
  {
    this->SetPickingStyle(PICKING_STYLE_NONE);
  }
  void SetPickingStyleToProp();
  void SetPickingStyleToRect();
  int GetPickingStyle() const;
  void SetPickingStyle(int newPickingStyle);

  void CombiningOn();
  void CombiningOff();

  void MarkingOn();
  void MarkingOff();

  void MovingOn();
  void MovingOff();

  void SetSceneWidget(SceneWidget * widget);
  void SetMainWindow(ModelEditorWindow * window);

protected:
  InteractorStyleTrackballCameraPicking();
  ~InteractorStyleTrackballCameraPicking() override;

  virtual void RectPick();
  void RedrawRubberBand();

  int m_pickingStyle;

  int StartPosition[2];
  int EndPosition[2];

  int Moving;

  vtkUnsignedCharArray* PixelArray;

  vtkActor * m_lastPickedActor;
  vtkProperty * m_lastPickedProperty;

  vtkPropPicker * m_propPicker;
  vtkAreaPicker * m_areaPicker;
  vtkHardwareSelector * m_hardwareSelector;

  bool m_isCombining;
  bool m_isMarking;
  bool m_isMoving;

  SceneWidget * m_sceneWidget;
  ModelEditorWindow * m_mainWindow;
private:
  InteractorStyleTrackballCameraPicking(const InteractorStyleTrackballCameraPicking &) = delete;
  void operator=(const InteractorStyleTrackballCameraPicking &) = delete;
};


#endif // InteractorStyleTrackballCameraPicking_H
            
