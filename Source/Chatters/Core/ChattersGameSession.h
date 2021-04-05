// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
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

	void LevelLoaded();

public:
	bool bStarted = false;

	ESessionType SessionType = ESessionType::Generated;

	UPROPERTY(EditDefaultsOnly)
		int32 MaxPlayers = 25;
};
