#include "AbsoluteLayout.h"

#include <QWidget>

AbsoluteLayout::AbsoluteLayout(QWidget * parent)
  :QLayout(parent)
{
}

AbsoluteLayout::~AbsoluteLayout()
{
  QLayoutItem * item;
  while ((item = AbsoluteLayout::takeAt(0)))
    delete item;
}

void AbsoluteLayout::addItem(QLayoutItem * item)
{
  this->m_items.push_back(item);
}

QSize AbsoluteLayout::sizeHint() const
{
  QSize s(0, 0);
  for (auto e : m_items)
  {
    s = s.expandedTo(e->sizeHint());
  }

  return s;
}

QSize AbsoluteLayout::minimumSize() const
{
  QSize s(0, 0);
  for (auto e : m_items)
  {
    s = s.expandedTo(e->minimumSize());
  }

  return s;
}

int AbsoluteLayout::count() const
{
  return m_items.count();
}

QLayoutItem * AbsoluteLayout::itemAt(int index) const
{
  return m_items.value(index);
}

QLayoutItem * AbsoluteLayout::takeAt(int index)
{
  invalidate();

  return index >= 0 &&
      index < m_items.size() ?
      m_items.takeAt(index) : 0;
}

void AbsoluteLayout::setGeometry(const QRect& r)
{
  QLayout::setGeometry(r);
}
