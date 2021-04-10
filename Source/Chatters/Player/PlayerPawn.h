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

	void AttachToBot(ABot* Bot);

	void DetachFromBot();

	UPROPERTY(EditDefaultsOnly)
		float MaxAttachedZoom = 3000.0f;

	UPROPERTY(EditDefaultsOnly)
		float MinAttachedZoom = 500.0f;

	UPROPERTY(EditDefaultsOnly)
		float DefaultAttachedZoom = 2400.0f;

	bool bAttachedToBot = false;

private:
	void UpdateBotNicknameWidgetsSize();
};
