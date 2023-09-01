#ifndef DOUBLE_SLIDER_H
#define DOUBLE_SLIDER_H

#include <QWidget>

class DoubleSlider : public QWidget
{
  Q_OBJECT
public:
  DoubleSlider(QWidget * parent = nullptr);
  void setRange(float min, float max);
  void setSingleStep(float step);

  enum DoubleSliderState{ MinHandle, MaxHandle, None };

  float minValue() const;
  float maxValue() const;

  float minRange() const;
  float maxRange() const;

public slots:
  void setLabel(const QString & label);
  void setMaxValue(float val);
  void setMinValue(float val);

signals:
  void minValueChanged(float);
  void maxValueChanged(float);

private:
  float m_min;
  float m_max;
  float m_singleStep;

  float m_minValue;
  float m_maxValue;

  QRect minHandleRegion;
  QRect maxHandleRegion;

  DoubleSliderState m_state;

  QString m_label;

protected:
  void paintEvent(QPaintEvent * event);
  void paintColoredRect(QRect rect, QColor color, QPainter* painter);
  void paintValueLabel(QPainter * painter);

  void mousePressEvent(QMouseEvent * event);
  void mouseMoveEvent(QMouseEvent * event);

  void keyPressEvent(QKeyEvent * event);
  void leaveEvent(QEvent * event);
};

#endif // DOUBLE_SLIDER_H
