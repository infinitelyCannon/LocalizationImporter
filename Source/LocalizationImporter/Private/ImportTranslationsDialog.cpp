// Copyright (C) 2022 Dakarai Simmons - All Rights Reserved

#include "ImportTranslationsDialog.h"
#include "PythonBridge.h"
#include "LICommandletExecutor.h"
#include "DesktopPlatformModule.h"
#include "EditorDirectories.h"
#include "EditorStyleSet.h"
#include "FileHelpers.h"
#include "IDesktopPlatform.h"
#include "ISourceControlModule.h"
#include "Components/VerticalBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "LocalizationCommandletExecution.h"
#include "LocalizationConfigurationScript.h"
#include "LocalizationSettings.h"
#include "LocalizationTargetTypes.h"
#include "Misc/MessageDialog.h"
#include "SourceControlOperations.h"
#include "HAL/PlatformFilemanager.h"
#include "Interfaces/IPluginManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"


#define LOCTEXT_NAMESPACE "ImportTranslationsDialog"

void SImportTranslationsDialog::Construct(const FArguments& InArgs)
{
	ParentWindowPtr = InArgs._ParentWindow;

	ChildSlot
	[
		SNew(SBorder)
		.HAlign(HAlign_Fill)
		.BorderImage(FEditorStyle::GetBrush("ChildWindow.Background"))
		.Padding(4.0f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f)
			[
				SNew(SBox)
				.WidthOverride(500)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("DetailsView.CategoryBottom"))
					.Padding(FMargin(4.0f, 12.0f))
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("FileName", "Please select a spreadsheet"))
							]
							+SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(this, &SImportTranslationsDialog::GetFileButtonText)
								.OnClicked(this, &SImportTranslationsDialog::ChooseFile)
							]
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 20.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ScriptSection", "Options"))
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(2.0f)
						.VAlign(VAlign_Center)
						[
							SNew(SSeparator)
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
                                .Text(LOCTEXT("PagesLabel", "Pages:"))
							]
							+SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SComboButton)
								.ToolTipText(LOCTEXT("PageComboTooltip", "Select which pages to pull translations from"))
								.OnGetMenuContent(this, &SImportTranslationsDialog::GeneratePageSelector)
								.IsEnabled(this, &SImportTranslationsDialog::IsPageButtonEnabled)
								.ButtonContent()
								[
									SNew(STextBlock)
									.Text(this, &SImportTranslationsDialog::GetPageButtonText)
								]
							]
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 3.0f)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("LangLabel", "Languages:"))
							]
							+SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SComboButton)
								.ToolTipText(LOCTEXT("LangComboTooltip", "Select which language(s) to update"))
								.OnGetMenuContent(this, &SImportTranslationsDialog::GenerateLanguageSelector)
								.IsEnabled(this, &SImportTranslationsDialog::IsLangButtonEnabled)
								.ButtonContent()
								[
									SNew(STextBlock)
									.Text(this, &SImportTranslationsDialog::GetLangButtonText)
								]
							]
						]
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 0.0f, 5.0f, 0.0f)
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged(this, &SImportTranslationsDialog::OnCaseChecked)
							]
							+SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("CaseText", "Case-Sensitive"))
							]
							+SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(10.0f, 0.0f, 5.0f, 0.0f)
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.OnCheckStateChanged(this, &SImportTranslationsDialog::OnForceRefreshChecked)
								.ToolTipText(LOCTEXT("ForceRefreshTooltip", "If checked all items will be updated instead of just empty ones."))
							]
							+SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ForceRefresh","Force Full Refresh"))
								.ToolTipText(LOCTEXT("ForceRefreshTooltip", "If checked all items will be updated instead of just empty ones."))
							]
						]
					]
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 16.0f, 8.0f, 8.0f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Right)
				[
					SNew(SUniformGridPanel)
					.SlotPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
					+SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("ProceedBtn", "Proceed..."))
						.OnClicked(this, &SImportTranslationsDialog::OnAcceptSettings)
						.IsEnabled(this, &SImportTranslationsDialog::IsProceedButtonEnabled)
					]
					+SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("CancleBtn", "Cancel"))
						.OnClicked(this, &SImportTranslationsDialog::OnCancelSettings)
						// Maybe add an 'IsEnabled' method to make sure they can't cancel during the process
					]
				]
			]
		]
	];
}

bool SImportTranslationsDialog::CheckOutOrAddFile(const FString& File, bool ForceSourceControlUpdate, bool ShowErrorInNotification, FText* OutErrorMsg)
{
	FText errorMessage;
	bool bSuccessfullyCheckedOutOrAddedFile = false;

	if(ISourceControlModule::Get().IsEnabled())
	{
		ISourceControlProvider &SourceControlProvider = ISourceControlModule::Get().GetProvider();
		FSourceControlStatePtr SourceControlState = SourceControlProvider.GetState(File, ForceSourceControlUpdate ? EStateCacheUsage::ForceUpdate : EStateCacheUsage::Use);

		TArray<FString> filesToCheckOut;
		filesToCheckOut.Add(File);

		if(SourceControlState.IsValid())
		{
			if(SourceControlState->IsSourceControlled())
			{
				if(SourceControlState->IsDeleted())
					errorMessage = LOCTEXT("ConfigFileMarkedForDeleteError", "Error: The configuration file is marked for deletion.");

				// Note: Here we attempt to check out files that are read only even if the internal state says they cannot be checked out.
				// This is to work around cases were the file is reverted or checked in and the internal state has not been updated yet
				else if(SourceControlState->CanCheckout() || SourceControlState->IsCheckedOutOther() || FPlatformFileManager::Get().GetPlatformFile().IsReadOnly(*File))
				{
					ECommandResult::Type CommandResult = SourceControlProvider.Execute(ISourceControlOperation::Create<FCheckOut>(), filesToCheckOut);

					if(CommandResult == ECommandResult::Failed)
						errorMessage = LOCTEXT("FailedToCheckOutConfigFileError", "Error: Failed to check out the configuration file.");
					else if(CommandResult == ECommandResult::Cancelled)
						errorMessage = LOCTEXT("CancelledCheckOutConfigFile", "Checkout was cancelled.  File will be marked writable.");
					else
						bSuccessfullyCheckedOutOrAddedFile = true;
				}
			}
			else if(!SourceControlState->IsUnknown())
			{
				if(!FPlatformFileManager::Get().GetPlatformFile().FileExists(*File))
					return true; // hasn't been created yet

				ECommandResult::Type CommandResult = SourceControlProvider.Execute(ISourceControlOperation::Create<FMarkForAdd>(), filesToCheckOut);

				if(CommandResult == ECommandResult::Failed)
					errorMessage = LOCTEXT("FailedToAddConfigFileError", "Error: Failed to add the configuration file.");
				else if(CommandResult == ECommandResult::Cancelled)
					errorMessage = LOCTEXT("CancelledAddConfigFile", "Add was cancelled.  File will be marked writable.");
				else
					bSuccessfullyCheckedOutOrAddedFile = true;
			}
		}		
	}

	if(!errorMessage.IsEmpty())
	{
		if(OutErrorMsg != nullptr)
			*OutErrorMsg = errorMessage;

		if(ShowErrorInNotification)
		{
			// Show a notification that the file could not be checked out
			FNotificationInfo CheckOutError(errorMessage);
			CheckOutError.ExpireDuration = 3.0f;
			FSlateNotificationManager::Get().AddNotification(CheckOutError);
		}
	}

	return bSuccessfullyCheckedOutOrAddedFile;
}

FReply SImportTranslationsDialog::ChooseFile()
{
	TArray<FString> OpenedFiles;
	const FString DefaultLocation(FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT));
	IDesktopPlatform *Platform = FDesktopPlatformModule::Get();
	bool bOpened = false;

	if(Platform)
	{
		bOpened = Platform->OpenFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			LOCTEXT("FileDialogTitle", "Please select a spreadsheet").ToString(),
			DefaultLocation,
			TEXT(""),
			FString("Spreadsheet files (*.xlsx)|*.xlsx"),
			EFileDialogFlags::None,
			OpenedFiles
		);
	}

	if(bOpened && OpenedFiles.Num() > 0)
	{
		SpreadsheetPath = OpenedFiles[0];
		
		UPythonBridge *bridge = UPythonBridge::Get();
		
		if(IsValid(bridge))
		{
			TArray<FUpdateTranslationsSettings> pages = bridge->ImportSpreadsheet(SpreadsheetPath);
			if(pages.Num() > 0)
			{
				for(int i = 0; i < pages.Num(); ++i)
				{
					if(pages[i].Checked)
						SelectedLanguages.Add(MakeShareable(new FUpdateTranslationsSettings(pages[i].Title, true)));
					else
						SelectedPages.Add(MakeShareable(new FUpdateTranslationsSettings(pages[i].Title, true)));
				}
			}
		}
	}
	
	return FReply::Handled();
}

TSharedRef<SWidget> SImportTranslationsDialog::GenerateLanguageSelector()
{
	return SNew(SBox)
	[
		SNew(SListView<TSharedPtr<FUpdateTranslationsSettings>>)
		.SelectionMode(ESelectionMode::None)
		.ListItemsSource(&SelectedLanguages)
		.OnGenerateRow(this, &SImportTranslationsDialog::OnGenerateLanguagesRow)
	];
}

TSharedRef<SWidget> SImportTranslationsDialog::GeneratePageSelector()
{
	return SNew(SBox)
	[
		SNew(SListView<TSharedPtr<FUpdateTranslationsSettings>>)
		.SelectionMode(ESelectionMode::None)
		.ListItemsSource(&SelectedPages)
		.OnGenerateRow(this, &SImportTranslationsDialog::OnGeneratePagesRow)
	];
}

FText SImportTranslationsDialog::GetFileButtonText() const
{
	int32 lastSlash;
	int32 dotIdx;
	
	if(!SpreadsheetPath.FindLastChar('/', lastSlash))
		SpreadsheetPath.FindLastChar('\\', lastSlash);

	SpreadsheetPath.FindLastChar('.', dotIdx);
	
	return SpreadsheetPath.IsEmpty() ? LOCTEXT("FileButtonText", "Choose File...") :
	FText::FromString(SpreadsheetPath.Mid(lastSlash + 1));
}

FText SImportTranslationsDialog::GetPageButtonText() const
{
	return IsPageButtonEnabled() ? LOCTEXT("PageBtnText", "Select Pages") :
	LOCTEXT("DisabledPageBtnText", "No Spreadsheet Selected");
}

FText SImportTranslationsDialog::GetLangButtonText() const
{
	return IsLangButtonEnabled() ? LOCTEXT("LangBtnText", "Select Languages") :
	LOCTEXT("DisabledLangBtnText", "No Languages Found");
}

EVisibility SImportTranslationsDialog::GetSettingsVisibility() const
{
	return EVisibility::Visible;
}

bool SImportTranslationsDialog::IsPageButtonEnabled() const
{
	return SelectedPages.Num() > 0;
}

ECheckBoxState SImportTranslationsDialog::IsPageSettingChecked(const TSharedPtr<FUpdateTranslationsSettings> item) const
{
	return item->Checked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

bool SImportTranslationsDialog::IsProceedButtonEnabled() const
{
	return !SpreadsheetPath.IsEmpty();
}

bool SImportTranslationsDialog::IsLangButtonEnabled() const
{
	return SelectedLanguages.Num() > 0;
}

ECheckBoxState SImportTranslationsDialog::IsLanguageSettingChecked(const TSharedPtr<FUpdateTranslationsSettings> item) const
{
	return item->Checked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

bool SImportTranslationsDialog::MakeWritable(const FString& file, bool showErrorInNotification, FText* outErrorMessage)
{
	if(!FPlatformFileManager::Get().GetPlatformFile().FileExists(*file))
		return true;

	bool bSuccess = FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*file, false);
	if(!bSuccess)
	{
		FText errorMessage = FText::Format(LOCTEXT("FailedToMakeWritable", "Could not make {0} writable."), FText::FromString(file));

		if(outErrorMessage != nullptr)
			*outErrorMessage = errorMessage;

		if(showErrorInNotification)
		{
			FNotificationInfo MakeWritableNotification(errorMessage);
			MakeWritableNotification.ExpireDuration = 3.0f;

			FSlateNotificationManager::Get().AddNotification(MakeWritableNotification);
		}
	}

	return bSuccess;
}

FReply SImportTranslationsDialog::OnAcceptSettings()
{
	// From LocalizationTargetDetailCustomization.cpp::GatherText() Line 858
	// Save unsaved packages.

	const bool bPromptUserToSave = true;
	const bool bSaveMapPackages = true;
	const bool bSaveContentPackages = true;
	const bool bFastSave = false;
	const bool bNotifyNoPackagesSaved = false;
	const bool bCanBeDeclined = true;
	bool DidPackagesNeedSaving;
	const bool WerePackagesSaved = FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages, bFastSave, bNotifyNoPackagesSaved, bCanBeDeclined, &DidPackagesNeedSaving);

	if(DidPackagesNeedSaving && !WerePackagesSaved)
	{
		const FText MessageText = LOCTEXT("UnsavedPackagesWarningMsg", "There are unsaved changes. These changes may not be gathered from correctly.");
		const FText TitleText = LOCTEXT("UnsavedPackagesWarningTitle", "Unsaved Changes Before Gather");

		FMessageDialog::Open(EAppMsgType::Ok, MessageText, &TitleText);

		return FReply::Handled();
	}

	{
		FString ConfigFilePath = FPaths::ProjectConfigDir() / "DefaultEditor.ini";
		bool bSuccess = CheckOutOrAddFile(ConfigFilePath, true);

		if(!bSuccess)
			bSuccess = MakeWritable(ConfigFilePath);
	}

	const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(this->AsShared());

	// From LocalizationCommandletTasks.cpp::GatherTextForTarget() Line 37
	ULocalizationTargetSet *GameTargetSet = ULocalizationSettings::GetGameTargetSet();
	TArray<ULocalizationTarget*> Targets;

	if(GameTargetSet)
		Targets.Append(GameTargetSet->TargetObjects);

	if(Targets.Num() > 0)
	{
		{
			UPythonBridge *Bridge = UPythonBridge::Get();

			if(Bridge)
			{
				FString Pages = "";
				FString Langs = "";
				int32 PageLen = 0, LangLen = 0;

				for(int i = 0; i < SelectedPages.Num(); ++i)
				{
					if(SelectedPages[i]->Checked)
					{
						Pages += SelectedPages[i]->Title + "\n";
						++PageLen;
					}
				}					

				for(int i = 0; i < SelectedLanguages.Num(); ++i)
				{
					if(SelectedLanguages[i]->Checked)
					{
						Langs += SelectedLanguages[i]->Title + "\n";
						++LangLen;
					}
				}

				Bridge->UpdateSelection(SpreadsheetPath, FString::Printf(TEXT("%d %d\n%s"), PageLen, LangLen, *Pages), Langs, bForceRefresh, IsCaseSensitive);
			}
		}
		
		ULocalizationTarget *LocalizationTarget = Targets[0];
		TArray<LocalizationCommandletExecution::FTask> Tasks;
        const bool bShouldUseProjectFile = true; // True because we're targeting game content separate from engine

		// GatherText setup
        const FString GatherScriptPath = LocalizationConfigurationScript::GetGatherTextConfigPath(LocalizationTarget);
		LocalizationConfigurationScript::GenerateGatherTextConfigFile(LocalizationTarget).WriteWithSCC(GatherScriptPath);

		Tasks.Add(LocalizationCommandletExecution::FTask(LOCTEXT("GatherTaskName", "Gather Text"), GatherScriptPath, bShouldUseProjectFile));

		// ExportText setup
		const FString ExportPath = FPaths::ConvertRelativePathToFull(LocalizationConfigurationScript::GetDataDirectory(LocalizationTarget));
		const FString ExportScriptPath = LocalizationConfigurationScript::GetExportTextConfigPath(LocalizationTarget, TOptional<FString>());
		LocalizationConfigurationScript::GenerateExportTextConfigFile(LocalizationTarget, TOptional<FString>(), ExportPath).WriteWithSCC(ExportScriptPath);
		
		Tasks.Add(LocalizationCommandletExecution::FTask(LOCTEXT("ExportTaskName", "Export Translations"), ExportScriptPath, bShouldUseProjectFile));

		//Super Special Python Task
		const FString PluginPath = IPluginManager::Get().FindPlugin("LocalizationImporter")->GetBaseDir();
		const FString PythonScriptPath = FPaths::Combine(*PluginPath, TEXT("Content/Python"));
		
		// Apparently there's a bug in 4.24 where the python command can take files directly,
		// but won't work if the path has spaces (even with quotes).
		// This is a workaround to pass in literal code that imports and runs the script.
		const FString PythonCode = FString::Printf(TEXT("import os\\nimport sys\\nsys.path.append('%s')\\nfrom update_translations import *\\nupdate()"), *PythonScriptPath);
		Tasks.Add(LocalizationCommandletExecution::FTask(LOCTEXT("PythonTask","(Python) Update Translations"), PythonCode, true));

		// ImportText setup
		const FString ImportScriptPath = LocalizationConfigurationScript::GetImportTextConfigPath(LocalizationTarget, TOptional<FString>());
		LocalizationConfigurationScript::GenerateImportTextConfigFile(LocalizationTarget, TOptional<FString>(), ExportPath).WriteWithSCC(ImportScriptPath);
		Tasks.Add(LocalizationCommandletExecution::FTask(LOCTEXT("ImportTaskName", "Import Translations"), ImportScriptPath, bShouldUseProjectFile));

		const FString ReportScriptPath = LocalizationConfigurationScript::GetWordCountReportConfigPath(LocalizationTarget);
		LocalizationConfigurationScript::GenerateWordCountReportConfigFile(LocalizationTarget).WriteWithSCC(ReportScriptPath);
		Tasks.Add(LocalizationCommandletExecution::FTask(LOCTEXT("ReportTaskName", "Generate Reports"), ReportScriptPath, bShouldUseProjectFile));

		// CompileText setup
		const FString CompileScriptPath = LocalizationConfigurationScript::GetCompileTextConfigPath(LocalizationTarget);
		LocalizationConfigurationScript::GenerateCompileTextConfigFile(LocalizationTarget).WriteWithSCC(CompileScriptPath);
		Tasks.Add(LocalizationCommandletExecution::FTask(LOCTEXT("CompileTaskName", "Compile Translations"), CompileScriptPath, bShouldUseProjectFile));

		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("TargetName"), FText::FromString(LocalizationTarget->Settings.Name));
		const FText windowTitle = FText::Format(LOCTEXT("LocalizationTaskWindowTitle", "Updating Translations for ({TargetName})"), Arguments);
		const TSharedRef<SWindow> CommandletWindow = SNew(SWindow)
		.Title(windowTitle)
		.SupportsMinimize(false)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.ClientSize(FVector2D(600, 400))
		.ActivationPolicy(EWindowActivationPolicy::Always)
		.FocusWhenFirstShown(true);
		const TSharedRef<SLICommandletExecutor> CommandletExecutor = SNew(SLICommandletExecutor, CommandletWindow, Tasks);
		CommandletWindow->SetContent(CommandletExecutor);

		FSlateApplication::Get().AddModalWindow(CommandletWindow, ParentWindow, false);

		bool bSuccessful = CommandletExecutor->WasSuccessful();

		if(bSuccessful)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("TaskPassed", "Update Successful!"));
			if(ParentWindowPtr.IsValid())
            {
            	ParentWindowPtr.Pin()->RequestDestroyWindow();
            }
		}			
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("TaskFailed", "Update Failed\nPlease Check your setup and try again."));
			if(ParentWindowPtr.IsValid())
			{
				ParentWindowPtr.Pin()->RequestDestroyWindow();
			}
		}
	}
	else
	{
		const FText ErrMessageText = LOCTEXT("TargetErrorMsg", "Could not find suitable target for localization");
		const FText ErrMessageTitle = LOCTEXT("TargetErrorTitle", "Error!");
		
		FMessageDialog::Open(EAppMsgType::Ok, ErrMessageText,&ErrMessageTitle);
	}
	
	return FReply::Handled();
}

FReply SImportTranslationsDialog::OnCancelSettings()
{
	if(ParentWindowPtr.IsValid())
	{
		ParentWindowPtr.Pin()->RequestDestroyWindow();
	}

	return FReply::Handled();
}

void SImportTranslationsDialog::OnCaseChecked(ECheckBoxState state)
{
	switch (state)
	{
	case ECheckBoxState::Checked:
		IsCaseSensitive = true;
		break;
	case ECheckBoxState::Unchecked:
		IsCaseSensitive = false;
		break;
	case ECheckBoxState::Undetermined:
		IsCaseSensitive = false;
		break;
	}	
}

void SImportTranslationsDialog::OnForceRefreshChecked(ECheckBoxState state)
{
	switch (state)
	{
	case ECheckBoxState::Checked:
		bForceRefresh = true;
		break;
	case ECheckBoxState::Unchecked:
	case ECheckBoxState::Undetermined:
		bForceRefresh = false;
		break;
	}
}

TSharedRef<ITableRow> SImportTranslationsDialog::OnGeneratePagesRow(TSharedPtr<FUpdateTranslationsSettings> item, const TSharedRef<STableViewBase>& table)
{
	return SNew(STableRow<TSharedPtr<FUpdateTranslationsSettings>>, table)
	.ShowSelection(true)
	.Content()
	[
		SNew(SCheckBox)
		.OnCheckStateChanged_Lambda([this, item](ECheckBoxState state)
		{
			OnUpdatePageSettings(item, state);
		})
		.IsChecked_Lambda([this, item]()->ECheckBoxState
		{
			return IsPageSettingChecked(item);
		})
		.Content()
		[
			SNew(STextBlock)
			.Text(FText::FromString(item->Title))
		]
	];
}

TSharedRef<ITableRow> SImportTranslationsDialog::OnGenerateLanguagesRow(TSharedPtr<FUpdateTranslationsSettings> item, const TSharedRef<STableViewBase>& table)
{
	return SNew(STableRow<TSharedPtr<FUpdateTranslationsSettings>>, table)
	.ShowSelection(true)
	.Content()
	[
		SNew(SCheckBox)
		.OnCheckStateChanged_Lambda([this, item](ECheckBoxState state)
		{
			OnUpdateLanguageSettings(item, state);
		})
		.IsChecked_Lambda([this, item]()->ECheckBoxState
		{
			return IsLanguageSettingChecked(item);
		})
		.Content()
		[
			SNew(STextBlock)
			.Text(FText::FromString(item->Title))
		]
	];
}

void SImportTranslationsDialog::OnUpdatePageSettings(const TSharedPtr<FUpdateTranslationsSettings> item, const ECheckBoxState state)
{
	if(state == ECheckBoxState::Checked)
		item->Checked = true;
	else
		item->Checked = false;
}

void SImportTranslationsDialog::OnUpdateLanguageSettings(const TSharedPtr<FUpdateTranslationsSettings> item, const ECheckBoxState state)
{
	if(state == ECheckBoxState::Checked)
		item->Checked = true;
	else
		item->Checked = false;
}

#undef LOCTEXT_NAMESPACE