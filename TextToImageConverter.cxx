
#include "TextToImageConverter.h"

#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkDataObject.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkQImageToImageSource.h>

#include <QPainter>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QPen>
#include <QRectF>

vtkStandardNewMacro(TextToImageConverter);

int TextToImageConverter::FillInputPortInformation( int port, vtkInformation* info )
{
  vtkErrorMacro("This filter does not have input port!");
  return 0;
}

int TextToImageConverter::RequestData(vtkInformation *vtkNotUsed(request),
    vtkInformationVector ** inputVector,
    vtkInformationVector * outputVector)
{
  // get the info objects
  vtkInformation * outInfo = outputVector->GetInformationObject(0);
  vtkImageData * output = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (this->m_text == nullptr)
  {
    vtkErrorMacro("Text is not specified.");
    std::cerr << "Text is not specified." << std::endl;
    return 0;
  }

  if(this->m_width ==0 || this->m_height == 0)
  {
    vtkErrorMacro("Size is not specified.");
    std::cerr << "Size is not specified!" << std::endl;
    return 0;
  }

  QImage qimage(this->m_width, this->m_height, QImage::Format_ARGB32);
  QPainter painter(&qimage);

  // draw background of the button
  if(this->m_backgroundColor != nullptr)
  {
    QBrush brush(*this->m_backgroundColor);
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawRoundedRect(QRectF(0,0, this->m_width , this->m_height), 15, 15);
  }

  // draw text
  QFont font("黑体", 15, QFont::Normal, false);
  if(this->m_foregroundColor == nullptr)
  {
    painter.setPen(QPen(QColor(117, 105, 103)));
  }
  else
  {
    painter.setPen(QPen(*this->m_foregroundColor));
  }
  painter.setFont(font);
  QRectF rect(0, 0, this->m_width, this->m_height);
  painter.drawText(rect, Qt::AlignCenter, *this->m_text);

  painter.end();

  vDNSP(vtkQImageToImageSource, qimageToImageSource);
  qimageToImageSource->SetQImage(&qimage);
  qimageToImageSource->Update();

  output->ShallowCopy(qimageToImageSource->GetOutput());

  return 1;
}

QSharedPointer<QColor> TextToImageConverter::getForegroundColor() const
{
  return m_foregroundColor;
}

void TextToImageConverter::setForegroundColor(QSharedPointer<QColor> newForegroundColor)
{
  m_foregroundColor = newForegroundColor;
}

QSharedPointer<QColor> TextToImageConverter::getBackgroundColor() const
{
  return m_backgroundColor;
}

void TextToImageConverter::setBackgroundColor(QSharedPointer<QColor> newBackgroundColor)
{
  m_backgroundColor = newBackgroundColor;
}

QSharedPointer<QString> TextToImageConverter::getText() const
{
  return m_text;
}

void TextToImageConverter::setText(QSharedPointer<QString> newText)
{
  m_text = newText;
}

int TextToImageConverter::getHeight() const
{
  return m_height;
}

void TextToImageConverter::setHeight(int newHeight)
{
  m_height = newHeight;
  this->Modified();
}

void TextToImageConverter::setSize(int newWidth, int newHeight)
{
  m_width = newWidth;
  m_height = newHeight;
  this->Modified();
}

int TextToImageConverter::getWidth() const
{
  return m_width;
}

void TextToImageConverter::setWidth(int newWidth)
{
  m_width = newWidth;
  this->Modified();
}

TextToImageConverter::TextToImageConverter()
  :vtkImageAlgorithm()
{
  this->SetNumberOfInputPorts(0);
}

TextToImageConverter::~TextToImageConverter()
{
}

void TextToImageConverter::PrintSelf(ostream & os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
            
