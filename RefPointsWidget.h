#ifndef RefPointsWidget_H
#define RefPointsWidget_H

#include <vtkAbstractWidget.h>

class vtkHandleRepresentation;
class vtkHandleWidget;
class RefPointsSeedList;
class vtkSeedRepresentation;

class RefPointsWidget : public vtkAbstractWidget
{
public:
  /**
   * Instantiate this class.
   */
  static RefPointsWidget* New();

  //@{
  /**
   * Standard methods for a VTK class.
   */
  vtkTypeMacro(RefPointsWidget, vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /**
   * The method for activating and deactivating this widget. This method
   * must be overridden because it is a composite widget and does more than
   * its superclasses' vtkAbstractWidget::SetEnabled() method.
   */
  void SetEnabled(int) override;

  /**
   * Set the current renderer. This method also propagates to all the child
   * handle widgets, if any exist
   */
  void SetCurrentRenderer(vtkRenderer*) override;

  /**
   * Set the interactor. This method also propagates to all the child
   * handle widgets, if any exist
   */
  void SetInteractor(vtkRenderWindowInteractor*) override;

  /**
   * Specify an instance of vtkWidgetRepresentation used to represent this
   * widget in the scene. Note that the representation is a subclass of vtkProp
   * so it can be added to the renderer independent of the widget.
   */
  void SetRepresentation(vtkSeedRepresentation* rep)
  {
    this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(rep));
  }

  /**
   * Return the representation as a vtkSeedRepresentation.
   */
  vtkSeedRepresentation* GetSeedRepresentation()
  {
    return reinterpret_cast<vtkSeedRepresentation*>(this->WidgetRep);
  }

  /**
   * Create the default widget representation if one is not set.
   */
  void CreateDefaultRepresentation() override;

  /**
   * Methods to change the whether the widget responds to interaction.
   * Overridden to pass the state to component widgets.
   */
  void SetProcessEvents(vtkTypeBool) override;

  /**
   * Method to be called when the seed widget should stop responding to
   * the place point interaction. The seed widget, when defined allows you
   * place seeds by clicking on the render window. Use this method to
   * indicate that you would like to stop placing seeds interactively. If
   * you'd like the widget to stop responding to *any* user interaction
   * simply disable event processing by the widget by calling
   * widget->ProcessEventsOff()
   */
  virtual void CompleteInteraction();

  /**
   * Method to be called when the seed widget should start responding
   * to the interaction.
   */
  virtual void RestartInteraction();

  /**
   * Use this method to programmatically create a new handle. In interactive
   * mode, (when the widget is in the PlacingSeeds state) this method is
   * automatically invoked. The method returns the handle created.
   * A valid seed representation must exist for the widget to create a new
   * handle.
   */
  virtual vtkHandleWidget* CreateNewHandle();

  /**
   * Delete the nth seed.
   */
  void DeleteSeed(int n);

  void RemoveAllSeeds();

  /**
   * Get the nth seed
   */
  vtkHandleWidget* GetSeed(int n);

  /**
   * Get the number of seeds.
   */
  int GetNumberOfSeeds();

  //@{
  /**
   * Get the widget state.
   */
  vtkGetMacro(WidgetState, int);
  //@}

  // The state of the widget

  enum
  {
    Start = 1,
    PlacingSeeds = 2,
    PlacedSeeds = 4,
    MovingSeed = 8
  };

  void SetPointLimit(int limit);
  int GetPointLimit();

protected:
  RefPointsWidget();
  ~RefPointsWidget() override;

  int WidgetState;

  // Callback interface to capture events when
  // placing the widget.
  static void AddPointAction(vtkAbstractWidget*);
  static void CompletedAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  static void DeleteAction(vtkAbstractWidget*);

  // The positioning handle widgets
  RefPointsSeedList* Seeds;

  // Manipulating or defining ?
  int Defining;

  int PointLimit {3}; // RainSia:: default is 3

private:
  RefPointsWidget(const RefPointsWidget&) = delete;
  void operator=(const RefPointsWidget&) = delete;
};

#endif // RefPointsWidget_H
