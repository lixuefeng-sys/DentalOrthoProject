#include "RefPointsWidget.h"

#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkCoordinate.h>
#include <vtkEvent.h>
#include <vtkHandleRepresentation.h>
#include <vtkHandleWidget.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSeedRepresentation.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>

#include <iterator>
#include <list>

vtkStandardNewMacro(RefPointsWidget);

// The vtkSeedList is a PIMPLed list<T>.
class RefPointsSeedList : public std::list<vtkHandleWidget*>
{
};
typedef std::list<vtkHandleWidget*>::iterator RefPointsSeedListIterator;
typedef std::list<vtkHandleWidget*>::reverse_iterator RefPointsSeedListReverseIterator;

//----------------------------------------------------------------------
RefPointsWidget::RefPointsWidget()
{
  this->ManagesCursor = 1;
  this->WidgetState = RefPointsWidget::Start;

  // The widgets for moving the seeds.
  this->Seeds = new RefPointsSeedList;

  // These are the event callbacks supported by this widget
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
    vtkWidgetEvent::AddPoint, this, RefPointsWidget::AddPointAction);
//  this->CallbackMapper->SetCallbackMethod(vtkCommand::RightButtonPressEvent,
//    vtkWidgetEvent::Completed, this, RefPointsWidget::CompletedAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MouseMoveEvent, vtkWidgetEvent::Move, this, RefPointsWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
    vtkWidgetEvent::EndSelect, this, RefPointsWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::KeyPressEvent, vtkEvent::NoModifier, 127, 1,
    "Delete", vtkWidgetEvent::Delete, this, RefPointsWidget::DeleteAction);
  this->Defining = 1;
}

//----------------------------------------------------------------------
void RefPointsWidget::DeleteSeed(int i)
{
  if (this->Seeds->size() <= static_cast<size_t>(i))
  {
    return;
  }

  vtkSeedRepresentation* rep = static_cast<vtkSeedRepresentation*>(this->WidgetRep);
  if (rep)
  {
    rep->RemoveHandle(i);
  }

  RefPointsSeedListIterator iter = this->Seeds->begin();
  std::advance(iter, i);
  (*iter)->SetEnabled(0);
  (*iter)->RemoveObservers(vtkCommand::StartInteractionEvent);
  (*iter)->RemoveObservers(vtkCommand::InteractionEvent);
  (*iter)->RemoveObservers(vtkCommand::EndInteractionEvent);
  vtkHandleWidget* w = (*iter);
  this->Seeds->erase(iter);
  w->Delete();
}

void RefPointsWidget::RemoveAllSeeds()
{
  int size = this->Seeds->size();

  // remove all seed from end to beginning
  for(int i = size - 1; i >= 0; i--)
  {
    this->DeleteSeed(i);
  }
}

//----------------------------------------------------------------------
RefPointsWidget::~RefPointsWidget()
{
  // Loop over all seeds releasing their observers and deleting them
  while (!this->Seeds->empty())
  {
    this->DeleteSeed(static_cast<int>(this->Seeds->size()) - 1);
  }
  delete this->Seeds;
}

//----------------------------------------------------------------------
vtkHandleWidget* RefPointsWidget::GetSeed(int i)
{
  if (this->Seeds->size() <= static_cast<size_t>(i))
  {
    return nullptr;
  }
  RefPointsSeedListIterator iter = this->Seeds->begin();
  std::advance(iter, i);
  return *iter;
}

int RefPointsWidget::GetNumberOfSeeds()
{
  return this->Seeds->size();
}

void RefPointsWidget::SetPointLimit(int limit)
{
  this->PointLimit = limit;
}

int RefPointsWidget::GetPointLimit()
{
  return this->PointLimit;
}

//----------------------------------------------------------------------
void RefPointsWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = vtkSeedRepresentation::New();
  }
}

//----------------------------------------------------------------------
void RefPointsWidget::SetEnabled(int enabling)
{
  this->Superclass::SetEnabled(enabling);

  RefPointsSeedListIterator iter;
  for (iter = this->Seeds->begin(); iter != this->Seeds->end(); ++iter)
  {
    (*iter)->SetEnabled(enabling);
  }

  if (!enabling)
  {
    this->RequestCursorShape(VTK_CURSOR_DEFAULT);
    this->WidgetState = RefPointsWidget::Start;
  }

  this->Render();
}

// The following methods are the callbacks that the seed widget responds to.
//-------------------------------------------------------------------------
void RefPointsWidget::AddPointAction(vtkAbstractWidget* w)
{
  RefPointsWidget* self = reinterpret_cast<RefPointsWidget*>(w);


  // Need to distinguish between placing handles and manipulating handles
  if (self->WidgetState == RefPointsWidget::MovingSeed)
  {
    return;
  }

  self->InvokeEvent(vtkCommand::MouseMoveEvent, nullptr);

  // compute some info we need for all cases
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // When a seed is placed, a new handle widget must be created and enabled.
  int state = self->WidgetRep->ComputeInteractionState(X, Y);
  if (state == vtkSeedRepresentation::NearSeed)
  {
    self->WidgetState = RefPointsWidget::MovingSeed;

    // Invoke an event on ourself for the handles
    self->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
    self->Superclass::StartInteraction();
    vtkSeedRepresentation* rep = static_cast<vtkSeedRepresentation*>(self->WidgetRep);
    int seedIdx = rep->GetActiveHandle();
    self->InvokeEvent(vtkCommand::StartInteractionEvent, &seedIdx);

    self->EventCallbackCommand->SetAbortFlag(1);
    self->Render();
  }

  else if (self->WidgetState != RefPointsWidget::PlacedSeeds)
  {
    // RainSia:: return if we already have 3 points
    if(self->Seeds->size() == self->PointLimit) {
      return;
    }

    // we are placing a new seed. Just make sure we aren't in a mode which
    // dictates we've placed all seeds.

    self->WidgetState = RefPointsWidget::PlacingSeeds;
    double e[3];
    e[2] = 0.0;
    e[0] = static_cast<double>(X);
    e[1] = static_cast<double>(Y);

    vtkSeedRepresentation* rep = reinterpret_cast<vtkSeedRepresentation*>(self->WidgetRep);
    // if the handle representation is constrained, check to see if
    // the position follows the constraint.
    if (!rep->GetHandleRepresentation()->CheckConstraint(self->GetCurrentRenderer(), e))
    {
      return;
    }
    int currentHandleNumber = rep->CreateHandle(e);
    vtkHandleWidget* currentHandle = self->CreateNewHandle();
    rep->SetSeedDisplayPosition(currentHandleNumber, e);
    currentHandle->SetEnabled(1);
    self->InvokeEvent(vtkCommand::PlacePointEvent, &(currentHandleNumber));
    self->InvokeEvent(vtkCommand::InteractionEvent, &(currentHandleNumber));

    self->EventCallbackCommand->SetAbortFlag(1);
    self->Render();
  }
}

//-------------------------------------------------------------------------
void RefPointsWidget::CompletedAction(vtkAbstractWidget* w)
{
  RefPointsWidget* self = reinterpret_cast<RefPointsWidget*>(w);

  // Do something only if we are in the middle of placing the seeds
  if (self->WidgetState == RefPointsWidget::PlacingSeeds)
  {
    self->CompleteInteraction();
  }
}

//-------------------------------------------------------------------------
void RefPointsWidget::CompleteInteraction()
{
  this->WidgetState = RefPointsWidget::PlacedSeeds;
  this->EventCallbackCommand->SetAbortFlag(1);
  this->Defining = 0;
}

//-------------------------------------------------------------------------
void RefPointsWidget::RestartInteraction()
{
  this->WidgetState = RefPointsWidget::Start;
  this->Defining = 1;
}

//-------------------------------------------------------------------------
void RefPointsWidget::MoveAction(vtkAbstractWidget* w)
{
  RefPointsWidget* self = reinterpret_cast<RefPointsWidget*>(w);

  self->InvokeEvent(vtkCommand::MouseMoveEvent, nullptr);

  // set the cursor shape to a hand if we are near a seed.
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  int state = self->WidgetRep->ComputeInteractionState(X, Y);

  // Change the cursor shape to a hand and invoke an interaction event if we
  // are near the seed
  if (state == vtkSeedRepresentation::NearSeed)
  {
    self->RequestCursorShape(VTK_CURSOR_HAND);

    vtkSeedRepresentation* rep = static_cast<vtkSeedRepresentation*>(self->WidgetRep);
    int seedIdx = rep->GetActiveHandle();
    self->InvokeEvent(vtkCommand::InteractionEvent, &seedIdx);

    self->EventCallbackCommand->SetAbortFlag(1);
  }
  else
  {
    self->RequestCursorShape(VTK_CURSOR_DEFAULT);
  }

  self->Render();
}

//-------------------------------------------------------------------------
void RefPointsWidget::EndSelectAction(vtkAbstractWidget* w)
{
  RefPointsWidget* self = reinterpret_cast<RefPointsWidget*>(w);

  // Do nothing if outside
  if (self->WidgetState != RefPointsWidget::MovingSeed)
  {
    return;
  }

  // Revert back to the mode we were in prior to selection.
  self->WidgetState = self->Defining ? RefPointsWidget::PlacingSeeds : RefPointsWidget::PlacedSeeds;

  // Invoke event for seed handle
  self->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, nullptr);
  self->EventCallbackCommand->SetAbortFlag(1);
  self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
  self->Superclass::EndInteraction();
  self->Render();
}

//-------------------------------------------------------------------------
void RefPointsWidget::DeleteAction(vtkAbstractWidget* w)
{
  RefPointsWidget* self = reinterpret_cast<RefPointsWidget*>(w);

  // Do nothing if outside
  if (self->WidgetState != RefPointsWidget::PlacingSeeds)
  {
    return;
  }

  // Remove last seed
  vtkSeedRepresentation* rep = reinterpret_cast<vtkSeedRepresentation*>(self->WidgetRep);
  int removeId = rep->GetActiveHandle();
  removeId = removeId != -1 ? removeId : static_cast<int>(self->Seeds->size()) - 1;
  // Invoke event for seed handle before actually deleting
  self->InvokeEvent(vtkCommand::DeletePointEvent, &(removeId));

  self->DeleteSeed(removeId);
  // Got this event, abort processing if it
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Render();
}

//----------------------------------------------------------------------
void RefPointsWidget::SetProcessEvents(vtkTypeBool pe)
{
  this->Superclass::SetProcessEvents(pe);

  RefPointsSeedListIterator iter = this->Seeds->begin();
  for (; iter != this->Seeds->end(); ++iter)
  {
    (*iter)->SetProcessEvents(pe);
  }
}

//----------------------------------------------------------------------
void RefPointsWidget::SetInteractor(vtkRenderWindowInteractor* rwi)
{
  this->Superclass::SetInteractor(rwi);
  RefPointsSeedListIterator iter = this->Seeds->begin();
  for (; iter != this->Seeds->end(); ++iter)
  {
    (*iter)->SetInteractor(rwi);
  }
}

//----------------------------------------------------------------------
void RefPointsWidget::SetCurrentRenderer(vtkRenderer* ren)
{
  this->Superclass::SetCurrentRenderer(ren);
  RefPointsSeedListIterator iter = this->Seeds->begin();
  for (; iter != this->Seeds->end(); ++iter)
  {
    if (!ren)
    {
      // Disable widget if it's being removed from the renderer
      (*iter)->EnabledOff();
    }
    (*iter)->SetCurrentRenderer(ren);
  }
}

//----------------------------------------------------------------------
// Programmatically create a new handle.
vtkHandleWidget* RefPointsWidget::CreateNewHandle()
{
  vtkSeedRepresentation* rep = vtkSeedRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
  {
    vtkErrorMacro(<< "Please set, or create a default seed representation "
                  << "before adding requesting creation of a new handle.");
    return nullptr;
  }

  // Create the handle widget or reuse an old one
  int currentHandleNumber = static_cast<int>(this->Seeds->size());
  vtkHandleWidget* widget = vtkHandleWidget::New();

  // Configure the handle widget
  widget->SetParent(this);
  widget->SetInteractor(this->Interactor);
  vtkHandleRepresentation* handleRep = rep->GetHandleRepresentation(currentHandleNumber);
  if (!handleRep)
  {
    widget->Delete();
    return nullptr;
  }
  else
  {
    handleRep->SetRenderer(this->CurrentRenderer);
    widget->SetRepresentation(handleRep);

    // Now place the widget into the list of handle widgets (if not already there)
    this->Seeds->push_back(widget);
    return widget;
  }
}

//----------------------------------------------------------------------
void RefPointsWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  // Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  os << indent << "WidgetState: " << this->WidgetState << endl;
}
