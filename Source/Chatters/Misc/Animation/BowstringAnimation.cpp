// Fill out your copyright notice in the Description page of Project Settings.


#include "BowstringAnimation.h"
#include "../../Character/Bot.h"
#include "../../Character/Equipment/Weapon/Instances/FirearmWeaponInstance.h"

void UBowstringAnimation::AnimTick(float DeltaTime)
{
	if (!Bot || !FirearmInstance)
	{
		return;
	}

	this->bAttachedToHand = Bot->CombatAction == ECombatAction::Shooting;

	if (bAttachedToHand)
	{
		if (!FirearmInstance->bShouldAttachBowstring)
		{
			this->bAttachedToHand = false;
		}
	}

	if (bAttachedToHand)
	{
		this->CenterWorldPosition = Bot->GetMesh()->GetSocketLocation(TEXT("L_arm_5"));
	}
}