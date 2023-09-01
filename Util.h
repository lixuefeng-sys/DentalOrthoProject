#ifndef Util_H
#define Util_H

#include <iostream>

namespace Util
{

#define DbOp DatabaseOperation::getInstance()

template<typename T>
void print(const T & item)
{
  std::cout << item << std::endl;
}

//extern void print(double & item);

/*
void sortPointsCounterClockwise(vtkSmartPointer<vtkPoints> points)
{

  if(sum > 0) // clockwise
  {
  }
}
*/
} // namespace Util

#endif // Util_H
