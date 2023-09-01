#include "SceneWidget.h"

#include <map>
#include <vector>

#include <QObject>
#include <QAction>
#include <QMessageBox>
#include <QProgressDialog>
#include <QtDebug>

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSTLReader.h>
#include <vtkCameraOrientationWidget.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkSeedRepresentation.h>
#include <vtkPolygonalSurfacePointPlacer.h>
#include <vtkSeedRepresentation.h>
#include <vtkButtonWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkHandleWidget.h>
#include <vtkPoints.h>
#include <vtkTriangle.h>
#include <vtkCellData.h>
#include <vtkPlaneSource.h>
#include <vtkCurvatures.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkExtractEdges.h>
#include <vtkLine.h>
#include <vtkDijkstraGraphGeodesicPath.h>
#include <vtkCallbackCommand.h>
#include <vtkAppendDataSets.h>
#include <vtkPointData.h>
#include <vtkCellArrayIterator.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkColorSeries.h>
#include <vtkProgressBarWidget.h>
#include <vtkProgressBarRepresentation.h>
#include <vtkProp3DCollection.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkOBBTree.h>
#include <vtkDoubleArray.h>
#include <vtkAxesActor.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkArrowSource.h>

#include "ModelEditorWindow.h"
#include "QtUtil.h"
#include "ui_ModelEditorWindow.h"
//#include "FloatForm.h"
//#include "ui_FloatForm.h"

#include "vtkCommon.h"
#include "RefPointsWidget.h"
#include "PolySurfacePointHandleRepresentation.h"
#include "TextButtonRepresentation2D.h"
#include "Util.h"
#include "ConstrainedPlaneWidget.h"
#include "ScalarRangeFilter.h"
#include "ScalarPointToEdgeExtractor.h"
#include "PolyDataPointPointPlacer.h"
#include "NonDestructiveConnectivityFilter.h"
#include "PolyDataPointComplexityFilter.h"
#include "ErodeFilter.h"
#include "PointDataFilter.h"
#include "ScalarSeparationFilter.h"
#include "InteractorStyleTrackballCameraPicking.h"
#include "vtkUtil.h"

vSP<vtkNamedColors> SceneWidget::s_colors = vNSP(vtkNamedColors);

SceneWidget::SceneWidget(QWidget * parent)
  : QVTKOpenGLNativeWidget(parent)
{
  this->initColorSeries();

  QWidget * widget = parent->parentWidget();
  ModelEditorWindow * window = qobject_cast<ModelEditorWindow *>(widget);
  if(window != nullptr)
  {
    this->setParentWindow(window);
  }

  vVSP(vtkGenericOpenGLRenderWindow, this->m_renderWindow);
  this->setRenderWindow(this->m_renderWindow);

  // VTK Renderer
  vVSP(vtkRenderer, this->m_renderer);
  this->m_renderer->SetBackground(s_colors->GetColor3d("SteelBlue").GetData());
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
  vVSP(InteractorStyleTrackballCameraPicking, this->m_interactorStyle);
  this->m_interactorStyle->SetDefaultRenderer(this->m_renderer);
  this->m_interactor->SetInteractorStyle(this->m_interactorStyle);
  this->m_interactorStyle->SetSceneWidget(this);
  this->m_interactorStyle->SetMainWindow(this->m_parentWindow);

  // COW
  vVSP(vtkCameraOrientationWidget, this->m_camOrientManipulator);
  this->m_camOrientManipulator->SetParentRenderer(this->m_renderer);
  this->m_camOrientManipulator->On();

  this->m_markingLastActor = nullptr;
  this->m_movingLastActor = nullptr;

  this->m_renderWindow->Render();
}

void SceneWidget::setParentWindow(ModelEditorWindow * window)
{
  this->m_parentWindow = window;

  initializeSplittingButtons();
}

void SceneWidget::initColorSeries()
{
  vVSP(vtkColorSeries, this->m_greenColors);
  m_greenColors->SetColorSchemeByName("GreenColors");

  m_greenColors->AddColor(s_colors->GetColor3ub("chartreuse"));
  m_greenColors->AddColor(s_colors->GetColor3ub("chrome_oxide_green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("cinnabar_green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("cobalt_green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("emerald_green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("forest_green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("green_dark"));
  m_greenColors->AddColor(s_colors->GetColor3ub("green_pale"));
  m_greenColors->AddColor(s_colors->GetColor3ub("green_yellow"));
  m_greenColors->AddColor(s_colors->GetColor3ub("lawn_green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("lime_green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("mint"));
  m_greenColors->AddColor(s_colors->GetColor3ub("olive"));
  m_greenColors->AddColor(s_colors->GetColor3ub("olive_drab"));
  m_greenColors->AddColor(s_colors->GetColor3ub("olive_green_dark"));
  m_greenColors->AddColor(s_colors->GetColor3ub("permanent_green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("sap_green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("sea_green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("sea_green_dark"));
  m_greenColors->AddColor(s_colors->GetColor3ub("sea_green_medium"));
  m_greenColors->AddColor(s_colors->GetColor3ub("sea_green_light"));
  m_greenColors->AddColor(s_colors->GetColor3ub("spring_green"));
  m_greenColors->AddColor(s_colors->GetColor3ub("spring_green_medium"));
  m_greenColors->AddColor(s_colors->GetColor3ub("terre_verte"));
  m_greenColors->AddColor(s_colors->GetColor3ub("viridian_light"));
  m_greenColors->AddColor(s_colors->GetColor3ub("yellow_green"));

  vVSP(vtkColorSeries, this->m_redColors);
  this->m_redColors->SetColorSchemeByName("RedColors");

  this->m_redColors->AddColor(s_colors->GetColor3ub("alizarin_crimson"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("brick"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("cadmium_red_deep"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("coral"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("coral_light"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("deep_pink"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("english_red"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("firebrick"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("geranium_lake"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("hot_pink"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("indian_red"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("light_salmon"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("madder_lake_deep"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("maroon"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("pink"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("pink_light"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("raspberry"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("red"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("rose_madder"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("salmon"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("tomato"));
  this->m_redColors->AddColor(s_colors->GetColor3ub("venetian_red"));

  vVSP(vtkColorSeries, this->m_blueColors);
  this->m_blueColors->SetColorSchemeByName("VTKBlueColors");

  this->m_blueColors->AddColor(s_colors->GetColor3ub("alice_blue"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("blue"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("blue_light"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("blue_medium"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("cadet"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("cobalt"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("cornflower"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("cerulean"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("dodger_blue"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("indigo"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("manganese_blue"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("midnight_blue"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("navy"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("peacock"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("powder_blue"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("royal_blue"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("slate_blue"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("slate_blue_dark"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("slate_blue_light"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("slate_blue_medium"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("sky_blue"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("sky_blue_deep"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("sky_blue_light"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("steel_blue"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("steel_blue_light"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("turquoise_blue"));
  this->m_blueColors->AddColor(s_colors->GetColor3ub("ultramarine"));
}

void SceneWidget::initializeSplittingButtons()
{
  // splitting buttons list
  this->m_parentWindow->ui->filterPlaneShowBtn->setProperty("buttonIndex", this->m_splittingButtons.count());
  this->m_splittingButtons.append(this->m_parentWindow->ui->filterPlaneShowBtn);
  this->m_splittingShowOnFunctions.append([this]() {this->showFilterPlaneOn();} );
  this->m_splittingShowOffFunctions.append([this](bool hideAll) {this->showFilterPlaneOff(hideAll);} );

  this->m_parentWindow->ui->filterCurvatureShowBtn->setProperty("buttonIndex", this->m_splittingButtons.count());
  this->m_splittingButtons.append(this->m_parentWindow->ui->filterCurvatureShowBtn);
  this->m_splittingShowOnFunctions.append([this]() {this->showFilterCurvatureOn();} );
  this->m_splittingShowOffFunctions.append([this](bool hideAll) {this->showFilterCurvatureOff(hideAll);} );

  this->m_parentWindow->ui->addConnectionShowBtn->setProperty("buttonIndex", this->m_splittingButtons.count());
  this->m_splittingButtons.append(this->m_parentWindow->ui->addConnectionShowBtn);
  this->m_splittingShowOnFunctions.append([this]() {this->showAddConnectionOn();} );
  this->m_splittingShowOffFunctions.append([this](bool hideAll) {this->showAddConnectionOff(hideAll);} );

  this->m_parentWindow->ui->removeConnectionShowBtn->setProperty("buttonIndex", this->m_splittingButtons.count());
  this->m_splittingButtons.append(this->m_parentWindow->ui->removeConnectionShowBtn);
  this->m_splittingShowOnFunctions.append([this]() {this->showRemoveConnectionOn();} );
  this->m_splittingShowOffFunctions.append([this](bool hideAll) {this->showRemoveConnectionOff(hideAll);} );

  this->m_parentWindow->ui->separationShowBtn->setProperty("buttonIndex", this->m_splittingButtons.count());
  this->m_splittingButtons.append(this->m_parentWindow->ui->separationShowBtn);
  this->m_splittingShowOnFunctions.append([this]() {this->showSeparationOn();} );
  this->m_splittingShowOffFunctions.append([this](bool hideAll) {this->showSeparationOff(hideAll);} );

  this->m_parentWindow->ui->combiningShowBtn->setProperty("buttonIndex", this->m_splittingButtons.count());
  this->m_splittingButtons.append(this->m_parentWindow->ui->combiningShowBtn);
  this->m_splittingShowOnFunctions.append([this]() {this->showCombiningOn();} );
  this->m_splittingShowOffFunctions.append([this](bool hideAll) {this->showCombiningOff(hideAll);} );

  this->m_parentWindow->ui->markingShowBtn->setProperty("buttonIndex", this->m_splittingButtons.count());
  this->m_splittingButtons.append(this->m_parentWindow->ui->markingShowBtn);
  this->m_splittingShowOnFunctions.append([this]() {this->showMarkingOn();} );
  this->m_splittingShowOffFunctions.append([this](bool hideAll) {this->showMarkingOff(hideAll);} );

  this->m_parentWindow->ui->splittingDoneBtn->setProperty("buttonIndex", this->m_splittingButtons.count());
  this->m_splittingButtons.append(this->m_parentWindow->ui->splittingDoneBtn);
  this->m_splittingShowOnFunctions.append([this]() {this->showMarkingOn();} );
}

vtkDataArray * SceneWidget::getCoordinateVectors(vtkActor * actor)
{
  // get data set from actor
  vtkDataSet * dataSet = actor->GetMapper()->GetInput();
  // get field data from data set
  vtkFieldData * fieldData = dataSet->GetFieldData();

  // get array from field data by array name
  vtkDataArray * array = fieldData->GetArray("coordVectors");

  if(array == nullptr)
  {
    double center[3];
    dataSet->GetCenter(center);

    double max[3];
    double min[3];
    double mid[3];

    //z
    max[0] = -center[0];
    max[1] = -center[1];
    max[2] = -center[2];

    std::cout << "max: " << "(" << max[0] << ", " <<
                 max[1] << ", " << max[2] << ")" << std::endl;

    //y
    min[0] = 0;
    min[1] = 0;
    min[2] = 1;

    //x
    mid[0] = 1;
    mid[1] = -center[0]/center[1];
    mid[2] = 0;

    // create a new double array
    vDNSP(vtkDoubleArray, coordVectors);
    coordVectors->SetName("coordVectors");
    coordVectors->SetNumberOfComponents(3); // 3-tuple
    coordVectors->SetNumberOfTuples(3);
    coordVectors->SetTuple(0, max);
    coordVectors->SetTuple(1, min);
    coordVectors->SetTuple(2, mid);

    // add the array to field data
    fieldData->AddArray(coordVectors);
  }

  vtkDataArray * coordVectors = fieldData->GetArray("coordVectors");
  double max[3];
  coordVectors->GetTuple(0, max);

  return coordVectors;
}

void SceneWidget::movingAddPicking(vtkActor * actor)
{
  if(actor != this->m_movingLastActor) // not selected the same actor
  {
    // revert last selected actor
    if(this->m_movingLastActor != nullptr)
    {
      this->m_movingLastActor->SetProperty(this->m_movingLastProperty);
    }

    // show the axes
    if(this->m_toothAxes == nullptr)
    {
      vVSP(vtkArrowSource, this->m_toothAxes);
      this->m_toothAxes->Update();
      this->m_toothAxesPolyData = this->m_toothAxes->GetOutput();

      vVSP(vtkTransformPolyDataFilter, m_toothXTransform);
      vVSP(vtkTransformPolyDataFilter, m_toothYTransform);
      vVSP(vtkTransformPolyDataFilter, m_toothZTransform);

      vVSP(vtkPolyDataMapper, m_toothXAxesMapper);
      vVSP(vtkPolyDataMapper, m_toothYAxesMapper);
      vVSP(vtkPolyDataMapper, m_toothZAxesMapper);

      vVSP(vtkActor, m_toothXAxesActor);
      vVSP(vtkActor, m_toothYAxesActor);
      vVSP(vtkActor, m_toothZAxesActor);

      this->m_toothXAxesActor->GetProperty()->SetColor(this->s_colors->GetColor3d("red").GetData());
      this->m_toothYAxesActor->GetProperty()->SetColor(this->s_colors->GetColor3d("green").GetData());
      this->m_toothZAxesActor->GetProperty()->SetColor(this->s_colors->GetColor3d("blue").GetData());

      this->m_toothXAxesActor->SetMapper(this->m_toothXAxesMapper);
      this->m_toothYAxesActor->SetMapper(this->m_toothYAxesMapper);
      this->m_toothZAxesActor->SetMapper(this->m_toothZAxesMapper);

      this->m_renderer->AddActor(this->m_toothXAxesActor);
      this->m_renderer->AddActor(this->m_toothYAxesActor);
      this->m_renderer->AddActor(this->m_toothZAxesActor);
    }

    // highlight the new selector actor
    if(actor != nullptr)
    {
      vtkDataSet * dataSet = actor->GetMapper()->GetInput();
      vtkFieldData * fieldData = dataSet->GetFieldData();
      // vtkStringArray is not a data array
      vtkAbstractArray * array = fieldData->GetAbstractArray("marking");
      if(array == nullptr)
      {
        return;
      }
      else
      {
        vtkStringArray * nameArray = vtkStringArray::SafeDownCast(array);
        vtkStdString value = nameArray->GetValue(0);

        // picked gums
        if(value.find("gums") != std::string::npos)
        {
          return;
        }

        this->m_parentWindow->ui->comboBox->setCurrentText(value.c_str());
      }

      vVSP(vtkProperty, this->m_movingLastProperty);
      this->m_movingLastProperty->DeepCopy(actor->GetProperty());

      vtkColor3ub color = m_redColors->GetColorRepeating(this->m_currentHighlightColor++);
      double nColor[3];
      vtkUtil::convertColor(color, nColor);

      actor->GetProperty()->SetColor(nColor);

      int * clickPos = this->m_interactor->GetEventPosition();

      this->m_movingLastActor = actor;

      vtkDataArray * coordinateVectors = getCoordinateVectors(actor);

      double max[3];
      double min[3];
      double mid[3];

      coordinateVectors->GetTuple(0, max);
      coordinateVectors->GetTuple(1, min);
      coordinateVectors->GetTuple(2, mid);

      double * bounds = actor->GetBounds();

      double xLen = bounds[1] - bounds[0];

      double Origin[3] = {1.0,0.0,0.0};
      double Orthogonal[3];
      double thetaRadians;
      double thetaDegree;

      // X
      vDNSP(vtkTransform, transX);
      transX->PostMultiply();
      transX->Scale(3 * xLen, 5, 5);


      Orthogonal[0]= 0;
      Orthogonal[1]= -mid[2]/mid[1];
      Orthogonal[2]= 1;

      thetaRadians = vtkMath::AngleBetweenVectors(Origin,mid);
      thetaDegree = vtkMath::DegreesFromRadians(thetaRadians);

      if(max[0] > 0)
          thetaDegree = 360 - thetaDegree;

      transX->RotateWXYZ(thetaDegree,Orthogonal[0],Orthogonal[1],Orthogonal[2]);

      transX->Translate(-max[0], -max[1], -max[2]);


      this->m_toothXTransform->SetTransform(transX);
      this->m_toothXTransform->SetInputData(this->m_toothAxes->GetOutput());
      this->m_toothXTransform->Update();

      this->m_toothXAxesMapper->SetInputData(this->m_toothXTransform->GetOutput());
      this->m_toothXAxesActor->VisibilityOn();
      // Y
      vDNSP(vtkTransform, transY);
      transY->PostMultiply();
      transY->Scale(2 * xLen, 5, 5);

      if(this->m_modelState.mandibleVisible)
      {
          transY->RotateY(270);
      }
      else
      {
          transY->RotateY(90);
      }


      transY->Translate(-max[0], -max[1], -max[2]);

      this->m_toothYTransform->SetTransform(transY);
      this->m_toothYTransform->SetInputData(this->m_toothAxes->GetOutput());
      this->m_toothYTransform->Update();

      this->m_toothYAxesMapper->SetInputData(this->m_toothYTransform->GetOutput());
      this->m_toothYAxesActor->VisibilityOn();
      // Z
      vDNSP(vtkTransform, transZ);
      transZ->PostMultiply();
      transZ->Scale(2 * xLen, 5, 5);

      Orthogonal[0]= 0;
      Orthogonal[1]= -max[2]/max[1];
      Orthogonal[2]= 1;

      thetaRadians = vtkMath::AngleBetweenVectors(Origin,max);
      thetaDegree = vtkMath::DegreesFromRadians(thetaRadians);



      transZ->RotateWXYZ(thetaDegree,Orthogonal[0],Orthogonal[1],Orthogonal[2]);


      transZ->Translate(-max[0], -max[1], -max[2]);

      this->m_toothZTransform->SetTransform(transZ);
      this->m_toothZTransform->SetInputData(this->m_toothAxes->GetOutput());
      this->m_toothZTransform->Update();

      this->m_toothZAxesMapper->SetInputData(this->m_toothZTransform->GetOutput());
      this->m_toothZAxesActor->VisibilityOn();
    }
    else
    {
      this->m_movingLastActor = nullptr;
      this->m_toothXAxesActor->VisibilityOff();
      this->m_toothYAxesActor->VisibilityOff();
      this->m_toothZAxesActor->VisibilityOff();
    }
  }
}

void SceneWidget::moveLastActor(double * xyz, int direction)
{
   double x[3] = {1.0, 0.0, 0.0};
   double y[3] = {0.0, 1.0, 0.0};
   double z[3] = {0.0, 0.0, 1.0};

   double thetaX = vtkMath::AngleBetweenVectors(x, xyz);
   double thetaY = vtkMath::AngleBetweenVectors(y, xyz);
   double thetaZ = vtkMath::AngleBetweenVectors(z, xyz);

   double tt = 0.2;
   double deltaX = direction * tt * cos(thetaX);
   double deltaY = direction * tt * cos(thetaY);
   double deltaZ = direction * tt * cos(thetaZ);

   vtkLinearTransform * preTrans = this->m_movingLastActor->GetUserTransform();
   vDNSP(vtkTransform, trans);
   trans->PostMultiply();
   if(preTrans != nullptr)
   {
     trans->Concatenate(preTrans);
   }
   trans->Translate(deltaX, deltaY, deltaZ);

   this->m_movingLastActor->SetUserTransform(trans);

   renderWindow()->Render();
}

void SceneWidget::rotateLastActor(double * origin, double * xyz, int direction)
{
   vtkLinearTransform * preTrans = this->m_movingLastActor->GetUserTransform();
   vDNSP(vtkTransform, trans);
   trans->PostMultiply();
   if(preTrans != nullptr)
   {
     trans->Concatenate(preTrans);
   }
   trans->Translate(-origin[0], -origin[1], -origin[2]);
   trans->RotateWXYZ(direction * 3, xyz[0], xyz[1], xyz[2]);
   trans->Translate(origin[0], origin[1], origin[2]);

   this->m_movingLastActor->SetUserTransform(trans);

   renderWindow()->Render();
}

void SceneWidget::movingClickSelectionInternal(bool checked)
{
  if(!checked)
  {
    this->movingClickSelectionOff();
  }
  else
  {
    this->movingClickSelectionOn();
  }
}

void SceneWidget::movingClickSelectionOn()
{
  this->m_interactorStyle->SetPickingStyleToProp();
  this->m_parentWindow->ui->movingClickSelectionBtn->setChecked(true);

  this->m_renderWindow->Render();
}

void SceneWidget::movingClickSelectionOff()
{
  this->m_interactorStyle->SetPickingStyleToNone();
  this->m_parentWindow->ui->movingClickSelectionBtn->setChecked(false);

  this->m_renderWindow->Render();
}

void SceneWidget::setMandibleVisibility(bool visible)
{
  this->m_parentWindow->ui->actionHideMandible->setChecked(!visible);
  this->m_modelState.mandibleVisible = visible;

  if(this->m_modelState.mandibleSplitted == false)  // original model
  {
    this->m_mandibleActor->SetVisibility(visible);
  }
  else  // already splitted
  {
    std::map<vSP<vtkActor>, std::string>::iterator iter =
        this->m_actorMarkingMapMandible.begin();
    for(; iter != this->m_actorMarkingMapMandible.end(); iter++)
    {
      iter->first->SetVisibility(visible);
    }
  }
}

bool SceneWidget::getMandibleVisibility()
{
  return this->m_modelState.mandibleVisible;
}

void SceneWidget::setMaxillaVisibility(bool visible)
{
  this->m_parentWindow->ui->actionHideMaxilla->setChecked(!visible);
  this->m_modelState.maxillaVisible = visible;

  if(this->m_modelState.maxillaSplitted == false)  // original model
  {
    this->m_maxillaActor->SetVisibility(visible);
  }
  else  // already splitted
  {
    std::map<vSP<vtkActor>, std::string>::iterator iter =
        this->m_actorMarkingMapMaxilla.begin();
    for(; iter != this->m_actorMarkingMapMaxilla.end(); iter++)
    {
      iter->first->SetVisibility(visible);
    }
  }
}

bool SceneWidget::getMaxillaVisibility()
{
  return this->m_modelState.maxillaVisible;
}

void SceneWidget::createProgressDialog()
{
  // qt progress dialog
  QVSP1(QProgressDialog, this->m_progressDialog, this->m_parentWindow);
  this->m_progressDialog->setWindowTitle("处理进度");
  this->m_progressDialog->setCancelButton(nullptr);
  this->m_progressDialog->setWindowModality(Qt::WindowModality::WindowModal);
  this->m_progressDialog->setFixedWidth(300);
  this->m_progressDialog->setRange(0, 100);

  // progress command
  vVSP(vtkCallbackCommand, this->m_progressCommand);
  this->m_progressCommand->SetCallback(updateProgress);
  this->m_progressCommand->SetClientData(this);
}

void SceneWidget::showRefPlaneInternal(bool checked)
{
  if(!checked) // not checked
  {
    if(this->m_refPlaneMandibleActor != nullptr)
    {
      this->m_refPlaneMandibleActor->VisibilityOff();
    }
    this->m_refPointWidget->ProcessEventsOn();
    this->m_renderer->UseDepthPeelingOff();
  }
  else // checked
  {
    if(this->m_refPointWidget->GetNumberOfSeeds() == 3)
    {
      vDNSP(vtkPoints, points);
      double pointsPos[3][3];
      for(int i = 0; i < 3; i++)
      {
        vtkHandleWidget * seed = this->m_refPointWidget->GetSeed(i);
        vtkHandleRepresentation * handleRep = seed->GetHandleRepresentation();
        double pos[3];
        handleRep->GetWorldPosition(pos);
        points->InsertNextPoint(pos);

        memcpy(pointsPos[i], pos, sizeof(double) * 3);
      }

      double center[3];
      vtkTriangle::ComputeCentroid(points, nullptr, center);

      double normal[3];
      if(!vtkUtil::checkPointsCounterClockwise(points)) // make sure normal is counter clockwise
      {
        vtkTriangle::ComputeNormal(pointsPos[2], pointsPos[1], pointsPos[0], normal);
      }
      else
      {
        vtkTriangle::ComputeNormal(pointsPos[0], pointsPos[1], pointsPos[2], normal);
      }

      double bds[6];
      this->m_mandibleActor->GetBounds(bds);

      if(this->m_refPlaneMandible == nullptr)
      {
        vVSP(vtkPlaneSource, this->m_refPlaneMandible);
        vVSP(vtkPolyDataMapper, this->m_refPlaneMandibleMapper);
        vVSP(vtkActor, this->m_refPlaneMandibleActor);
        this->m_refPlaneMandibleMapper->AddInputConnection(this->m_refPlaneMandible->GetOutputPort());
        this->m_renderer->AddActor(this->m_refPlaneMandibleActor);
      }
      this->m_refPlaneMandible->SetOrigin(bds[0] - 10, bds[2] - 10, bds[5]);
      this->m_refPlaneMandible->SetPoint1(bds[1] + 10, bds[2] - 10, bds[5]);
      this->m_refPlaneMandible->SetPoint2(bds[0] - 10, bds[3] + 10, bds[5]);
      this->m_refPlaneMandible->SetNormal(normal);
      this->m_refPlaneMandible->SetCenter(center);

      this->m_refPlaneMandibleActor->SetMapper(this->m_refPlaneMandibleMapper);
      this->m_refPlaneMandibleActor->GetProperty()->SetOpacity(0.8);
      double * color = this->s_colors->GetColor4d("Green").GetData();
      this->m_refPlaneMandibleActor->GetProperty()->SetColor(color);
      this->m_refPlaneMandibleActor->VisibilityOn();

      this->m_refPointWidget->ProcessEventsOff();

      // this will greatly affect the performance
      this->m_renderer->UseDepthPeelingOn();

      this->m_renderWindow->Render();
    }
    else
    {
      QMessageBox::critical(this->m_parentWindow,
          "错误", "需要指定三个参考点才能定位咬合平面！");
      this->m_parentWindow->ui->addRefPlaneBtn->setChecked(false);
    }
  }
}

void SceneWidget::showSplittingSubbuttonsInternal(bool checked)
{
  QObject * object = QObject::sender();
  QPushButton * button = qobject_cast<QPushButton *>(object);

  if(!checked)
  {
    // do not allow user to turn off manually
    button->setChecked(true);
  }
  else
  {
    ActionDirection direction = this->turnOffSplittingButtons(button);
    if(direction == AD_Forward || direction == AD_Backward)
    {
      this->turnOnSplittingButtons(button);
    }
    else if(direction == AD_Skip)
    {
      QMessageBox::critical(this->m_parentWindow,
                              "错误", "不能跳过步骤！");
    }

    this->m_renderWindow->Render();
  }
}

void SceneWidget::showFilterPlaneOn()
{
  std::cout << "plane on" << std::endl;
  if(this->m_modelState.mandibleVisible)
  {
    showFilterPlaneDo(this->m_mandibleActor, this->m_filterPlaneMandibleWidget);
  }
  else if(this->m_modelState.maxillaVisible)
  {
    std::cout << "maxilla" << std::endl;
    showFilterPlaneDo(this->m_maxillaActor, this->m_filterPlaneMaxillaWidget);
  }
}

void SceneWidget::showFilterPlaneDo(vtkActor * actorOriginal,
                                    vSP<ConstrainedPlaneWidget> & filterPlaneWidget)
{
  if(filterPlaneWidget == nullptr)
  {
    vVSP(ConstrainedPlaneWidget, filterPlaneWidget);
    filterPlaneWidget->SetInteractor(this->m_interactor);
    double bds[6];
    actorOriginal->GetBounds(bds);
    filterPlaneWidget->SetOrigin(bds[0] - 10, bds[2] - 10, bds[5]);
    filterPlaneWidget->SetPoint1(bds[1] + 10, bds[2] - 10, bds[5]);
    filterPlaneWidget->SetPoint2(bds[0] - 10, bds[3] + 10, bds[5]);
    filterPlaneWidget->SetResolution(1); // one big plane
    filterPlaneWidget->SetRepresentationToSurface();
    filterPlaneWidget->GetPlaneProperty()->SetColor(s_colors->GetColor3d("Red").GetData());
//      filterPlane->GetPlaneProperty()->SetOpacity(0.5);
    filterPlaneWidget->SetHandlesOff(); // no handles to resize plane
    filterPlaneWidget->SetNormalOff(); // no normal line to rotate plane
    filterPlaneWidget->LimitMovement(ConstrainedPlaneWidget::Z_Axis);
//      filterPlane->GetSelectedPlaneProperty()->SetOpacity(0.8);
  }
  filterPlaneWidget->On();
  //    this->m_renderer->UseDepthPeelingOn();
}

void SceneWidget::showFilterPlaneOff(bool hideAll)
{
  this->m_parentWindow->ui->filterPlaneShowBtn->setChecked(false);

  if(this->m_modelState.mandibleVisible == true && this->m_filterPlaneMandibleWidget != nullptr)
  {
    this->m_filterPlaneMandibleWidget->Off();
  }
  else if(this->m_modelState.maxillaVisible == true && this->m_filterPlaneMaxillaWidget != nullptr)
  {
    this->m_filterPlaneMaxillaWidget->Off();
  }
}

void SceneWidget::showFilterCurvatureOn()
{
  if(this->m_modelState.mandibleVisible == true)
  {
    showFilterCurvatureDo(this->m_mandible,
                          this->m_curvatureFilterMandible,
                          this->m_curvatureRangeFilterMandible,
                          this->m_extractEdgesFilterMandible,
                          this->m_scalarPointEdgeExtractorMandible,
                          this->m_curvatureMapperMandible,
                          this->m_curvatureActorMandible,
                          this->m_filterPlaneMandibleWidget,
                          ScalarRangeFilter::FilterZBelow);
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    showFilterCurvatureDo(this->m_maxilla,
                          this->m_curvatureFilterMaxilla,
                          this->m_curvatureRangeFilterMaxilla,
                          this->m_extractEdgesFilterMaxilla,
                          this->m_scalarPointEdgeExtractorMaxilla,
                          this->m_curvatureMapperMaxilla,
                          this->m_curvatureActorMaxilla,
                          this->m_filterPlaneMaxillaWidget,
                          ScalarRangeFilter::FilterZAbove);
  }

  // show sliders
  QtUtil::showLayout(this->m_parentWindow->ui->splittingCurvatureSliders);
}

void SceneWidget::showFilterCurvatureDo(vtkPolyData * polydataOriginal,
                                        vSP<vtkCurvatures> & curvatureFilter,
                                        vSP<ScalarRangeFilter> & curvatureRangeFilter,
                                        vSP<vtkExtractEdges> & extractEdgesFilter,
                                        vSP<ScalarPointToEdgeExtractor> & scalarPointEdgeExtractor,
                                        vSP<vtkPolyDataMapper> & curvatureMapper,
                                        vSP<vtkActor> & curvatureActor,
                                        ConstrainedPlaneWidget * filterPlaneWidget,
                                        ScalarRangeFilter::FilterZType filterZType)
{
  // calculate the curvature
  if(curvatureFilter == nullptr)
  {
    // curvature filter
    vVSP(vtkCurvatures, curvatureFilter);
    curvatureFilter->SetCurvatureTypeToMean();
    curvatureFilter->SetInputData(polydataOriginal);

    // curvature range filter
    vVSP(ScalarRangeFilter, curvatureRangeFilter);
    curvatureRangeFilter->setMinValue(-10);
    curvatureRangeFilter->setMaxValue(-0.08);
    curvatureRangeFilter->setSourceScalarName("Mean_Curvature");
    curvatureRangeFilter->setDestScalarName("FMean_Curvature");
    curvatureRangeFilter->setDataType(ScalarRangeFilter::PointData);
    curvatureRangeFilter->SetInputConnection(curvatureFilter->GetOutputPort());

    vVSP(vtkExtractEdges, extractEdgesFilter);
    extractEdgesFilter->SetInputConnection(curvatureRangeFilter->GetOutputPort());

    vVSP(ScalarPointToEdgeExtractor, scalarPointEdgeExtractor);
    scalarPointEdgeExtractor->SetInputConnection(extractEdgesFilter->GetOutputPort());

    vVSP(vtkPolyDataMapper, curvatureMapper);
    curvatureMapper->SetInputConnection(scalarPointEdgeExtractor->GetOutputPort());
    vVSP(vtkActor, curvatureActor);
    curvatureActor->SetMapper(curvatureMapper);
    curvatureActor->GetProperty()->EdgeVisibilityOn();
    curvatureActor->GetProperty()->SetEdgeColor(s_colors->GetColor3d("Red").GetData());
    curvatureActor->GetProperty()->SetColor(s_colors->GetColor3d("Red").GetData());
    curvatureActor->GetProperty()->SetLineWidth(3);

    this->m_renderer->AddActor(curvatureActor);
  }

  this->m_parentWindow->ui->curvatureRangeSlider->setMinValue(curvatureRangeFilter->getMinValue());
  this->m_parentWindow->ui->curvatureRangeSlider->setMaxValue(curvatureRangeFilter->getMaxValue());

  curvatureRangeFilter->setFilterZType(filterZType);
  curvatureRangeFilter->setFilterZValue(filterPlaneWidget->GetOrigin()[2]);

  curvatureActor->VisibilityOn();
}

void SceneWidget::showFilterCurvatureOff(bool hideAll)
{
  this->m_parentWindow->ui->filterCurvatureShowBtn->setChecked(false);

  QtUtil::hideLayout(this->m_parentWindow->ui->splittingCurvatureSliders);

  if(this->m_modelState.mandibleVisible == true)
  {
    if(hideAll)
    {
      this->m_curvatureActorMandible->VisibilityOff();
      this->m_filterPlaneMandibleWidget->On();
    }
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    if(hideAll)
    {
      this->m_curvatureActorMaxilla->VisibilityOff();
      this->m_filterPlaneMaxillaWidget->On();
    }
  }
}

void SceneWidget::minCurvatureSliderChangeInternal(float value)
{
  if(this->m_modelState.mandibleVisible)
  {
    this->m_curvatureRangeFilterMandible->setMinValue(value);
  }
  else if(this->m_modelState.maxillaVisible)
  {
    this->m_curvatureRangeFilterMaxilla->setMinValue(value);
  }
  this->m_renderWindow->Render();
}

void SceneWidget::maxCurvatureSliderChangeInternal(float value)
{
  if(this->m_modelState.mandibleVisible)
  {
    this->m_curvatureRangeFilterMandible->setMaxValue(value);
  }
  else if(this->m_modelState.maxillaVisible)
  {
    this->m_curvatureRangeFilterMaxilla->setMaxValue(value);
  }
  this->m_renderWindow->Render();
}

void SceneWidget::addConnectionPointPickingToggleInternal(bool checked)
{
  if(checked)
  {
    this->addConnectionPointPickingOn();
  }
  else
  {
    this->addConnectionPointPickingOff();
  }
}

void SceneWidget::addConnectionUndoInternal()
{
  if(this->m_modelState.mandibleVisible == true)
  {
    if(this->m_additionalConnectionStackMandible.size() > 0)
    {
      std::vector<std::vector<vSP<vtkLine>> *>::const_reference back =
          this->m_additionalConnectionStackMandible.back();
      this->m_additionalConnectionStackMandible.pop_back();

      vtkNew<vtkCellArray> newLines;

      std::vector<std::vector<vSP<vtkLine>> *>::iterator stackIter =
          this->m_additionalConnectionStackMandible.begin();

      // add all the existing lines
      for(; stackIter != this->m_additionalConnectionStackMandible.end(); stackIter++)
      {
        std::vector<vSP<vtkLine>>::iterator lineIter = (*stackIter)->begin();
        for(; lineIter != (*stackIter)->end(); lineIter++)
        {
          newLines->InsertNextCell(*lineIter);
        }
      }

      // set all the new lines
      this->m_additionalConnectionMandible->SetLines(newLines);

      delete back;
    }
    else
    {
      QMessageBox::critical(this->m_parentWindow, "错误", "没有更多的额外轮廓可以撤销！");
    }
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    if(this->m_additionalConnectionStackMaxilla.size() > 0)
    {
      std::vector<std::vector<vSP<vtkLine>> *>::const_reference back =
          this->m_additionalConnectionStackMaxilla.back();
      this->m_additionalConnectionStackMaxilla.pop_back();

      vtkNew<vtkCellArray> newLines;

      std::vector<std::vector<vSP<vtkLine>> *>::iterator stackIter =
          this->m_additionalConnectionStackMaxilla.begin();

      // add all the existing lines
      for(; stackIter != this->m_additionalConnectionStackMaxilla.end(); stackIter++)
      {
        std::vector<vSP<vtkLine>>::iterator lineIter = (*stackIter)->begin();
        for(; lineIter != (*stackIter)->end(); lineIter++)
        {
          newLines->InsertNextCell(*lineIter);
        }
      }

      // set all the new lines
      this->m_additionalConnectionMaxilla->SetLines(newLines);

      delete back;
    }
    else
    {
      QMessageBox::critical(this->m_parentWindow, "错误", "没有更多的额外轮廓可以撤销！");
    }
  }
}

void SceneWidget::addConnectionPointPickingOn()
{
  if(this->m_modelState.mandibleVisible == true)
  {
    this->m_addConnectionWidgetMandible->On();
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    this->m_addConnectionWidgetMaxilla->On();
  }
}

void SceneWidget::addConnectionPointPickingOff()
{
  if(this->m_modelState.mandibleVisible == true)
  {
    this->m_addConnectionWidgetMandible->Off();
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    this->m_addConnectionWidgetMaxilla->Off();
  }
}

void SceneWidget::showAddConnectionOn()
{
  if(this->m_modelState.mandibleVisible == true)
  {
    showAddConnectionDo(this->m_mandibleActor,
                        this->m_curvatureActorMandible,
                        this->m_addConnectionWidgetMandible,
                        this->m_addConnectionHandleRepMandible,
                        this->m_addConnectionSeedRepMandible,
                        this->m_addConnectionPointPlacerMandible,
                        this->m_additionalConnectionActorMandible);
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    showAddConnectionDo(this->m_maxillaActor,
                        this->m_curvatureActorMaxilla,
                        this->m_addConnectionWidgetMaxilla,
                        this->m_addConnectionHandleRepMaxilla,
                        this->m_addConnectionSeedRepMaxilla,
                        this->m_addConnectionPointPlacerMaxilla,
                        this->m_additionalConnectionActorMaxilla);
  }
}

void SceneWidget::showAddConnectionDo(vtkActor * actorOriginal,
                                      vtkActor * curvatureActor,
                                      vSP<RefPointsWidget> & addConnectionWidget,
                                      vSP<PolySurfacePointHandleRepresentation> & addConnectionHandleRep,
                                      vSP<vtkSeedRepresentation> & addConnectionSeedRep,
                                      vSP<PolyDataPointPointPlacer> & addConnectionPointPlacer,
                                      vSP<vtkActor> & additionalConnectionActor)
{
  if(addConnectionWidget == nullptr)
  {
    vVSP(RefPointsWidget, addConnectionWidget);
    vVSP(PolySurfacePointHandleRepresentation, addConnectionHandleRep);
    vVSP(vtkSeedRepresentation, addConnectionSeedRep);
    vVSP(PolyDataPointPointPlacer, addConnectionPointPlacer);

    addConnectionHandleRep->GetProperty()->SetColor(s_colors->GetColor3d("Green").GetData());
    addConnectionHandleRep->GetProperty()->SetLineWidth(6);
    addConnectionHandleRep->SetHandleSize(20);
    addConnectionHandleRep->GetSelectedProperty()->SetColor(s_colors->GetColor3d("Blue").GetData());
    addConnectionHandleRep->GetSelectedProperty()->SetLineWidth(6);
    addConnectionHandleRep->SetPointPlacer(addConnectionPointPlacer);

    addConnectionSeedRep->SetHandleRepresentation(addConnectionHandleRep);

//      this->m_addConnectionPointPlacer->AddProp(this->m_curvatureActorMandible);
    addConnectionPointPlacer->AddProp(actorOriginal);

    addConnectionWidget->SetPointLimit(2);
    addConnectionWidget->SetInteractor(this->m_interactor);
    addConnectionWidget->SetRepresentation(addConnectionSeedRep);
  }

  curvatureActor->VisibilityOn();
  if(additionalConnectionActor != nullptr)
  {
    additionalConnectionActor->VisibilityOn();
  }

  // show extra buttons
  this->m_parentWindow->ui->addConnectionEnableBtn->show();
  this->m_parentWindow->ui->addConnectionUndoBtn->show();

  // bind external key press event
    // press 'e' to enable point picking
  if(this->m_addConnectionPressECallback == nullptr)
  {
    vVSP(vtkCallbackCommand, this->m_addConnectionPressECallback);
    this->m_addConnectionPressECallback->SetCallback(addConnectionPressE);
    this->m_addConnectionPressECallback->SetClientData(this);
  }

  this->m_interactor->AddObserver(vtkCommand::KeyPressEvent, this->m_addConnectionPressECallback);

    // press 'enter' to add connection
  if(this->m_addConnectionPressEnterCallback == nullptr)
  {
    vVSP(vtkCallbackCommand, this->m_addConnectionPressEnterCallback);
    this->m_addConnectionPressEnterCallback->SetCallback(addConnectionPressEnter);
    this->m_addConnectionPressEnterCallback->SetClientData(this);
  }

  this->m_interactor->AddObserver(vtkCommand::KeyPressEvent, this->m_addConnectionPressEnterCallback);
}

void SceneWidget::showAddConnectionOff(bool hideAll)
{
  this->m_parentWindow->ui->addConnectionShowBtn->setChecked(false);

  // hide extra buttons
  this->m_parentWindow->ui->addConnectionEnableBtn->setChecked(false);
  this->m_parentWindow->ui->addConnectionEnableBtn->hide();
  this->m_parentWindow->ui->addConnectionUndoBtn->hide();

  if(this->m_modelState.mandibleVisible == true)
  {
    this->m_addConnectionWidgetMandible->Off();

    if(this->m_additionalConnectionActorMandible != nullptr)
    {
      this->m_additionalConnectionActorMandible->VisibilityOff();
    }

    if(hideAll)
    {
      if(this->m_additionalConnectionMandible != nullptr
         && this->m_additionalConnectionMandible->GetNumberOfLines() > 0)
      {
        vtkNew<vtkCellArray> newLines;
        this->m_additionalConnectionMandible->SetLines(newLines);
      }

      this->m_curvatureActorMandible->VisibilityOn();
      QtUtil::showLayout(this->m_parentWindow->ui->splittingCurvatureSliders);
    }
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    this->m_addConnectionWidgetMaxilla->Off();

    if(this->m_additionalConnectionActorMaxilla != nullptr)
    {
      this->m_additionalConnectionActorMaxilla->VisibilityOff();
    }

    if(hideAll)
    {
      if(this->m_additionalConnectionMaxilla != nullptr
         && this->m_additionalConnectionMaxilla->GetNumberOfLines() > 0)
      {
        vtkNew<vtkCellArray> newLines;
        this->m_additionalConnectionMaxilla->SetLines(newLines);
      }

      this->m_curvatureActorMaxilla->VisibilityOn();
      QtUtil::showLayout(this->m_parentWindow->ui->splittingCurvatureSliders);
    }
  }

  // unbind hot keys
  this->m_interactor->RemoveObserver(this->m_addConnectionPressECallback);
  this->m_interactor->RemoveObserver(this->m_addConnectionPressEnterCallback);
}

void SceneWidget::addConnectionPressEInternal()
{
  char * keySym = this->m_interactor->GetKeySym();

  if(strcmp(keySym, "e") == 0 || strcmp(keySym, "E") == 0)
  {
    if(this->m_parentWindow->ui->addConnectionEnableBtn->isChecked()) // enable, set to disable
    {
      this->addConnectionPointPickingOff();
      this->m_parentWindow->ui->addConnectionEnableBtn->setChecked(false);
    }
    else // disable, set to enable
    {
      this->addConnectionPointPickingOn();
      this->m_parentWindow->ui->addConnectionEnableBtn->setChecked(true);
    }
  }

  this->m_renderWindow->Render();
}

void SceneWidget::addConnectionPressEnterInternal()
{
  char * keySym = this->m_interactor->GetKeySym();

  if(strcmp(keySym, "Return") == 0)
  {
    if(this->m_modelState.mandibleVisible == true)
    {
      addConnectionPressEnterDo(this->m_mandible,
                                this->m_addConnectionWidgetMandible,
                               this->m_addConnectionPointPlacerMandible,
                               this->m_additionalConnectionMandible,
                               this->m_additionalConnectionMapperMandible,
                               this->m_additionalConnectionActorMandible,
                               this->m_dijkstraMandible,
                                this->m_additionalConnectionStackMandible);
    }
    else if(this->m_modelState.maxillaVisible == true)
    {
      addConnectionPressEnterDo(this->m_maxilla,
                                this->m_addConnectionWidgetMaxilla,
                               this->m_addConnectionPointPlacerMaxilla,
                               this->m_additionalConnectionMaxilla,
                               this->m_additionalConnectionMapperMaxilla,
                               this->m_additionalConnectionActorMaxilla,
                               this->m_dijkstraMaxilla,
                                this->m_additionalConnectionStackMaxilla);
    }
  }
}

void SceneWidget::addConnectionPressEnterDo(vtkPolyData * polydataOriginal,
                                            RefPointsWidget * addConnectionWidget,
                                            PolyDataPointPointPlacer * addConnectionPointPlacer,
                                            vSP<vtkPolyData> & additionalConnection,
                                            vSP<vtkPolyDataMapper> & additionalConnectionMapper,
                                            vSP<vtkActor> & additionalConnectionActor,
                                            vSP<vtkDijkstraGraphGeodesicPath> & dijkstra,
                                            std::vector<std::vector<vSP<vtkLine>> *> & additionalConnectionStack)
{
  int numberOfSeeds = addConnectionWidget->GetNumberOfSeeds();
  if(numberOfSeeds == 2) {

    vtkIdType ids[2];
    int nodeCount = 0;
    for(int i = 0; i < numberOfSeeds; i++)
    {
      vtkHandleWidget * seed = addConnectionWidget->GetSeed(i);
      vtkHandleRepresentation * handleRep = seed->GetHandleRepresentation();
      double pos[3];
      handleRep->GetWorldPosition(pos);

      PolyDataPointPointPlacer::Node * node =
          addConnectionPointPlacer->GetNodeAtWorldPosition(pos);

      if(node != nullptr)
      {
        nodeCount++;
        ids[i] = node->PointId;
        std::cout << "point id: " << node->PointId << std::endl;
      }
    }

    if(nodeCount == 2)
    {
      if(additionalConnection == nullptr)
      {
//          this->m_additionalConnection = this->m_scalarPointEdgeExtractor->GetOutput();
        vVSP(vtkPolyData, additionalConnection);
        additionalConnection->SetPoints(polydataOriginal->GetPoints());
//          vtkCellArray *lines = this->m_additionalConnection->GetLines();
//          lines->Initialize(); // clear and free memory
//          this->m_additionalConnection->SetLines(lines);

        vVSP(vtkPolyDataMapper, additionalConnectionMapper);
        additionalConnectionMapper->SetInputData(additionalConnection);

        vVSP(vtkActor, additionalConnectionActor);
        additionalConnectionActor->SetMapper(additionalConnectionMapper);
        additionalConnectionActor->GetProperty()->EdgeVisibilityOn();
        additionalConnectionActor->GetProperty()->SetEdgeColor(s_colors->GetColor3d("Purple").GetData());
        additionalConnectionActor->GetProperty()->SetLineWidth(3);
        additionalConnectionActor->GetProperty()->SetColor(this->s_colors->GetColor3d("Purple").GetData());

        this->m_renderer->AddActor(additionalConnectionActor);

        vVSP(vtkDijkstraGraphGeodesicPath, dijkstra);
        dijkstra->SetInputData(polydataOriginal);
      }

      dijkstra->SetStartVertex(ids[0]);
      dijkstra->SetEndVertex(ids[1]);
      dijkstra->Update();

      vtkIdList * idList = dijkstra->GetIdList();
      std::cout << "number of ids: " << idList->GetNumberOfIds() << std::endl;

      vtkNew<vtkCellArray> newLines;

      std::vector<std::vector<vSP<vtkLine>> *>::iterator stackIter =
          additionalConnectionStack.begin();

      // add all the existing lines
      for(; stackIter != additionalConnectionStack.end(); stackIter++)
      {
        std::vector<vSP<vtkLine>>::iterator lineIter = (*stackIter)->begin();
        for(; lineIter != (*stackIter)->end(); lineIter++)
        {
          newLines->InsertNextCell(*lineIter);
        }
      }

      // add new lines
      vtkIdType idCount = idList->GetNumberOfIds();
      if(idCount > 0)
      {
        std::vector<vSP<vtkLine>> * stackLines = new std::vector<vSP<vtkLine>>;
        for(int i = 0; i < idCount - 1; i++)
        {
          // an additional line for each path
          vDNSP(vtkLine, line);
          line->GetPointIds()->SetNumberOfIds(2);
          line->GetPointIds()->SetId(0, idList->GetId(i));
          line->GetPointIds()->SetId(1, idList->GetId(i + 1));

          newLines->InsertNextCell(line);
          stackLines->push_back(line);
        }
        additionalConnectionStack.push_back(stackLines);
      }
      // set all the new lines
      additionalConnection->SetLines(newLines);

      addConnectionWidget->RemoveAllSeeds();

      this->m_renderWindow->Render();
    }
    else
    {
      QMessageBox::critical(this->m_parentWindow, "错误", "必须选中两个点才能连接，点数据错误！");
    }
  }
  else
  {
    QMessageBox::critical(this->m_parentWindow, "错误", "必须选中两个点才能连接！");
  }
}

void SceneWidget::showRemoveConnectionOn()
{
  if(this->m_modelState.mandibleVisible == true)
  {
    showRemoveConnectionDo(this->m_mandible,
                           this->m_mandibleActor,
                           this->m_largestConnFilterMandible,
                           this->m_erodeFilterMandible,
                           this->m_removedConnectionMandible,
                           this->m_removedConnectionMapperMandible,
                           this->m_removedConnectionActorMandible,
                           this->m_scalarPointEdgeExtractorMandible,
                           this->m_curvatureActorMandible,
                           this->m_additionalConnectionMandible,
                           this->m_additionalConnectionActorMandible);
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    showRemoveConnectionDo(this->m_maxilla,
                           this->m_maxillaActor,
                           this->m_largestConnFilterMaxilla,
                           this->m_erodeFilterMaxilla,
                           this->m_removedConnectionMaxilla,
                           this->m_removedConnectionMapperMaxilla,
                           this->m_removedConnectionActorMaxilla,
                           this->m_scalarPointEdgeExtractorMaxilla,
                           this->m_curvatureActorMaxilla,
                           this->m_additionalConnectionMaxilla,
                           this->m_additionalConnectionActorMaxilla);
  }
}

void SceneWidget::showRemoveConnectionDo(vtkPolyData * polydataOriginal,
                                         vtkActor * actorOriginal,
                                         vSP<NonDestructiveConnectivityFilter> & largestConnFilter,
                                         vSP<ErodeFilter> & erodeFilter,
                                         vSP<vtkPolyData> & removedConnection,
                                         vSP<vtkPolyDataMapper> & removedConnectionMapper,
                                         vSP<vtkActor> & removedConnectionActor,
                                         ScalarPointToEdgeExtractor * scalarPointEdgeExtractor,
                                         vtkActor * curvatureActor,
                                         vtkPolyData * additionalConnection,
                                         vtkActor * additionalConnectionActor)
{
  if(largestConnFilter == nullptr)
  {
    vVSP(NonDestructiveConnectivityFilter, largestConnFilter);

    vVSP(ErodeFilter, erodeFilter);

    createProgressDialog();
    erodeFilter->AddObserver(vtkCommand::ProgressEvent, this->m_progressCommand);

    vVSP(vtkPolyDataMapper, removedConnectionMapper);
    vVSP(vtkActor, removedConnectionActor);

    removedConnectionActor->SetMapper(removedConnectionMapper);

    vtkProperty * property = removedConnectionActor->GetProperty();
    property->EdgeVisibilityOn();
    property->SetEdgeColor(s_colors->GetColor3d("Green").GetData());
    property->SetLineWidth(5);
    property->SetVertexVisibility(true);
    property->VertexVisibilityOn();
    property->SetPointSize(5);
    property->SetVertexColor(this->s_colors->GetColor3d("Green").GetData());
    property->SetColor(this->s_colors->GetColor3d("Green").GetData());

    this->m_renderer->AddActor(removedConnectionActor);
  }

  vDNSP(vtkPolyData, appendPolyData);
  appendPolyData->SetPoints(polydataOriginal->GetPoints());
  vDNSP(vtkCellArray, appendLines);

  vtkPolyData * output1 = scalarPointEdgeExtractor->GetOutput();
  vtkCellArray * lines = output1->GetLines();
  auto iter = vtk::TakeSmartPointer(lines->NewIterator());
  for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell())
  {
    appendLines->InsertNextCell(iter->GetCurrentCell());
  }
  curvatureActor->VisibilityOff();
  if(additionalConnection != nullptr)
  {
    vtkCellArray * lines = additionalConnection->GetLines();
    auto iter = vtk::TakeSmartPointer(lines->NewIterator());
    int i = 0;
    for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell())
    {
      std::cout << "additonal line: " << i++ << std::endl;
      appendLines->InsertNextCell(iter->GetCurrentCell());
    }
    additionalConnectionActor->VisibilityOff();
  }
  appendPolyData->SetLines(appendLines);

  largestConnFilter->SetInputData(appendPolyData);

  this->m_progressDialog->setValue(0);
  this->m_progressDialog->show();

  erodeFilter->SetInputConnection(0, largestConnFilter->GetOutputPort());
  erodeFilter->SetInputData(1, polydataOriginal);
  erodeFilter->Update();

  removedConnection = erodeFilter->GetOutput();
  removedConnectionMapper->SetInputData(removedConnection);

//  vtkIdType numberOfPoints = removedConnection->GetNumberOfPoints();

  this->m_progressDialog->hide();

  actorOriginal->VisibilityOn();
  removedConnectionActor->VisibilityOn();
}

void SceneWidget::showRemoveConnectionOff(bool hideAll)
{
  this->m_parentWindow->ui->removeConnectionShowBtn->setChecked(false);

  if(this->m_modelState.mandibleVisible == true)
  {
    this->m_removedConnectionActorMandible->VisibilityOff();
    if(hideAll)
    {
      this->m_curvatureActorMandible->VisibilityOn();
      if(this->m_additionalConnectionMandible != nullptr)
      {
        this->m_additionalConnectionActorMandible->VisibilityOn();
      }

      // extra buttons
      this->m_parentWindow->ui->addConnectionEnableBtn->show();
      this->m_parentWindow->ui->addConnectionUndoBtn->show();
    }
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    this->m_removedConnectionActorMaxilla->VisibilityOff();
    if(hideAll)
    {
      this->m_curvatureActorMaxilla->VisibilityOn();
      if(this->m_additionalConnectionMaxilla != nullptr)
      {
        this->m_additionalConnectionActorMaxilla->VisibilityOn();
      }

      // extra buttons
      this->m_parentWindow->ui->addConnectionEnableBtn->show();
      this->m_parentWindow->ui->addConnectionUndoBtn->show();
    }
  }
}

void SceneWidget::addConnectionPressDelInternal()
{

}

void SceneWidget::showSeparationOn()
{
  if(this->m_modelState.mandibleVisible == true)
  {
    showSeparationDo(this->m_mandible,
                     this->m_mandibleActor,
                     this->m_separationFilterMandible,
                     this->m_pointDataFilterMandible,
                     this->m_removedConnectionMandible,
                     this->m_separationActorsMandible);
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    showSeparationDo(this->m_maxilla,
                     this->m_maxillaActor,
                     this->m_separationFilterMaxilla,
                     this->m_pointDataFilterMaxilla,
                     this->m_removedConnectionMaxilla,
                     this->m_separationActorsMaxilla);
  }
}

void SceneWidget::showSeparationDo(vtkPolyData * polydataOriginal,
                                   vtkActor * actorOriginal,
                                   vSP<ScalarSeparationFilter> & separationFilter,
                                   vSP<PointDataFilter> & pointDataFilter,
                                   vtkPolyData * removedConnection,
                                   std::vector<vSP<vtkActor> > & separationActors)
{
  if(separationFilter == nullptr)
  {
    vVSP(PointDataFilter, pointDataFilter);
    vVSP(ScalarSeparationFilter, separationFilter);

    createProgressDialog();
    separationFilter->AddObserver(vtkCommand::ProgressEvent, this->m_progressCommand);
  }

  pointDataFilter->SetInputData(0, polydataOriginal);
  pointDataFilter->SetInputData(1, removedConnection);

  this->m_progressDialog->setValue(0);
  this->m_progressDialog->show();

  separationFilter->SetInputConnection(pointDataFilter->GetOutputPort());
  separationFilter->Update();
  int numRegions = separationFilter->GetNumberOfOutputPorts();
  std::cout << "regions: " << numRegions << std::endl;

  for(int i = 0; i < numRegions; i++)
  {
    vtkPolyData * polydata = separationFilter->GetOutput(i);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(polydata);
    mapper->ScalarVisibilityOff();

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    vtkColor3ub color = m_greenColors->GetColorRepeating(i);
    double nColor[3];
    vtkUtil::convertColor(color, nColor);
    actor->GetProperty()->SetColor(nColor);
    this->m_renderer->AddActor(actor);
    separationActors.push_back(actor);
  }

  this->m_progressDialog->hide();

  actorOriginal->VisibilityOff();
}

void SceneWidget::showSeparationOff(bool hideAll)
{
  this->m_parentWindow->ui->separationShowBtn->setChecked(false);

  if(this->m_modelState.mandibleVisible == true)
  {
    if(hideAll)
    {
      std::vector<vSP<vtkActor>>::const_iterator iter =
          this->m_separationActorsMandible.begin();
      for(;iter != this->m_separationActorsMandible.end(); iter++)
      {
        this->m_renderer->RemoveActor(*iter);
      }
      this->m_separationActorsMandible.clear();
      this->m_mandibleActor->VisibilityOn();
      this->m_removedConnectionActorMandible->VisibilityOn();
    }
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    if(hideAll)
    {
      std::vector<vSP<vtkActor>>::const_iterator iter =
          this->m_separationActorsMaxilla.begin();
      for(;iter != this->m_separationActorsMaxilla.end(); iter++)
      {
        this->m_renderer->RemoveActor(*iter);
      }
      this->m_separationActorsMaxilla.clear();
      this->m_maxillaActor->VisibilityOn();
      this->m_removedConnectionActorMaxilla->VisibilityOn();
    }
  }
}

void SceneWidget::updateProgressInternal(double progress)
{
  std::cout << "update progress: " << progress << std::endl;
  if(m_progressDialog != nullptr)
  {
    m_progressDialog->setValue((int)(progress * 100));
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  }
}

void SceneWidget::showCombiningOn()
{
  this->m_interactorStyle->CombiningOn();

  // show extra buttons
  this->m_parentWindow->ui->combiningClickSelectionBtn->show();
  this->m_parentWindow->ui->combiningRectSelectionBtn->show();
  this->m_parentWindow->ui->combiningUndoBtn->show();

  // bind external key press event
  if(this->m_combiningPressEnterCallback == nullptr)
  {
    vVSP(vtkCallbackCommand, this->m_combiningPressEnterCallback);
    this->m_combiningPressEnterCallback->SetCallback(combiningPressEnter);
    this->m_combiningPressEnterCallback->SetClientData(this);
  }

  this->m_interactor->AddObserver(vtkCommand::KeyPressEvent, this->m_combiningPressEnterCallback);

  if(this->m_modelState.mandibleVisible)
  {
    this->m_combiningMergedActorsMandible.clear();
  this->m_oldPropertyMapMandible.clear();

    if(this->m_combiningAppenderMandible == nullptr)
    {
      vVSP(vtkAppendDataSets, this->m_combiningAppenderMandible);
      this->m_combiningAppenderMandible->MergePointsOn();
      this->m_combiningAppenderMandible->SetOutputDataSetType(VTK_POLY_DATA);
    }
    this->m_combiningAppenderMandible->RemoveAllInputs();
  }
  else if(this->m_modelState.maxillaVisible)
  {
    this->m_combiningMergedActorsMaxilla.clear();
    this->m_oldPropertyMapMaxilla.clear();

    if(this->m_combiningAppenderMaxilla == nullptr)
    {
      vVSP(vtkAppendDataSets, this->m_combiningAppenderMaxilla);
      this->m_combiningAppenderMaxilla->MergePointsOn();
      this->m_combiningAppenderMaxilla->SetOutputDataSetType(VTK_POLY_DATA);
    }
    this->m_combiningAppenderMaxilla->RemoveAllInputs();
  }
}

void SceneWidget::showCombiningOff(bool hideAll)
{
  this->m_parentWindow->ui->combiningShowBtn->setChecked(false);

  // TODO turn off the selection buttons
  // hide the extra buttons
  this->m_parentWindow->ui->combiningClickSelectionBtn->hide();
  this->m_parentWindow->ui->combiningClickSelectionBtn->setChecked(false);
  this->m_parentWindow->ui->combiningRectSelectionBtn->hide();
  this->m_parentWindow->ui->combiningRectSelectionBtn->setChecked(false);
  this->m_parentWindow->ui->combiningUndoBtn->hide();

  // exit combining state
  this->m_interactorStyle->CombiningOff();

  // no picking state
  this->m_interactorStyle->SetPickingStyleToNone();

  // remove keyboard event
  this->m_interactor->RemoveObserver(this->m_combiningPressEnterCallback);

  if(this->m_modelState.mandibleVisible == true)
  {
    // cancel the selection
    std::map<vSP<vtkActor>, vSP<vtkProperty>>::iterator iterMap =
        this->m_oldPropertyMapMandible.begin();
    for(; iterMap != this->m_oldPropertyMapMandible.end(); iterMap++)
    {
      vtkActor * actor = iterMap->first;
//      std::cout << "actor: " << actor << std::endl;

      vtkProperty * property = iterMap->second;
//      std::cout << "property: " << property << std::endl;

      actor->SetProperty(property);
    }

    if(hideAll)
    {
      // show all the separated pieces
      std::vector<vSP<vtkActor>>::iterator iter =
          this->m_combiningRemovedActorsMandible.begin();
      for(; iter != this->m_combiningRemovedActorsMandible.end(); iter++)
      {
//        this->m_renderer->AddActor(*iter);
        (*iter)->VisibilityOn();
      }

      // remove all the merged pieces
      std::vector<vSP<vtkActor>>::iterator iter2 =
          this->m_combiningMergedActorsMandible.begin();
      for(; iter2 != this->m_combiningMergedActorsMandible.end(); iter2++)
      {
        vtkActor * actor = *iter2;
        this->m_renderer->RemoveActor(actor);
      }
    }
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    // cancel the selection
    std::map<vSP<vtkActor>, vSP<vtkProperty>>::iterator iterMap =
        this->m_oldPropertyMapMaxilla.begin();
    for(; iterMap != this->m_oldPropertyMapMaxilla.end(); iterMap++)
    {
      vtkActor * actor = iterMap->first;
//      std::cout << "actor: " << actor << std::endl;

      vtkProperty * property = iterMap->second;
//      std::cout << "property: " << property << std::endl;

      actor->SetProperty(property);
    }

    if(hideAll)
    {
      // show all the hidden separated pieces
      std::vector<vSP<vtkActor>>::iterator iter =
          this->m_combiningRemovedActorsMaxilla.begin();
      for(; iter != this->m_combiningRemovedActorsMaxilla.end(); iter++)
      {
//        this->m_renderer->AddActor(*iter);
        (*iter)->VisibilityOn();
      }

      // remove all the merged pieces
      std::vector<vSP<vtkActor>>::iterator iter2 =
          this->m_combiningMergedActorsMaxilla.begin();
      for(; iter2 != this->m_combiningMergedActorsMaxilla.end(); iter2++)
      {
        vtkActor * actor = *iter2;
        this->m_renderer->RemoveActor(actor);
      }
    }
  }

  this->m_renderWindow->Render();
}

void SceneWidget::combiningRectSelectionInternal(bool checked)
{
  if(checked)
  {
    this->combiningRectSelectionOn();
  }
  else
  {
    this->combiningRectSelectionOff();
  }
}

void SceneWidget::combiningRectSelectionOn()
{
  // note that rect selection and click selection are mutually conflicting
  // disalbe the other selection if selected

    // turn click selection off
  this->combiningClickSelectionOff();

    // turn rect selection on
  this->m_interactorStyle->SetPickingStyleToRect();
  this->m_parentWindow->ui->combiningRectSelectionBtn->setChecked(true);

  this->m_renderWindow->Render();
}

void SceneWidget::combiningRectSelectionOff()
{
  this->m_interactorStyle->SetPickingStyleToNone(); // set to none picking
  this->m_parentWindow->ui->combiningRectSelectionBtn->setChecked(false);

  this->m_renderWindow->Render();
}

void SceneWidget::combiningClickSelectionInternal(bool checked)
{
  if(!checked) // disable rect slection
  {
    this->combiningClickSelectionOff();
  }
  else
  {
    this->combiningClickSelectionOn();
  }
}

void SceneWidget::combiningClickSelectionOn()
{
  // note that rect selection and click selection are mutually conflicting
  // disalbe the other selection if selected

    // turn rect selection off
  this->combiningRectSelectionOff();

    // turn rect selection on
  this->m_interactorStyle->SetPickingStyleToProp();
  this->m_parentWindow->ui->combiningClickSelectionBtn->setChecked(true);

  this->m_renderWindow->Render();
}

void SceneWidget::combiningClickSelectionOff()
{
  this->m_interactorStyle->SetPickingStyleToNone(); // set to none picking
  this->m_parentWindow->ui->combiningClickSelectionBtn->setChecked(false);

  this->m_renderWindow->Render();
}

void SceneWidget::combiningAddPicking(vtkActor * actor)
{
  if(this->m_modelState.mandibleVisible == true)
  {
    if(this->m_oldPropertyMapMandible.find(actor) ==
       this->m_oldPropertyMapMandible.end()) // not already picked
    {
      // keep a copy of original property
      vtkNew<vtkProperty> oldProperty;
      oldProperty->DeepCopy(actor->GetProperty());
      this->m_oldPropertyMapMandible[actor] = oldProperty;

      vtkColor3ub color = m_redColors->GetColorRepeating(this->m_currentHighlightColor++);
      double nColor[3];
      vtkUtil::convertColor(color, nColor);

      if(this->m_currentHighlightColor == m_redColors->GetNumberOfColors())
      {
        this->m_currentHighlightColor = 0;
      }

      // change actor color to red
      vtkProperty * property = actor->GetProperty();
      property->SetColor(nColor);
//      property->EdgeVisibilityOn();
//      property->SetEdgeColor(SceneWidget::s_colors->GetColor3d("Black").GetData());
    }
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    if(this->m_oldPropertyMapMaxilla.find(actor) ==
       this->m_oldPropertyMapMaxilla.end()) // not already picked
    {
      std::cout << "add picking: " << actor  << std::endl;
      // keep a copy of original property
      vtkNew<vtkProperty> oldProperty;
      oldProperty->DeepCopy(actor->GetProperty());
      this->m_oldPropertyMapMaxilla[actor] = oldProperty;

      std::cout << "property: " << oldProperty << std::endl;

      vtkColor3ub color = m_redColors->GetColorRepeating(this->m_currentHighlightColor++);
      double nColor[3];
      vtkUtil::convertColor(color, nColor);

      if(this->m_currentHighlightColor == m_redColors->GetNumberOfColors())
      {
        this->m_currentHighlightColor = 0;
      }

      // change actor color to red
      vtkProperty * property = actor->GetProperty();
      property->SetColor(nColor);
//      property->EdgeVisibilityOn();
//      property->SetEdgeColor(SceneWidget::s_colors->GetColor3d("Black").GetData());
    }
  }
}

void SceneWidget::combiningRemovePicking(vtkActor * actor)
{
  if(this->m_modelState.mandibleVisible == true)
  {
    std::map<vSP<vtkActor>, vSP<vtkProperty>>::const_iterator iter
        = this->m_oldPropertyMapMandible.find(actor);
    if(iter != this->m_oldPropertyMapMandible.end()) // already picked
    {
      std::cout << "remove picking: " << actor << std::endl;
      // recovery to the original property
      vtkProperty * property = iter->second;
      actor->SetProperty(property);
      // remove from map
      this->m_oldPropertyMapMandible.erase(iter);
    }
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    std::map<vSP<vtkActor>, vSP<vtkProperty>>::const_iterator iter
        = this->m_oldPropertyMapMaxilla.find(actor);
    if(iter != this->m_oldPropertyMapMaxilla.end()) // already picked
    {
      std::cout << "remove picking: " << actor << std::endl;
      // recovery to the original property
      vtkProperty * property = iter->second;
      actor->SetProperty(property);
      // remove from map
      this->m_oldPropertyMapMaxilla.erase(iter);
    }
  }
}

void SceneWidget::combiningPressEnterInternal()
{
  char * keySym = this->m_interactor->GetKeySym();

  if(strcmp(keySym, "Return") == 0)
  {
    if(this->m_modelState.mandibleVisible == true)
    {
      combiningPressEnterDo(this->m_oldPropertyMapMandible,
                            this->m_separationActorsMandible,
                            this->m_combiningRemovedActorsMandible,
                            this->m_combiningMergedActorsMandible);


    }
    else if(this->m_modelState.maxillaVisible == true)
    {
      combiningPressEnterDo(this->m_oldPropertyMapMaxilla,
                            this->m_separationActorsMaxilla,
                            this->m_combiningRemovedActorsMaxilla,
                            this->m_combiningMergedActorsMaxilla);
    }

    this->m_renderWindow->Render();
  }
}

void SceneWidget::combiningPressEnterDo(std::map<vSP<vtkActor>, vSP<vtkProperty>> & oldPropertyMap,
                                        std::vector<vSP<vtkActor>> & separationActors,
                                        std::vector<vSP<vtkActor>> & combiningRemovedActors,
                                        std::vector<vSP<vtkActor>> & combiningMergedActors)
{
  vtkNew<vtkAppendDataSets> appender;
  appender->MergePointsOn();
  appender->SetOutputDataSetType(VTK_POLY_DATA);

  std::map<vSP<vtkActor>, vSP<vtkProperty>>::iterator iter
      = oldPropertyMap.begin();

  for(; iter != oldPropertyMap.end(); iter++)
  {
    vtkActor * actor = iter->first;
    vtkDataSet * input = actor->GetMapper()->GetInput();

    appender->AddInputData(input);

    actor->SetProperty(iter->second);
    actor->VisibilityOff();

    std::vector<vSP<vtkActor>>::const_iterator findResult =
      std::find(separationActors.begin(),
                separationActors.end(),
                actor);
    // if this is the original separated piece
    if(findResult != separationActors.end())
    {
      // keep a copy of later reverse action
      combiningRemovedActors.push_back(actor);
    }
    else // not the original separated piece
    {
      findResult =
        std::find(combiningMergedActors.begin(),
                  combiningMergedActors.end(),
                  actor);
      if(findResult != combiningMergedActors.end()) // found
      {
        combiningMergedActors.erase(findResult);
      }

      // remove directly
      this->m_renderer->RemoveActor(actor);
    }
  }

  // clear the marged pieces
  oldPropertyMap.clear();

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(appender->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  combiningMergedActors.push_back(actor);

  vtkColor3ub color = this->m_blueColors->GetColorRepeating(this->m_currentCombinedColor++);
  double nColor[3];
  vtkUtil::convertColor(color, nColor);
  // wrap around
  if(this->m_currentCombinedColor == this->m_blueColors->GetNumberOfColors())
  {
    this->m_currentCombinedColor = 0;
  }

  this->m_renderer->AddActor(actor);
  actor->GetProperty()->SetColor(nColor);
}

void SceneWidget::showMarkingOn()
{
  if(this->m_modelState.mandibleVisible)
  {
    this->m_actorMarkingMapMandible.clear();
    this->m_markingActorMapMandible.clear();
  }

  if(this->m_modelState.maxillaVisible)
  {
    this->m_actorMarkingMapMaxilla.clear();
    this->m_markingActorMapMaxilla.clear();
  }

  this->m_interactorStyle->MarkingOn();

  // show extra button
  this->m_parentWindow->ui->markingClickSelectionBtn->show();

  if(m_markingLastProperty == nullptr)
  {
    vVSP(vtkProperty, this->m_markingLastProperty);
  }
}

void SceneWidget::showMarkingOff(bool hideAll)
{
  this->m_parentWindow->ui->markingShowBtn->setChecked(false);

  // hide extra button
  this->m_parentWindow->ui->markingClickSelectionBtn->hide();
  this->m_parentWindow->ui->markingClickSelectionBtn->setChecked(false);

  this->m_interactorStyle->MarkingOff();
  this->m_interactorStyle->SetPickingStyleToNone();

  // cancel any selection
  if(this->m_markingLastActor != nullptr)
  {
    this->m_markingLastActor->SetProperty(this->m_markingLastProperty);
    this->m_markingLastActor = nullptr;
  }

  // hide combobox
  this->hideChooseMarking();

  // hide axes
  if(this->m_toothXAxesActor != nullptr)
  {
    this->m_toothXAxesActor->VisibilityOff();
  }
  if(this->m_toothYAxesActor != nullptr)
  {
    this->m_toothYAxesActor->VisibilityOff();
  }
  if(this->m_toothZAxesActor != nullptr)
  {
    this->m_toothZAxesActor->VisibilityOff();
  }

  if(this->m_modelState.mandibleVisible == true)
  {
    if(hideAll)
    {
      // remove all the marking
      std::map<vSP<vtkActor>, std::string>::iterator iter
        = this->m_actorMarkingMapMandible.begin();

      for(; iter != this->m_actorMarkingMapMandible.end(); iter++)
      {
        vtkActor * actor = iter->first;
        vtkDataSet * dataSet = actor->GetMapper()->GetInput();

        vtkFieldData * fieldData = dataSet->GetFieldData();
        fieldData->RemoveArray("marking");
      }
    }
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    if(hideAll)
    {
      // remove all the marking
      std::map<vSP<vtkActor>, std::string>::iterator iter
        = this->m_actorMarkingMapMaxilla.begin();

      for(; iter != this->m_actorMarkingMapMaxilla.end(); iter++)
      {
        vtkActor * actor = iter->first;
        vtkDataSet * dataSet = actor->GetMapper()->GetInput();

        vtkFieldData * fieldData = dataSet->GetFieldData();
        fieldData->RemoveArray("marking");
      }
    }
  }
}

void SceneWidget::markingClickSelectionInternal(bool checked)
{
  if(!checked) // disable rect slection
  {
    this->markingClickSelectionOff();
  }
  else
  {
    this->markingClickSelectionOn();
  }
}

void SceneWidget::markingComboBoxActivated(int index)
{
  QObject * object = QObject::sender();
  QComboBox * chooseMarking = qobject_cast<QComboBox *>(object);

  if(this->m_markingLastActor != nullptr)
  {
    if(index == 0)  // remove marking??
    {
      this->unmarkActor(this->m_markingLastActor);
    }
    else
    {
      std::string markingText =
        chooseMarking->currentText().toStdString();

      this->markActor(markingText, this->m_markingLastActor);
    }

  }
}

void SceneWidget::markingClickSelectionOn()
{
  this->m_interactorStyle->SetPickingStyleToProp();
  this->m_parentWindow->ui->markingClickSelectionBtn->setChecked(true);

  this->m_renderWindow->Render();
}

void SceneWidget::markingClickSelectionOff()
{
  this->m_interactorStyle->SetPickingStyleToNone();
  this->m_parentWindow->ui->markingClickSelectionBtn->setChecked(false);

  this->m_renderWindow->Render();
}

void SceneWidget::markingAddPicking(vtkActor * actor)
{
  if(actor != this->m_markingLastActor) // not selected the same actor
  {
    // revert last selected actor
    if(this->m_markingLastActor != nullptr)
    {
      this->m_markingLastActor->SetProperty(this->m_markingLastProperty);
    }

    if(this->m_toothAxes == nullptr)
    {
      vVSP(vtkArrowSource, this->m_toothAxes);
      this->m_toothAxes->Update();
      this->m_toothAxesPolyData = this->m_toothAxes->GetOutput();

      vVSP(vtkTransformPolyDataFilter, m_toothXTransform);
      vVSP(vtkTransformPolyDataFilter, m_toothYTransform);
      vVSP(vtkTransformPolyDataFilter, m_toothZTransform);

      vVSP(vtkPolyDataMapper, m_toothXAxesMapper);
      vVSP(vtkPolyDataMapper, m_toothYAxesMapper);
      vVSP(vtkPolyDataMapper, m_toothZAxesMapper);

      vVSP(vtkActor, m_toothXAxesActor);
      vVSP(vtkActor, m_toothYAxesActor);
      vVSP(vtkActor, m_toothZAxesActor);

      this->m_toothXAxesActor->GetProperty()->SetColor(this->s_colors->GetColor3d("red").GetData());
      this->m_toothYAxesActor->GetProperty()->SetColor(this->s_colors->GetColor3d("green").GetData());
      this->m_toothZAxesActor->GetProperty()->SetColor(this->s_colors->GetColor3d("blue").GetData());

      this->m_toothXAxesActor->SetMapper(this->m_toothXAxesMapper);
      this->m_toothYAxesActor->SetMapper(this->m_toothYAxesMapper);
      this->m_toothZAxesActor->SetMapper(this->m_toothZAxesMapper);

      this->m_renderer->AddActor(this->m_toothXAxesActor);
      this->m_renderer->AddActor(this->m_toothYAxesActor);
      this->m_renderer->AddActor(this->m_toothZAxesActor);
    }

    if(this->m_toothAxesActor == nullptr)
    {
      vVSP(vtkAxesActor, this->m_toothAxesActor);
      this->m_renderer->AddActor(this->m_toothAxesActor);
      this->m_toothAxesActor->VisibilityOff();
      this->m_toothAxesActor->AxisLabelsOff();
    }

    if(actor != nullptr)
    {
      // highlight the new selector actor
      vVSP(vtkProperty, this->m_markingLastProperty);
      this->m_markingLastProperty->DeepCopy(actor->GetProperty());

      vtkColor3ub color = m_redColors->GetColorRepeating(this->m_currentHighlightColor++);
      double nColor[3];
      vtkUtil::convertColor(color, nColor);

      actor->GetProperty()->SetColor(nColor);

      // show combobox
      int * clickPos = this->m_interactor->GetEventPosition();

      int local[2];

      vtkUtil::displayPositionToLocalDisplayPosition(this->m_renderer,
                                                     clickPos, local);

      QPoint pos = this->m_parentWindow->ui->qvtkWidget->mapTo(
            this->m_parentWindow,
            this->m_parentWindow->ui->qvtkWidget->rect().topLeft());

      this->showChooseMarking(local[0] + pos.x(), local[1] + pos.y(), actor);

      this->m_markingLastActor = actor;

      // show tooth axes
      vtkDataArray * coordinateVectors = getCoordinateVectors(actor);

      double max[3];
      double min[3];
      double mid[3];

      coordinateVectors->GetTuple(0, max);
      coordinateVectors->GetTuple(1, min);
      coordinateVectors->GetTuple(2, mid);

      double * bounds = actor->GetBounds();

      double xLen = bounds[1] - bounds[0];

      //
      double Origin[3] = {1.0,0.0,0.0};
      double Orthogonal[3];
      double thetaRadians;
      double thetaDegree;

      // X
      vDNSP(vtkTransform, transX);
      transX->PostMultiply();
      transX->Scale(3 * xLen, 5, 5);


      Orthogonal[0]= 0;
      Orthogonal[1]= -mid[2]/mid[1];
      Orthogonal[2]= 1;

      thetaRadians = vtkMath::AngleBetweenVectors(Origin,mid);
      thetaDegree = vtkMath::DegreesFromRadians(thetaRadians);

      if(max[0] > 0)
      {
        thetaDegree = 360 - thetaDegree;
      }

      transX->RotateWXYZ(thetaDegree,Orthogonal[0],Orthogonal[1],Orthogonal[2]);

      transX->Translate(-max[0], -max[1], -max[2]);


      this->m_toothXTransform->SetTransform(transX);
      this->m_toothXTransform->SetInputData(this->m_toothAxes->GetOutput());
      this->m_toothXTransform->Update();

      this->m_toothXAxesMapper->SetInputData(this->m_toothXTransform->GetOutput());
      this->m_toothXAxesActor->VisibilityOn();

      // Y
      vDNSP(vtkTransform, transY);
      transY->PostMultiply();
      transY->Scale(2 * xLen, 5, 5);

      if(m_modelState.mandibleVisible)
      {
          transY->RotateY(270);
      }
      else
      {
          transY->RotateY(90);
      }

      transY->Translate(-max[0], -max[1], -max[2]);

      this->m_toothYTransform->SetTransform(transY);
      this->m_toothYTransform->SetInputData(this->m_toothAxes->GetOutput());
      this->m_toothYTransform->Update();

      this->m_toothYAxesMapper->SetInputData(this->m_toothYTransform->GetOutput());
      this->m_toothYAxesActor->VisibilityOn();

      // Z
      vDNSP(vtkTransform, transZ);
      transZ->PostMultiply();
      transZ->Scale(2 * xLen, 5, 5);

      Orthogonal[0]= 0;
      Orthogonal[1]= -max[2]/max[1];
      Orthogonal[2]= 1;

      thetaRadians = vtkMath::AngleBetweenVectors(Origin,max);
      thetaDegree = vtkMath::DegreesFromRadians(thetaRadians);

      transZ->RotateWXYZ(thetaDegree,Orthogonal[0],Orthogonal[1],Orthogonal[2]);

      transZ->Translate(-max[0], -max[1], -max[2]);

      this->m_toothZTransform->SetTransform(transZ);
      this->m_toothZTransform->SetInputData(this->m_toothAxes->GetOutput());
      this->m_toothZTransform->Update();

      this->m_toothZAxesMapper->SetInputData(this->m_toothZTransform->GetOutput());
      this->m_toothZAxesActor->VisibilityOn();
    }
    else // selected empty area
    {
      this->hideChooseMarking();
      this->m_markingLastActor = nullptr;
//      this->m_toothAxesActor->VisibilityOff();
      this->m_toothXAxesActor->VisibilityOff();
      this->m_toothYAxesActor->VisibilityOff();
      this->m_toothZAxesActor->VisibilityOff();
    }
  }
}

void SceneWidget::showChooseMarking(int x, int y, vtkActor * actor)
{
  if(actor == nullptr)
  {
    return;
  }

//  this->m_currentActor = actor;
  vtkDataSet * dataSet = actor->GetMapper()->GetInput();

  vtkFieldData * fieldData = dataSet->GetFieldData();
  // vtkStringArray is not a data array but an abstract array
  vtkAbstractArray * array = fieldData->GetAbstractArray("marking");
  vtkStdString mark = std::string();
  if(array != nullptr)
  {
    vtkStringArray * nameArray = vtkStringArray::SafeDownCast(array);
    mark = nameArray->GetValue(0);
  }

  if(this->m_modelState.mandibleVisible == true)
  {
    this->showChooseMarkingDo(mark, this->m_parentWindow->ui->chooseMarkingMandible,
                              x, y);
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    this->showChooseMarkingDo(mark, this->m_parentWindow->ui->chooseMarkingMaxilla,
                              x, y);
  }
}

void SceneWidget::showChooseMarkingDo(vtkStdString mark,
                                      QComboBox * chooseMarking,
                                      int x, int y)
{
  if(mark.empty())
  {
    chooseMarking->setCurrentIndex(0);
  }
  else
  {
    chooseMarking->setCurrentText(mark.c_str());
  }
  const QRect geo = chooseMarking->geometry();
  chooseMarking->setGeometry(x,
                             y - geo.height() - 10,
                             geo.width(), geo.height());
  chooseMarking->show();
}

void SceneWidget::hideChooseMarking()
{
  if(this->m_modelState.mandibleVisible == true)
  {
    this->m_parentWindow->ui->chooseMarkingMandible->hide();
  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    this->m_parentWindow->ui->chooseMarkingMaxilla->hide();
  }
}

void SceneWidget::resetChooseMarking(std::string originalMark)
{
  if(originalMark.empty())
  {
    if(this->m_modelState.mandibleVisible == true)
    {
      this->m_parentWindow->ui->chooseMarkingMandible->setCurrentIndex(0);
    }
    else if(this->m_modelState.maxillaVisible == true)
    {
      this->m_parentWindow->ui->chooseMarkingMandible->setCurrentIndex(0);
    }
  }
  else
  {
    if(this->m_modelState.mandibleVisible == true)
    {
      this->m_parentWindow->ui->chooseMarkingMandible->setCurrentText(originalMark.c_str());
    }
    else if(this->m_modelState.maxillaVisible == true)
    {
      this->m_parentWindow->ui->chooseMarkingMaxilla->setCurrentText(originalMark.c_str());
    }
  }
}

void SceneWidget::showFinishSplitInternal()
{
  // turn off the previous button
  if(this->m_parentWindow->ui->markingShowBtn->isChecked())
  {
    this->showMarkingOff();
    this->showFinishSplitOn();
    this->showFinishSplitOff();
  }
  else
  {
    QMessageBox::critical(this->m_parentWindow,
                            "错误", "不能跳过步骤，必须完成分离并标记牙齿！");
  }

  // no more other actions

  this->m_renderWindow->Render();
}


void SceneWidget::showFinishSplitOn()
{

  if(this->m_modelState.mandibleVisible == true)
  {
    showFinishSplitDo(this->m_separationActorsMandible,
                 this->m_actorMarkingMapMandible,
                 this->m_markingActorMapMandible,
                 this->m_modelState.mandibleSplitted);

  }
  else if(this->m_modelState.maxillaVisible == true)
  {
    showFinishSplitDo(this->m_separationActorsMaxilla,
                 this->m_actorMarkingMapMaxilla,
                 this->m_markingActorMapMaxilla,
                 this->m_modelState.maxillaSplitted);
  }
}

void SceneWidget::showFinishSplitDo(std::vector<vSP<vtkActor> > & separationActors,
                               std::map<vSP<vtkActor>, std::string> & actorMarkingMap,
                               std::map<std::string, vSP<vtkActor>> & markingActorMap,
                               bool & splittedState)
{
  double gumColor[3];
  vtkUtil::convertColor(245, 101, 126, gumColor);

  double toothColor[3];
  vtkUtil::convertColor(246, 245, 250, toothColor);

  // not marked pieces are gums
  vDNSP(vtkAppendDataSets, appender);
  appender->SetOutputDataSetType(VTK_POLY_DATA);
  appender->MergePointsOn();

  // for all pieces of actors separated
  std::vector<vSP<vtkActor>>::iterator iter =
      separationActors.begin();
  for(; iter != separationActors.end(); iter++)
  {
    vtkActor * actor = *iter;
    // actor should not be hidden
    if(actor->GetVisibility() == true)
    {
      std::map<vSP<vtkActor>, std::string>::iterator findIter =
          actorMarkingMap.find(actor);

      // if the actor is not mapped to tooth
      if(findIter == actorMarkingMap.end())
      {
        appender->AddInputData(actor->GetMapper()->GetInput());
        actor->VisibilityOff();
      }
    }
  }

  vDNSP(vtkPolyDataMapper, mapper);
  vDNSP(vtkActor, actor);

  mapper->SetInputConnection(appender->GetOutputPort());
  actor->SetMapper(mapper);


  actor->GetProperty()->SetColor(gumColor);
  actor->GetProperty()->SetDiffuseColor(gumColor);
  actor->GetProperty()->SetDiffuse(0.8);
  actor->GetProperty()->SetSpecular(0.3);
  actor->GetProperty()->SetSpecularColor(
      this->s_colors->GetColor3d("White").GetData());
  actor->GetProperty()->SetSpecularPower(10.0);

  this->m_renderer->AddActor(actor);

  // for all the marked tooth
  std::map<vSP<vtkActor>, std::string>::iterator iter2 =
      actorMarkingMap.begin();
  for(; iter2 != actorMarkingMap.end(); iter2++)
  {
    vtkActor * actor = iter2->first;

    // set the color for tooth
    actor->GetProperty()->SetColor(toothColor);
    actor->GetProperty()->SetDiffuseColor(toothColor);
    actor->GetProperty()->SetDiffuse(0.8);
    actor->GetProperty()->SetSpecular(0.3);
    actor->GetProperty()->SetSpecularColor(
        this->s_colors->GetColor3d("White").GetData());
    actor->GetProperty()->SetSpecularPower(20.0);
  }

  markingActorMap["gums"] = actor;
  actorMarkingMap[actor] = "gums";

//    this->m_finished = true;
  splittedState = true;
}

void SceneWidget::showFinishSplitOff()
{
  splitToothDone();
}

SceneWidget::ActionDirection SceneWidget::turnOffSplittingButtons(QPushButton * button)
{
  // user clicked button index
  int buttonIndex = 0; // null button means turn off all
  if(button != nullptr) // otherwise, check buttonn index
  {
    buttonIndex = button->property("buttonIndex").toInt();
  }

  ActionDirection direction = AD_Skip;
  int after = 0;
  int i;
  for(i = 0; i < this->m_splittingButtons.count(); i++)
  {
    // do not count current button
    if(buttonIndex == i)
    {
      continue;
    }
    // checked button
    QPushButton * const checkedButton = this->m_splittingButtons.at(i);
    if(checkedButton->isChecked())
    {
      // checked button is right before this button
      if(i == buttonIndex - 1)
      {
        direction = AD_Forward;
        this->m_splittingShowOffFunctions[i](false);
        break;
      }
      // checked button is after this button (needs undo)
      else if(i > buttonIndex)
      {
        direction = AD_Backward;
        after = i;
        break;
      }
    }
  }

  qDebug() << "after" << after;

  // no any button was checked
  if(i == this->m_splittingButtons.count())
  {
    direction = AD_Forward;
  }
  else if(after > 0)
  {
    // perform the undo one by one
    for(i = after; i > buttonIndex; i--)
    {
      this->m_splittingShowOffFunctions[i](true);
    }
  }
  else if(direction == AD_Skip)
  {
    if(button != nullptr) // this is not gonna happen, but just in case ...
    {
      button->setChecked(false);
    }
  }

  if(button == nullptr) // needs to turn off all
  {
    this->m_splittingShowOffFunctions[0](true);
  }

  return direction;
}

void SceneWidget::turnOnSplittingButtons(QPushButton * button)
{
  // user clicked button index
  int buttonIndex = button->property("buttonIndex").toInt();
  this->m_splittingShowOnFunctions[buttonIndex]();
}


void SceneWidget::loadMandible(QString & filename)
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

    // reset the actor visibiliy
    this->m_modelState.mandibleLoaded = true;
    this->m_modelState.mandibleVisible = true;
//    this->m_modelState.mandibleVisible = true;
//    this->setMandibleVisibility(true);

    this->m_renderer->ResetCamera();
    this->m_renderWindow->Render();
}

void SceneWidget::loadMaxilla(QString & filename)
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

    // reset the actor visibility
    this->m_modelState.maxillaLoaded = true;
    this->m_modelState.maxillaVisible = true;
//    this->m_modelState.maxillaVisible = true;
//    this->setMaxillaVisibility(true);

    this->m_renderer->ResetCamera();
    this->m_renderWindow->Render();
}

void SceneWidget::hideMandible(bool checked)
{
  QObject * object = QObject::sender();
  QAction * action = qobject_cast<QAction *>(object);

  if(this->m_modelState.mandibleLoaded == false)
  {
    QMessageBox::critical(this->m_parentWindow,
                            "错误", "下颌数据未加载！");
    action->setChecked(false);
    return;
  }

  if (this->m_editState == ES_Splitting)
  {
    QMessageBox::critical(this->m_parentWindow,
                            "错误", "牙齿分离中不能切换上下颌可视！");
    action->setChecked(!action->isChecked());
    return;
  }

  if(action->isChecked())
  {
    this->setMandibleVisibility(false);
  }
  else
  {
    this->setMandibleVisibility(true);
  }

  this->m_renderWindow->Render();
}

void SceneWidget::hideMaxilla(bool checked)
{
  QObject * object = QObject::sender();
  QAction * action = qobject_cast<QAction *>(object);

  if(this->m_modelState.maxillaLoaded == false)
  {
    QMessageBox::critical(this->m_parentWindow,
                            "错误", "上颌数据未加载！");
    action->setChecked(false);
    return;
  }

  if (this->m_editState == ES_Splitting) {
    QMessageBox::critical(this->m_parentWindow,
                            "错误", "牙齿分离中不能切换上下颌可视！");
    action->setChecked(!action->isChecked());
    return;
  }

  if(action->isChecked())
  {
    this->setMaxillaVisibility(false);
  }
  else
  {
    this->setMaxillaVisibility(true);
  }

  this->m_renderWindow->Render();
}

void SceneWidget::addRefPoints()
{
  QObject * object = QObject::sender();
  QAction * action = qobject_cast<QAction *>(object);
  if(action->isChecked())
  {
    if(this->m_editState != ES_None)
    {
      QMessageBox::critical(this->m_parentWindow,
                            "错误", "当前处于编辑状态，请先完成当前编辑。");
      action->setChecked(false);
      return;
    }

    this->m_editState = ES_RefPlane;

    if(this->m_modelState.mandibleLoaded
        && this->m_modelState.maxillaLoaded)
    {
      // remember model visibility
      this->m_mandibleVisible = this->getMandibleVisibility();
      this->m_maxillaVisible = this->getMaxillaVisibility();

      this->setMandibleVisibility(true);
      this->setMaxillaVisibility(false);

      if(this->m_refPointWidget == nullptr) // widget is not created
      {
        ///////// create RefPointsWidget (Seeds Widget)
        vVSP(RefPointsWidget, this->m_refPointWidget);
        vVSP(PolySurfacePointHandleRepresentation, this->m_refHandleRep);
        vVSP(vtkSeedRepresentation, this->m_refRep);
        vVSP(vtkPolygonalSurfacePointPlacer, this->m_pointPlacer);

        this->m_refHandleRep->GetProperty()->SetColor(s_colors->GetColor3d("Red").GetData());
        this->m_refHandleRep->GetProperty()->SetLineWidth(6);
        this->m_refHandleRep->SetHandleSize(20);
        this->m_refHandleRep->GetSelectedProperty()->SetColor(s_colors->GetColor3d("Green").GetData());
        this->m_refHandleRep->GetSelectedProperty()->SetLineWidth(6);
        this->m_refHandleRep->SetPointPlacer(this->m_pointPlacer);
        this->m_refRep->SetHandleRepresentation(this->m_refHandleRep);

        this->m_refPointWidget->SetInteractor(this->m_interactor);
        this->m_refPointWidget->SetRepresentation(m_refRep);

        ///////// create show RefPlane button
        /*
        vVSP(TextButtonRepresentation2D, this->m_refPlaneShowBtnPres);
        this->m_refPlaneShowBtnPres->SetNumberOfStates(2);
        this->m_refPlaneShowBtnPres->setSize(150, 50);
        this->m_refPlaneShowBtnPres->setText(QNSP1(QString, "咬合平面"));
        this->m_refPlaneShowBtnPres->setPosition(40, 40);
        this->m_refPlaneShowBtnPres->generate();
        this->m_refPlaneShowBtnPres->place();

        vVSP(vtkButtonWidget, this->m_refPlaneShowBtn);
        this->m_refPlaneShowBtn->SetInteractor(this->m_interactor);
        this->m_refPlaneShowBtn->SetRepresentation(this->m_refPlaneShowBtnPres);
        */
      }

      this->m_pointPlacer->RemoveAllProps();
      this->m_pointPlacer->AddProp(this->m_mandibleActor);
      this->m_refPointWidget->On();
      this->m_parentWindow->ui->refPlaneButtonsContainer->show();

      // show ref plane depending on state of ref plane button
//      this->showRefPlaneInternal();
    } else {
      QMessageBox::critical(this->m_parentWindow,
                            "错误", "必须已经加载上、下颌模型！");
      action->setChecked(false);
    }
  }
  else
  {
    // restore model visibility
    this->setMandibleVisibility(this->m_mandibleVisible);
    this->setMaxillaVisibility(this->m_maxillaVisible);

    this->m_refPointWidget->Off();
    this->m_parentWindow->ui->refPlaneButtonsContainer->hide();

    // hide ref plane
    if(this->m_refPlaneMandibleActor != nullptr)
    {
      this->m_refPlaneMandibleActor->VisibilityOff();
    }

    this->m_editState = ES_None;
  }
  this->m_renderWindow->Render();
}

void SceneWidget::splitTooth(bool checked)
{
  // check if action is selected
  QObject * object = QObject::sender();
  QAction * action = qobject_cast<QAction *>(object);

  if(action->isChecked())
  {
    if(this->m_editState != ES_None)
    {
      QMessageBox::critical(this->m_parentWindow,
                            "错误", "当前处于编辑状态，请先完成当前编辑。");
      action->setChecked(false);
      return;
    }

    if(this->m_modelState.mandibleLoaded
        && this->m_modelState.maxillaLoaded)
    {
      if(this->m_modelState.mandibleVisible == false
         && this->m_modelState.maxillaVisible == false)
      {
        QMessageBox::critical(this->m_parentWindow,
                              "错误", "上下颌必须有一个可见。");
        action->setChecked(false);
        return;
      }
      else if(this->m_modelState.mandibleVisible == true
              && this->m_modelState.maxillaVisible == true)
      {
        QMessageBox::critical(this->m_parentWindow,
                              "错误", "上下颌只能有一个可见。");
        action->setChecked(false);
        return;
      }

      if(this->m_modelState.mandibleVisible == true
         && this->m_modelState.mandibleSplitted == true)
      {
        QMessageBox::StandardButton rb =
            QMessageBox::question(this->m_parentWindow, "提示",
                                  "下颌模型已经分割，是否要重新分割？",
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
        if(rb == QMessageBox::Yes)
        {
          this->setMandibleVisibility(false);
          this->m_modelState.mandibleSplitted = false;
          this->setMandibleVisibility(true);
        }
      }

      if(this->m_modelState.maxillaVisible == true
         && this->m_modelState.maxillaSplitted == true)
      {
        QMessageBox::StandardButton rb =
            QMessageBox::question(this->m_parentWindow, "提示",
                                  "上颌模型已经分割，是否要重新分割？",
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);
        if(rb == QMessageBox::Yes)
        {
          this->setMaxillaVisibility(false);
          this->m_modelState.maxillaSplitted = false;
          this->setMaxillaVisibility(true);
        }
      }

      this->m_editState = ES_Splitting;

      // use qt buttons instead of vtk buttons
      this->m_parentWindow->ui->splittingButtonsContainer->show();
    }
    else
    {
      QMessageBox::critical(this->m_parentWindow,
                            "错误", "必须先加载上、下颌模型！");
      action->setChecked(false);
    }
  }
  else  // uncheck toggle button, finish the operation
  {
    QMessageBox::StandardButton rb =
        QMessageBox::question(this->m_parentWindow, "提示",
                              "退出会丢失当前的编辑状态，要继续吗？",
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::Yes);
    if(rb == QMessageBox::Yes)
    {
      splitToothDone();
      turnOffSplittingButtons(nullptr);
    }
  }
  this->m_renderWindow->Render();
}

void SceneWidget::splitToothDone()
{
  // hide all the splitting buttons
  this->m_parentWindow->ui->splittingButtonsContainer->hide();

  // uncheck the action
  this->m_parentWindow->ui->actionSplitTooth->setChecked(false);

  this->m_editState = ES_None;
}


void SceneWidget::moveTooth(bool checked)
{
  QObject * object = QObject::sender();
  QAction * action = qobject_cast<QAction *>(object);

  if(this->m_modelState.mandibleSplitted
     && this->m_modelState.maxillaSplitted)
  {
    // check if action is selected
    if(checked) // checked
    {
      this->m_parentWindow->ui->dockWidget->show();

      // show moving buttons panel
      this->m_parentWindow->ui->movingButtonsContainer->show();

      if(m_movingLastProperty == nullptr)
      {
        vVSP(vtkProperty, this->m_movingLastProperty);
      }

      this->m_interactorStyle->MovingOn();
    }
    else
    {
      this->m_parentWindow->ui->dockWidget->hide();
      this->m_editState = ES_None;
    }
  }
  else
  {
    QMessageBox::critical(this->m_parentWindow,
                          "错误", "必须已经完成牙齿分离");
    action->setChecked(false);
  }

  this->m_renderWindow->Render();
}

void SceneWidget::doMarkActor(std::string mark, vtkActor * actor)
{
  if(this->m_modelState.mandibleVisible)
  {
    this->m_markingActorMapMandible[mark] = actor;
    this->m_actorMarkingMapMandible[actor] = mark;
  }
  else if(this->m_modelState.maxillaVisible)
  {
    this->m_markingActorMapMaxilla[mark] = actor;
    this->m_actorMarkingMapMaxilla[actor] = mark;
  }

  vDNSP(vtkStringArray, nameArray);
  nameArray->SetName("marking");
  nameArray->InsertNextValue(mark);

  vtkDataSet * dataSet = actor->GetMapper()->GetInput();
  dataSet->GetFieldData()->AddArray(nameArray);
}

void SceneWidget::doUnmarkActor(std::string mark, vtkActor *actor)
{
  if(this->m_modelState.mandibleVisible)
  {
    this->m_actorMarkingMapMandible.erase(actor);
    this->m_markingActorMapMandible.erase(mark);
  }
  else if(this->m_modelState.maxillaVisible)
  {
    this->m_actorMarkingMapMaxilla.erase(actor);
    this->m_markingActorMapMaxilla.erase(mark);
  }

  vtkDataSet * dataSet = actor->GetMapper()->GetInput();
  dataSet->GetFieldData()->RemoveArray("marking");
}

void SceneWidget::markActor(std::string mark, vtkActor * actor)
{
  if(this->checkActorMarked(actor)) // actor already marked
  {
    std::string marked = this->getMark(actor);
    if(mark != marked) // mark changed
    {
      if(this->checkMarkingExists(mark, marked) == false)
      {
        doUnmarkActor(marked, actor);
        doMarkActor(mark, actor);
      }
    }
  }
  else // not marked before
  {
    if(this->checkMarkingExists(mark) == false)
    {
      doMarkActor(mark, actor);
    }
  }
}

void SceneWidget::unmarkActor(vtkActor * actor)
{
  std::string mark = this->getMark(actor);
  doUnmarkActor(mark, actor);
}

void SceneWidget::movingComboBoxActivated(int index)
{
  if(index != 0)
  {
    std::string marking = this->m_parentWindow->ui->comboBox->currentText().toStdString();

    vtkActor * actor = this->m_markingActorMapMandible[marking];

    if(actor == nullptr)
    {
      actor = this->m_markingActorMapMaxilla[marking];
    }

    if(actor != nullptr)
    {
      this->movingAddPicking(actor);
    }
  }

  this->m_renderWindow->Render();
}

void SceneWidget::moveToothY()
{
  if(this->m_movingLastActor != nullptr)
  {
    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    double min[3];
    coordinateVectors->GetTuple(1, min);

    if(m_modelState.mandibleVisible)
    {
      moveLastActor(min, -1);
    }
    else
    {
      moveLastActor(min, 1);
    }

  }
}

void SceneWidget::moveToothY2()
{
  if(this->m_movingLastActor != nullptr)
  {
    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    double min[3];
    coordinateVectors->GetTuple(1, min);

    if(m_modelState.mandibleVisible)
    {
      moveLastActor(min, 1);
    }
    else
    {
      moveLastActor(min, -1);
    }
  }
}

void SceneWidget::moveToothX()
{
  if(this->m_movingLastActor != nullptr)
  {
    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    double max[3];
    coordinateVectors->GetTuple(0, max);

    moveLastActor(max, 1);
  }
}

void SceneWidget::moveToothX2()
{
  if(this->m_movingLastActor != nullptr)
  {
    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    double max[3];
    coordinateVectors->GetTuple(0, max);

    moveLastActor(max, -1);
  }
}

void SceneWidget::moveToothZ()
{
  if(this->m_movingLastActor != nullptr)
  {
    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    double mid[3];
    coordinateVectors->GetTuple(2, mid);

    moveLastActor(mid, 1);
  }
}

void SceneWidget::moveToothZ2()
{
  if(this->m_movingLastActor != nullptr)
  {
    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    double mid[3];
    coordinateVectors->GetTuple(2, mid);

    moveLastActor(mid, -1);
  }
}

void SceneWidget::rotateToothX()
{
  if(this->m_movingLastActor != nullptr)
  {
    double max[3];
    double mid[3];

    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    coordinateVectors->GetTuple(0, max);
    coordinateVectors->GetTuple(2, mid);

    double origin[3];
    origin[0] = - max[0];
    origin[1] = - max[1];
    origin[2] = - max[2];


    rotateLastActor(origin, mid, 1);
  }
}

void SceneWidget::rotateToothX2()
{
  if(this->m_movingLastActor != nullptr)
  {
    double max[3];
    double mid[3];

    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    coordinateVectors->GetTuple(0, max);
    coordinateVectors->GetTuple(2, mid);

    double origin[3];
    origin[0] = - max[0];
    origin[1] = - max[1];
    origin[2] = - max[2];

    rotateLastActor(origin, mid, -1);
  }
}

void SceneWidget::rotateToothY()
{
  if(this->m_movingLastActor != nullptr)
  {
    double max[3];

    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    coordinateVectors->GetTuple(0, max);

    double origin[3];
    origin[0] = - max[0];
    origin[1] = - max[1];
    origin[2] = - max[2];

    rotateLastActor(origin, max, 1);
  }
}

void SceneWidget::rotateToothY2()
{
  if(this->m_movingLastActor != nullptr)
  {
    double max[3];

    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    coordinateVectors->GetTuple(0, max);

    double origin[3];
    origin[0] = - max[0];
    origin[1] = - max[1];
    origin[2] = - max[2];

    rotateLastActor(origin, max, -1);
  }
}

void SceneWidget::rotateToothZ()
{
  if(this->m_movingLastActor != nullptr)
  {
    double max[3];
    double min[3];
    double mid[3];

    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    coordinateVectors->GetTuple(0, max);
    coordinateVectors->GetTuple(1, min);

    double origin[3];
    origin[0] = - max[0];
    origin[1] = - max[1];
    origin[2] = - max[2];

    rotateLastActor(origin, min, 1);
  }
}

void SceneWidget::rotateToothZ2()
{
  if(this->m_movingLastActor != nullptr)
  {
    double max[3];
    double min[3];
    double mid[3];

    vtkDataArray * coordinateVectors = getCoordinateVectors(this->m_movingLastActor);
    coordinateVectors->GetTuple(0, max);
    coordinateVectors->GetTuple(1, min);

    double origin[3];
    origin[0] = - max[0];
    origin[1] = - max[1];
    origin[2] = - max[2];

    rotateLastActor(origin, min, -1);
  }
}

bool SceneWidget::checkActorMarked(vtkActor * actor)
{
  if(this->m_modelState.mandibleVisible)
  {
    std::map<vSP<vtkActor>, std::string>::const_iterator iter
      = this->m_actorMarkingMapMandible.find(actor);
    return iter != this->m_actorMarkingMapMandible.end();
  }
  else if(this->m_modelState.maxillaVisible)
  {
    std::map<vSP<vtkActor>, std::string>::const_iterator iter
      = this->m_actorMarkingMapMaxilla.find(actor);
    return iter != this->m_actorMarkingMapMaxilla.end();
  }

  assert(false);
  return false;
}

bool SceneWidget::checkMarkingExists(std::string mark, std::string originalMark)
{
  if(this->m_modelState.mandibleVisible)
  {
    std::map<std::string, vSP<vtkActor>>::const_iterator findResult =
      this->m_markingActorMapMandible.find(mark);

    // marking duplicated
    if(findResult != this->m_markingActorMapMandible.end())
    {
      QMessageBox::critical(this->m_parentWindow,
                            "错误", "该标记已使用");
      this->resetChooseMarking(originalMark);

      return true;
    }
    return false;
  }
  else if(this->m_modelState.maxillaVisible)
  {
    std::map<std::string, vSP<vtkActor>>::const_iterator findResult =
      this->m_markingActorMapMaxilla.find(mark);

    // marking duplicated
    if(findResult != this->m_markingActorMapMaxilla.end())
    {
      QMessageBox::critical(this->m_parentWindow,
                            "错误", "该标记已使用");
      this->resetChooseMarking(originalMark);

      return true;
    }
    return false;
  }

  assert(false);
  return false;
}

std::string SceneWidget::getMark(vtkActor * actor)
{
  if(this->m_modelState.mandibleVisible)
  {
    std::string marked = this->m_actorMarkingMapMandible[actor];
    return marked;
  }
  else if(this->m_modelState.maxillaVisible)
  {
    std::string marked = this->m_actorMarkingMapMaxilla[actor];
    return marked;
  }

  assert(false);
  return std::string();
}

//////////////////////////event/////////////////////////

void addConnectionPressE(vtkObject * caller, unsigned long eId, void * clientData, void * callData)
{
  SceneWidget * scene = qobject_cast<SceneWidget *>((QObject *)clientData);
  scene->addConnectionPressEInternal();
}

void addConnectionPressEnter(vtkObject * caller, unsigned long eId, void * clientData, void * callData)
{
  SceneWidget * scene = qobject_cast<SceneWidget *>((QObject *)clientData);
  scene->addConnectionPressEnterInternal();
}

void updateProgress(vtkObject * caller, unsigned long eId, void * clientData, void * callData)
{
  double * progress = (double *)callData;
  SceneWidget * scene = qobject_cast<SceneWidget *>((QObject *)clientData);
  scene->updateProgressInternal(* progress);
}

void combiningPressEnter(vtkObject * caller, unsigned long eId, void * clientData, void * callData)
{
  SceneWidget * scene = qobject_cast<SceneWidget *>((QObject *)clientData);
  scene->combiningPressEnterInternal();
}


