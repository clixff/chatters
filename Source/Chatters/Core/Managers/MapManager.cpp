// Fill out your copyright notice in the Description page of Project Settings.


#include "MapManager.h"
#include "Kismet/GameplayStatics.h"
#include "../ChattersGameInstance.h"


UMapManager::UMapManager()
{
	UE_LOG(LogTemp, Display, TEXT("[UMapManager] MapManager created"));
}

UMapManager::~UMapManager()
{
	UE_LOG(LogTemp, Display, TEXT("[UMapManager] MapManager destroyed"));
}

UWorld* UMapManager::GetWorldObject()
{
	UWorld* WorldObject = nullptr;

	UChattersGameInstance* GameInstance = UChattersGameInstance::Get();

	if (GameInstance != nullptr)
	{
		WorldObject = GameInstance->GetWorld();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UMapManager::GetWorld] GameInstance is null"));
	}

	return WorldObject;
}

void UMapManager::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	FString OldWorldName = UMapManager::GetWorldName(OldWorld);
	FString NewWorldName = UMapManager::GetWorldName(NewWorld);

	UE_LOG(LogTemp, Display, TEXT("[UMapManager] World changed from \"%s\" to \"%s\""), *OldWorldName, *NewWorldName);

	this->WorldName = NewWorldName;

	UChattersGameInstance* GameInstance = UChattersGameInstance::Get();

	if (GameInstance != nullptr)
	{
		bool bInMainMenu = NewWorldName == this->MenuWorldName;

		GameInstance->SetIsInMainMenu(bInMainMenu);
	}
}

void UMapManager::LoadComplete(const float LoadTime, const FString& MapName)
{
	UE_LOG(LogTemp, Display, TEXT("[UMapManager] Level %s loaded in %f sec"), *MapName, LoadTime);
	this->bLevelLoaded = true;
}

void UMapManager::LoadLevel(FString MapName, bool bAbsolute)
{
	UE_LOG(LogTemp, Display, TEXT("[UMapManager] Start loading level %s"), *MapName);
	UWorld* World = this->GetWorldObject();
	UGameplayStatics::OpenLevel(World, FName(*MapName), bAbsolute);
	this->bLevelLoaded = false;
}

bool UMapManager::IsLevelLoaded()
{
	return this->bLevelLoaded;
}

FString UMapManager::GetWorldName(UWorld* World)
{
	FString WorldName = NULL_WORLD_NAME;
	if (World != nullptr)
	{
		WorldName = World->GetName();
	}
	return WorldName;
}
