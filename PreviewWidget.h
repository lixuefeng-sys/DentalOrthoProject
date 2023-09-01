#ifndef PREVIEW_WIDGET_H
#define PREVIEW_WIDGET_H

#include "vtkCommon.h"

#include <QVTKOpenGLNativeWidget.h>

class vtkActor;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkGenericOpenGLRenderWindow;
class vtkInteractorStyleTrackballCamera;
class vtkNamedColors;

class PreviewWidget : public QVTKOpenGLNativeWidget
{
  Q_OBJECT
public:
  explicit PreviewWidget(QWidget* parent = nullptr);

  void loadMandible(const QString& filename);
  void loadMaxilla(const QString& filename);

  void hideMandible();
  void hideMaxilla();

private:
  vtkSmartPointer<vtkPolyData> loadModel(const QString& filename);

  /////////////////////// render pipeline //////////////////////
  //////other pipeline
  vDSP(vtkGenericOpenGLRenderWindow, m_renderWindow);
  vDSP(vtkRenderer, m_renderer);

  /////interactor
  vDSP(vtkRenderWindowInteractor, m_interactor);
  vDSP(vtkInteractorStyleTrackballCamera, m_interactorStyle);

  //////mandible
  std::string m_mandibleFilename;

  vDSP(vtkPolyData, m_mandible);
  vDSP(vtkPolyDataMapper, m_mandibleMapper);
  vDSP(vtkActor, m_mandibleActor);

  //////maxilla
  std::string m_maxillaFilename;

  vDSP(vtkPolyData, m_maxilla);
  vDSP(vtkPolyDataMapper, m_maxillaMapper);
  vDSP(vtkActor, m_maxillaActor);

  ///////////////////// colors //////////////////////
  vDSP(vtkNamedColors, m_colors);
};

#endif // PREVIEW_WIDGET_H
