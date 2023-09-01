#ifndef MainWindow_H
#define MainWindow_H

#include <vtkSmartPointer.h>

#include <QList>
#include <QMainWindow>

/*
 * See "The Single Inheritance Approach" in this link:
 * [Using a Designer UI File in Your C++
 * Application](https://doc.qt.io/qt-5/designer-using-a-ui-file.html)
 */

class OverlapLayout;

class QPushButton;
namespace Ui {
class ModelEditorWindow;
}

class FloatForm;

class ModelEditorWindow : public QMainWindow
{
  Q_OBJECT
public:
  // Constructor/Destructor
  explicit ModelEditorWindow(QWidget* parent = nullptr);
  virtual ~ModelEditorWindow();

  // events
  void closeEvent(QCloseEvent * event) override;

  void loadMandible(QString filename);
  void loadMaxilla(QString filename);

  void setPatientId(qint64 patientId)
  {
    this->m_patientId = patientId;
  }

private:
  // Designer form
  Ui::ModelEditorWindow * ui;

  OverlapLayout * m_overlapLayout;

  qint64 m_patientId;

  void customizeLayout();
  void bindSignals();

  friend class SceneWidget;
  friend class InteractorStyleTrackballCameraPicking;

signals:
  void closeWindow();

public slots:
  void slotExit();

  //! Show the 'Open file...' dialog
  void showOpenFileDialog();
};

#endif // MainWindow_H
