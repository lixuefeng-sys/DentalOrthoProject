#ifndef QT_UTIL_H
#define QT_UTIL_H


class QLayout;
namespace QtUtil
{

// change the visibility of widgets in a layout (recursively)
extern void setLayoutVisible(QLayout * layout, bool visibility);
// shortcut for setLayoutVisible
extern void hideLayout(QLayout * layout);
// shortcut for setLayoutVisible
extern void showLayout(QLayout * layout);

} // namespace QtUtil

#endif // QT_UTIL_H
