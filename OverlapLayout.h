#ifndef OVERLAY_LAYOUT_H
#define OVERLAY_LAYOUT_H

#include <QLayout>
#include <QList>

/**
 * @brief The OverlapLayout class
 * Define the layered layout of widgets.
 * Layers are defined by sublayouts.
 * Different widgets are organized by different sublayouts.
 * The sublayouts added earlier are on lower layers.
 *
 */
class OverlapLayout : public QLayout
{
public:
  OverlapLayout(QWidget * parent = nullptr);
  ~OverlapLayout() = default;

  void addItem(QLayoutItem * item) override;

  QSize sizeHint() const override;
  QSize minimumSize() const override;
  int count() const override;
  QLayoutItem * itemAt(int) const override;
  QLayoutItem * takeAt(int) override;
  void setGeometry(const QRect & rect) override;

  void AddLayout(QLayout *);

private:
  QList<QLayout *> m_subLayouts;
};

#endif // OVERLAY_LAYOUT_H
