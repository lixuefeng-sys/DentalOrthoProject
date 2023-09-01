#include "FloatForm.h"
#include "ui_FloatForm.h"

#include <iostream>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkActor.h>
#include <vtkFieldData.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkMapper.h>

#include "vtkCommon.h"

FloatForm::FloatForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FloatForm)
{
    this->ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    this->hideChooseMarking();

    QObject::connect(this->ui->chooseMarking,
                     QOverload<int>::of(&QComboBox::activated),
                     this, &FloatForm::markingComboBoxActivated);
}

FloatForm::~FloatForm()
{
    delete this->ui;
}

void FloatForm::focusInEvent(QFocusEvent * event)
{
  this->parentWidget()->focusWidget();

  std::cout << "gain focus" << std::endl;
}

void FloatForm::showChooseMarking(int x, int y, vtkActor * actor)
{
  if(actor == nullptr)
  {
    return;
  }

  this->m_currentActor = actor;
  vtkDataSet * dataSet = actor->GetMapper()->GetInput();

  vtkFieldData * fieldData = dataSet->GetFieldData();
  // vtkStringArray is not a data array
  vtkAbstractArray * array = fieldData->GetAbstractArray("marking");
  if(array == nullptr)
  {
    this->ui->chooseMarking->setCurrentIndex(0);
  }
  else
  {
    vtkStringArray * nameArray = vtkStringArray::SafeDownCast(array);
    vtkStdString value = nameArray->GetValue(0);
    this->ui->chooseMarking->setCurrentText(value.c_str());
  }

  const QRect geo = this->ui->chooseMarking->geometry();
  this->ui->chooseMarking->setGeometry(x,
                                       y - geo.height() - 10,
                                       geo.width(), geo.height());
  this->ui->chooseMarking->show();
}

void FloatForm::hideChooseMarking()
{
  this->ui->chooseMarking->hide();
}

void FloatForm::resetChooseMarking(std::string originalMark)
{
  if(originalMark.empty())
  {
    this->ui->chooseMarking->setCurrentIndex(0);
  }
  else
  {
    this->ui->chooseMarking->setCurrentText(originalMark.c_str());
  }
}

void FloatForm::markingComboBoxActivated(int index)
{
  if(this->m_currentActor != nullptr)
  {
    if(index == 0)  // remove marking??
    {
      emit this->unmarkActor(this->m_currentActor);
    }
    else
    {
      std::string markingText =
        this->ui->chooseMarking->currentText().toStdString();


      emit this->markActor(markingText, this->m_currentActor);
    }

  }
}
