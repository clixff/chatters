// Fill out your copyright notice in the Description page of Project Settings.

#include "Process.h"
#include "Misc/Paths.h"


FNodeChildProcess* FNodeChildProcess::Singleton = nullptr;

FNodeChildProcess::FNodeChildProcess()
{
	UE_LOG(LogTemp, Display, TEXT("[FNodeChildProcess] Creating FNodeChildProcess"));
	this->Thread = FRunnableThread::Create(this, TEXT("FNodeChildProcess"));
	FNodeChildProcess::Singleton = this;
}

FNodeChildProcess::~FNodeChildProcess()
{
	UE_LOG(LogTemp, Display, TEXT("[FNodeChildProcess] Deleting FNodeChildProcess"));
}

void FNodeChildProcess::StartProcess()
{
	static const FString ExePathString = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + TEXT("Binaries/Win64/") + TEXT("twitch-app.exe");

	UE_LOG(LogTemp, Display, TEXT("[FNodeChildProcess] Exe path is %s"), *ExePathString);

	static const LPWSTR ExePath = _tcsdup(*ExePathString);


	auto ProcessCreationResult = CreateProcess(NULL, ExePath, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &this->StartupInfo, &this->ProcessInformation);

	if (!ProcessCreationResult)
	{
		UE_LOG(LogTemp, Error, TEXT("[FNodeChildProcess] CreateProcess failed: %d"), GetLastError());
	}

}

void FNodeChildProcess::Shutdown()
{
	if (FNodeChildProcess::Singleton)
	{
		TerminateProcess(FNodeChildProcess::Singleton->ProcessInformation.hProcess, 0);
		FNodeChildProcess::Singleton->Stop();
	}
}

void FNodeChildProcess::OnShutdown()
{
	if (FNodeChildProcess::Singleton)
	{
		delete FNodeChildProcess::Singleton;
		FNodeChildProcess::Singleton = nullptr;
	}
}

bool FNodeChildProcess::Init()
{
	ZeroMemory(&this->StartupInfo, sizeof(this->StartupInfo));
	this->StartupInfo.cb = sizeof(this->StartupInfo);
	ZeroMemory(&this->ProcessInformation, sizeof(this->ProcessInformation));

	UE_LOG(LogTemp, Display, TEXT("[FNodeChildProcess] ::Init()"));
	return true;
}


uint32 FNodeChildProcess::Run()
{
	UE_LOG(LogTemp, Display, TEXT("[FNodeChildProcess] ::Run()"));
	this->StartProcess();

	return 0;
}

void FNodeChildProcess::Stop()
{
	UE_LOG(LogTemp, Display, TEXT("[FNodeChildProcess] ::Stop()"));

	CloseHandle(this->ProcessInformation.hProcess);
	CloseHandle(this->ProcessInformation.hThread);

	FNodeChildProcess::OnShutdown();
}

FNodeChildProcess* FNodeChildProcess::Create()
{
	if (FNodeChildProcess::Singleton)
	{
		UE_LOG(LogTemp, Error, TEXT("[FNodeChildProcess] FNodeChildProcess already exists"));
	}
	else
	{
		if (FPlatformProcess::SupportsMultithreading())
		{
			FNodeChildProcess::Singleton = new FNodeChildProcess();
		}
	}

	return FNodeChildProcess::Singleton;
}