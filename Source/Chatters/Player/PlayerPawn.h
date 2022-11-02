// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "../Core/ChattersGameSession.h"
#include "../Character/Bot.h"
#include "PlayerPawn.generated.h"

USTRUCT(BlueprintType)
struct FCinematicCameraData
{
	GENERATED_BODY()
public:
	bool bActivated = false;

	FVector StartPoint;
	FVector EndPoint;

	FManualTimer Timer = FManualTimer(7.5f);

	UPROPERTY()
	AActor* Target = nullptr;

	UPROPERTY()
	AActor* ProjectileActor = nullptr;

	FManualTimer ProjectileTimeout = FManualTimer(7.0f);
};


UCLASS()
class CHATTERS_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Sets to true when the playing level loaded */
	bool bReady = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USphereComponent* SphereCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UCameraComponent* Camera;


	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UPawnMovementComponent* MovementComponent;

	UPROPERTY()
		ABot* BotToAttach = nullptr;

	UChattersGameSession* GameSession = nullptr;

	void AttachToBot(ABot* Bot, bool bNoFirstPerson = false);

	void DetachFromBot();

	UPROPERTY(EditDefaultsOnly)
		float MaxAttachedZoom = 3000.0f;

	UPROPERTY(EditDefaultsOnly)
		float MinAttachedZoom = 500.0f;

	UPROPERTY(EditDefaultsOnly)
		float DefaultAttachedZoom = 2400.0f;

	float LastZoomValue;

	bool bAttachedToBot = false;

	float GetDistanceFromCamera(FVector Location);

	/** Get singleton */
	UFUNCTION(BlueprintCallable)
		static APlayerPawn* Get();

	void Init();

	FVector GetCameraLocation();

	void RespawnAttachedBot();

	bool bFirstPersonCamera = false;

	void SetThirdPersonCamera();

	void SetFirstPersonCamera();

	void ResetAttachedBotHeadVisibility();

	UPROPERTY(EditDefaultsOnly)
		float FirstPersonFOV = 90.0f;

	void ActivateCinematicCamera(AActor* ActorToAttach = nullptr, bool bBlockCameraControls = true);

	inline bool IsCinematicCameraEnabled()
	{
		return CinematicCameraData.bActivated;
	};

	void DeactivateCinematicCamera(bool bAttachToPlayer = false);

	FManualTimer BlockControlsOnCinematicCameraTimer = FManualTimer(0.3f);

	UPROPERTY()
		FCinematicCameraData CinematicCameraData;
private:
	void UpdateBotNicknameWidgets();

	void SetSpectatorMenuVisibiliy(bool bVisible);

	UChattersGameSession* GetGameSession();

	FVector GetAttachedCameraWorldLocation(float Distance, FRotator CameraRotation);

	bool IsBotVisibleFromCamera(float Distance, FRotator CameraRotation);

	FRotator FindNewAcceptableCameraRotation(FRotator StartRotation);

	static APlayerPawn* Singleton;

	FVector CachedCameraLocation = FVector(0.0f);

	float ThirdPersonFOV = 90.0f;

	void CinematicCameraTick(float DeltaTime);
};
