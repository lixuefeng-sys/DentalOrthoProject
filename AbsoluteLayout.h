#ifndef ABSOLUTE_LAYOUT_H
#define ABSOLUTE_LAYOUT_H

#include <QLayout>
#include <QList>

class AbsoluteLayout : public QLayout
{
public:
  AbsoluteLayout(QWidget * parent = nullptr);
  ~AbsoluteLayout() override;

  void addItem(QLayoutItem * item) override;
  QSize sizeHint() const override;
  QSize minimumSize() const override;
  int count() const override;
  QLayoutItem * itemAt(int) const override;
  QLayoutItem * takeAt(int) override;
  void setGeometry(const QRect & rect) override;

private:
  QList<QLayoutItem*> m_items;
};

#endif // ABSOLUTE_LAYOUT_H
