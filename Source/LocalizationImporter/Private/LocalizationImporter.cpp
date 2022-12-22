// Copyright (C) 2022 Dakarai Simmons - All Rights Reserved

#include "LocalizationImporter.h"
#include "LevelEditor.h"
#include "ToolMenus.h"
#include "Settings/EditorExperimentalSettings.h"
#include "ImportTranslationsDialog.h"

#define LOCTEXT_NAMESPACE "FLocalizationImporterModule"
DEFINE_LOG_CATEGORY(LocalizationImporterPlugin);

void FLocalizationImporterModule::StartupModule()
{
	// Register a tick handle to to run setup after the engine fully starts up.
	TickHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FLocalizationImporterModule::Tick));
}

void FLocalizationImporterModule::AddMenuExtension(/*FToolBarBuilder& Builder*/)
{
    UToolMenu *Menu = UToolMenus::Get()->FindMenu("MainFrame.MainMenu.Window");
    const bool bLocalizationDashboard = GetDefault<UEditorExperimentalSettings>()->bEnableLocalizationDashboard;

    if(Menu && bLocalizationDashboard)
    {
        FToolMenuSection *Section = Menu->FindSection("ExperimentalTabSpawners");// AddSection("Tools", LOCTEXT("ToolsSectionLabel", "Localization Tools"));
        
        Section->AddMenuEntry(
            "LocalizationImporterTab",
            LOCTEXT("LocalizationImporterLabel", "Import Localizations"),
            LOCTEXT("LocalizationImporterTooltip", "Import localizations from a spreadsheet file."),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateRaw(this, &FLocalizationImporterModule::UpdateTranslations))
            );
    }
}

void FLocalizationImporterModule::OnUpdateTranslationDialogClosed(const TSharedRef<SWindow>& Window)
{
    UpdateTranslationsDialogWindow = nullptr;
	UpdateTranslationsDialog = nullptr;
}

bool FLocalizationImporterModule::Tick(const float DeltaTime)
{
	/*
	 * Initially this function was called in StartupModule to register
	 * a localization option with the Dashboard option. But calling it that
	 * early registers it before anything in that Window tab and somehow causes
	 * every localization commandlet to crash. So it's called here
	 * after the engine is initialized.
	 */
	AddMenuExtension();
	return false;
}

void FLocalizationImporterModule::UpdateTranslations()
{
    if(UpdateTranslationsDialogWindow.IsValid())
    {
	    UpdateTranslationsDialogWindow->BringToFront();
    }
    else
    {
	    UpdateTranslationsDialogWindow = SNew(SWindow)
    		.Title(LOCTEXT("UpdateTranslationsWindowTitle", "Update Translations | ImportSettings"))
    		.SupportsMaximize(false)
    		.SupportsMinimize(false)
    		.SizingRule(ESizingRule::Autosized);

    	UpdateTranslationsDialogWindow->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FLocalizationImporterModule::OnUpdateTranslationDialogClosed));
    	UpdateTranslationsDialogWindow->SetContent(
    		SNew(SBox)
    		.WidthOverride(700.0f)
    		[
    			SAssignNew(UpdateTranslationsDialog, SImportTranslationsDialog)
    			.ParentWindow(UpdateTranslationsDialogWindow)
    		]);

    	TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
    	if(RootWindow)
    	{
    		FSlateApplication::Get().AddWindowAsNativeChild(UpdateTranslationsDialogWindow.ToSharedRef(), RootWindow.ToSharedRef());
    	}
        else
        {
	        FSlateApplication::Get().AddWindow(UpdateTranslationsDialogWindow.ToSharedRef());
        }
    }
}

void FLocalizationImporterModule::ShutdownModule()
{
    UToolMenus::UnregisterOwner(this);

	if(TickHandle.IsValid())
	{
		FTicker::GetCoreTicker().RemoveTicker(TickHandle);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLocalizationImporterModule, LocalizationImporter)