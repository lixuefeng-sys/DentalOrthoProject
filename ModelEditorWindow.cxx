#include "ModelEditorWindow.h"
#include "FloatForm.h"
#include "ui_ModelEditorWindow.h"
#include "SceneWidget.h"
#include "OverlapLayout.h"
#include "QtUtil.h"
#include "AbsoluteLayout.h"

#include <QTest>

#include <QCompleter>
#include <QFileDialog>

// Constructor

ModelEditorWindow::ModelEditorWindow(
    QWidget * parent)
  : QMainWindow(parent), ui(new Ui::ModelEditorWindow)
{
  this->ui->setupUi(this);

  customizeLayout();

  bindSignals();

  /*
  QString mandible("41807738_lprofile_occlusion_l.stl");
  this->ui->qvtkWidget->loadMandible(mandible);

  QString maxilla("41807738_lprofile_occlusion_u.stl");
  this->ui->qvtkWidget->loadMaxilla(maxilla);
  */
}

void ModelEditorWindow::customizeLayout()
{
  // adjust layout
  this->ui->qvtkWidget->setParent(nullptr);
  this->ui->refPlaneButtons->setParent(nullptr);
  this->ui->splittingButtons->setParent(nullptr);
  this->ui->splittingCurvatureSliders->setParent(nullptr);
  this->ui->chooseMarkingMandible->setParent(nullptr);
  this->ui->chooseMarkingMaxilla->setParent(nullptr);
  this->ui->movingButtons->setParent(nullptr);

  this->m_overlapLayout = new OverlapLayout;

    // background layer: vtk render window
  QHBoxLayout * hLayout = new QHBoxLayout;
  hLayout->addWidget(this->ui->qvtkWidget);
  this->ui->qvtkWidget->setParentWindow(this);

  this->m_overlapLayout->AddLayout(hLayout);

    // add ref plane
  this->m_overlapLayout->AddLayout(this->ui->refPlaneButtons);

    // splitting buttons panel
  this->m_overlapLayout->AddLayout(this->ui->splittingButtons);
      // transparent background for buttons panel
  QPalette p = palette();
  p.setColor(QPalette::Window,Qt::blue);
  p.setColor(QPalette::Background, QColor(0xff, 0xff, 0xff, 0x32)); // rgba
  this->ui->splittingButtonsContainer->setPalette(p);

    // curvature filter sliders panel
  this->m_overlapLayout->AddLayout(this->ui->splittingCurvatureSliders);

    // tooth marking combox
  AbsoluteLayout * aLayout = new AbsoluteLayout;
  aLayout->addWidget(this->ui->chooseMarkingMandible);
  aLayout->addWidget(this->ui->chooseMarkingMaxilla);

  this->m_overlapLayout->AddLayout(aLayout);

    // moving tooth
  this->m_overlapLayout->AddLayout(this->ui->movingButtons);

  delete this->ui->centralWidget->layout();
  this->ui->centralWidget->setLayout(this->m_overlapLayout);

  this->ui->containerWidget->deleteLater();

  // initialize components
    // initialize ref plane panel
  this->ui->refPlaneButtonsContainer->hide();

    // initialize splitting tooth panel
  this->ui->addConnectionEnableBtn->hide();
  this->ui->addConnectionUndoBtn->hide();

  this->ui->combiningUndoBtn->hide();
  this->ui->combiningRectSelectionBtn->hide();
  this->ui->combiningClickSelectionBtn->hide();

  this->ui->markingClickSelectionBtn->hide();

  this->ui->splittingButtonsContainer->hide();

    // initialize curvature sliders
  QtUtil::hideLayout(this->ui->splittingCurvatureSliders);
  this->ui->curvatureRangeSlider->setRange(-20, 0);

    // initialize dock
  this->ui->dockWidget->hide();

    // initialize chooseMarking combobox
  this->ui->chooseMarkingMandible->hide();
  this->ui->chooseMarkingMaxilla->hide();
//  QCompleter * completer = this->ui->chooseMarking->completer();
//  completer->setMaxVisibleItems(16);

    // initialize moving buttons panel
  this->ui->movingButtonsContainer->hide();
}

void ModelEditorWindow::bindSignals()
{
  // main actions
    // exit
  connect(this->ui->actionExit, &QAction::triggered,
          this, &ModelEditorWindow::slotExit);
    // hide mandible
  connect(this->ui->actionHideMandible, &QAction::triggered,
          this->ui->qvtkWidget, &SceneWidget::hideMandible);
    // hide maxilla
  connect(this->ui->actionHideMaxilla, &QAction::triggered,
          this->ui->qvtkWidget, &SceneWidget::hideMaxilla);
    // define reference plane
  connect(this->ui->actionRefPoint, &QAction::triggered,
          this->ui->qvtkWidget, &SceneWidget::addRefPoints);
    // curvature analysis (splitting)
  connect(this->ui->actionSplitTooth, &QAction::triggered,
          this->ui->qvtkWidget, &SceneWidget::splitTooth);
    // move tooth
  connect(this->ui->actionMoveTooth, &QAction::triggered,
          this->ui->qvtkWidget, &SceneWidget::moveTooth);

  // show ref plane
  connect(this->ui->addRefPlaneBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::showRefPlaneInternal);

  // splitting tooth
  connect(this->ui->filterPlaneShowBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::showSplittingSubbuttonsInternal);
  connect(this->ui->filterCurvatureShowBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::showSplittingSubbuttonsInternal);
  connect(this->ui->addConnectionShowBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::showSplittingSubbuttonsInternal);
  connect(this->ui->removeConnectionShowBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::showSplittingSubbuttonsInternal);
  connect(this->ui->separationShowBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::showSplittingSubbuttonsInternal);
  connect(this->ui->combiningShowBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::showSplittingSubbuttonsInternal);
  connect(this->ui->markingShowBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::showSplittingSubbuttonsInternal);
  connect(this->ui->splittingDoneBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::showFinishSplitInternal);

    // curvature slider change
  connect(this->ui->curvatureRangeSlider, &DoubleSlider::minValueChanged,
          this->ui->qvtkWidget, &SceneWidget::minCurvatureSliderChangeInternal);
  connect(this->ui->curvatureRangeSlider, &DoubleSlider::maxValueChanged,
          this->ui->qvtkWidget, &SceneWidget::maxCurvatureSliderChangeInternal);

    // add connection: toggle point picking
  connect(this->ui->addConnectionEnableBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::addConnectionPointPickingToggleInternal);
    // add connection: undo
  connect(this->ui->addConnectionUndoBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::addConnectionUndoInternal);

    // combining: toggle picking mode
  connect(this->ui->combiningClickSelectionBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::combiningClickSelectionInternal);
  connect(this->ui->combiningRectSelectionBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::combiningRectSelectionInternal);

    // marking: toggle picking mode
  connect(this->ui->markingClickSelectionBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::markingClickSelectionInternal);

    // marking: choose combobox marking
  connect(this->ui->chooseMarkingMandible, QOverload<int>::of(&QComboBox::activated),
          this->ui->qvtkWidget, &SceneWidget::markingComboBoxActivated);
  connect(this->ui->chooseMarkingMaxilla, QOverload<int>::of(&QComboBox::activated),
          this->ui->qvtkWidget, &SceneWidget::markingComboBoxActivated);


  // moving tooth
    // moving: selection
  connect(this->ui->movingClickSelectionBtn, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::movingClickSelectionInternal);

    // moving: rotate
  connect(this->ui->btnRotateZPlus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::rotateToothZ2);
  connect(this->ui->btnRotateZMinus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::rotateToothZ);
  connect(this->ui->btnRotateYPlus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::rotateToothY2);
  connect(this->ui->btnRotateYMinus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::rotateToothY);
  connect(this->ui->btnRotateXPlus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::rotateToothX2);
  connect(this->ui->btnRotateXMinus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::rotateToothX);

    // moving: move
  connect(this->ui->btnMoveZPlus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::moveToothZ2);
  connect(this->ui->btnMoveZMinus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::moveToothZ);
  connect(this->ui->btnMoveYPlus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::moveToothY2);
  connect(this->ui->btnMoveYMinus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::moveToothY);
  connect(this->ui->btnMoveXPlus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::moveToothX2);
  connect(this->ui->btnMoveXMinus, &QPushButton::clicked,
          this->ui->qvtkWidget, &SceneWidget::moveToothX);

    // moving: reset
  // TODO::RainSia:: implement reset tooth position
}

ModelEditorWindow::~ModelEditorWindow()
{
  delete this->ui;
}

///////////////events/////////////////
void ModelEditorWindow::closeEvent(QCloseEvent * event)
{
  emit this->closeWindow();
}

void ModelEditorWindow::loadMandible(QString filename)
{
  this->ui->qvtkWidget->loadMandible(filename);
}

void ModelEditorWindow::loadMaxilla(QString filename)
{
  this->ui->qvtkWidget->loadMaxilla(filename);
}


///////////////slots//////////////////

void
ModelEditorWindow::showOpenFileDialog()
{
  QObject *object = QObject::sender();
  QAction *action = qobject_cast<QAction *>(object);


  QString filename = QFileDialog::getOpenFileName(
    this, tr("Open file"), "", "STL Files (*.stl)");

  // Open file
  QFile file(filename);

  // Return on Cancel
  if (!file.exists())
    return;

  if(action == this->ui->actionOpenMandibleFile) {
    this->ui->qvtkWidget->loadMandible(filename);
  } else if(action == this->ui->actionOpenMaxillaFile) {
    this->ui->qvtkWidget->loadMaxilla(filename);
  }
}

void ModelEditorWindow::slotExit()
{
  this->close();
}
