#ifndef SceneWidget_H
#define SceneWidget_H

#include <QVTKOpenGLNativeWidget.h>

#include <vtkActor.h>

#include <string>
#include <vector>


#include "vtkCommon.h"
#include "QCommon.h"
#include "ScalarRangeFilter.h"

class QComboBox;
class QPushButton;
class vtkTransform;
class vtkProperty;
class InteractorStyleTrackballCameraPicking;
class ScalarSeparationFilter;
class PointDataFilter;
class PolyDataPointComplexityFilter;
class vtkCallbackCommand;
class PolyDataPointPointPlacer;
class ScalarPointToEdgeExtractor;
class vtkPolyData;
class ConstrainedPlaneWidget;
class vtkPlaneSource;
class TextButtonRepresentation2D;
class vtkNamedColors;
class vtkSphereSource;
class vtkPolyDataMapper;
class vtkActor;
class vtkCameraOrientationWidget;
class vtkInteractorStyleTrackballCamera;
class ModelEditorWindow;
class RefPointsWidget;
class vtkPointHandleRepresentation3D;
class vtkSeedRepresentation;
class vtkPolygonalSurfacePointPlacer;
class PolySurfacePointHandleRepresentation;
class vtkCurvatures;
class vtkSliderRepresentation2D;
class vtkSliderWidget;
class vtkExtractEdges;
class vtkDijkstraGraphGeodesicPath;
class vtkAppendDataSets;
class NonDestructiveConnectivityFilter;
class ErodeFilter;
class vtkPolyDataConnectivityFilter;
class vtkColorSeries;
class vtkProgressBarWidget;
class vtkProgressBarRepresentation;
class QProgressDialog;
class vtkProp3DCollection;
class vtkDoubleArray;
class vtkDataArray;
class vtkAxesActor;
class vtkArrowSource;
class vtkTransformPolyDataFilter;

void addRefPlanePressEnter(vtkObject * caller, unsigned long eId, void * clientData, void * callData);

void showAddConnection(vtkObject * caller, unsigned long eId, void * clientData, void * callData);

void addConnectionPressE(vtkObject * caller, unsigned long eId, void * clientData, void * callData);
void addConnectionPressEnter(vtkObject * caller, unsigned long eId, void * clientData, void * callData);

void combiningPressEnter(vtkObject * caller, unsigned long eId, void * clientData, void * callData);

void updateProgress(vtkObject * caller, unsigned long eId, void * clientData, void * callData);

class SceneWidget : public QVTKOpenGLNativeWidget
{
  Q_OBJECT
public:
  typedef enum {
    AD_Forward,
    AD_Backward,
    AD_Skip
  } ActionDirection;

  typedef enum {
    ES_None,
    ES_RefPlane,
    ES_Splitting,
    ES_Moving
  } EditState;

  typedef struct {
    bool mandibleLoaded;
    bool maxillaLoaded;

    bool mandibleVisible;
    bool maxillaVisible;

    bool mandibleSplitted;
    bool maxillaSplitted;
  } ModelState;

  explicit SceneWidget(QWidget * parent = nullptr);

  //////color util
  static vSP<vtkNamedColors> s_colors;

  void setParentWindow(ModelEditorWindow * window);

public:
  //////////////////////////////internal handler for events//////////////////////////

  // splitting tooth
    // filter plane
  void showFilterPlaneOn();
  void showFilterPlaneDo(vtkActor * actorOriginal, vSP<ConstrainedPlaneWidget> & filterPlaneWidget);
  void showFilterPlaneOff(bool hideAll = false);

    // filter curvature
  void showFilterCurvatureOn();
  void showFilterCurvatureDo(vtkPolyData * polydataOriginal,
                             vSP<vtkCurvatures> & curvatureFilter,
                             vSP<ScalarRangeFilter> & curvatureRangeFilter,
                             vSP<vtkExtractEdges> & extractEdgesFilter,
                             vSP<ScalarPointToEdgeExtractor> & scalarPointEdgeExtractor,
                             vSP<vtkPolyDataMapper> & curvatureMapper,
                             vSP<vtkActor> & curvatureActor,
                             ConstrainedPlaneWidget * filterPlaneWidget,
                             ScalarRangeFilter::FilterZType filterZType);
  void showFilterCurvatureOff(bool hideAll = false);

    // add connection
  void addConnectionPointPickingOn();
  void addConnectionPointPickingOff();
  void showAddConnectionOn();
  void showAddConnectionDo(vtkActor * actorOriginal,
                           vtkActor * curvatureActor,
                           vSP<RefPointsWidget> & addConnectionWidget,
                           vSP<PolySurfacePointHandleRepresentation> & addConnectionHandleRep,
                           vSP<vtkSeedRepresentation> & addConnectionSeedRep,
                           vSP<PolyDataPointPointPlacer> & addConnectionPointPlacer,
                           vSP<vtkActor> & additionalConnectionActor);
  void showAddConnectionOff(bool hideAll = false);
  void addConnectionPressEInternal();
  void addConnectionPressEnterInternal();
  void addConnectionPressEnterDo(vtkPolyData * polydataOriginal,
                                 RefPointsWidget * addConnectionWidget,
                                 PolyDataPointPointPlacer * addConnectionPointPlacer,
                                 vSP<vtkPolyData> & additionalConnection,
                                 vSP<vtkPolyDataMapper> & additionalConnectionMapper,
                                 vSP<vtkActor> & additionalConnectionActor,
                                 vSP<vtkDijkstraGraphGeodesicPath> & dijkstra,
                                 std::vector<std::vector<vSP<vtkLine>> *> & additionalConnectionStack);

    // remove connection
  void showRemoveConnectionOn();
  void showRemoveConnectionDo(vtkPolyData * polydataOriginal,
                              vtkActor * actorOriginal,
                              vSP<NonDestructiveConnectivityFilter> & largestConnFilter,
                              vSP<ErodeFilter> & erodeFilter,
                              vSP<vtkPolyData> & removedConnection,
                              vSP<vtkPolyDataMapper> & removedConnectionMapper,
                              vSP<vtkActor> & removedConnectionActor,
                              ScalarPointToEdgeExtractor * scalarPointEdgeExtractor,
                              vtkActor * curvatureActor,
                              vtkPolyData * additionalConnection,
                              vtkActor * additionalConnectionActor);
  void showRemoveConnectionOff(bool hideAll = false);
  void addConnectionPressDelInternal();

    // separation
  void showSeparationOn();
  void showSeparationDo(vtkPolyData * polydataOriginal,
                        vtkActor * actorOriginal,
                        vSP<ScalarSeparationFilter> & separationFilter,
                        vSP<PointDataFilter> & pointDataFilter,
                        vtkPolyData * removedConnection,
                        std::vector<vSP<vtkActor>> & separationActors);
  void showSeparationOff(bool hideAll = false);

  void updateProgressInternal(double progress);

    // combining
  void showCombiningOn();
  void showCombiningOff(bool hideAll = false);
  void combiningRectSelectionOn();
  void combiningRectSelectionOff();
  void combiningClickSelectionOn();
  void combiningClickSelectionOff();
  void combiningAddPicking(vtkActor * actor);
  void combiningRemovePicking(vtkActor * actor);
  void combiningPressEnterInternal();
  void combiningPressEnterDo(std::map<vSP<vtkActor>, vSP<vtkProperty>> & oldPropertyMap,
                             std::vector<vSP<vtkActor>> & separationActors,
                             std::vector<vSP<vtkActor>> & combiningRemovedActors,
                             std::vector<vSP<vtkActor>> & combiningMergedActors);

    // marking
  void showMarkingOn();
  void showMarkingOff(bool hideAll = false);
  void markingClickSelectionOn();
  void markingClickSelectionOff();

  void markingAddPicking(vtkActor * actor);
//  void markingRemovePicking(vtkActor * actor);

  void showChooseMarking(int x, int y, vtkActor * actor);
  void showChooseMarkingDo(vtkStdString mark, QComboBox * chooseMarking, int x, int y);
  void hideChooseMarking();
  void resetChooseMarking(std::string originalMark);

    // finish
  void showFinishSplitOn();
  void showFinishSplitDo(std::vector<vSP<vtkActor>> & separationActors,
                    std::map<vSP<vtkActor>, std::string> & actorMarkingMap,
                    std::map<std::string, vSP<vtkActor>> & markingActorMap,
                    bool & splittedState);
  void showFinishSplitOff();
//  void finishStore

    // turn off splitting buttons
  ActionDirection turnOffSplittingButtons(QPushButton * button);
  void turnOnSplittingButtons(QPushButton * button);

  //////moving
  void movingClickSelectionOn();
  void movingClickSelectionOff();

  void movingAddPicking(vtkActor * actor);
  void moveLastActor(double * xyz, int direction);
  void rotateLastActor(double * origin, double * xyz, int direction);
protected:
  vDSP(vtkColorSeries, m_greenColors);
  vDSP(vtkColorSeries, m_redColors);
  vDSP(vtkColorSeries, m_blueColors);

  void initColorSeries();

  ModelEditorWindow * m_parentWindow;
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

  //////COW
  vDSP(vtkCameraOrientationWidget, m_camOrientManipulator);

  /////Progress dialog
  QDSP(QProgressDialog, m_progressDialog);
  // Progress command
  vDSP(vtkCallbackCommand, m_progressCommand);

  /////RefPointWidget
  vDSP(RefPointsWidget, m_refPointWidget);
  vDSP(PolySurfacePointHandleRepresentation, m_refHandleRep);
  vDSP(vtkSeedRepresentation, m_refRep);
  vDSP(vtkPolygonalSurfacePointPlacer, m_pointPlacer);

  /////RefPlane
  vDSP(vtkPlaneSource, m_refPlaneMandible);
  vDSP(vtkPolyDataMapper, m_refPlaneMandibleMapper);
  vDSP(vtkActor, m_refPlaneMandibleActor);

//  vDSP(vtkPlaneSource, m_refPlaneMaxilla);
//  vDSP(vtkPolyDataMapper, m_refPlaneMaxillaMapper);
//  vDSP(vtkActor, m_refPlaneMaxillaActor);

  /////FilterPlane
  vDSP(ConstrainedPlaneWidget, m_filterPlaneMandibleWidget);
  vDSP(ConstrainedPlaneWidget, m_filterPlaneMaxillaWidget);

  /////FilterCurvature
  vDSP(vtkCurvatures, m_curvatureFilterMandible);
  vDSP(ScalarRangeFilter, m_curvatureRangeFilterMandible);

  vDSP(vtkExtractEdges, m_extractEdgesFilterMandible);
  vDSP(ScalarPointToEdgeExtractor, m_scalarPointEdgeExtractorMandible);

  vDSP(vtkPolyDataMapper, m_curvatureMapperMandible);
  vDSP(vtkActor, m_curvatureActorMandible);

  // min slider
  vDSP(vtkSliderWidget, m_minSliderWidgetMandible);
  vDSP(vtkSliderRepresentation2D, m_minSliderRepMandible);
  // max slider
  vDSP(vtkSliderWidget, m_maxSliderWidgetMandible);
  vDSP(vtkSliderRepresentation2D, m_maxSliderRepMandible);

  vDSP(vtkCurvatures, m_curvatureFilterMaxilla);
  vDSP(ScalarRangeFilter, m_curvatureRangeFilterMaxilla);

  vDSP(vtkExtractEdges, m_extractEdgesFilterMaxilla);
  vDSP(ScalarPointToEdgeExtractor, m_scalarPointEdgeExtractorMaxilla);

  vDSP(vtkPolyDataMapper, m_curvatureMapperMaxilla);
  vDSP(vtkActor, m_curvatureActorMaxilla);

  // min slider
  vDSP(vtkSliderWidget, m_minSliderWidgetMaxilla);
  vDSP(vtkSliderRepresentation2D, m_minSliderRepMaxilla);
  // max slider
  vDSP(vtkSliderWidget, m_maxSliderWidgetMaxilla);
  vDSP(vtkSliderRepresentation2D, m_maxSliderRepMaxilla);

  /////Add connection
  vDSP(vtkCallbackCommand, m_addConnectionPressECallback);
  vDSP(vtkCallbackCommand, m_addConnectionPressEnterCallback);

  vDSP(vtkDijkstraGraphGeodesicPath, m_dijkstraMandible);
  // add connection ref points widget
  vDSP(RefPointsWidget, m_addConnectionWidgetMandible);
  vDSP(PolySurfacePointHandleRepresentation, m_addConnectionHandleRepMandible);
  vDSP(vtkSeedRepresentation, m_addConnectionSeedRepMandible);
  vDSP(PolyDataPointPointPlacer, m_addConnectionPointPlacerMandible);

  vDSP(vtkDijkstraGraphGeodesicPath, m_dijkstraMaxilla);
  // add connection ref points widget
  vDSP(RefPointsWidget, m_addConnectionWidgetMaxilla);
  vDSP(PolySurfacePointHandleRepresentation, m_addConnectionHandleRepMaxilla);
  vDSP(vtkSeedRepresentation, m_addConnectionSeedRepMaxilla);
  vDSP(PolyDataPointPointPlacer, m_addConnectionPointPlacerMaxilla);

  // additional connection
  vDSP(vtkPolyData, m_additionalConnectionMandible);
  vDSP(vtkPolyDataMapper, m_additionalConnectionMapperMandible);
  vDSP(vtkActor, m_additionalConnectionActorMandible);

  // additional connection
  vDSP(vtkPolyData, m_additionalConnectionMaxilla);
  vDSP(vtkPolyDataMapper, m_additionalConnectionMapperMaxilla);
  vDSP(vtkActor, m_additionalConnectionActorMaxilla);

  std::vector<std::vector<vSP<vtkLine>> *> m_additionalConnectionStackMandible;
  std::vector<std::vector<vSP<vtkLine>> *> m_additionalConnectionStackMaxilla;

  /////Remove connection
  vDSP(NonDestructiveConnectivityFilter, m_largestConnFilterMandible);

  vDSP(NonDestructiveConnectivityFilter, m_largestConnFilterMaxilla);

  // removed connection
  vDSP(vtkPolyData, m_removedConnectionMandible);
  vDSP(vtkPolyDataMapper, m_removedConnectionMapperMandible);
  vDSP(vtkActor, m_removedConnectionActorMandible);

  vDSP(vtkPolyData, m_removedConnectionMaxilla);
  vDSP(vtkPolyDataMapper, m_removedConnectionMapperMaxilla);
  vDSP(vtkActor, m_removedConnectionActorMaxilla);

  vDSP(PolyDataPointComplexityFilter, m_complexityFilterMandible);
  vDSP(ErodeFilter, m_erodeFilterMandible);

  vDSP(PolyDataPointComplexityFilter, m_complexityFilterMaxilla);
  vDSP(ErodeFilter, m_erodeFilterMaxilla);

  /////Seperation
  // separation connection
  vDSP(vtkPolyData, m_separationMandible);
  vDSP(vtkPolyDataMapper, m_separationMapperMandible);
  vDSP(vtkActor, m_separationActorMandible);

  vDSP(PointDataFilter, m_pointDataFilterMandible);
  vDSP(ScalarSeparationFilter, m_separationFilterMandible);

  std::vector<vSP<vtkActor>> m_separationActorsMandible;

  // separation connection
  vDSP(vtkPolyData, m_separationMaxilla);
  vDSP(vtkPolyDataMapper, m_separationMapperMaxilla);
  vDSP(vtkActor, m_separationActorMaxilla);

  vDSP(PointDataFilter, m_pointDataFilterMaxilla);
  vDSP(ScalarSeparationFilter, m_separationFilterMaxilla);

  std::vector<vSP<vtkActor>> m_separationActorsMaxilla;

  /////Combine
  int m_currentHighlightColor {0};
  std::map<vSP<vtkActor>, vSP<vtkProperty>> m_oldPropertyMapMandible;
  std::map<vSP<vtkActor>, vSP<vtkProperty>> m_oldPropertyMapMaxilla;

  std::vector<vSP<vtkActor>> m_combiningRemovedActorsMandible;
  std::vector<vSP<vtkActor>> m_combiningRemovedActorsMaxilla;

  std::vector<vSP<vtkActor>> m_combiningMergedActorsMandible;
  std::vector<vSP<vtkActor>> m_combiningMergedActorsMaxilla;

  vDSP(vtkCallbackCommand, m_combiningPressEnterCallback);
  int m_currentCombinedColor {0};

  vDSP(vtkAppendDataSets, m_combiningAppenderMandible);
  vDSP(vtkAppendDataSets, m_combiningAppenderMaxilla);

  /////Marking
  vDSP(vtkProperty, m_markingLastProperty);
  vtkActor * m_markingLastActor;

  std::map<std::string, vSP<vtkActor>> m_markingActorMapMandible;
  std::map<vSP<vtkActor>, std::string> m_actorMarkingMapMandible;

  std::map<std::string, vSP<vtkActor>> m_markingActorMapMaxilla;
  std::map<vSP<vtkActor>, std::string> m_actorMarkingMapMaxilla;

//  bool m_finishedMandible {false};
//  bool m_finishedMaxilla {false};

  //////marking each tooth
  vtkDataArray * getCoordinateVectors(vtkActor * actor);

  vDSP(vtkAxesActor, m_toothAxesActor);

  vDSP(vtkArrowSource, m_toothAxes);
  vDSP(vtkPolyData, m_toothAxesPolyData);
  vDSP(vtkTransformPolyDataFilter, m_toothXTransform);
  vDSP(vtkTransformPolyDataFilter, m_toothYTransform);
  vDSP(vtkTransformPolyDataFilter, m_toothZTransform);
  vDSP(vtkPolyDataMapper, m_toothXAxesMapper);
  vDSP(vtkPolyDataMapper, m_toothYAxesMapper);
  vDSP(vtkPolyDataMapper, m_toothZAxesMapper);
  vDSP(vtkActor, m_toothXAxesActor);
  vDSP(vtkActor, m_toothYAxesActor);
  vDSP(vtkActor, m_toothZAxesActor);

  //moving
  vDSP(vtkProperty, m_movingLastProperty);
  vtkActor * m_movingLastActor;

  //////other pipeline
  vDSP(vtkGenericOpenGLRenderWindow, m_renderWindow);
  vDSP(vtkRenderer, m_renderer);

  /////interactor
  vDSP(vtkRenderWindowInteractor, m_interactor);
  vDSP(InteractorStyleTrackballCameraPicking, m_interactorStyle);

  EditState m_editState {ES_None};

  ModelState m_modelState {
    .mandibleLoaded = false,
    .maxillaLoaded = false,

    .mandibleVisible = false,
    .maxillaVisible = false,

    .mandibleSplitted = false,
    .maxillaSplitted = false
  };

  // temporarily remember model visibility
  bool m_mandibleVisible {false};
  bool m_maxillaVisible {false};

  bool checkActorMarked(vtkActor * actor);
  bool checkMarkingExists(std::string mark, std::string originalMark = std::string());
  std::string getMark(vtkActor * actor);
  void doMarkActor(std::string mark, vtkActor * actor);
  void doUnmarkActor(std::string mark, vtkActor * actor);

  QList<QPushButton *> m_splittingButtons;
  QList<std::function<void()>> m_splittingShowOnFunctions;
  QList<std::function<void(bool)>> m_splittingShowOffFunctions;

  void initializeSplittingButtons();

  void setMandibleVisibility(bool visible);
  void setMaxillaVisibility(bool visible);

  bool getMandibleVisibility();
  bool getMaxillaVisibility();

  void createProgressDialog();

public slots:
  // define ref plane
  void showRefPlaneInternal(bool checked);

  void addRefPoints();

  // hide mandible/manxilla
  void hideMandible(bool checked);
  void hideMaxilla(bool checked);


  // splitting tooth
    // main action
  void splitTooth(bool checked);
  void splitToothDone();

    // splitting: subbuttons
  void showSplittingSubbuttonsInternal(bool checked);
    // splitting: special subbuttons
  void showFinishSplitInternal();

    // curvature slider change
  void minCurvatureSliderChangeInternal(float value);
  void maxCurvatureSliderChangeInternal(float value);

    // add connection toggle point picking
  void addConnectionPointPickingToggleInternal(bool checked);
  void addConnectionUndoInternal();

    // combining toggle picking style
  void combiningRectSelectionInternal(bool checked);
  void combiningClickSelectionInternal(bool checked);

    // marking toggle picking
  void markingClickSelectionInternal(bool checked);

    // marking: selected marking
  void markingComboBoxActivated(int index);

  void markActor(std::string mark, vtkActor * actor);
  void unmarkActor(vtkActor * actor);


  // moving tooth
    // main action
  void moveTooth(bool checked);
    // moving: toggle picking
  void movingClickSelectionInternal(bool checked);
    // moving: comboBox picking
  void movingComboBoxActivated(int index);
    // moving: move tooth
  void moveToothY();
  void moveToothY2();
  void moveToothX();
  void moveToothX2();
  void moveToothZ();
  void moveToothZ2();
    // moving: rotate tooth
  void rotateToothX();
  void rotateToothX2();
  void rotateToothY();
  void rotateToothY2();
  void rotateToothZ();
  void rotateToothZ2();


  // load model
  void loadMandible(QString & filename);
  void loadMaxilla(QString & filename);

private:
  friend class InteractorStyleTrackballCameraPicking;
};

#endif // SceneWidget_H
