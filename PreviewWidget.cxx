#include "PreviewWidget.h"


#include <vtkDataSet.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkNamedColors.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkCamera.h>
#include <vtkDataSetMapper.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyDataCollection.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkColorTransferFunction.h>
#include <vtkPointData.h>
#include <vtkColorSeries.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPolyDataMapper.h>

#include <QObject>
#include <QAction>
#include <QMessageBox>


PreviewWidget::PreviewWidget(QWidget* parent)
  : QVTKOpenGLNativeWidget(parent)
{
  vVSP(vtkNamedColors, this->m_colors);

  vVSP(vtkGenericOpenGLRenderWindow, this->m_renderWindow);
  this->setRenderWindow(this->m_renderWindow);

  // VTK Renderer
  vVSP(vtkRenderer, this->m_renderer);
  this->m_renderer->SetBackground(this->m_colors->GetColor3d("SteelBlue").GetData());
  this->m_renderer->UseDepthPeelingOff();
  this->m_renderer->SetMaximumNumberOfPeels(1);
  this->m_renderer->SetOcclusionRatio(200);

  // VTK/Qt wedded
  this->renderWindow()->AddRenderer(this->m_renderer);
  this->renderWindow()->SetWindowName("Dental Ortho");
  this->m_renderWindow->AlphaBitPlanesOn();
  this->m_renderWindow->SetMultiSamples(0);

  // interactor
  this->m_interactor = this->m_renderWindow->GetInteractor();
  vVSP(vtkInteractorStyleTrackballCamera, this->m_interactorStyle);
  this->m_interactorStyle->SetDefaultRenderer(this->m_renderer);
  this->m_interactor->SetInteractorStyle(this->m_interactorStyle);

  this->m_renderWindow->Render();
}

void
PreviewWidget::loadMandible(const QString& filename)
{
  this->m_mandibleFilename = filename.toStdString();
  vDNSP(vtkSTLReader, reader);
  reader->SetFileName(this->m_mandibleFilename.c_str());
  reader->Update();

  if(this->m_mandibleActor == nullptr)  // model is not loaded before
  {
    vVSP(vtkPolyDataMapper, this->m_mandibleMapper);
    vVSP(vtkActor, this->m_mandibleActor);
    this->m_mandibleActor->SetMapper(this->m_mandibleMapper);
    this->m_renderer->AddActor(this->m_mandibleActor);
  }
  this->m_mandible = reader->GetOutput();
  this->m_mandibleMapper->SetInputData(this->m_mandible);
  this->m_mandibleActor->VisibilityOn();

  this->m_renderer->ResetCamera();
  this->m_renderWindow->Render();
}

void
PreviewWidget::loadMaxilla(const QString& filename)
{
  this->m_maxillaFilename = filename.toStdString();
  vDNSP(vtkSTLReader, reader);
  reader->SetFileName(this->m_maxillaFilename.c_str());
  reader->Update();

  if(this->m_maxillaActor == nullptr) // model is not loaded before
  {
    vVSP(vtkPolyDataMapper, this->m_maxillaMapper);
    vVSP(vtkActor, this->m_maxillaActor);
    this->m_maxillaActor->SetMapper(this->m_maxillaMapper);
    this->m_renderer->AddActor(this->m_maxillaActor);
  }
  this->m_maxilla = reader->GetOutput();
  this->m_maxillaMapper->SetInputData(this->m_maxilla);
  this->m_maxillaActor->VisibilityOn();

  this->m_renderer->ResetCamera();
  this->m_renderWindow->Render();
}

vtkSmartPointer<vtkPolyData>
PreviewWidget::loadModel(const QString &filename)
{
  vtkNew<vtkSTLReader> reader;
  reader->SetFileName(filename.toStdString().c_str());
  reader->Update();

  return reader->GetOutput();
}

void PreviewWidget::hideMandible()
{
  if(this->m_mandibleActor != nullptr)
  {
    this->m_mandibleActor->VisibilityOff();
    this->m_renderWindow->Render();
  }
}

void PreviewWidget::hideMaxilla()
{
  if(this->m_maxillaActor != nullptr)
  {
    this->m_maxillaActor->VisibilityOff();
    this->m_renderWindow->Render();
  }
}
