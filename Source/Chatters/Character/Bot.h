// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BotController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Bot.generated.h"

UCLASS()
class CHATTERS_API ABot : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FString DisplayName = FString();

	uint32 ID;

	uint32 HealthPoints;

	uint32 MaxHealthPoints;

	bool GetIsAlive();

	ABotController* GetAIController();
private:
	bool bAlive = true;

	void SetOutfit();

	void MoveToRandomLocation();

	bool bMovingToRandomLocation = false;

	FVector RandomLocationTarget;

public:
	static ABot* CreateBot(UWorld* World, FString NameToSet, uint32 IDToSet, TSubclassOf<ABot> Subclass);
public:
	UPROPERTY(VisibleAnywhere, BLueprintReadWrite)
		USkeletalMeshComponent* Head;
};
