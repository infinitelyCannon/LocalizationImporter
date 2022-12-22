// Copyright (C) 2022 Dakarai Simmons - All Rights Reserved

#include "PythonBridge.h"
#include "UObject/UObjectHash.h"

UPythonBridge* UPythonBridge::Get()
{
    TArray<UClass*> Classes;
    GetDerivedClasses(UPythonBridge::StaticClass(), Classes);
    const int32 numClasses = Classes.Num();
    
    if(numClasses > 0)
        return Cast<UPythonBridge>(Classes[numClasses - 1]->GetDefaultObject());

    return nullptr;
}
