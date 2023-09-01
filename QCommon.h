#ifndef QCommon_H
#define QCommon_H

// shortcut: shared pointer
#define QSP QSharedPointer
// shortcut: declaration shared pointer
#define QDSP(Type, Var) QSP<Type> Var
// shortcut: variable of shared pointer
#define QVSP(Type, Var) Var = QSP<Type>(new Type())
  // one parameter
#define QVSP1(Type, Var, Param) Var = QSP<Type>(new Type(Param))
// shortcut: new shared pointer
#define QNSP(Type)  QSP<Type>(new Type())
  // one parameter
#define QNSP1(Type, Param)  QSP<Type>(new Type(Param))
// shortcut: smart pointer full definition
#define QDNSP(Type, Var)  QSP<Type> Var(new Type())
  // one parameter
#define QNDSP1(Type, Var, Param)	QSP<Type> Var(new Type(Param))

#endif // QCommon_H
