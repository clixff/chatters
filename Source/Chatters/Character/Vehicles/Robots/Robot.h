// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../Equipment/Weapon/FirearmWeaponItem.h"
#include "Robot.generated.h"

UCLASS()
class CHATTERS_API ARobot : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARobot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveToNewLocation(FVector NewLocation);

	UPROPERTY(EditAnywhere)
		UFirearmWeaponItem* WeaponClass = nullptr;

	UPROPERTY(EditAnywhere)
		FTransform CharacterTransform;
};
