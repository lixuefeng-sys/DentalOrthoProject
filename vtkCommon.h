#ifndef VtkCommon_H
#define VtkCommon_H

// shortcut: smart pointer
#define vSP vtkSmartPointer
// shortcut: declaration smart pointer
#define vDSP(Type, Var) vSP<Type> Var
// shortcut: varialbe of smart pointer
#define vVSP(Type, Var)	Var = vSP<Type>::New()
// shortcut: new smart pointer
#define vNSP(Type)  vSP<Type>::New()
// shortcut: definition of new smart pointer
#define vDNSP(Type, Var)	vSP<Type> Var = vSP<Type>::New()

#define vEachIdList(Var, List) for(int Var = 0; Var < List->GetNumberOfIds(); Var++)
#define vEachIdListR(Var, List) for(int Var = List->GetNumberOfIds() - 1; Var >= 0; Var--)

#endif // VtkCommon_H
