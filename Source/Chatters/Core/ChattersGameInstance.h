// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Managers/MapManager.h"
#include "Managers/WidgetManager.h"
#include "ChattersGameSession.h"
#include "./Settings/SavedSettings.h"
#include "ChattersGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UChattersGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UChattersGameInstance();
	~UChattersGameInstance();

	/** virtual function to allow custom GameInstances an opportunity to set up what it needs */
	virtual void Init() override;

	/** virtual function to allow custom GameInstances an opportunity to do cleanup when shutting down */
	virtual void Shutdown() override;

	/** Callback from the world context when the world changes */
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;

	virtual void LoadComplete(const float LoadTime, const FString& MapName) override;

	void SetIsInMainMenu(bool bInMainMenu);

	void ReturnToTheMainMenu();

	/** 
	 * Map manager: 
	 */

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UMapManager> MapManagerClass;

	UMapManager* GetMapManager();

	void CreateMapManager();

	/**
	 * Widget manager:
	 */

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UWidgetManager> WidgetManagerClass;

	UWidgetManager* GetWidgetManager();

	/**
	 * Game Session:
	 */

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UChattersGameSession> GameSessionClass;

	UChattersGameSession* GetGameSession();


	void CreateWidgetManager();

	void ToggleMainMenuUI(bool MainMenuStatus);

	void ToggleMouseCursor(bool bShowMouseCursor);

	void StartGameSession(FString LevelName);

	bool GetIsInMainMenu();

	void SetIsGamePaused(bool bPauseStatus);

	bool GetIsGamePaused();

	void SetGraphicsQuality(EGraphicsQualityLevel GraphicsQuality);

	void FixShadowsQuality();

	UFUNCTION(BlueprintNativeEvent)
		float UpdateGameVolume(float Volume);
		
	float UpdateGameVolume_Implementation(float Volume);

	UPROPERTY(EditDefaultsOnly)
		bool bStartNodeChildProcess = true;

	UPROPERTY()
		FTwitchAuthData TwitchAuthData;

	void OnTwitchAuthDataLoaded(bool bSignedIn, FString DisplayName);

	bool bUpdateAvailable = false;

	FString GetGameVersion();

	void SetUpdateAvailable(bool bUpdateAvailableValue);
public:
	/** Get Singleton */
	static UChattersGameInstance* Get();

	/** Can be nullptr */
	static APlayerController* GetPlayerController();

	static FName CoreStringTablePath;

	static void SetUIControlMode(bool bAllowUIControl, bool bUpdateMousePosition = true);
private:
	static UChattersGameInstance* Singleton;

	bool bInitialized = false;
	
	bool bInMainMenu = false;

	UPROPERTY(VisibleAnywhere, Export)
		UMapManager* MapManager = nullptr;

	UPROPERTY(VisibleAnywhere, Export)
		UWidgetManager* WidgetManager = nullptr;

	UPROPERTY(VisibleAnywhere, Export)
		UChattersGameSession* GameSession = nullptr;

	UPROPERTY(VisibleAnywhere, Export)
		USavedSettings* SavedSettings = nullptr;

	bool bGamePaused = false;
};
