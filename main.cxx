#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>


#include "ModelEditorWindow.h"
 
int main( int argc, char** argv )
{
//  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

  // needed to ensure appropriate OpenGL context is created for VTK rendering.
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

  // QT Stuff
  QApplication app( argc, argv );

  ModelEditorWindow modelEditorWindow;
  modelEditorWindow.show();

  QString mandiblePath = "41807738_lprofile_occlusion_l.stl";
  modelEditorWindow.loadMandible(mandiblePath);

  QString maxillaPath = "41807738_lprofile_occlusion_u.stl";
  modelEditorWindow.loadMaxilla(maxillaPath);

  int result = app.exec();

  return result;
}
