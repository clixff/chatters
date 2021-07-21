// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EquipmentItem.h"
#include "NiagaraSystem.h"
#include "CostumeItem.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UCostumeItem : public USkeletalMeshEquipmentItem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
		bool bHideHeadMesh = false;

	UPROPERTY(EditDefaultsOnly)
		UNiagaraSystem* BloodParticle = nullptr;
};
