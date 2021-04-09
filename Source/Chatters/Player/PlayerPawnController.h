// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerPawn.h"
#include "PlayerPawnController.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API APlayerPawnController : public APlayerController
{
	GENERATED_BODY()
public:
	APlayerPawnController();
	~APlayerPawnController();


	APlayerPawn* GetPlayerPawn();

	UPROPERTY(EditDefaultsOnly)
		float MaxMovementSpeed = 1200.0f;
	
	UPROPERTY(EditDefaultsOnly)
		float MaxMovementSpeedWithShift = 2400.0f;

protected:
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
private:
	APlayerPawn* PlayerPawn = nullptr;

	bool bAttachedToActor = false;

	void MoveForward(float Value);
	void MoveRight(float Value);

	void TurnX(float Value);
	void TurnY(float Value);

	void MovePawn(EAxis::Type Axis, float Value);

	void OnShiftPressed();
	void OnShiftReleased();

	bool bShiftPressed = false;

	void UpdateMaxMovementSpeed(float MaxSpeed);
};
