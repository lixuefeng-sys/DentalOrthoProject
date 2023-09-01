
#ifndef TextToImageConverter_H
#define TextToImageConverter_H

#include <vtkImageAlgorithm.h>
#include <vtkSetGet.h>

#include <QSharedPointer>

#include "QCommon.h"
#include "vtkCommon.h"

class QColor;
class QString;

class TextToImageConverter : public vtkImageAlgorithm
{
public:
  static TextToImageConverter * New();

  vtkTypeMacro(TextToImageConverter, vtkImageAlgorithm);
  void PrintSelf(ostream & os, vtkIndent indent) override;

  int getWidth() const;
  void setWidth(int newWidth);

  int getHeight() const;
  void setHeight(int newHeight);

  void setSize(int newWidth, int newHeight);

  QSharedPointer<QString> getText() const;
  void setText(QSharedPointer<QString> newText);

  QSharedPointer<QColor> getBackgroundColor() const;
  void setBackgroundColor(QSharedPointer<QColor> newBackgroundColor);

  QSharedPointer<QColor> getForegroundColor() const;
  void setForegroundColor(QSharedPointer<QColor> newForegroundColor);

protected:
  TextToImageConverter();
  virtual ~TextToImageConverter() override;

  int FillInputPortInformation(int port, vtkInformation * info) override;
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  QDSP(QString, m_text);
  int m_width;
  int m_height;

  QDSP(QColor, m_backgroundColor);
  QDSP(QColor, m_foregroundColor);

private:
  TextToImageConverter(const TextToImageConverter &) = delete;
  void operator=(const TextToImageConverter &) = delete;
};

#endif  // TextToImageConverter_H
            
