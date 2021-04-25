// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "TestAim.generated.h"

UCLASS()
class CHATTERS_API ATestAim : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestAim();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		float Pitch = 0.0f;

	UPROPERTY(EditAnywhere)
		AActor* TargetActor = nullptr;

	UPROPERTY(EditAnywhere)
		USphereComponent* CenterPoint = nullptr;

	UPROPERTY(EditAnywhere)
		USphereComponent* ArmShoulder = nullptr;

	UPROPERTY(EditAnywhere)
		USphereComponent* OutBullet = nullptr;

	UPROPERTY(EditAnywhere)
		USphereComponent* TestPoint = nullptr;

	UPROPERTY(VisibleAnywhere)
		FVector DefaultArmLocation;

	UPROPERTY(VisibleAnywhere)
		FVector DefaultOutLocation;

	UPROPERTY(VisibleAnywhere)
		float ArmOutDiff = 0.0f;

	void Draw();

	void Aim();
};
