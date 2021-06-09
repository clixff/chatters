// Fill out your copyright notice in the Description page of Project Settings.


#include "ChattersGameSession.h"
#include "ChattersGameInstance.h"
#include "../Player/PlayerPawn.h"
#include "../Player/PlayerPawnController.h"
#include "./Settings/SavedSettings.h"
#include "Managers/MapManager.h"

UChattersGameSession* UChattersGameSession::Singleton = nullptr;

UChattersGameSession::UChattersGameSession()
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameSession] UChattersGameSession created"));
}

UChattersGameSession::~UChattersGameSession()
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameSession] UChattersGameSession destroyed"));
	if (UChattersGameSession::Singleton == this)
	{
		UChattersGameSession::Singleton = nullptr;
	}
}

void UChattersGameSession::Init(FString LevelName)
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameSession] UChattersGameSession init"));

	UChattersGameSession::Singleton = this;


	auto* SavedSettings = USavedSettings::Get();

	if (SavedSettings)
	{
		this->GameModeType = SavedSettings->DefaultSessionGameMode;
		this->SessionType = SavedSettings->DefaultSessionType;
		this->MaxPlayers = SavedSettings->DefaultMaxPlayers;
	}

	auto* GameInstance = UChattersGameInstance::Get();
	
	if (GameInstance)
	{
		auto* MapManager = GameInstance->GetMapManager();

		if (MapManager)
		{
			MapManager->LoadLevel(LevelName, true);
		}
	}

	if (MaxPlayers < 2)
	{
		//MaxPlayers = 25;
	}
}

void UChattersGameSession::Destroy()
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameSession] UChattersGameSession destroy"));

	UChattersGameSession::Singleton = nullptr;

	auto* World = GetWorld();

	if (World)
	{
		auto* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			auto* PlayerPawn = Cast<APlayerPawn>(PlayerController->GetPawn());
			if (PlayerPawn)
			{
				PlayerPawn->GameSession = nullptr;
				PlayerPawn->bReady = false;
			}
		}
	}

	if (this->SessionWidget && this->SessionWidget->IsValidLowLevel())
	{
		this->SessionWidget->ConditionalBeginDestroy();
	}

	if (this->PauseMenuWidget && this->PauseMenuWidget->IsValidLowLevel())
	{
		this->PauseMenuWidget->ConditionalBeginDestroy();
	}

	if (this->IsValidLowLevel())
	{
		this->ConditionalBeginDestroy();
	}
}

void UChattersGameSession::LevelLoaded(FString LevelName)
{

	this->BlueAlive = 0;
	this->RedAlive = 0;

	UE_LOG(LogTemp, Display, TEXT("[UChattersGameSession] Level loaded"));

	UWorld* World = GetWorld();

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[UChattersGameSession::LevelLoaded] World was nullptr"));
		return;
	}

	this->AvailableBotSpawnPoints = this->BotSpawnPoints;
	this->AvailableExplodingBarrels = this->ExplodingBarrels;

	if (this->AvailableBotSpawnPoints.Num() < this->MaxPlayers)
	{
		this->MaxPlayers = this->AvailableBotSpawnPoints.Num();
	}

	auto* EquipmentList = this->EquipmentListsForLevels.Find(LevelName);

	if (EquipmentList && *EquipmentList)
	{
		this->EquipmentListLevel = *EquipmentList;
	}

	if (!this->SessionWidgetClass)
	{
		this->SessionWidgetClass = USessionWidget::StaticClass();
	}

	this->SessionWidget = UCustomWidgetBase::CreateUserWidget(this->SessionWidgetClass);

	this->SessionWidget->SetTeamsWrapperVisibility(this->GameModeType == ESessionGameMode::Teams);
	this->SessionWidget->UpdateRoundSeconds(0.0f);

	if (!this->BotSubclass)
	{
		this->BotSubclass = ABot::StaticClass();
	}

	if (this->SessionType == ESessionType::Generated)
	{
		this->SessionWidget->SetPlayCommandVisibility(false);
		if (this->SessionMode == ESessionMode::TestAiming)
		{
			this->MaxPlayers = 1;
		}

		for (int32 i = 0; i < this->MaxPlayers; i++)
		{
			auto Name = FString::Printf(TEXT("Bot_%d"), i+1);
			ABot* Bot = ABot::CreateBot(World, Name, i, this->BotSubclass, this);
			if (Bot)
			{
				Bot->Team = this->GameModeType == ESessionGameMode::Teams ? (i % 2 ? EBotTeam::Blue : EBotTeam::Red)  : EBotTeam::White;

				if (Bot->Team == EBotTeam::Blue)
				{
					this->BlueAlive++;
				}
				else if (Bot->Team == EBotTeam::Red)
				{
					this->RedAlive++;
				}

				Bot->UpdateNameColor();

				this->Bots.Add(Bot);
				this->AliveBots.Add(Bot);
			}
		}
	}

	this->SessionWidget->SetTeamAliveNumber(EBotTeam::Blue, this->BlueAlive);
	this->SessionWidget->SetTeamAliveNumber(EBotTeam::Red, this->RedAlive);

	auto* PlayerController = World->GetFirstPlayerController();
	if (PlayerController)
	{
		auto* PlayerPawn = Cast<APlayerPawn>(PlayerController->GetPawn());
		if (PlayerPawn)
		{
			PlayerPawn->Init();

			if (this->Bots.Num() > 0)
			{
				int32 RandomBotID = FMath::RandRange(0, this->Bots.Num() - 1);

				auto* Bot = this->Bots[RandomBotID];

				if (Bot)
				{
					PlayerPawn->AttachToBot(Bot);
				}
			}
		}

		auto* PlayerPawnController = Cast<APlayerPawnController>(PlayerController);

		if (PlayerPawnController)
		{
			PlayerPawnController->bCanControl = true;
		}

		/** Fix shadow distance bug */
		PlayerController->ConsoleCommand(TEXT("r.Shadow.MaxCSMResolution 4096"), true);
		PlayerController->ConsoleCommand(TEXT("r.Shadow.RadiusThreshold 0"), true);

		//PlayerController->ConsoleCommand(TEXT("stat fps"), true);
		//PlayerController->ConsoleCommand(TEXT("stat unit"), true);
	}

	UChattersGameInstance::SetUIControlMode(false);


	this->SessionWidget->UpdateAliveBotsText(this->AliveBots.Num(), this->Bots.Num());

	this->SessionWidget->Show();

}

void UChattersGameSession::OnBotDied(int32 BotID)
{
	for (int32 i = 0; i < this->AliveBots.Num(); i++)
	{
		auto* AliveBot = this->AliveBots[i];
		if (AliveBot && AliveBot->ID == BotID)
		{
			this->AliveBots.RemoveAt(i, 1, true);

			if (AliveBot->Team == EBotTeam::Blue)
			{
				this->BlueAlive--;
				this->SessionWidget->SetTeamAliveNumber(EBotTeam::Blue, this->BlueAlive);

			}
			else if (AliveBot->Team == EBotTeam::Red)
			{
				this->RedAlive--;
				this->SessionWidget->SetTeamAliveNumber(EBotTeam::Red, this->RedAlive);

			}

			if (this->SessionWidget)
			{
				this->SessionWidget->UpdateAliveBotsText(this->AliveBots.Num(), this->Bots.Num());
			}

			if (this->GameModeType == ESessionGameMode::Teams && this->AliveBots.Num() > 1 && (this->BlueAlive == 0 || this->RedAlive == 0))
			{

				UWorld* World = this->GetWorld();

				for (ABot* Bot : this->AliveBots)
				{
					Bot->StopMovementAfterRound();
				}

				if (World)
				{
					FTimerHandle Timer;
					World->GetTimerManager().SetTimer(Timer, this, &UChattersGameSession::OnTeamsBattleEnd, 5.0f, false);
				}
			}

			if (this->AliveBots.Num() == 1)
			{
				auto* BotWinner = this->AliveBots[0];
				BotWinner->StopMovementAfterRound();
				if (this->SessionWidget)
				{
					this->SessionWidget->PlayWinnerAnimation(BotWinner->DisplayName, BotWinner->GetTeamColor());
					this->SessionWidget->bUpdateRoundTimer = false;
				}
			}

			break;
		}
	}

}

void UChattersGameSession::Start()
{
	if (!this->bStarted)
	{
		this->bStarted = true;

		if (this->SessionWidget)
		{
			this->SessionWidget->SetStartGameSessionTipVisibility(false);
			this->SessionWidget->SetPlayCommandVisibility(false);
			this->SessionWidget->bUpdateRoundTimer = true;
			this->SessionWidget->SetRoundTimerVisibility(true);
		}

		for (int32 i = 0; i < this->Bots.Num(); i++)
		{
			auto* Bot = this->Bots[i];

			if (Bot)
			{
				Bot->OnGameSessionStarted(this->SessionMode);
			}
		}
	}
}

void UChattersGameSession::AttachPlayerToAliveBot(EAttachCameraToBotType Type, int32 ActiveBotID)
{
	int32 AliveBotsNumber = this->AliveBots.Num();
	if (AliveBotsNumber < 1)
	{
		return;
	}

	int32 AliveBotToAttachIndex = -1;
	int32 AliveBotToAttachID = -1;

	bool bCheckPrev = false;

	for (int32 i = 0; i < AliveBotsNumber; i++)
	{
		auto* Bot = this->AliveBots[i];
		if (Bot)
		{
			if ((Type == EAttachCameraToBotType::NextBot && Bot->ID > ActiveBotID) ||
				(Type == EAttachCameraToBotType::PrevBot && Bot->ID < ActiveBotID))
			{
				if (bCheckPrev)
				{
					if ((Type == EAttachCameraToBotType::NextBot && Bot->ID > AliveBotToAttachID) ||
						(Type == EAttachCameraToBotType::PrevBot && Bot->ID < AliveBotToAttachID))
					{
						continue;
					}
				}

				AliveBotToAttachIndex = i;
				AliveBotToAttachID = this->AliveBots[i]->ID;
				bCheckPrev = true;
			}

		}
	}

	if (AliveBotToAttachIndex == -1)
	{
		if (Type == EAttachCameraToBotType::NextBot)
		{
			AliveBotToAttachIndex = 0;
			AliveBotToAttachID = this->AliveBots[AliveBotToAttachIndex]->ID;
		}
		else
		{
			AliveBotToAttachIndex = AliveBotsNumber - 1;
			AliveBotToAttachID = this->AliveBots[AliveBotToAttachIndex]->ID;
		}
	}

	if (ActiveBotID == AliveBotToAttachID)
	{
		return;
	}


	auto* World = GetWorld();

	if (World)
	{
		auto* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			auto* PlayerPawn = Cast<APlayerPawn>(PlayerController->GetPawn());
			if (PlayerPawn)
			{
				PlayerPawn->AttachToBot(this->AliveBots[AliveBotToAttachIndex]);
			}
		}
	}
}

UChattersGameSession* UChattersGameSession::Get()
{
	return UChattersGameSession::Singleton;
}

USessionWidget* UChattersGameSession::GetSessionWidget()
{
	return this->SessionWidget;
}

void UChattersGameSession::OnTeamsBattleEnd()
{
	this->AvailableBotSpawnPoints = this->BotSpawnPoints;
	this->AvailableExplodingBarrels = this->ExplodingBarrels;
	this->bStarted = false;
	this->RoundNumber++;

	int32 AliveBotsNumber = 0;
	this->RedAlive = 0;
	this->BlueAlive = 0;

	for (int32 i = 0; i < this->Bots.Num(); i++)
	{
		auto* Bot = this->Bots[i];
		if (!Bot)
		{
			continue;
		}

		if (!Bot->bAlive)
		{
			Bot->Clear();
			Bot->Destroy();
			continue;
		}
		else
		{
			FTransform SpawnPoint = this->GetAvailableSpawnPoint();

			Bot->SetActorLocation(SpawnPoint.GetLocation());
			Bot->SetActorRotation(SpawnPoint.GetRotation());
			Bot->Team = AliveBotsNumber % 2 ? EBotTeam::Blue : EBotTeam::Red;

			if (Bot->Team == EBotTeam::Blue)
			{
				this->BlueAlive++;
			}
			else
			{
				this->RedAlive++;
			}

			Bot->ResetOnNewRound();

			AliveBotsNumber++;
		}
	}

	this->SessionWidget->SetTeamAliveNumber(EBotTeam::Blue, this->BlueAlive);
	this->SessionWidget->SetTeamAliveNumber(EBotTeam::Red, this->RedAlive);


	auto* World = GetWorld();

	if (World && this->AliveBots.Num())
	{
		auto* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			auto* PlayerPawn = Cast<APlayerPawn>(PlayerController->GetPawn());
			if (PlayerPawn)
			{
				PlayerPawn->AttachToBot(this->AliveBots[0]);
			}
		}
	}

	for (int32 i = 0; i < this->AvailableExplodingBarrels.Num(); i++)
	{
		auto* ExplodingBarrel = this->AvailableExplodingBarrels[i];

		if (ExplodingBarrel)
		{
			ExplodingBarrel->bCanExplode = true;
			if (ExplodingBarrel->DestructibleComponent)
			{
				ExplodingBarrel->DestructibleComponent->ReregisterComponent();
			}
		}
	}

	this->Bots = this->AliveBots;

	if (this->SessionWidget)
	{
		this->SessionWidget->SetStartGameSessionTipVisibility(true);
		this->SessionWidget->UpdateAliveBotsText(this->AliveBots.Num(), this->Bots.Num());
		this->SessionWidget->PlayNewRoundAnimation(this->RoundNumber);
		this->SessionWidget->UpdateRoundSeconds(0.0f);
		this->SessionWidget->bUpdateRoundTimer = false;
	}
}

FTransform UChattersGameSession::GetAvailableSpawnPoint()
{
	FTransform SpawnPointTransform;
	
	int32 RandNumber = FMath::RandRange(0, this->AvailableBotSpawnPoints.Num() - 1);
	auto* SpawnPoint = this->AvailableBotSpawnPoints[RandNumber];

	if (SpawnPoint)
	{
		SpawnPointTransform.SetLocation(SpawnPoint->GetActorLocation());
		SpawnPointTransform.SetRotation(FQuat(SpawnPoint->GetRotation()));
		this->AvailableBotSpawnPoints.RemoveAt(RandNumber, 1, true);
	}

	return SpawnPointTransform;
}

UPauseMenuWidget* UChattersGameSession::GetPauseMenuWidget()
{
	if (!this->PauseMenuWidget)
	{
		if (!this->PauseMenuWidgetClass)
		{
			this->PauseMenuWidgetClass = UPauseMenuWidget::StaticClass();
		}

		this->PauseMenuWidget = UCustomWidgetBase::CreateUserWidget(this->PauseMenuWidgetClass);
	}

	return this->PauseMenuWidget;
}

void UChattersGameSession::PauseGame()
{
	auto* PauseMenuWidgetPtr = this->GetPauseMenuWidget();

	if (PauseMenuWidgetPtr)
	{
		auto* GameInstance = UChattersGameInstance::Get();

		if (GameInstance)
		{
			GameInstance->SetIsGamePaused(true);

			auto* PlayerController = Cast<APlayerPawnController>(GameInstance->GetPlayerController());

			if (PlayerController)
			{
				PlayerController->bCanControl = false;
			}
		}

		PauseMenuWidgetPtr->Show();

		if (this->SessionWidget)
		{
			this->SessionWidget->Hide();
		}
	}
}

void UChattersGameSession::UnpauseGame()
{
	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		auto* PlayerController = Cast<APlayerPawnController>(GameInstance->GetPlayerController());

		if (PlayerController)
		{
			PlayerController->bCanControl = true;
		}

		UChattersGameInstance::SetUIControlMode(false);

		GameInstance->SetIsGamePaused(false);

		if (this->SessionWidget)
		{
			this->SessionWidget->Show();
		}
	}
}
