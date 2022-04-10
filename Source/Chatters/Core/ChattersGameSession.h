// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Character/Bot.h"
#include "../Character/BotSpawnPoint.h"
#include "../UI/Widgets/SessionWidget.h"
#include "../UI/Widgets/PauseMenuWidget.h"
#include "../Misc/Misc.h"
#include "../Character/Equipment/EquipmentList.h"
#include "../Props/ExplodingBarrel.h"
#include "../Combat/FirearmProjectile.h"
#include "../Misc/Train.h"
#include "../Misc/Weather/WeatherManager.h"
#include "../Misc/BulletHolesManager.h"
#include "ChattersGameSession.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class CHATTERS_API UChattersGameSession : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	UChattersGameSession();
	~UChattersGameSession();

	void Init(FString LevelName);

	void Destroy();

	void LevelLoaded(FString LevelName);

	void OnBotDied(int32 BotID);

	void Start();

	void AttachPlayerToAliveBot(EAttachCameraToBotType Type, int32 ActiveBotID);

	static UChattersGameSession* Get();

	static UChattersGameSession* Singleton;

public:
	bool bStarted = false;

	ESessionType SessionType = ESessionType::Generated;

	UPROPERTY(EditDefaultsOnly)
		int32 MaxPlayers = 25;

	USessionWidget* GetSessionWidget();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		ESessionMode SessionMode = ESessionMode::Combat;

	UPROPERTY()
		TMap<FString, ABot*> BotsMap;

	bool bCanViewersJoin = false;

	ABot* OnViewerJoin(FString Name);

	void OnViewerMessage(FString Name, FString Message);

	void OnViewerTargetCommand(FString ViewerName, FString TargetName);
public:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<ABot> BotSubclass;

	UPROPERTY(VisibleAnywhere)
		TArray<ABot*> Bots;

	UPROPERTY(VisibleAnywhere)
		TArray<ABot*> AliveBots;

	UPROPERTY(VisibleAnywhere)
		int32 BlueAlive = 0;

	UPROPERTY(VisibleAnywhere)
		int32 RedAlive = 0;

	int32 BlueAliveMax = 0;
	int32 RedAliveMax = 0;


	/** Equipment lists for all levels */
	UPROPERTY(EditDefaultsOnly)
		TMap<FString, UEquipmentList*> EquipmentListsForLevels;

	/** Equipment list for current level */
	UPROPERTY()
		UEquipmentList* EquipmentListLevel;

	UPROPERTY()
		TArray<ABotSpawnPoint*> BotSpawnPoints;

	UPROPERTY()
		TArray<ABotSpawnPoint*> AvailableBotSpawnPoints;

	UPROPERTY()
		TArray<AExplodingBarrel*> ExplodingBarrels;

	UPROPERTY()
		TArray<AExplodingBarrel*> AvailableExplodingBarrels;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		ESessionGameMode GameModeType = ESessionGameMode::Default;
	
	void OnTeamsBattleEnd();

	FTransform GetAvailableSpawnPoint(bool bRemoveSpawnPoint = true);

	UPauseMenuWidget* GetPauseMenuWidget();

	void PauseGame();

	void UnpauseGame();

	TSet<FString> AvailableWeapons;

	void RespawnBotAfterStuck(ABot* Bot);
private:
	UPROPERTY(VisibleAnywhere)
		USessionWidget* SessionWidget = nullptr;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<USessionWidget> SessionWidgetClass;

	UPROPERTY(VisibleAnywhere)
		UPauseMenuWidget* PauseMenuWidget = nullptr;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

	int32 RoundNumber = 1;

	FCriticalSection Mutex;

public:
	// Begin FTickableGameObject Interface.
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableInEditor() const override;
	virtual bool IsTickableWhenPaused() const override;
	// End FTickableGameObject Interface.

public:
	bool bGameEndedSlomoActivated = false;
	FManualTimer GameEndedSlomoTimeout = FManualTimer(5.0f);

	FManualTimer UpdateHeadAnimationModesTimer = FManualTimer(1.0f);

	void UpdateHeadAnimationModes();

	float RoundTime = 0.0f;

	bool bUpdateRoundTimer = false;

	bool bDeathmatchTimeEnded = false;

	bool bDeathmatchRoundEnded = false;

	TArray<FDeathmatchLeaderboardElement> DeathmatchLeaderboard;

	void OnBotKill(ABot* Bot);

	void OnGameEnded(ABot* Winner);

	void FindDeathmatchWinner();

	void SelectDeathmatchLeader(int32 Index);

	ATrain* TrainRef = nullptr;

	UPROPERTY()
		AWeatherManager* WeatherManagerRef = nullptr;

	UPROPERTY()
		ABulletHolesManager* BulletHolesManager = nullptr;
};
