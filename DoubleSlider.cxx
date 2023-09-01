#include "DoubleSlider.h"

#include <QMouseEvent>
#include <QPainter>
#include <cmath>

DoubleSlider::DoubleSlider(QWidget * parent)
  : QWidget(parent)
  , m_min(0.0f)
  , m_max(99.0f)
  , m_singleStep(0.1f)
  , m_minValue(0.0f)
  , m_maxValue(99.0f)
  , m_state(None)
{
  setFixedHeight(50);
  setFocusPolicy(Qt::StrongFocus);
}

void DoubleSlider::paintEvent(QPaintEvent * event)
{
  QWidget::paintEvent(event);
  QPainter painter(this);
  paintValueLabel(&painter);
}

void DoubleSlider::paintColoredRect(QRect rect, QColor color ,QPainter * painter)
{
  painter->fillRect(rect,QBrush(color));
}

void DoubleSlider::paintValueLabel(QPainter* painter)
{
  painter->setBrush(Qt::NoBrush);
  painter->setPen(Qt::black);
  painter->setFont(QFont("Arial",12));

  QString minValueString = QString::number(m_minValue,'f',3);
  QString maxValueString = QString::number(m_maxValue,'f',3);

  QFontMetrics metrics = painter->fontMetrics();
  int mintextWidth = metrics.horizontalAdvance(minValueString);
  int maxtextWidth = metrics.horizontalAdvance(maxValueString);
  int textHeight = metrics.height();

  //---- paint text
  painter->drawText(QRectF(2,4,mintextWidth,textHeight),minValueString);
  painter->drawText(QRectF(width() - maxtextWidth -2, 4, maxtextWidth,textHeight), maxValueString);


  //----- paint label
  painter->setFont(QFont("Arial",12));
  metrics = painter->fontMetrics();
  int labelWidth = metrics.horizontalAdvance(m_label);
  int labelHeight = metrics.height();

  QRectF textRect = QRectF((width()/2 - labelWidth/2), 0,labelWidth,labelHeight);

  painter->drawText(textRect,m_label);

  int minPos = ( m_minValue - m_min ) * width() / (m_max - m_min);
  int maxPos = (m_maxValue - m_min ) * width() /  (m_max - m_min);

  if(minPos <= 4){
    minPos = 4;
  }else if(minPos >= width() - 8){
    minPos = width() - 8;
  }else{}

  if(maxPos <= 4){
    maxPos = 4;
  }else if(maxPos >= width() -8){
    maxPos = width() - 8;
  }else{}


  //----- paint groove
  paintColoredRect(QRect(4,37,width() - 8,2),Qt::gray,painter);
  paintColoredRect(QRect(minPos + 4,37,maxPos - minPos,2),QColor(51,153,155),painter);

  //----- handle

  minHandleRegion = QRect(minPos ,30,8,16);
  maxHandleRegion = QRect(maxPos ,30,8,16);

  //-----paint Handle
  QColor minColor  = (m_state == MinHandle) ? QColor(51,153,155) : Qt::darkGray;
  QColor maxColor  = (m_state == MaxHandle) ? QColor(51,153,155) : Qt::darkGray;
  paintColoredRect(minHandleRegion,minColor,painter);
  paintColoredRect(maxHandleRegion,maxColor,painter);
}

inline float getValidValue(float val, float min, float max)
{
  float tmp = std::max(val, min);
  return std::min(tmp, max);
}


void DoubleSlider::setRange(float min, float max)
{
  m_min = min;
  m_max = max;

  if(minValue() < m_min)
    setMinValue(m_min);

  if(maxValue() > m_max){
    setMaxValue(m_max);
  }
}

void DoubleSlider::setSingleStep(float step)
{
  m_singleStep = step;
}

float DoubleSlider::minValue() const
{
  return m_minValue;
}
void DoubleSlider::setMinValue(float val)
{
  if(fabs( m_minValue - val ) > 0.0001 ){
    m_minValue = val;
    emit minValueChanged(val);
  }
}

float DoubleSlider::maxValue() const
{
  return m_maxValue;
}
void DoubleSlider::setMaxValue(float val)
{
  if(fabs(m_maxValue - val) > 0.0001){
    m_maxValue = val;
    emit maxValueChanged(val);
  }
}

void DoubleSlider::setLabel(const QString& label)
{
  m_label = label;
  update();
}

void DoubleSlider::leaveEvent(QEvent* event)
{
  QWidget::leaveEvent(event);
  m_state = None;
  update();
}

float DoubleSlider::minRange() const
{
  return m_min;
}
float DoubleSlider::maxRange() const
{
  return m_max;
}


void DoubleSlider::mousePressEvent(QMouseEvent * event)
{
  if(minHandleRegion.contains(event->pos())){
    m_state = MinHandle;
  }else if(maxHandleRegion.contains(event->pos())){
    m_state = MaxHandle;
  }else{
    m_state = None;
  }

  if(fabs(minValue() - maxValue()) < 0.001){
    if(fabs(minValue() - m_min) < 0.001){
      m_state = MaxHandle;
    }else if(fabs(minValue() - m_max) < 0.001){
      m_state = MinHandle;
    }else{}
  }

  update();
}

void DoubleSlider::mouseMoveEvent(QMouseEvent* event)
{
  if(event->buttons() & Qt::LeftButton){
    float move = event->x() * (m_max - m_min) * 1.0/ width() + m_min;

    switch(m_state){
    case MinHandle:{
      float val = getValidValue( move, m_min,maxValue());
      setMinValue(val);
      break;
    }

    case MaxHandle:{
      float val = getValidValue(move, minValue(), m_max);
      setMaxValue(val);
      break;
    }
    case None:default: break;
    }
  }
  update();
}


void DoubleSlider::keyPressEvent(QKeyEvent *event)
{
  QWidget::keyPressEvent(event);

  if(event->key() == Qt::Key_Left){
    if(m_state == MinHandle){
      float val = getValidValue(minValue()- m_singleStep, m_min ,maxValue());
      setMinValue(val);
    }else if(m_state == MaxHandle) {
      float val = getValidValue(maxValue()- m_singleStep, minValue() ,m_max);
      setMaxValue(val);
    }else{}
  }else if(event->key() == Qt::Key_Right){
    if(m_state == MinHandle){
      float val = getValidValue(minValue() + m_singleStep, m_min ,maxValue());
      setMinValue(val);
    }else if(m_state == MaxHandle) {
      float val = getValidValue(maxValue() + m_singleStep, minValue() ,m_max);
      setMaxValue(val);
    }else{}
  }

  update();
}
