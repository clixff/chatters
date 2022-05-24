// Fill out your copyright notice in the Description page of Project Settings.


#include "ChattersGameSession.h"
#include "ChattersGameInstance.h"
#include "../Player/PlayerPawn.h"
#include "../Player/PlayerPawnController.h"
#include "./Settings/SavedSettings.h"
#include "../Sockets/SocketClient.h"
#include "Kismet/GameplayStatics.h"
#include "Managers/MapManager.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"


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
	this->BotsMap.Empty();
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
		//this->MaxPlayers = this->AvailableBotSpawnPoints.Num();
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

	if (this->GameModeType == ESessionGameMode::Deathmatch)
	{
		this->SessionWidget->SetLeaderboardVisibility(true);
	}

	auto* Settings = USavedSettings::Get();

	if (Settings)
	{
		this->SessionWidget->SetKillFeedPosition(Settings->KillFeedPosition);
		
	}

	if (!this->BotSubclass)
	{
		this->BotSubclass = ABot::StaticClass();
	}


	if (SessionType != ESessionType::Twitch)
	{
		SessionWidget->SetPlayCommandVisibility(false);
	}

	auto CreateBotOnLevel = [this, &World](FString Name, int32 ID)
	{
		ABot* Bot = ABot::CreateBot(World, Name, ID, this->BotSubclass, this);
		if (Bot)
		{
			if (Bot->Team == EBotTeam::Blue)
			{
				this->BlueAlive++;
				this->BlueAliveMax = this->BlueAlive;

			}
			else if (Bot->Team == EBotTeam::Red)
			{
				this->RedAlive++;
				this->RedAliveMax = this->RedAlive;
			}

			Bot->UpdateNameColor();

			this->Bots.Add(Bot);
			this->AliveBots.Add(Bot);
			this->BotsMap.Add(Bot->DisplayName.ToLower(), Bot);
		}

		return Bot;
	};

	switch (SessionType)
	{
	case ESessionType::Generated:
	{
		for (int32 i = 0; i < this->MaxPlayers; i++)
		{
			auto Name = FString::Printf(TEXT("Bot_%d"), i + 1);
			CreateBotOnLevel(Name, i);
		}
	}
		break;
	case ESessionType::Twitch:
	{
		this->bCanViewersJoin = true;
		this->SessionWidget->SetStreamerJoinTipVisible(true);
	}
		break;
	case ESessionType::Import:
	{
		auto* SavedSettings = USavedSettings::Get();

		if (!SavedSettings)
		{
			break;
		}

		int32 IDs = 0;

		for (auto& ImportedName : SavedSettings->ImportedNames)
		{
			if (ImportedName.Nickname.IsEmpty())
			{
				continue;
			}

			int32 MaxImportedBots = ImportedName.Amount;

			if (MaxImportedBots < 1)
			{
				MaxImportedBots = 1;
			}

			for (int32 i = 0; i < MaxImportedBots; i++)
			{
				CreateBotOnLevel(ImportedName.Nickname, IDs);
				IDs++;
			}
		}

		this->MaxPlayers = this->AliveBots.Num();
	}
		break;
	}

	this->SessionWidget->SetTeamAliveNumber(this->BlueAlive, this->RedAlive, this->BlueAliveMax, this->RedAliveMax);

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

			if (Settings)
			{
				PlayerPawnController->SetMouseSensitivity(Settings->MouseSensitivity);
			}
		}
	}

	UChattersGameInstance::SetUIControlMode(false);


	this->SessionWidget->UpdateAliveBotsText(this->AliveBots.Num(), this->MaxPlayers);
	
	this->SessionWidget->UpdateScoreBackground(this->GameModeType);
		
	this->SessionWidget->Show();

	if (DayTimeManagerRef)
	{
		if (TimeOfDay == ETimeOfDay::Day)
		{
			if (GameModeType == ESessionGameMode::Teams)
			{
				DayTimeManagerRef->SetSunRotationAtDay(0);
			}
			else
			{
				DayTimeManagerRef->SetSunRotationAtDay(-1);
			}
		}
		else
		{
			DayTimeManagerRef->SetNightTime();
		}

	}

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		GameInstance->FixShadowsQuality();
	}

	auto* SocketClient = FSocketClient::Singleton;

	if (SocketClient && this->SessionType == ESessionType::Twitch)
	{
		SocketClient->OnLevelLoaded();
	}
}

void UChattersGameSession::OnBotDied(int32 BotID)
{
	for (int32 i = 0; i < this->AliveBots.Num(); i++)
	{
		auto* AliveBot = this->AliveBots[i];
		if (AliveBot && AliveBot->ID == BotID)
		{
			if (GameModeType != ESessionGameMode::Deathmatch)
			{
				this->AliveBots.RemoveAt(i, 1, true);
			}

			if (AliveBot->Team == EBotTeam::Blue)
			{
				this->BlueAlive--;
			}
			else if (AliveBot->Team == EBotTeam::Red)
			{
				this->RedAlive--;
			}

			if (AliveBot->Team != EBotTeam::White)
			{
				this->SessionWidget->SetTeamAliveNumber(this->BlueAlive, this->RedAlive, this->BlueAliveMax, this->RedAliveMax);
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

				DeactivateProps();
			}

			if (this->AliveBots.Num() == 1)
			{
				if (this->GameModeType != ESessionGameMode::Deathmatch)
				{
					this->OnGameEnded(this->AliveBots[0]);
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
			this->SessionWidget->SetRoundTimerVisibility(true);
			this->SessionWidget->UpdateAliveBotsText(this->AliveBots.Num(), this->Bots.Num());
			this->SessionWidget->SetStreamerJoinTipVisible(false);
			this->SessionWidget->ClearAllNotifications();
		}


		bUpdateRoundTimer = true;

		this->bCanViewersJoin = false;

		this->AvailableBotSpawnPoints = this->BotSpawnPoints;

		bool bFirstRound = (this->GameModeType != ESessionGameMode::Teams || RoundNumber == 1);

		if (bFirstRound)
		{
			PlayerStats.SetNum(Bots.Num());
		}

		for (auto* Prop : PropsToActivate)
		{
			if (Prop)
			{
				bool bShouldActivate = Prop->bActivateEveryRound || bFirstRound;
				if (bShouldActivate)
				{
					Prop->Activate();
				}
			}
		}

		for (int32 i = 0; i < this->Bots.Num(); i++)
		{
			auto* Bot = this->Bots[i];

			if (Bot)
			{
				if (bFirstRound)
				{
					PlayerStats[i].DisplayName = Bot->DisplayName;
				}

				if (this->GameModeType == ESessionGameMode::Deathmatch)
				{
					FDeathmatchLeaderboardElement LeaderboardElement;
					LeaderboardElement.Nickname = Bot->DisplayName;
					LeaderboardElement.Kills = 0;
					LeaderboardElement.ID = Bot->ID;
					this->DeathmatchLeaderboard.Add(LeaderboardElement);
				}

				Bot->OnGameSessionStarted(this->SessionMode);
			}
		}

		if (this->TrainRef)
		{
			if (bFirstRound)
			{
				this->TrainRef->Activate();
			}
		}

		if (this->WeatherManagerRef)
		{
			if (bFirstRound)
			{
				this->WeatherManagerRef->Activate();
			}
		}

		if (this->GameModeType == ESessionGameMode::Deathmatch)
		{
			this->RoundTime = 60.0f * 3.0f;
			if (this->SessionWidget)
			{
				this->SessionWidget->UpdateLeaderboard(this->DeathmatchLeaderboard);
				this->SessionWidget->SetLeaderboardTipVisibility(true);
			}
		}
		else
		{
			this->RoundTime = 0.0f;
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

ABot* UChattersGameSession::OnViewerJoin(FString Name)
{
	this->Mutex.Lock();
	if (!this->bCanViewersJoin || this->SessionType != ESessionType::Twitch)
	{
		this->Mutex.Unlock();

		return nullptr;
	}

	if (this->Bots.Num() >= this->MaxPlayers)
	{
		this->Mutex.Unlock();

		return nullptr;
	}

	FString LowerCasedName = Name.ToLower();

	/** Bot with this name already exists */
	if (this->BotsMap.Contains(LowerCasedName))
	{
		this->Mutex.Unlock();
		return nullptr;
	}

	const int32 BotID = this->Bots.Num();

	ABot* Bot = ABot::CreateBot(GetWorld(), Name, BotID, this->BotSubclass, this);
	if (Bot)
	{
		if (Bot->Team == EBotTeam::Blue)
		{
			this->BlueAlive++;
			this->BlueAliveMax = this->BlueAlive;

		}
		else if (Bot->Team == EBotTeam::Red)
		{
			this->RedAlive++;
			this->RedAliveMax = this->RedAlive;

		}

		Bot->UpdateNameColor();

		this->Bots.Add(Bot);
		this->AliveBots.Add(Bot);
		this->BotsMap.Add(LowerCasedName, Bot);

		if (this->SessionWidget)
		{
			this->SessionWidget->SetTeamAliveNumber(this->BlueAlive, this->RedAlive, this->BlueAliveMax, this->RedAliveMax);
			this->SessionWidget->UpdateAliveBotsText(this->AliveBots.Num(), this->MaxPlayers);
			this->SessionWidget->OnViewerJoined(Bot->DisplayName, Bot->GetTeamColor());
		}

		if (BotID == 0)
		{
			auto* PlayerController = GetWorld()->GetFirstPlayerController();
			if (PlayerController)
			{
				auto* PlayerPawn = Cast<APlayerPawn>(PlayerController->GetPawn());
				if (PlayerPawn)
				{
					PlayerPawn->AttachToBot(Bot);
				}
			}
		}
		
	}

	this->Mutex.Unlock();

	return Bot;
}

void UChattersGameSession::OnViewerMessage(FString Name, FString Message)
{
	if (this->SessionType != ESessionType::Twitch)
	{
		return;
	}

	auto* BotResult = this->BotsMap.Find(Name);
	ABot* Bot = nullptr;

	if (!BotResult)
	{
		return;
	}

	Bot = *BotResult;

	if (!Bot || !Bot->bAlive)
	{
		return;
	}

	Bot->Say(Message);
}

void UChattersGameSession::OnViewerTargetCommand(FString ViewerName, FString TargetName)
{
	if (!this->bStarted)
	{
		return;
	}

	if (ViewerName == TargetName)
	{
		return;
	}

	ABot** ViewerBotRef = this->BotsMap.Find(ViewerName);
	ABot** TargetBotRef = this->BotsMap.Find(TargetName);

	if (!ViewerBotRef || !TargetBotRef)
	{
		return;
	}

	ABot* ViewerBot = *ViewerBotRef;
	ABot* TargetBot = *TargetBotRef;

	if (!ViewerBot || !TargetBot)
	{
		return;
	}

	if (!ViewerBot->bAlive || !TargetBot->bAlive)
	{
		return;
	}

	if (!ViewerBot->IsEnemy(TargetBot))
	{
		return;
	}

	ViewerBot->SetNewEnemyTarget(TargetBot);
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
	this->BotsMap.Empty();

	bool bRedFirst = FMath::RandRange(0, 1) ? true : false;

	EBotTeam TeamsList[2] = { EBotTeam::Blue, EBotTeam::Red };

	if (!bRedFirst)
	{
		TeamsList[0] = EBotTeam::Red;
		TeamsList[1] = EBotTeam::Blue;
	}

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

			auto PrevBotTeam = Bot->Team;

			Bot->Team = AliveBotsNumber % 2 ? TeamsList[0] : TeamsList[1];

			if (PrevBotTeam != Bot->Team && this->EquipmentListLevel && this->EquipmentListLevel->IsTeamEquipmentSetsExists())
			{
				Bot->SetEquipment();
			}

			Bot->UpdateEquipmentTeamColors();

			if (Bot->Team == EBotTeam::Blue)
			{
				this->BlueAlive++;
				this->BlueAliveMax = this->BlueAlive;
			}
			else
			{
				this->RedAlive++;
				this->RedAliveMax = this->RedAlive;
			}

			Bot->ResetOnNewRound();

			AliveBotsNumber++;

			this->BotsMap.Add(Bot->DisplayName.ToLower(), Bot);
		}
	}

	this->SessionWidget->SetTeamAliveNumber(this->BlueAlive, this->RedAlive, this->BlueAliveMax, this->RedAliveMax);


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
				ExplodingBarrel->DestructibleComponent->SetCanEverAffectNavigation(true);
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
	}
	bUpdateRoundTimer = false;

	if (DayTimeManagerRef)
	{
		if (TimeOfDay == ETimeOfDay::Day)
		{
			DayTimeManagerRef->SetSunRotationAtDay(RoundNumber - 1);
		}
	}
}

FTransform UChattersGameSession::GetAvailableSpawnPoint(bool bRemoveSpawnPoint, bool bAlwaysAround)
{
	FTransform SpawnPointTransform;
	SpawnPointTransform.SetLocation(FVector(0.0f, 0.0f, 70.0f));

	ABotSpawnPoint* SpawnPoint = nullptr;
	

	if (AvailableBotSpawnPoints.Num() && !bAlwaysAround)
	{
		int32 RandNumber = FMath::RandRange(0, AvailableBotSpawnPoints.Num() - 1);
		SpawnPoint = AvailableBotSpawnPoints[RandNumber];

		if (bRemoveSpawnPoint)
		{
			AvailableBotSpawnPoints.RemoveAt(RandNumber, 1, true);
		}
	}
	else
	{
		auto FindSpawnPointAround = [this](FVector& OutVector)
		{
			bool bSpawnPointValid = false;

			int32 RandNumber = FMath::RandRange(0, BotSpawnPoints.Num() - 1);
			FVector StartLocation = BotSpawnPoints[RandNumber]->GetActorLocation();

			FVector FoundLocation;

			bool bFound = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(GetWorld(), StartLocation, FoundLocation, 5000.0f);

			if (bFound)
			{
				UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

				if (NavSys)
				{
					FPathFindingQuery Query;
					Query.StartLocation = StartLocation;
					Query.EndLocation = FoundLocation;
					FNavAgentProperties Agent = FNavAgentProperties::DefaultProperties;
					Query.NavData = NavSys->GetNavDataForProps(Agent, StartLocation);
					FPathFindingResult PathFindingResult = NavSys->FindPathSync(Query, EPathFindingMode::Type::Regular);

					if (PathFindingResult.IsSuccessful())
					{
						bSpawnPointValid = true;
						OutVector = FoundLocation;
					}
				}
			}

			return bSpawnPointValid;
		};

		bool bFoundValidSpawnPoint = false;

		for (int32 i = 0; i < 5; i++)
		{
			FVector OutVector;
			bool bFoundSpawnPoint = FindSpawnPointAround(OutVector);

			if (bFoundSpawnPoint)
			{
				SpawnPointTransform.SetLocation(OutVector + FVector(0.0f, 0.0f, 70.0f));
				SpawnPointTransform.SetRotation(FQuat(FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f)));
				bFoundValidSpawnPoint = true;
				break;
			}
		}

		if (!bFoundValidSpawnPoint)
		{
			int32 RandNumber = FMath::RandRange(0, BotSpawnPoints.Num() - 1);
			SpawnPoint = BotSpawnPoints[RandNumber];
		}
	}

	if (SpawnPoint)
	{
		SpawnPointTransform.SetLocation(SpawnPoint->GetActorLocation());
		SpawnPointTransform.SetRotation(FQuat(SpawnPoint->GetRotation()));
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
	if (bActivateGameEndEffects)
	{
		return;
	}

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

		if (PlayerController && !bGameEnded)
		{
			PlayerController->bCanControl = true;
		}

		if (!bGameEnded)
		{
			UChattersGameInstance::SetUIControlMode(false);
			GameInstance->ToggleMouseCursor(false);
		}

		GameInstance->SetIsGamePaused(false);

		if (this->SessionWidget && !bGameEnded && !bActivateGameEndEffects)
		{
			this->SessionWidget->Show();
		}
	}
}

void UChattersGameSession::RespawnBotAfterStuck(ABot* Bot)
{
	if (!Bot)
	{
		return;
	}

	auto SpawnPoint = this->GetAvailableSpawnPoint(false);

	Bot->SetActorLocation(SpawnPoint.GetLocation());
}

void UChattersGameSession::Tick(float DeltaTime)
{
	if (bActivateGameEndEffects)
	{
		GameEndedSlomoTimeout.Add(DeltaTime);

		auto* PostProcess = GetPostProcessVolume();

		if (PostProcess)
		{
			if (GameEndBloomMin == -1.0f)
			{
				GameEndBloomMin = PostProcess->Settings.BloomIntensity;
			}

			float GameEndBloomMax = 8.0f;
			float BloomValue = FMath::Lerp(GameEndBloomMin, GameEndBloomMax, GameEndedSlomoTimeout.Current / GameEndedSlomoTimeout.Max);

			PostProcess->Settings.BloomIntensity = BloomValue;
		}

		if (GameEndedSlomoTimeout.IsEnded())
		{
			bActivateGameEndEffects = false;

			auto* GameInstance = UChattersGameInstance::Get();
			auto* PlayerController = Cast<APlayerPawnController>(UChattersGameInstance::GetPlayerController());

			if (PlayerController)
			{
				PlayerController->ConsoleCommand(TEXT("slomo 1.0"));
				PlayerController->bCanControl = false;
			}

			PostProcess->Settings.BloomIntensity = 8.0f;
			if (SessionWidget)
			{
				SessionWidget->Hide();
			}

			bGameEnded = true;

			UChattersGameInstance::SetUIControlMode(true);
			GameInstance->ToggleMouseCursor(true);
		}
	}

	UpdateHeadAnimationModesTimer.Add(DeltaTime);

	if (UpdateHeadAnimationModesTimer.IsEnded())
	{
		UpdateHeadAnimationModesTimer.Reset();

		this->UpdateHeadAnimationModes();
	}

	bool bGamePaused = UChattersGameInstance::Get()->GetIsGamePaused();

	if (GameModeType == ESessionGameMode::Default && Bots.Num() > 100 && !bGamePaused)
	{
		for (int32 i = 0; i < Bots.Num(); i++)
		{
			auto* Bot = Bots[i];

			if (Bot && !Bot->bAlive)
			{
				Bot->DestroyAfterDeathTimer.Add(DeltaTime);

				if (Bot->DestroyAfterDeathTimer.IsEnded())
				{
					Bot->Destroy();
					Bots[i] = nullptr;
				}
			}
		}
	}

	if (bUpdateRoundTimer && !bGamePaused)
	{
		if (this->GameModeType == ESessionGameMode::Deathmatch)
		{
			RoundTime -= DeltaTime;
			if (RoundTime < 0.0f)
			{
				RoundTime = 0.0f;
			}

			if (RoundTime == 0.0f)
			{
				this->bDeathmatchTimeEnded = true;
				this->FindDeathmatchWinner();
			}
		}
		else
		{
			RoundTime += DeltaTime;
		}

		if (this->SessionWidget)
		{
			this->SessionWidget->UpdateRoundSeconds(RoundTime);
		}
	}
}

bool UChattersGameSession::IsTickable() const
{
	return !IsDefaultSubobject();
}

TStatId UChattersGameSession::GetStatId() const
{
	return UObject::GetStatID();
}

bool UChattersGameSession::IsTickableInEditor() const
{
	return false;
}

bool UChattersGameSession::IsTickableWhenPaused() const
{
	return false;
}

void UChattersGameSession::UpdateHeadAnimationModes()
{
	auto* PlayerRef = APlayerPawn::Get();
	for (auto* Bot : AliveBots)
	{
		if (Bot)
		{
			Bot->UpdateHeadAnimationType(PlayerRef, false);
		}
	}
}

void UChattersGameSession::OnBotKill(ABot* Bot)
{
	if (this->GameModeType == ESessionGameMode::Deathmatch && !bDeathmatchRoundEnded)
	{
		for (auto& LeaderboardElement : this->DeathmatchLeaderboard)
		{
			if (LeaderboardElement.ID == Bot->ID)
			{
				LeaderboardElement.Kills++;
				break;
			}
		}

		DeathmatchLeaderboard.Sort([](const FDeathmatchLeaderboardElement& LHS, const FDeathmatchLeaderboardElement& RHS)
			{
				if (LHS.Kills == RHS.Kills)
				{
					return LHS.ID < RHS.ID;
				}
					
				return LHS.Kills > RHS.Kills;
			});

		if (this->SessionWidget)
		{
			this->SessionWidget->UpdateLeaderboard(DeathmatchLeaderboard);
		}

		if (this->bDeathmatchTimeEnded)
		{
			this->FindDeathmatchWinner();
		}
	}
}

void UChattersGameSession::OnGameEnded(ABot* Winner)
{
	this->bDeathmatchRoundEnded = true;
	bActivateGameEndEffects = true;

	Winner->StopMovementAfterRound();
	Winner->bWinner = true;

	bUpdateRoundTimer = false;

	auto* PlayerController = UChattersGameInstance::GetPlayerController();

	if (PlayerController)
	{
		PlayerController->ConsoleCommand(TEXT("slomo 0.06"));
		this->bGameEndedSlomoActivated = true;
		GameEndedSlomoTimeout.Reset();
	}

	if (this->GameModeType == ESessionGameMode::Deathmatch)
	{
		for (auto* Bot : this->AliveBots)
		{
			if (Bot)
			{
				Bot->StopMovementAfterRound();
			}
		}
	}

	auto* StatsWidgetRef = GetPlayerStatsWidget();

	if (StatsWidgetRef)
	{
		StatsWidgetRef->Show();
		StatsWidgetRef->Init();
		if (Winner)
		{
			StatsWidgetRef->WinnerName = FText::FromString(Winner->DisplayName);
		}

		FString EmptyStr = TEXT("");
		StatsWidgetRef->UpdateStatsType(EPlayerStatsType::DiedFirst, BotNameDiedFirst, EmptyStr);

		TransferPlayersStatsToWidget();
	}

	DeactivateProps();
}

void UChattersGameSession::FindDeathmatchWinner()
{
	if (this->bDeathmatchTimeEnded && DeathmatchLeaderboard.Num() > 1)
	{
		auto& FirstPlace = DeathmatchLeaderboard[0];
		auto& SecondPlace = DeathmatchLeaderboard[1];

		if (FirstPlace.Kills != SecondPlace.Kills)
		{
			if (FirstPlace.ID < Bots.Num())
			{
				ABot* Winner = Bots[FirstPlace.ID];
				if (Winner)
				{
					OnGameEnded(Winner);
				}
			}
		}
	}
}

void UChattersGameSession::SelectDeathmatchLeader(int32 Index)
{
	if (this->SessionWidget)
	{
		this->SessionWidget->SetLeaderboardTipVisibility(false);
	}

	if (Index < this->DeathmatchLeaderboard.Num())
	{
		auto& Leader = this->DeathmatchLeaderboard[Index];

		if (Leader.ID >= Bots.Num())
		{
			return;
		}

		ABot* Bot = Bots[Leader.ID];

		if (Bot)
		{
			auto* World = GetWorld();

			if (World)
			{
				auto* PlayerController = World->GetFirstPlayerController();
				if (PlayerController)
				{
					auto* PlayerPawn = Cast<APlayerPawn>(PlayerController->GetPawn());
					if (PlayerPawn)
					{
						PlayerPawn->AttachToBot(Bot);
					}
				}
			}
		}
	}
}

UPlayerStatsWidget* UChattersGameSession::GetPlayerStatsWidget()
{
	if (!PlayerStatsWidget)
	{
		if (!PlayerStatsWidgetClass)
		{
			PlayerStatsWidgetClass = UPlayerStatsWidget::StaticClass();
		}

		PlayerStatsWidget = UCustomWidgetBase::CreateUserWidget(PlayerStatsWidgetClass);
	}

	return PlayerStatsWidget;
}

void UChattersGameSession::DeactivateProps()
{
	for (auto* Prop : PropsToActivate)
	{
		if (Prop)
		{
			Prop->Deactivate();
		}
	}
}

void UChattersGameSession::AddExplosionAtLocation(FVector Location, UParticleSystem* ExplosionParticle, FVector ParticleScale, USoundBase* Sound, float Radius, float ImpulseForce, EWeaponType WeaponType)
{
	if (ExplosionParticle)
	{
		FTransform ParticleTransform;
		ParticleTransform.SetLocation(Location);
		ParticleTransform.SetScale3D(ParticleScale);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, ParticleTransform, true);
	}

	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, FMath::RandRange(0.8f, 1.0f));
	}

	auto AliveBotsCopy = AliveBots;

	for (auto* Bot : AliveBotsCopy)
	{
		if (!Bot->GetIsAlive())
		{
			continue;
		}

		FVector BotLocation = Bot->GetActorLocation();
		float Distance = FVector::Dist(BotLocation, Location);
		if (Distance <= Radius)
		{
			FVector ImpulseVector = UKismetMathLibrary::FindLookAtRotation(Location, BotLocation).Vector() * (ImpulseForce);

			Bot->ApplyDamage(100.0f, Bot, WeaponType, ImpulseVector, BotLocation);
		}
	}
}

APostProcessVolume* UChattersGameSession::GetPostProcessVolume()
{
	if (!PostProcessVolume)
	{
		PostProcessVolume = Cast<APostProcessVolume>(UGameplayStatics::GetActorOfClass(GetWorld(), APostProcessVolume::StaticClass()));
	}

	return PostProcessVolume;
}

void UChattersGameSession::TransferPlayersStatsToWidget()
{
	auto* StatsWidgetRef = GetPlayerStatsWidget();

	if (!StatsWidgetRef)
	{
		return;
	}

	struct FPlayerStatsResult
	{
		int32 PlayerID = -1;
		int32 Result = 0;
		FString Name;
	};

	TMap<EPlayerStatsType, FPlayerStatsResult> PlayerStatsResults;

	auto UpdateStats = [this, &PlayerStatsResults](int32 PlayerID, int32 Result, EPlayerStatsType Type, FGamePlayerStats& Stat)
	{
		if (!PlayerStatsResults.Contains(Type))
		{
			PlayerStatsResults.Add(Type);
		}

		auto OldStat = PlayerStatsResults.Find(Type);

		if (!OldStat)
		{
			return;
		}

		if (Result > OldStat->Result)
		{
			FPlayerStatsResult NewStat;
			NewStat.Result = Result;
			NewStat.PlayerID = PlayerID;
			NewStat.Name = Stat.DisplayName;
			PlayerStatsResults.Add(Type, NewStat);
		}
	};

	for (int32 i = 0; i < PlayerStats.Num(); i++)
	{
		auto& Stat = PlayerStats[i];
		UpdateStats(i, Stat.Kills, EPlayerStatsType::MostKills, Stat);
		UpdateStats(i, Stat.Hits, EPlayerStatsType::MostHits, Stat);
		UpdateStats(i, Stat.Shots, EPlayerStatsType::MostShots, Stat);
		UpdateStats(i, Stat.BarrelsExploded, EPlayerStatsType::Barrels, Stat);
		UpdateStats(i, Stat.HatsDropped, EPlayerStatsType::Hats, Stat);
		UpdateStats(i, Stat.Damage, EPlayerStatsType::MostDamage, Stat);
		UpdateStats(i, FMath::FloorToInt(Stat.Accuracy * 100.0f), EPlayerStatsType::Accuracy, Stat);
	}

	for (auto Result : PlayerStatsResults)
	{
		auto PlayerID = Result.Value.PlayerID;
		FString BotName = TEXT("");
		FString ResultString = TEXT("");
		if (PlayerID != -1)
		{
			BotName = Result.Value.Name;

			if (Result.Key == EPlayerStatsType::Accuracy)
			{
				ResultString = FString::Printf(TEXT("%.2f%%"), (float(Result.Value.Result) / 100.0f));
			}
			else
			{
				ResultString = FString::FromInt(Result.Value.Result);
			}

			ResultString = FString::Printf(TEXT("(%s)"), *ResultString);
		}


		StatsWidgetRef->UpdateStatsType(Result.Key, BotName, ResultString);
	}
}
