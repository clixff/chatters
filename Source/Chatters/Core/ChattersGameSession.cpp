// Fill out your copyright notice in the Description page of Project Settings.


#include "ChattersGameSession.h"
#include "ChattersGameInstance.h"
#include "../Player/PlayerPawn.h"
#include "Managers/MapManager.h"

UChattersGameSession::UChattersGameSession()
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameSession] UChattersGameSession created"));
}

UChattersGameSession::~UChattersGameSession()
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameSession] UChattersGameSession destroyed"));
}

void UChattersGameSession::Init()
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameSession] UChattersGameSession init"));

	auto* GameInstance = UChattersGameInstance::Get();
	
	if (GameInstance)
	{
		auto* MapManager = GameInstance->GetMapManager();

		if (MapManager)
		{
			MapManager->LoadLevel(MapManager->WildWestWorldName, true);
		}
	}

	if (MaxPlayers < 2)
	{
		MaxPlayers = 25;
	}
}

void UChattersGameSession::Destroy()
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameSession] UChattersGameSession destroy"));

	if (this->IsValidLowLevel())
	{
		this->ConditionalBeginDestroy();
	}
}

void UChattersGameSession::LevelLoaded()
{
	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[UChattersGameSession::LevelLoaded] World was nullptr"));
		return;
	}

	if (!this->BotSubclass)
	{
		this->BotSubclass = ABot::StaticClass();
	}

	if (this->SessionType == ESessionType::Generated)
	{
		for (int32 i = 0; i < this->MaxPlayers; i++)
		{
			auto Name = FString::Printf(TEXT("Bot_%d"), i);
			ABot* Bot = ABot::CreateBot(World, Name, i, this->BotSubclass);
			this->Bots.Add(Bot);
			this->AliveBots.Add(Bot);
		}
	}

	auto* PlayerController = World->GetFirstPlayerController();
	if (PlayerController)
	{
		auto* PlayerPawn = Cast<APlayerPawn>(PlayerController->GetPawn());
		if (PlayerPawn)
		{
			PlayerPawn->bReady = true;
		}
	}
}
