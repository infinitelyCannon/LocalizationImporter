// Copyright (C) 2022 Dakarai Simmons - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LocalizationCommandletExecution.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "HAL/Runnable.h"

class FLICommandletProcess : public TSharedFromThis<FLICommandletProcess>
{
public:
	static TSharedPtr<FLICommandletProcess> Execute(const FString &ConfigFilePath, const bool UseProjectFile = true);

private:
	FLICommandletProcess(void* const InReadPipe, void* const InWritePipe, const FProcHandle InProcessHandle, const FString &InProcessArguments)
		: ReadPipe(InReadPipe),
	WritePipe(InWritePipe),
	ProcessHandle(InProcessHandle),
	ProcessArguments(InProcessArguments)
	{}

public:
	~FLICommandletProcess();

	void* GetReadPipe() const
	{
		return ReadPipe;		
	}

	FProcHandle& GetHandle()
	{
		return ProcessHandle;
	}

	const FString& GetProcessArguments() const
	{
		return ProcessArguments;
	}

private:
	void* ReadPipe;
	void* WritePipe;
	FProcHandle ProcessHandle;
	FString ProcessArguments;
};

/*
 * Pulled from engine source.
 * The widget they use to display the progress
 * when you run a command from the Localization Dashboard
 * is not an exported class, so it's recreated in here.
 */
class SLICommandletExecutor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLICommandletExecutor) {}
	SLATE_END_ARGS()

private:
	struct FTaskListModel
	{
		enum class EState
		{
			Queued,
			InProgress,
			Failed,
			Succeeded
		};

		FTaskListModel()
			: State(EState::Queued) {}

		LocalizationCommandletExecution::FTask Task;
		EState State;
		FString LogOutput;
		FString ProcessArguments;
	};

	friend class STaskRow;

public:
	SLICommandletExecutor();
	~SLICommandletExecutor();

	void Construct(const FArguments& Arguments, const TSharedRef<SWindow>& InParentWindow, const TArray<LocalizationCommandletExecution::FTask>& Tasks);
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	bool WasSuccessful() const;
	void Log(const FString &String);

private:
	//static TSharedPtr<FLocalizationCommandletProcess> PyExecute(const FString& ConfigFilePath, const bool UseProjectFile);
	void ExecuteCommandlet(const TSharedRef<FTaskListModel>& TaskListModel);
	void OnCommandletProcessCompletion(const int32 ReturnCode);
	void CancelCommandlet();
	void CleanUpProcessAndPump();

	bool HasCompleted() const;

	FText GetProgressMessageText() const;
	TOptional<float> GetProgressPercentage() const;

	TSharedRef<ITableRow> OnGenerateTaskListRow(TSharedPtr<FTaskListModel> TaskListModel, const TSharedRef<STableViewBase>& Table);
	TSharedPtr<FTaskListModel> GetCurrentTaskToView() const;

	FText GetCurrentTaskProcessArguments() const;
	FText GetLogString() const;

	FReply OnCopyLogClicked();
	void CopyLogToClipboard();

	FReply OnSaveLogClicked();

	FText GetCloseButtonText() const;
	FReply OnCloseButtonClicked();
	
	int32 CurrentTaskIndex;
	TArray< TSharedPtr<FTaskListModel> > TaskListModels;
	TSharedPtr<SProgressBar> ProgressBar;
	TSharedPtr< SListView< TSharedPtr<FTaskListModel> > > TaskListView;

	struct
	{
		FCriticalSection CriticalSection;
		FString String;
	} PendingLogData;

	TSharedPtr<SWindow> ParentWindow;
	TSharedPtr<FLICommandletProcess> CommandletProcess;
	FRunnable* Runnable;
	FRunnableThread* RunnableThread;
};
