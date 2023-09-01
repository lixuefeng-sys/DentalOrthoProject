#ifndef TextButtonRepresentation2D_H
#define TextButtonRepresentation2D_H

#include <vtkTexturedButtonRepresentation2D.h>
#include <vtkSmartPointer.h>
#include <QSharedPointer>

#include <QString>

#include "vtkCommon.h"
#include "QCommon.h"

class vtkQImageToImageSource;
class vtkImageData;

class TextButtonRepresentation2D : public vtkTexturedButtonRepresentation2D
{
public:
  static TextButtonRepresentation2D * New();
  vtkTypeMacro(TextButtonRepresentation2D, vtkTexturedButtonRepresentation2D);
  void PrintSelf(ostream & os, vtkIndent indent) override;

  int getHeight() const;
  void setHeight(int newHeight);

  int getWidth() const;
  void setWidth(int newWidth);

  void setSize(int newWidth, int newHeight);
  void setSize(int size[2]);

  int getX() const;
  void setX(int newX);

  int getY() const;
  void setY(int newY);

  void setPosition(int newX, int newY);
  void setPosition(int position[2]);

  /**
   * @brief generate
   *
   * first set size and location
   * second set text for the button
   * finally set the number of states
   * call generate at last
   */
  void generate();

  /**
   * @brief place
   *
   * place should be called after render
   */
  void place();

  QSharedPointer<QString> getText() const;
  void setText(QSharedPointer<QString> newText);

protected:
  vDSP(vtkQImageToImageSource, m_imageFilter);

  int m_width;
  int m_height;

  int m_x;
  int m_y;

  QDSP(QString, m_text);

  TextButtonRepresentation2D();
  ~TextButtonRepresentation2D() override;

private:
  TextButtonRepresentation2D(const TextButtonRepresentation2D &) = delete;
  void operator=(const TextButtonRepresentation2D &) = delete;

  vtkSmartPointer<vtkImageData> generateImage(int state);
};

#endif // TextButtonRepresentation2D_H

