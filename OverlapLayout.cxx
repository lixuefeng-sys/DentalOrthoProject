#include "OverlapLayout.h"

#include <QWidget>

OverlapLayout::OverlapLayout(QWidget * parent)
  :QLayout(parent)
{
}

void OverlapLayout::addItem(QLayoutItem * item)
{
  if(!m_subLayouts.empty())
  {
    m_subLayouts.last()->addItem(item);
  }
}

QSize OverlapLayout::sizeHint() const
{
  QSize s(0, 0);
  for (auto e : m_subLayouts)
  {
    s = s.expandedTo(e->sizeHint());
  }

  return s;
}

QSize OverlapLayout::minimumSize() const
{
  QSize s(0, 0);
  for (auto e : m_subLayouts)
  {
    s = s.expandedTo(e->minimumSize());
  }

  return s;
}

int OverlapLayout::count() const
{
  int num = 0;
  for (auto e : m_subLayouts)
  {
    num += e->count();
  }

  return num;
}

QLayoutItem* OverlapLayout::itemAt(int index) const
{
  int num = 0;
  int next = 0;
  for (auto e : m_subLayouts)
  {
    next = num + e->count();
    if (index < next)
    {
      return e->itemAt(index - num);
    }
    num = next;
  }

  return nullptr;
}

QLayoutItem* OverlapLayout::takeAt(int index)
{
  invalidate();

  int num = 0;
  int next = 0;
  for (auto e : qAsConst(m_subLayouts))
  {
    next = num + e->count();
    if (index < next)
    {
      e->invalidate();
      return e->takeAt(index - num);
    }
    num = next;
  }

  return nullptr;
}

void OverlapLayout::setGeometry(const QRect& r)
{
  if (m_subLayouts.size() == 0)
    return;

  for (auto e : qAsConst(m_subLayouts))
  {
    e->invalidate();
    e->setGeometry(r);
  }
}

void OverlapLayout::AddLayout(QLayout* layout)
{
  if(layout)
  {
    invalidate();
    m_subLayouts.append(layout);
    addChildLayout(layout);
    layout->invalidate();
  }
}
