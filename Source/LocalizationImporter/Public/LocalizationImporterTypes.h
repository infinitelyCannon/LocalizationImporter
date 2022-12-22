// Copyright (C) 2022 Dakarai Simmons - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LocalizationImporterTypes.generated.h"

USTRUCT(BlueprintType)
struct FUpdateTranslationsSettings
{
    GENERATED_BODY()

    FUpdateTranslationsSettings() :
    Title(""),
    Checked(false)
    {}

    FUpdateTranslationsSettings(FString title) :
    Title(title),
    Checked(false)
    {}

    FUpdateTranslationsSettings(FString title, bool checked) :
    Title(title),
    Checked(checked)
    {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Settings)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Settings)
    bool Checked;
};
