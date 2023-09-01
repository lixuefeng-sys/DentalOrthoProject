#include "QtUtil.h"

#include <QLayout>
#include <QWidget>

namespace QtUtil
{

void setLayoutVisible(QLayout * layout, bool visibility)
{
  for(int i = 0; i < layout->count(); i++)
  {
    QLayoutItem * item = layout->itemAt(i);
    QLayout * childLayout = item->layout();
    if(childLayout != nullptr)
    {
      setLayoutVisible(childLayout, visibility);
    }
    QWidget * childWidget = item->widget();
    if(childWidget != nullptr)
    {
      childWidget->setVisible(visibility);
    }
  }
}

void hideLayout(QLayout * layout)
{
  setLayoutVisible(layout, false);
}

void showLayout(QLayout * layout)
{
  setLayoutVisible(layout, true);
}

} // namespace QtUtil
