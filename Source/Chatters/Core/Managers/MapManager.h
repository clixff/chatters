// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MapManager.generated.h"

#define NULL_WORLD_NAME TEXT("NULLWORLD")

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class CHATTERS_API UMapManager : public UObject
{
	GENERATED_BODY()
	
public:
	UMapManager();
	~UMapManager();

public:

	UWorld* GetWorldObject();

	void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld);

	void LoadComplete(const float LoadTime, const FString& MapName);

	void LoadLevel(FString MapName, bool bAbsolute = true);

	/** Store loaded world name */
	FString WorldName;

	bool IsLevelLoaded();

public:
	
	UPROPERTY(EditDefaultsOnly)
		FString MenuWorldName;

	UPROPERTY(EditDefaultsOnly)
		FString WildWestWorldName;

public:
	static FString GetWorldName(UWorld* World);

private:
	bool bLevelLoaded = false;
};
