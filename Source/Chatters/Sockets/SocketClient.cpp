// Fill out your copyright notice in the Description page of Project Settings.


#include "SocketClient.h"
#include "../Core/ChattersGameInstance.h"

FSocketClient* FSocketClient::Singleton = nullptr;

FSocketClient::FSocketClient()
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] Socket client created"));
	FSocketClient::Singleton = this;
}

FSocketClient::~FSocketClient()
{
	if (FSocketClient::Singleton == this)
	{
		UE_LOG(LogTemp, Display, TEXT("[FSocketClient] Socket client destroyed"));
		FSocketClient::Singleton = nullptr;
	}
}

FSocketClient* FSocketClient::Create()
{
	if (!FSocketClient::Singleton)
	{
		FSocketClient::Singleton = new FSocketClient();
	}

	return FSocketClient::Singleton;
}

void FSocketClient::Destroy()
{
	if (FSocketClient::Singleton)
	{
		delete FSocketClient::Singleton;
	}
}
