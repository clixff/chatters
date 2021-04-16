// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Character/Bot.h"
#include "../UI/Widgets/SessionWidget.h"
#include "../Misc/Misc.h"
#include "../Character/Equipment/EquipmentList.h"
#include "ChattersGameSession.generated.h"

UENUM(BlueprintType)
enum class ESessionType : uint8
{
	/**
	 * Fill battleground with generated bots
	 */
	Generated	UMETA(DisplayName = "Generated"),

	/**
	 * Spawn twitch viewers with 
	 */
	Twitch	UMETA(DisplayName = "Twitch")
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class CHATTERS_API UChattersGameSession : public UObject
{
	GENERATED_BODY()
	
public:
	UChattersGameSession();
	~UChattersGameSession();

	void Init();

	void Destroy();

	void LevelLoaded(FString LevelName);

	void OnBotDied(int32 BotID);

	void Start();

	void AttachPlayerToAliveBot(EAttachCameraToBotType Type, int32 ActiveBotID);

public:
	bool bStarted = false;

	ESessionType SessionType = ESessionType::Generated;

	UPROPERTY(EditDefaultsOnly)
		int32 MaxPlayers = 25;

	USessionWidget* GetSessionWidget();
public:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<ABot> BotSubclass;

	UPROPERTY(VisibleAnywhere)
		TArray<ABot*> Bots;

	UPROPERTY(VisibleAnywhere)
		TArray<ABot*> AliveBots;

	/** Equipment lists for all levels */
	UPROPERTY(EditDefaultsOnly)
		TMap<FString, UEquipmentList*> EquipmentListsForLevels;

	/** Equipment list for current level */
	UPROPERTY()
		UEquipmentList* EquipmentListLevel;
private:
	UPROPERTY(VisibleAnywhere)
		USessionWidget* SessionWidget = nullptr;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<USessionWidget> SessionWidgetClass;
};
