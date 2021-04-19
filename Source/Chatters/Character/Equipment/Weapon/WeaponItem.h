// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EquipmentItem.h"
#include "GameFramework/Character.h"
#include "WeaponItem.generated.h"
	
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	Melee,
	Firearm,
	Bow
};

/**
 * 
 */
UCLASS()
class CHATTERS_API UWeaponItem : public UStaticMeshEquipmentItem
{
	GENERATED_BODY()
public:
	UWeaponItem();
	~UWeaponItem();

	UPROPERTY(EditDefaultsOnly)
		int32 Damage = 0;

	UPROPERTY(VisibleAnywhere)
		EWeaponType Type = EWeaponType::None;
};
