// Copyright (C) 2022 Dakarai Simmons - All Rights Reserved

#include "LICommandletExecutor.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorStyle.h"
#include "SourceControlHelpers.h"
#include "Widgets/Images/SThrobber.h"
#include "HAL/PlatformApplicationMisc.h"
#include "DesktopPlatformModule.h"
#include "LocalizationConfigurationScript.h"
#include "LocalizationSettings.h"
#include "UnrealEdMisc.h"
#include "Android/AndroidErrorOutputDevice.h"
#include "Commandlets/CommandletHelpers.h"

#define LOCTEXT_NAMESPACE "LICommandletExe"

SLICommandletExecutor::SLICommandletExecutor() :
CurrentTaskIndex(INDEX_NONE),
Runnable(nullptr),
RunnableThread(nullptr)
{}

void SLICommandletExecutor::Construct(const FArguments& Arguments, const TSharedRef<SWindow>& InParentWindow, const TArray<LocalizationCommandletExecution::FTask>& Tasks)
{
	ParentWindow = InParentWindow;

	for (const LocalizationCommandletExecution::FTask& Task : Tasks)
	{
		const TSharedRef<FTaskListModel> Model = MakeShareable(new FTaskListModel());
		Model->Task = Task;
		TaskListModels.Add(Model);
	}

	TSharedPtr<SScrollBar> VerticalScrollBar;
	TSharedPtr<SScrollBar> HorizontalScrollBar;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0, 16.0, 16.0, 0.0)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(this, &SLICommandletExecutor::GetProgressMessageText)
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 4.0f, 0.0f, 0.0)
			[
				SAssignNew(ProgressBar, SProgressBar)
				.Percent(this, &SLICommandletExecutor::GetProgressPercentage)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(0.5)
		.Padding(0.0, 32.0, 8.0, 0.0)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(0.0f)
			[
				SAssignNew(TaskListView, SListView< TSharedPtr<FTaskListModel> >)
				.HeaderRow
				(
				SNew(SHeaderRow)
				+ SHeaderRow::Column("StatusIcon")
				.DefaultLabel(FText::GetEmpty())
				.FixedWidth(20.0)
				+ SHeaderRow::Column("TaskName")
				.DefaultLabel(LOCTEXT("TaskListNameColumnLabel", "Task"))
				.FillWidth(1.0)
				)
				.ListItemsSource(&TaskListModels)
				.OnGenerateRow(this, &SLICommandletExecutor::OnGenerateTaskListRow)
				.ItemHeight(24.0)
				.SelectionMode(ESelectionMode::Single)
			]
		]
	+ SVerticalBox::Slot()
		.FillHeight(0.5)
		.Padding(0.0, 32.0, 8.0, 0.0)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(0.0f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SMultiLineEditableText)
						.TextStyle(FEditorStyle::Get(), "LocalizationDashboard.CommandletLog.Text")
						.Text(this, &SLICommandletExecutor::GetLogString)
						.IsReadOnly(true)
						.HScrollBar(HorizontalScrollBar)
						.VScrollBar(VerticalScrollBar)
					]
					+SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(HorizontalScrollBar, SScrollBar)
							.Orientation(EOrientation::Orient_Horizontal)
						]
				]
				+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(VerticalScrollBar, SScrollBar)
						.Orientation(EOrientation::Orient_Vertical)
					]
			]
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 5.0f, 0.0f, 0.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.ContentPadding(FMargin(6.0f, 2.0f))
				.Text(LOCTEXT("CopyLogButtonText", "Copy Log"))
				.ToolTipText(LOCTEXT("CopyLogButtonTooltip", "Copy the logged text to the clipboard."))
				.OnClicked(this, &SLICommandletExecutor::OnCopyLogClicked)
			]
			+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.ContentPadding(FMargin(6.0f, 2.0f))
					.Text(LOCTEXT("SaveLogButtonText", "Save Log..."))
					.ToolTipText(LOCTEXT("SaveLogButtonToolTip", "Save the logged text to a file."))
					.OnClicked(this, &SLICommandletExecutor::OnSaveLogClicked)
				]
			+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.ContentPadding(FMargin(6.0f, 2.0f))
					.OnClicked(this, &SLICommandletExecutor::OnCloseButtonClicked)
					[
						SNew(STextBlock)
						.Text(this, &SLICommandletExecutor::GetCloseButtonText)
					]
				]
		]
	];

	if(TaskListModels.Num() > 0)
	{
		CurrentTaskIndex = 0;
		ExecuteCommandlet(TaskListModels[CurrentTaskIndex].ToSharedRef());

		if (TaskListView.IsValid())
		{
			TaskListView->SetSelection(TaskListModels[CurrentTaskIndex]);
		}
	}
}

void SLICommandletExecutor::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Poll for log output data.
	if (!PendingLogData.String.IsEmpty())
	{
		FString String;

		// Copy the pending data string to the local string
		{
			FScopeLock ScopeLock(&PendingLogData.CriticalSection);
			String = PendingLogData.String;
			PendingLogData.String.Empty();
		}

		// Forward string to proper log.
		if (TaskListModels.IsValidIndex(CurrentTaskIndex))
		{
			const TSharedPtr<FTaskListModel> CurrentTaskModel = TaskListModels[CurrentTaskIndex];
			CurrentTaskModel->LogOutput.Append(String);
		}
	}

	// On Task Completed.
	if (CommandletProcess.IsValid())
	{
		FProcHandle CurrentProcessHandle = CommandletProcess->GetHandle();
		int32 ReturnCode;
		if (CurrentProcessHandle.IsValid() && FPlatformProcess::GetProcReturnCode(CurrentProcessHandle, &ReturnCode))
		{
			OnCommandletProcessCompletion(ReturnCode);
		}
	}
}

bool SLICommandletExecutor::WasSuccessful() const
{
	return HasCompleted() && !TaskListModels.ContainsByPredicate([](const TSharedPtr<FTaskListModel>& TaskListModel){return TaskListModel->State != FTaskListModel::EState::Succeeded;});
}

void SLICommandletExecutor::Log(const FString& String)
{
	FScopeLock ScopeLock(&PendingLogData.CriticalSection);
	PendingLogData.String += String;
}

void SLICommandletExecutor::OnCommandletProcessCompletion(const int32 ReturnCode)
{
	CleanUpProcessAndPump();

	// Handle return code.
	TSharedPtr<FTaskListModel> CurrentTaskModel = TaskListModels[CurrentTaskIndex];

	// Restore engine's source control settings if necessary.
	if (!CurrentTaskModel->Task.ShouldUseProjectFile)
	{
		const FString& EngineIniFile = SourceControlHelpers::GetGlobalSettingsIni();
		const FString BackupEngineIniFile = FPaths::EngineSavedDir() / FPaths::GetCleanFilename(EngineIniFile) + TEXT(".bak");
		if(!IFileManager::Get().Move(*EngineIniFile, *BackupEngineIniFile))
		{
			// (From engine source code) TODO: Error failed to restore engine source control settings INI.
		}
	}

	// Zero code is successful.
	if (ReturnCode == 0)
	{
		CurrentTaskModel->State = FTaskListModel::EState::Succeeded;

		++CurrentTaskIndex;

		// Begin new task if possible.
		if (TaskListModels.IsValidIndex(CurrentTaskIndex))
		{
			CurrentTaskModel = TaskListModels[CurrentTaskIndex];
			
			ExecuteCommandlet(CurrentTaskModel.ToSharedRef());

			if (TaskListView.IsValid())
			{
				TaskListView->SetSelection(TaskListModels[CurrentTaskIndex]);
			}
		}
	}
	// Non-zero is a failure.
	else
	{
		CurrentTaskModel->State = FTaskListModel::EState::Failed;
	}
}

TSharedPtr<FLICommandletProcess> FLICommandletProcess::Execute(const FString& ConfigFilePath, const bool UseProjectFile)
{
	// Create pipes.
	void* ReadPipe;
	void* WritePipe;
	if (!FPlatformProcess::CreatePipe(ReadPipe, WritePipe))
	{
		return nullptr;
	}

	// Create process.
	const bool isPython = ConfigFilePath.Contains(TEXT("update_translations"));
	FString CommandletArguments;
	
	const FString ConfigFileRelativeToGameDir = LocalizationConfigurationScript::MakePathRelativeForCommandletProcess(ConfigFilePath, UseProjectFile);
	CommandletArguments = isPython ? FString::Printf(TEXT("-script=\"%s\""), *ConfigFilePath) : FString::Printf( TEXT("-config=\"%s\""), *ConfigFileRelativeToGameDir );

	if (FLocalizationSourceControlSettings::IsSourceControlEnabled())
	{
		CommandletArguments += TEXT(" -EnableSCC");

		if (!FLocalizationSourceControlSettings::IsSourceControlAutoSubmitEnabled())
		{
			CommandletArguments += TEXT(" -DisableSCCSubmit");
		}
	}

	const FString ProjectFilePath = FString::Printf(TEXT("\"%s\""), *FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()));
	const FString ProcessArguments = CommandletHelpers::BuildCommandletProcessArguments(isPython ? TEXT("pythonscript") : TEXT("GatherText"), UseProjectFile ? *ProjectFilePath : nullptr, *CommandletArguments);
	FProcHandle CommandletProcessHandle = FPlatformProcess::CreateProc(*FUnrealEdMisc::Get().GetExecutableForCommandlets(), *ProcessArguments, true, true, true, nullptr, 0, nullptr, WritePipe);

	// Close pipes if process failed.
	if (!CommandletProcessHandle.IsValid())
	{
		FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
		return nullptr;
	}

	return MakeShareable(new FLICommandletProcess(ReadPipe, WritePipe, CommandletProcessHandle, ProcessArguments));
}

FLICommandletProcess::~FLICommandletProcess()
{
	if (ProcessHandle.IsValid() && FPlatformProcess::IsProcRunning(ProcessHandle))
	{
		FPlatformProcess::TerminateProc(ProcessHandle);
	}
	FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
}

void SLICommandletExecutor::ExecuteCommandlet(const TSharedRef<FTaskListModel>& TaskListModel)
{
	// Handle source control settings if not using project file for commandlet executable process.
	if (!TaskListModel->Task.ShouldUseProjectFile)
	{
		const FString& EngineIniFile = SourceControlHelpers::GetGlobalSettingsIni();

		// Backup engine's source control settings.
		const FString BackupEngineIniFile = FPaths::EngineSavedDir() / FPaths::GetCleanFilename(EngineIniFile) + TEXT(".bak");
		if (COPY_OK == IFileManager::Get().Copy(*BackupEngineIniFile, *EngineIniFile))
		{
			// Replace engine's source control settings with project's.
			const FString& ProjectIniFile = SourceControlHelpers::GetSettingsIni();
			if (COPY_OK == IFileManager::Get().Copy(*EngineIniFile, *ProjectIniFile))
			{
				// (From engine source code) TODO: Error failed to overwrite engine source control settings INI.
			}
		}
		else
		{
			// (From engine source code) TODO: Error failed to backup engine source control settings INI.
		}
	}

	CommandletProcess = FLICommandletProcess::Execute(TaskListModel->Task.ScriptPath, TaskListModel->Task.ShouldUseProjectFile);
	
	if (CommandletProcess.IsValid())
	{
		TaskListModel->State = FTaskListModel::EState::InProgress;
		TaskListModel->ProcessArguments = CommandletProcess->GetProcessArguments();
	}
	else
	{
		TaskListModel->State = FTaskListModel::EState::Failed;
		return;
	}

	class FCommandletLogPump : public FRunnable
	{
	public:
		FCommandletLogPump(void* const InReadPipe, const FProcHandle& InCommandletProcessHandle, SLICommandletExecutor& InCommandletWidget)
			: ReadPipe(InReadPipe)
			, CommandletProcessHandle(InCommandletProcessHandle)
			, CommandletWidget(&InCommandletWidget)
		{
		}

		uint32 Run() override
		{
			for(;;)
			{
				// Read from pipe.
				const FString PipeString = FPlatformProcess::ReadPipe(ReadPipe);

				// Process buffer.
				if (!PipeString.IsEmpty())
				{
					// Add strings to log.
					if (CommandletWidget)
					{
						CommandletWidget->Log(PipeString);
					}
				}

				// If the process isn't running and there's no data in the pipe, we're done.
				if (!FPlatformProcess::IsProcRunning(CommandletProcessHandle) && PipeString.IsEmpty())
				{
					break;
				}

				// Sleep.
				FPlatformProcess::Sleep(0.0f);
			}

			int32 ReturnCode = 0;
			return FPlatformProcess::GetProcReturnCode(CommandletProcessHandle, &ReturnCode) ? ReturnCode : -1;
		}

	private:
		void* const ReadPipe;
		FProcHandle CommandletProcessHandle;
		SLICommandletExecutor* const CommandletWidget;
	};

	// Launch runnable thread.
	Runnable = new FCommandletLogPump(CommandletProcess->GetReadPipe(), CommandletProcess->GetHandle(), *this);
	RunnableThread = FRunnableThread::Create(Runnable, TEXT("Localization Commandlet Log Pump Thread"));
}

void SLICommandletExecutor::CancelCommandlet()
{
	CleanUpProcessAndPump();
}

void SLICommandletExecutor::CleanUpProcessAndPump()
{
	if (CommandletProcess.IsValid())
	{
		FProcHandle CommandletProcessHandle = CommandletProcess->GetHandle();
		if (CommandletProcessHandle.IsValid() && FPlatformProcess::IsProcRunning(CommandletProcessHandle))
		{
			FPlatformProcess::TerminateProc(CommandletProcessHandle, true);
		}
		CommandletProcess.Reset();
	}

	if (RunnableThread)
	{
		RunnableThread->WaitForCompletion();
		delete RunnableThread;
		RunnableThread = nullptr;
	}

	if (Runnable)
	{
		delete Runnable;
		Runnable = nullptr;
	}

}

bool SLICommandletExecutor::HasCompleted() const
{
	return CurrentTaskIndex == TaskListModels.Num();
}

FText SLICommandletExecutor::GetProgressMessageText() const
{
	return TaskListModels.IsValidIndex(CurrentTaskIndex) ? TaskListModels[CurrentTaskIndex]->Task.Name : FText::GetEmpty();
}

TOptional<float> SLICommandletExecutor::GetProgressPercentage() const
{
	return TOptional<float>(float(CurrentTaskIndex) / float(TaskListModels.Num()));
}

#pragma region TaskRow
class STaskRow : public SMultiColumnTableRow< TSharedPtr<SLICommandletExecutor::FTaskListModel> >
{
public:
	void Construct(const FTableRowArgs& InArgs, const TSharedRef<STableViewBase>& OwnerTableView, const TSharedRef<SLICommandletExecutor::FTaskListModel>& InTaskListModel);
	TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName);

private:
	FSlateColor HandleIconColorAndOpacity() const;
	const FSlateBrush* HandleIconImage() const;
	EVisibility HandleThrobberVisibility() const;
	
	TSharedPtr<SLICommandletExecutor::FTaskListModel> TaskListModel;
};

void STaskRow::Construct(const FTableRowArgs& InArgs, const TSharedRef<STableViewBase>& OwnerTableView, const TSharedRef<SLICommandletExecutor::FTaskListModel>& InTaskListModel)
{
	TaskListModel = InTaskListModel;

	FSuperRowType::Construct(InArgs, OwnerTableView);
}

TSharedRef<SWidget> STaskRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == "StatusIcon")
	{
		return SNew(SOverlay)

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SThrobber)
				.Animate(SThrobber::VerticalAndOpacity)
				.NumPieces(1)
				.Visibility(this, &STaskRow::HandleThrobberVisibility)
			]
		+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.ColorAndOpacity(this, &STaskRow::HandleIconColorAndOpacity)
				.Image(this, &STaskRow::HandleIconImage)
			];
	}
	else if (ColumnName == "TaskName")
	{
		return SNew(STextBlock)
			.Text(TaskListModel->Task.Name)
			.ToolTipText_Lambda( [this]{ return FText::FromString(TaskListModel->ProcessArguments); } );
	}
	else
	{
		return SNullWidget::NullWidget;
	}
}

FSlateColor STaskRow::HandleIconColorAndOpacity( ) const
{
	if (TaskListModel.IsValid())
	{
		switch(TaskListModel->State)
		{
			case SLICommandletExecutor::FTaskListModel::EState::InProgress:
				return FLinearColor::Yellow;
			case SLICommandletExecutor::FTaskListModel::EState::Succeeded:
				return FLinearColor::Green;
			case SLICommandletExecutor::FTaskListModel::EState::Failed:
				return FLinearColor::Red;
			default:
				return FSlateColor::UseForeground();
		}
	}

	return FSlateColor::UseForeground();
}

const FSlateBrush* STaskRow::HandleIconImage( ) const
{
	if (TaskListModel.IsValid())
	{
		switch(TaskListModel->State)
		{
			case SLICommandletExecutor::FTaskListModel::EState::Succeeded:
				return FEditorStyle::GetBrush("Symbols.Check");
			case SLICommandletExecutor::FTaskListModel::EState::Failed:
				return FEditorStyle::GetBrush("Icons.Cross");
			default:
				return nullptr;
		}
	}

	return nullptr;
}

EVisibility STaskRow::HandleThrobberVisibility( ) const
{
	if (TaskListModel.IsValid())
	{
		switch(TaskListModel->State)
		{
			case SLICommandletExecutor::FTaskListModel::EState::InProgress:
				return EVisibility::Visible;
			default:
				return EVisibility::Hidden;
		}
	}

	return EVisibility::Hidden;
}
#pragma endregion

TSharedRef<ITableRow> SLICommandletExecutor::OnGenerateTaskListRow(TSharedPtr<FTaskListModel> TaskListModel, const TSharedRef<STableViewBase>& Table)
{
	return SNew(STaskRow, Table, TaskListModel.ToSharedRef());
}

TSharedPtr<SLICommandletExecutor::FTaskListModel> SLICommandletExecutor::GetCurrentTaskToView() const
{
	if (TaskListView.IsValid())
	{
		const TArray< TSharedPtr<FTaskListModel> > Selection = TaskListView->GetSelectedItems();
		return Selection.Num() > 0 ? Selection.Top() : nullptr;
	}
	return nullptr;
}

FText SLICommandletExecutor::GetCurrentTaskProcessArguments() const
{
	const TSharedPtr<SLICommandletExecutor::FTaskListModel> TaskToView = GetCurrentTaskToView();
	return TaskToView.IsValid() ? FText::FromString(TaskToView->ProcessArguments) : FText::GetEmpty();
}

FText SLICommandletExecutor::GetLogString() const
{
	const TSharedPtr<SLICommandletExecutor::FTaskListModel> TaskToView = GetCurrentTaskToView();
	return TaskToView.IsValid() ? FText::FromString(TaskToView->LogOutput) : FText::GetEmpty();
}

FReply SLICommandletExecutor::OnCopyLogClicked()
{
	CopyLogToClipboard();

	return FReply::Handled();
}

void SLICommandletExecutor::CopyLogToClipboard()
{
	FPlatformApplicationMisc::ClipboardCopy(*(GetLogString().ToString()));
}

FReply SLICommandletExecutor::OnSaveLogClicked()
{
	const FString TextFileDescription = LOCTEXT("TextFileDescription", "Text File").ToString();
	const FString TextFileExtension = TEXT("txt");
	const FString TextFileExtensionWildcard = FString::Printf(TEXT("*.%s"), *TextFileExtension);
	const FString FileTypes = FString::Printf(TEXT("%s (%s)|%s"), *TextFileDescription, *TextFileExtensionWildcard, *TextFileExtensionWildcard);
	const FString DefaultFilename = FString::Printf(TEXT("%s.%s"), TEXT("Log"), *TextFileExtension);
	const FString DefaultPath = FPaths::ProjectSavedDir();

	TArray<FString> SaveFilenames;
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	// Prompt the user for the filename
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = NULL;

		const TSharedPtr<SWindow>& ParentWindowPtr = FSlateApplication::Get().FindWidgetWindow(AsShared());
		if (ParentWindowPtr.IsValid() && ParentWindowPtr->GetNativeWindow().IsValid())
		{
			ParentWindowWindowHandle = ParentWindowPtr->GetNativeWindow()->GetOSWindowHandle();
		}

		if (DesktopPlatform->SaveFileDialog(
			ParentWindowWindowHandle,
			LOCTEXT("SaveLogDialogTitle", "Save Log to File").ToString(),
			DefaultPath,
			DefaultFilename,
			FileTypes,
			EFileDialogFlags::None,
			SaveFilenames
			))
		{
			// Save to file.
			FFileHelper::SaveStringToFile( GetLogString().ToString(), *(SaveFilenames.Last()) );
		}
	}

	return FReply::Handled();
}

FText SLICommandletExecutor::GetCloseButtonText() const
{
	return HasCompleted() ? LOCTEXT("OkButtonText", "OK") : LOCTEXT("CancelButtonText", "Cancel");
}

FReply SLICommandletExecutor::OnCloseButtonClicked()
{
	if (!HasCompleted())
	{
		CancelCommandlet();
	}
	ParentWindow->RequestDestroyWindow();
	return FReply::Handled();
}


SLICommandletExecutor::~SLICommandletExecutor()
{
	CancelCommandlet();
}

#undef LOCTEXT_NAMESPACE