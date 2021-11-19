// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Train.generated.h"

UCLASS()
class CHATTERS_API ATrain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrain();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UBoxComponent* Collision = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* Root = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USplineComponent* Spline = nullptr;

	bool bActivated = false;

	UFUNCTION(BlueprintCallable)
		void Activate();

	float PathValue = 0.0f;

	FVector Start;
	FVector End;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Speed = 0.01f;

	UFUNCTION(BlueprintCallable)
		void TrainCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USoundBase* HitSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float ImpulseForce = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAudioComponent* Sound = nullptr;
};
