// Fill out your copyright notice in the Description page of Project Settings.

#include "ChattersGameInstance.h"
#include "../Player/PlayerPawnController.h"
#include "Slate/SceneViewport.h"
#include "Kismet/GameplayStatics.h"


UChattersGameInstance* UChattersGameInstance::Singleton = nullptr;
FName UChattersGameInstance::CoreStringTablePath = TEXT("/Game/Blueprint/Misc/CoreStringTable");

UChattersGameInstance::UChattersGameInstance()
{
	this->MapManagerClass = UMapManager::StaticClass();
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameInstance] GameInstance created"));
}

UChattersGameInstance::~UChattersGameInstance()
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameInstance] GameInstance destroyed"));
}

void UChattersGameInstance::Init()
{
	float RandNumber = FMath::RandRange(0.0f, 100.0f);

	UE_LOG(LogTemp, Display, TEXT("[UChattersGameInstance] GameInstance init"));

	UGameInstance::Init();

	this->Singleton = this;
	UChattersGameSession::Singleton = nullptr;

	this->CreateMapManager();
	this->CreateWidgetManager();

	this->bInitialized = true;

	this->ReturnToTheMainMenu();
}

void UChattersGameInstance::Shutdown()
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameInstance] GameInstance shutdown"));
	UGameInstance::Shutdown();
}

void UChattersGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameInstance] World Changed. bInitialized: %d"), this->bInitialized);
	UGameInstance::OnWorldChanged(OldWorld, NewWorld);

	if (bInitialized)
	{
		this->GetMapManager()->OnWorldChanged(OldWorld, NewWorld);
	}
}

void UChattersGameInstance::LoadComplete(const float LoadTime, const FString& MapName)
{
	UGameInstance::LoadComplete(LoadTime, MapName);

	this->GetMapManager()->LoadComplete(LoadTime, MapName);

	UE_LOG(LogTemp, Display, TEXT("[UChattersGameInstance] In main menu: %d"), this->bInMainMenu);

	if (this->bInMainMenu)
	{
		this->ToggleMainMenuUI(true);
	}
	else if (this->GetMapManager()->WorldName != NULL_WORLD_NAME)
	{
		if (this->GameSession)
		{
			this->GameSession->LevelLoaded(this->GetMapManager()->WorldName);
		}
	}

	this->SetIsGamePaused(false);
}

void UChattersGameInstance::SetIsInMainMenu(bool bInMainMenuNew)
{
	if (!bInMainMenuNew)
	{
		this->ToggleMainMenuUI(false);
	}

	this->bInMainMenu = bInMainMenuNew;

	auto* PlayerPawnController = Cast<APlayerPawnController>(this->GetPlayerController());

	if (PlayerPawnController)
	{
		PlayerPawnController->bCanControl = !this->bInMainMenu;
	}

	if (bInMainMenu)
	{
		if (this->GameSession)
		{
			this->GameSession->Destroy();
			this->GameSession = nullptr;
		}
	}
}

void UChattersGameInstance::ReturnToTheMainMenu()
{
	this->GetMapManager()->LoadLevel(this->GetMapManager()->MenuWorldName, true);
}

UMapManager* UChattersGameInstance::GetMapManager()
{
	if (this->MapManager == nullptr)
	{
		this->CreateMapManager();
	}

	return this->MapManager;
}

void UChattersGameInstance::CreateMapManager()
{
	if (this->MapManager == nullptr)
	{
		if (this->MapManagerClass == nullptr)
		{
			this->MapManagerClass = UMapManager::StaticClass();
			UE_LOG(LogTemp, Warning, TEXT("[UChattersGameInstance] MapManagerClass was nullptr"));
		}
		this->MapManager = NewObject<UMapManager>(this, this->MapManagerClass, TEXT("MapManager"));
	}
}

UWidgetManager* UChattersGameInstance::GetWidgetManager()
{
	if (this->WidgetManager == nullptr)
	{
		this->CreateWidgetManager();
	}

	return this->WidgetManager;
}

UChattersGameSession* UChattersGameInstance::GetGameSession()
{
	if (this->GameSession == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UChattersGameSession::GetGameSession] GameSession was nullptr"));
	}

	return this->GameSession;
}

void UChattersGameInstance::CreateWidgetManager()
{
	if (this->WidgetManager == nullptr)
	{
		if (this->WidgetManagerClass == nullptr)
		{
			this->WidgetManagerClass = UWidgetManager::StaticClass();
			UE_LOG(LogTemp, Warning, TEXT("[UChattersGameInstance] WidgetManagerClass was nullptr"));
		}

		this->WidgetManager = NewObject<UWidgetManager>(this, this->WidgetManagerClass, TEXT("WidgetManager"));
	}
}

void UChattersGameInstance::ToggleMainMenuUI(bool MainMenuStatus)
{
	UE_LOG(LogTemp, Display, TEXT("[UChattersGameInstance] Toggle Main menu UI to: %d"), MainMenuStatus);

	if (MainMenuStatus)
	{
		this->GetWidgetManager()->CreateMainMenuWidget();

		auto* MainMenuWidget = this->GetWidgetManager()->MainMenuWidget;

		UE_LOG(LogTemp, Display, TEXT("[UChattersGameInstance] Is mainMenu widget nullptr: %d"), MainMenuWidget == nullptr);


		if (MainMenuWidget != nullptr)
		{
			MainMenuWidget->Show();
			this->ToggleMouseCursor(true);
		}

		UChattersGameInstance::SetUIControlMode(true);

	}
	else
	{
		this->GetWidgetManager()->RemoveMainMenuWidget();
		this->ToggleMouseCursor(false);

		UChattersGameInstance::SetUIControlMode(false);

	}
}

void UChattersGameInstance::ToggleMouseCursor(bool bShowMouseCursor)
{
	auto* PlayerController = this->GetPlayerController();
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = bShowMouseCursor;
		PlayerController->bEnableClickEvents = bShowMouseCursor;
		PlayerController->bEnableMouseOverEvents = bShowMouseCursor;
	}
}

UChattersGameInstance* UChattersGameInstance::Get()
{
	if (UChattersGameInstance::Singleton == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[UChattersGameInstance] Singleton was nullptr on ::Get()"));
	}

	return UChattersGameInstance::Singleton;
}

APlayerController* UChattersGameInstance::GetPlayerController()
{
	UChattersGameInstance* GameInstance = UChattersGameInstance::Get();

	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UChattersGameInstance::GetPlayerController] GameInstance was nullptr"));
		return nullptr;
	}

	UWorld* World = GameInstance->GetWorld();

	if (World == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UChattersGameInstance::GetPlayerController] World was nullptr"));
		return nullptr;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);

	if (PlayerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UChattersGameInstance::GetPlayerController] PlayerController was nullptr. WorldName is %s"), *World->GetName());
		return nullptr;
	}

	return PlayerController;
}

void UChattersGameInstance::SetUIControlMode(bool bAllowUIControl)
{
	auto* PlayerController = UChattersGameInstance::GetPlayerController();

	if (!PlayerController)
	{
		return;
	}

	if (bAllowUIControl)
	{
		auto InputMode = FInputModeGameAndUI();
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);

		FVector2D ViewportSize = FVector2D(1.0f, 1.0f);

		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);

			auto* GameViewport = GEngine->GameViewport->GetGameViewport();

			if (!GameViewport)
			{
				return;
			}

			auto CachedGeometry = GameViewport->GetCachedGeometry();

			if (CachedGeometry.GetLocalSize() != FVector2D(0.0f, 0.0f))
			{
				FVector2D ViewportCenter = ViewportSize / 2.0f;

				PlayerController->SetMouseLocation(FMath::RoundToInt(ViewportCenter.X), FMath::RoundToInt(ViewportCenter.Y));
			}
		}


	}
	else
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

void UChattersGameInstance::StartGameSession()
{
	if (this->GameSession)
	{
		return;
	}

	if (!this->GameSessionClass)
	{
		this->GameSessionClass = UChattersGameSession::StaticClass();
		UE_LOG(LogTemp, Warning, TEXT("[UChattersGameInstance] GameSessionClass was nullptr"));
	}

	this->GameSession = NewObject<UChattersGameSession>(this, this->GameSessionClass, TEXT("GameSession"));

	this->GameSession->Init();
}

bool UChattersGameInstance::GetIsInMainMenu()
{
	return this->bInMainMenu;
}

void UChattersGameInstance::SetIsGamePaused(bool bPauseStatus)
{
	this->bGamePaused = bPauseStatus;

	auto* PlayerController = this->GetPlayerController();

	if (PlayerController)
	{
		PlayerController->SetPause(this->bGamePaused);
	}
}

bool UChattersGameInstance::GetIsGamePaused()
{
	return this->bGamePaused;
}
