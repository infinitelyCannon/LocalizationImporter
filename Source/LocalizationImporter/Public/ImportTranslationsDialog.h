// Copyright (C) 2022 Dakarai Simmons - All Rights Reserved

#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Views/STableRow.h"
#include "LocalizationImporterTypes.h"

class SImportTranslationsDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SImportTranslationsDialog) {}
	SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs);

private:
	// Callback for when the 'Proceed' button is clicked
	FReply OnAcceptSettings();

	// Callback for when the 'Cancel' button is clicked
	FReply OnCancelSettings();

	FReply ChooseFile();
	
	// Delegate to determine 'Proceed' button enabled state
	bool IsProceedButtonEnabled() const;

	// Delegate to determine visibility of excel settings
	EVisibility GetSettingsVisibility() const;

	TSharedRef<SWidget> GeneratePageSelector();
	TSharedRef<SWidget> GenerateLanguageSelector();
	bool IsPageButtonEnabled() const;
	bool IsLangButtonEnabled() const;
	FText GetPageButtonText() const;
	FText GetLangButtonText() const;
	FText GetFileButtonText() const;
	void OnCaseChecked(ECheckBoxState State);
	void OnForceRefreshChecked(ECheckBoxState State);

	// Source Control Checking (taken from Engine source)
	bool CheckOutOrAddFile(const FString &File, bool ForceSourceControlUpdate = false, bool ShowErrorInNotification = true, FText *OutErrorMsg = nullptr);
	bool MakeWritable(const FString &File, bool ShowErrorInNotification = true, FText *OutErrorMsg = nullptr);

	TSharedRef<ITableRow> OnGeneratePagesRow(TSharedPtr<FUpdateTranslationsSettings> Item, const TSharedRef<STableViewBase> &Table);
	TSharedRef<ITableRow> OnGenerateLanguagesRow(TSharedPtr<FUpdateTranslationsSettings> Item, const TSharedRef<STableViewBase> &Table);
	ECheckBoxState IsPageSettingChecked(const TSharedPtr<FUpdateTranslationsSettings> Item) const;
	ECheckBoxState IsLanguageSettingChecked(const TSharedPtr<FUpdateTranslationsSettings> Item) const;
	void OnUpdatePageSettings(const TSharedPtr<FUpdateTranslationsSettings> Item, const ECheckBoxState State);
	void OnUpdateLanguageSettings(const TSharedPtr<FUpdateTranslationsSettings> Item, const ECheckBoxState State);

	TWeakPtr<SWindow> ParentWindowPtr;
	TArray<TSharedPtr<FUpdateTranslationsSettings>> SelectedPages;
	TArray<TSharedPtr<FUpdateTranslationsSettings>> SelectedLanguages;
	bool IsCaseSensitive = false;
	bool bForceRefresh = false;
	FString SpreadsheetPath = "";
};
