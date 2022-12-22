// Copyright (C) 2022 Dakarai Simmons - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SWidget.h"

DECLARE_LOG_CATEGORY_EXTERN(LocalizationImporterPlugin, Log, All)

class SImportTranslationsDialog;

class FLocalizationImporterModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

    // Mapped UI Commands
    void UpdateTranslations();

private:
    void AddMenuExtension(/*FToolBarBuilder &Builder*/);
    void OnUpdateTranslationDialogClosed(const TSharedRef<SWindow> &Window);
	bool Tick(const float DeltaTime);

protected:
    TSharedPtr<SWindow> UpdateTranslationsDialogWindow;
    TSharedPtr<SImportTranslationsDialog> UpdateTranslationsDialog;
	FDelegateHandle TickHandle;
};
