// Copyright (C) 2022 Dakarai Simmons - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LocalizationImporterTypes.h"
#include "PythonBridge.generated.h"

/**
 * Bridge class to allow C++ to call Python functions
 * By adding the init_unreal.py script to the list of python
 * files to auto-run, the script will create a child class the can be queried.
 */
UCLASS(Blueprintable)
class LOCALIZATIONIMPORTER_API UPythonBridge : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category=Python)
    static UPythonBridge *Get();

    UFUNCTION(BlueprintImplementableEvent, Category=Python)
    TArray<FUpdateTranslationsSettings> ImportSpreadsheet(const FString &path) const;

    UFUNCTION(BlueprintImplementableEvent, Category=Python)
    void UpdateSelection(const FString &path, const FString &pages, const FString &languages, const bool refresh, const bool case_sensitive) const;
};
