// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/RunnableThread.h"
#include "HAL/Runnable.h"
#include <Windows.h>

/** Node.js child process for server */
class FNodeChildProcess : public FRunnable
{
public:
	FNodeChildProcess();

	~FNodeChildProcess();

	void StartProcess();

	void TerminateOldProcess();

	static void Shutdown();

	static void OnShutdown();
	
	FRunnableThread* Thread = nullptr;

	static FNodeChildProcess* Singleton;

	static FNodeChildProcess* Create();
public:

	// Begin FRunnable interface.
	virtual bool Init() override;

	virtual uint32 Run() override;

	virtual void Stop() override;
	// End FRunnable interface

private:
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;
};