// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerPawn.h"
#include "../Misc/Misc.h"
#include "PlayerPawnController.generated.h"

enum class ERotationType : uint8
{
	Yaw,
	Pitch,
	Roll
};

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
		float MaxMovementSpeed = 1700.0f;
	
	UPROPERTY(EditDefaultsOnly)
		float MaxMovementSpeedWithShift = 3500.0f;

	UPROPERTY(EditDefaultsOnly)
		float ZoomScale = 2500.0f;

	float ZoomValue = 0.0f;

	float ZoomSeconds = 0.0f;

	UPROPERTY(EditDefaultsOnly)
		float SecondsForZoom = 0.25f;

	UPROPERTY(EditDefaultsOnly)
		float AttachedCameraRotationScale = 2.5f;

	UPROPERTY(EditDefaultsOnly)
		float AttachedCameraMaxPitchRotation = 0.0f;

	UPROPERTY(EditDefaultsOnly)
		float AttachedCameraMinPitchRotation = -70.0f;

	bool bCanControl = true;
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

	void OnSpacePressed();

	void OnMouseWheelUp();
	void OnMouseWheelDown();

	void Zoom(float Value);

	void ZoomTick(float DeltaTime);

	void RotateAttachedCamera(ERotationType Type, float Value);

	void OnLeftMouseClick();
	void OnRightMouseClick();

	void AttachPlayerToAliveBot(EAttachCameraToBotType Type);

	void OnSlowmoStart();
	void OnSlowmoEnd();

	void OnEscPressed();

};
