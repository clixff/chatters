// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "BotSpawnPoint.generated.h"

UCLASS()
class CHATTERS_API ABotSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABotSpawnPoint();

	~ABotSpawnPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UCapsuleComponent* CapsuleComponent = nullptr;;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UArrowComponent* ArrowComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		bool bRandomYawRotation = false;

	FRotator GetRotation();
};
