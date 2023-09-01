#include "TextButtonRepresentation2D.h"
#include "TextToImageConverter.h"

#include <vtkObjectFactory.h>
#include <vtkImageData.h>
#include <vtkQImageToImageSource.h>

#include <QPixmap>
#include <QColor>
#include <QPainter>

vtkStandardNewMacro(TextButtonRepresentation2D);

TextButtonRepresentation2D::TextButtonRepresentation2D()
  :vtkTexturedButtonRepresentation2D()
{
  vVSP(vtkQImageToImageSource, this->m_imageFilter);
}

vtkSmartPointer<vtkImageData> TextButtonRepresentation2D::generateImage(int state)
{
  QImage qimage(this->m_width, this->m_height, QImage::Format_ARGB32);

  // set colors
  QDNSP(QColor, bgc);
  QDNSP(QColor, fc);
  if (state == 0)
  {
    bgc->setRgb(214, 214, 214);
    fc->setRgb(117, 105, 103);
  }
  else if(state == 1)
  {
    bgc->setRgb(93, 173, 226);
    fc->setRgb(215, 219, 221);
//    bgc.setRgb(46, 134, 193);
  }
  else
  {
    bgc->setRgb(53, 152, 219);
    fc->setRgb(215, 219, 221);
  }

  vDNSP(TextToImageConverter, m_converter);
  m_converter->setSize(this->m_width, this->m_height);
  m_converter->setText(this->m_text);
  m_converter->setBackgroundColor(bgc);
  m_converter->setForegroundColor(fc);
  m_converter->Update();

  vDNSP(vtkImageData, image);
  image->ShallowCopy(m_converter->GetOutput());

  return image;
}

TextButtonRepresentation2D::~TextButtonRepresentation2D()
{
}

void TextButtonRepresentation2D::PrintSelf(std::ostream & os, vtkIndent indent)
{
  vtkTexturedButtonRepresentation2D::PrintSelf(os, indent);
}

int TextButtonRepresentation2D::getHeight() const
{
  return m_height;
}

void TextButtonRepresentation2D::setHeight(int newHeight)
{
  m_height = newHeight;
}

int TextButtonRepresentation2D::getWidth() const
{
  return m_width;
}

void TextButtonRepresentation2D::setWidth(int newWidth)
{
  m_width = newWidth;
}

void TextButtonRepresentation2D::setSize(int newWidth, int newHeight)
{
  m_width = newWidth;
  m_height = newHeight;
}

void TextButtonRepresentation2D::setSize(int size[2])
{
  m_width = size[0];
  m_height = size[1];
}

int TextButtonRepresentation2D::getX() const
{
  return m_x;
}

void TextButtonRepresentation2D::setX(int newX)
{
  m_x = newX;
}

int TextButtonRepresentation2D::getY() const
{
  return m_y;
}

void TextButtonRepresentation2D::setY(int newY)
{
  m_y = newY;
}

void TextButtonRepresentation2D::setPosition(int newX, int newY)
{
  m_x = newX;
  m_y = newY;
}

void TextButtonRepresentation2D::setPosition(int position[2])
{
  m_x = position[0];
  m_y = position[1];
}

void TextButtonRepresentation2D::generate()
{
  int states = this->NumberOfStates;
  if(states == 0)
  {
    vtkErrorMacro("A button with zero state! At least one state should be assigned!");
    std::cerr << "A button with zero state! At least one state should be assigned!" << std::endl;
    return;
  }
  if(m_width == 0 || m_height == 0)
  {
    vtkErrorMacro("Size is not specified.");
    std::cerr << "Size is not specified!" << std::endl;
    return;
  }
  if(m_x == 0 || m_y == 0)
  {
    vtkErrorMacro("Position is not specified! And position 0,0 is not a good idea!");
    std::cerr << "Position is not specified! And position 0,0 is not a good idea!" << std::endl;
    return;
  }
  if(m_text == nullptr)
  {
    vtkErrorMacro("Text is not specified!");
    std::cerr << "Text is not specified!" << std::endl;
    return;
  }

  for(int i = 0; i < states; i++)
  {
    vtkSmartPointer<vtkImageData> image = this->generateImage(i);
    this->SetButtonTexture(i, image);
  }

}

void TextButtonRepresentation2D::place()
{
  this->SetPlaceFactor(1);
  double bds[6];
  bds[0] = this->m_x;
  bds[1] = this->m_x + this->m_width;
  bds[2] = this->m_y;
  bds[3] = this->m_y + this->m_height;
  bds[4] = bds[5] = 0;
  this->PlaceWidget(bds);
}

QSharedPointer<QString> TextButtonRepresentation2D::getText() const
{
  return m_text;
}

void TextButtonRepresentation2D::setText(QSharedPointer<QString> newText)
{
  m_text = newText;
}


