#ifndef FloatForm_H
#define FloatForm_H

#include <QWidget>

namespace Ui {
class FloatForm;
}

class vtkActor;
class ModelEditorWindow;

class FloatForm : public QWidget
{
    Q_OBJECT
public:
    explicit FloatForm(QWidget *parent = nullptr);
    ~FloatForm();

    void focusInEvent(QFocusEvent * event) override;

    void showChooseMarking(int x, int y, vtkActor * actor);
    void hideChooseMarking();
    void resetChooseMarking(std::string originalMark);

public slots:
    void markingComboBoxActivated(int index);

private:
    Ui::FloatForm *ui;
    vtkActor * m_currentActor {nullptr};
    friend class ModelEditorWindow;

signals:
    void markActor(std::string mark, vtkActor * actor);
    void unmarkActor(vtkActor * actor);
};

#endif // FloatForm_H
