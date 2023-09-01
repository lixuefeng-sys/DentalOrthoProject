
#include "InteractorStyleTrackballCameraPicking.h"
#include "ModelEditorWindow.h"
#include "SceneWidget.h"
#include "TextButtonRepresentation2D.h"
#include "ui_ModelEditorWindow.h"
#include "vtkAssemblyPath.h"
#include "vtkMapper.h"
#include "vtkRenderWindow.h"
#include "vtkUnsignedCharArray.h"
#include <vtkObjectFactory.h>

#include <vtkNew.h>
#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkPropPicker.h>
#include <vtkAreaPicker.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPoints.h>
#include <vtkProp3DCollection.h>
#include <vtkDataSet.h>
#include <vtkHardwareSelector.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkInformationObjectBaseKey.h>
#include <vtkInformation.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(InteractorStyleTrackballCameraPicking)

InteractorStyleTrackballCameraPicking::InteractorStyleTrackballCameraPicking()
  :vtkInteractorStyleTrackballCamera(),
    m_pickingStyle(PICKING_STYLE_NONE),
    StartPosition{0, 0},
    EndPosition{0, 0},
    Moving(0),
    PixelArray(vtkUnsignedCharArray::New()),
    m_lastPickedActor(nullptr),
    m_lastPickedProperty(vtkProperty::New()),
    m_propPicker(vtkPropPicker::New()),
    m_areaPicker(vtkAreaPicker::New()),
    m_hardwareSelector(vtkHardwareSelector::New()),
    m_isCombining(false),
    m_isMarking(false),
    m_isMoving(false)
{
}

InteractorStyleTrackballCameraPicking::~InteractorStyleTrackballCameraPicking()
{
  m_lastPickedProperty->Delete();
  m_propPicker->Delete();
  m_areaPicker->Delete();
}

void InteractorStyleTrackballCameraPicking::RectPick()
{
  // find rubber band lower left, upper right and center
  double rbcenter[3];
  const int* size = this->Interactor->GetRenderWindow()->GetSize();
  int min[2], max[2];
  min[0] =
    this->StartPosition[0] <= this->EndPosition[0] ? this->StartPosition[0] : this->EndPosition[0];
  if (min[0] < 0)
  {
    min[0] = 0;
  }
  if (min[0] >= size[0])
  {
    min[0] = size[0] - 2;
  }

  min[1] =
    this->StartPosition[1] <= this->EndPosition[1] ? this->StartPosition[1] : this->EndPosition[1];
  if (min[1] < 0)
  {
    min[1] = 0;
  }
  if (min[1] >= size[1])
  {
    min[1] = size[1] - 2;
  }

  max[0] =
    this->EndPosition[0] > this->StartPosition[0] ? this->EndPosition[0] : this->StartPosition[0];
  if (max[0] < 0)
  {
    max[0] = 0;
  }
  if (max[0] >= size[0])
  {
    max[0] = size[0] - 2;
  }

  max[1] =
    this->EndPosition[1] > this->StartPosition[1] ? this->EndPosition[1] : this->StartPosition[1];
  if (max[1] < 0)
  {
    max[1] = 0;
  }
  if (max[1] >= size[1])
  {
    max[1] = size[1] - 2;
  }

  rbcenter[0] = (min[0] + max[0]) / 2.0;
  rbcenter[1] = (min[1] + max[1]) / 2.0;
  rbcenter[2] = 0;

  std::cout << "rect pick" << std::endl;

  std::cout << "state: " << this->State << ", (" << VTKIS_NONE << ")" << std::endl;

  if (this->State == VTKIS_NONE)
  {
    // use hardware selector
    this->m_hardwareSelector->SetRenderer(this->CurrentRenderer);
    this->m_hardwareSelector->SetArea(min[0], min[1], max[0], max[1]);
    vtkSelection * selection = this->m_hardwareSelector->Select();

    unsigned int numberOfNodes = selection->GetNumberOfNodes();

    for(int i = 0; i < numberOfNodes; i++)
    {
      std::cout << "actor " << i << std::endl;
      vtkSelectionNode * node = selection->GetNode(i);
      vtkObjectBase * obj = node->GetProperties()->Get(vtkSelectionNode::PROP());
      vtkActor * actor = static_cast<vtkActor *>(obj);

      int controlKey = this->Interactor->GetControlKey();
      std::cout << "control" << controlKey << std::endl;
      if(controlKey == 0)
      {
        this->m_sceneWidget->combiningAddPicking(actor);
      }
      else
      {
        this->m_sceneWidget->combiningRemovePicking(actor);
      }

    }
  }

  this->Interactor->GetRenderWindow()->Render();
}

void InteractorStyleTrackballCameraPicking::RedrawRubberBand()
{
  // update the rubber band on the screen
  const int* size = this->Interactor->GetRenderWindow()->GetSize();

  vtkUnsignedCharArray* tmpPixelArray = vtkUnsignedCharArray::New();
  tmpPixelArray->DeepCopy(this->PixelArray);
  unsigned char* pixels = tmpPixelArray->GetPointer(0);

  int min[2], max[2];

  min[0] =
    this->StartPosition[0] <= this->EndPosition[0] ? this->StartPosition[0] : this->EndPosition[0];
  if (min[0] < 0)
  {
    min[0] = 0;
  }
  if (min[0] >= size[0])
  {
    min[0] = size[0] - 1;
  }

  min[1] =
    this->StartPosition[1] <= this->EndPosition[1] ? this->StartPosition[1] : this->EndPosition[1];
  if (min[1] < 0)
  {
    min[1] = 0;
  }
  if (min[1] >= size[1])
  {
    min[1] = size[1] - 1;
  }

  max[0] =
    this->EndPosition[0] > this->StartPosition[0] ? this->EndPosition[0] : this->StartPosition[0];
  if (max[0] < 0)
  {
    max[0] = 0;
  }
  if (max[0] >= size[0])
  {
    max[0] = size[0] - 1;
  }

  max[1] =
    this->EndPosition[1] > this->StartPosition[1] ? this->EndPosition[1] : this->StartPosition[1];
  if (max[1] < 0)
  {
    max[1] = 0;
  }
  if (max[1] >= size[1])
  {
    max[1] = size[1] - 1;
  }

  int i;
  for (i = min[0]; i <= max[0]; i++)
  {
    pixels[4 * (min[1] * size[0] + i)] = 255 ^ pixels[4 * (min[1] * size[0] + i)];
    pixels[4 * (min[1] * size[0] + i) + 1] = 255 ^ pixels[4 * (min[1] * size[0] + i) + 1];
    pixels[4 * (min[1] * size[0] + i) + 2] = 255 ^ pixels[4 * (min[1] * size[0] + i) + 2];
    pixels[4 * (max[1] * size[0] + i)] = 255 ^ pixels[4 * (max[1] * size[0] + i)];
    pixels[4 * (max[1] * size[0] + i) + 1] = 255 ^ pixels[4 * (max[1] * size[0] + i) + 1];
    pixels[4 * (max[1] * size[0] + i) + 2] = 255 ^ pixels[4 * (max[1] * size[0] + i) + 2];
  }
  for (i = min[1] + 1; i < max[1]; i++)
  {
    pixels[4 * (i * size[0] + min[0])] = 255 ^ pixels[4 * (i * size[0] + min[0])];
    pixels[4 * (i * size[0] + min[0]) + 1] = 255 ^ pixels[4 * (i * size[0] + min[0]) + 1];
    pixels[4 * (i * size[0] + min[0]) + 2] = 255 ^ pixels[4 * (i * size[0] + min[0]) + 2];
    pixels[4 * (i * size[0] + max[0])] = 255 ^ pixels[4 * (i * size[0] + max[0])];
    pixels[4 * (i * size[0] + max[0]) + 1] = 255 ^ pixels[4 * (i * size[0] + max[0]) + 1];
    pixels[4 * (i * size[0] + max[0]) + 2] = 255 ^ pixels[4 * (i * size[0] + max[0]) + 2];
  }

  this->Interactor->GetRenderWindow()->SetRGBACharPixelData(
    0, 0, size[0] - 1, size[1] - 1, pixels, 0);
  this->Interactor->GetRenderWindow()->Frame();

  tmpPixelArray->Delete();
}

int InteractorStyleTrackballCameraPicking::GetPickingStyle() const
{
  return m_pickingStyle;
}

void InteractorStyleTrackballCameraPicking::SetPickingStyle(int newPickingStyle)
{
  m_pickingStyle = newPickingStyle;
}

void InteractorStyleTrackballCameraPicking::CombiningOn()
{
  this->m_isCombining = true;
  this->m_isMarking = false;
  this->m_isMoving = false;
}

void InteractorStyleTrackballCameraPicking::CombiningOff()
{
  this->m_isCombining = false;
}

void InteractorStyleTrackballCameraPicking::MarkingOn()
{
  this->m_isMarking = true;
  this->m_isCombining = false;
  this->m_isMoving = false;
}

void InteractorStyleTrackballCameraPicking::MarkingOff()
{
  this->m_isMarking = false;
}

void InteractorStyleTrackballCameraPicking::MovingOn()
{
  this->m_isMoving = true;
  this->m_isCombining = false;
  this->m_isMarking = false;
}

void InteractorStyleTrackballCameraPicking::MovingOff()
{
  this->m_isMoving = false;
}

void InteractorStyleTrackballCameraPicking::SetSceneWidget(SceneWidget * widget)
{
  this->m_sceneWidget = widget;
}

void InteractorStyleTrackballCameraPicking::SetMainWindow(ModelEditorWindow * window)
{
  this->m_mainWindow = window;
}

void InteractorStyleTrackballCameraPicking::PrintSelf(std::ostream & os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

void InteractorStyleTrackballCameraPicking::OnLeftButtonDown()
{

  if (this->m_pickingStyle == PICKING_STYLE_NONE)
  {
    // if no picking style used, let the parent class handle it
    this->Superclass::OnLeftButtonDown();
    return;
  }

  if (!this->Interactor)
  {
    return;
  }

  vtkNew<vtkNamedColors> colors;

  // otherwise record the rubber band starting coordinate
  if(this->m_pickingStyle == PICKING_STYLE_RECT)
  {
    this->Moving = 1;

    vtkRenderWindow* renWin = this->Interactor->GetRenderWindow();

    this->StartPosition[0] = this->Interactor->GetEventPosition()[0];
    this->StartPosition[1] = this->Interactor->GetEventPosition()[1];
    this->EndPosition[0] = this->StartPosition[0];
    this->EndPosition[1] = this->StartPosition[1];

    this->PixelArray->Initialize();
    this->PixelArray->SetNumberOfComponents(4);
    const int* size = renWin->GetSize();
    this->PixelArray->SetNumberOfTuples(size[0] * size[1]);

    renWin->GetRGBACharPixelData(0, 0, size[0] - 1, size[1] - 1, 1, this->PixelArray);

    this->FindPokedRenderer(this->StartPosition[0], this->StartPosition[1]);
  }
  else if(m_pickingStyle == PICKING_STYLE_PROP) // normal click picking
  {
    this->Superclass::OnLeftButtonDown();
    int* clickPos = this->GetInteractor()->GetEventPosition();

    // Pick from this location.
    m_propPicker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());
    this->m_lastPickedActor = m_propPicker->GetActor();
    if (this->m_lastPickedActor)
    {
      if(this->m_isCombining)
      {
        int controlKey = this->Interactor->GetControlKey();
        std::cout << "control" << controlKey << std::endl;
        if(controlKey == 0)
        {
          this->m_sceneWidget->combiningAddPicking(this->m_lastPickedActor);
        }
        else
        {
          this->m_sceneWidget->combiningRemovePicking(this->m_lastPickedActor);
        }
      }
      else if(this->m_isMarking)
      {
        this->m_sceneWidget->markingAddPicking(this->m_lastPickedActor);
      }
      else if(this->m_isMoving)
      {
        this->m_sceneWidget->movingAddPicking(this->m_lastPickedActor);
      }
    }
    else // nothing is selected
    {
      if(this->m_isMarking)
      {
        this->m_sceneWidget->markingAddPicking(nullptr);
      }
    }
  }

  // Forward events
  this->Superclass::OnLeftButtonDown();

  this->Interactor->Render();
}

void InteractorStyleTrackballCameraPicking::OnMouseMove()
{
  if (this->m_pickingStyle == PICKING_STYLE_NONE)
  {
    // if no picking style used,  let the parent class handle it
    this->Superclass::OnMouseMove();
    return;
  }

  if (!this->Interactor || !this->Moving)
  {
    return;
  }

  this->EndPosition[0] = this->Interactor->GetEventPosition()[0];
  this->EndPosition[1] = this->Interactor->GetEventPosition()[1];
  const int* size = this->Interactor->GetRenderWindow()->GetSize();
  if (this->EndPosition[0] > (size[0] - 1))
  {
    this->EndPosition[0] = size[0] - 1;
  }
  if (this->EndPosition[0] < 0)
  {
    this->EndPosition[0] = 0;
  }
  if (this->EndPosition[1] > (size[1] - 1))
  {
    this->EndPosition[1] = size[1] - 1;
  }
  if (this->EndPosition[1] < 0)
  {
    this->EndPosition[1] = 0;
  }
  this->RedrawRubberBand();
}

void InteractorStyleTrackballCameraPicking::OnLeftButtonUp()
{
  if (this->m_pickingStyle == PICKING_STYLE_NONE)
  {
    // if no picking style used, let the parent class handle it
    this->Superclass::OnLeftButtonUp();
    return;
  }

  if (!this->Interactor)
  {
    return;
  }

  this->Superclass::OnLeftButtonUp();

  if(this->m_pickingStyle == PICKING_STYLE_RECT)
  {
    if(this->Moving)
    {
      // otherwise record the rubber band end coordinate and then fire off a pick
      if ((this->StartPosition[0] != this->EndPosition[0]) ||
        (this->StartPosition[1] != this->EndPosition[1]))
      {
        this->RectPick();
      }
      this->Moving = 0;
    }
  }
}

void InteractorStyleTrackballCameraPicking::OnChar()
{
  vtkRenderWindowInteractor* rwi = this->Interactor;
  int state;

  switch (rwi->GetKeyCode())
  {
    case 'r':
    case 'R':
      if(m_isCombining)
      {
        bool isChecked = this->m_sceneWidget->m_parentWindow->ui->
            combiningRectSelectionBtn->isChecked();
        if(!isChecked) // disable rect slection
        {
          this->m_sceneWidget->combiningRectSelectionOn();
        }
        else
        {
          this->m_sceneWidget->combiningRectSelectionOff();
        }
      }
      break;
    case 'e':
    case 'E':
      if(m_isCombining)
      {
        bool isChecked = this->m_sceneWidget->m_parentWindow->ui->
            combiningClickSelectionBtn->isChecked();
        if(!isChecked) // disable click slection
        {
          this->m_sceneWidget->combiningClickSelectionOn();
        }
        else
        {
          this->m_sceneWidget->combiningClickSelectionOff();
        }
      }
      else if(m_isMarking)
      {
        bool isChecked = this->m_sceneWidget->m_parentWindow->ui->
            markingClickSelectionBtn->isChecked();
        if(!isChecked) // disable rect slection
        {
          this->m_sceneWidget->markingClickSelectionOn();
        }
        else
        {
          this->m_sceneWidget->markingClickSelectionOff();
        }
      }
      else if(this->m_isMoving)
      {
        bool isChecked = this->m_sceneWidget->m_parentWindow->ui->
            movingClickSelectionBtn->isChecked();
        if(!isChecked) // disable rect slection
        {
          this->m_sceneWidget->movingClickSelectionOn();
        }
        else
        {
          this->m_sceneWidget->movingClickSelectionOff();
        }
      }
      break;
  }
}
            
void InteractorStyleTrackballCameraPicking::SetPickingStyleToProp()
{
  this->SetPickingStyle(PICKING_STYLE_PROP);
//  this->GetInteractor()->SetPicker(this->m_propPicker);
}

void InteractorStyleTrackballCameraPicking::SetPickingStyleToRect()
{
  this->SetPickingStyle(PICKING_STYLE_RECT);
//  this->GetInteractor()->SetPicker(this->m_areaPicker);
}
