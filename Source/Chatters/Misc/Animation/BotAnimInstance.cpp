// Fill out your copyright notice in the Description page of Project Settings.


#include "BotAnimInstance.h"

void UBotAnimInstance::AnimTick(float DeltaTime)
{
	auto* Bot = this->GetBotRef();

	if (!Bot)
	{
		return;
	}

	this->Speed = Bot->GetSpeedForAnimationBlueprint();

	this->AimingAngle = Bot->AimingAngle;

	this->GunPitchRotation = Bot->GetGunPitchRotation();

	this->WeaponRef = Bot->GetWeaponRef();
	this->WeaponInstance = Bot->WeaponInstance;

	if (this->WeaponRef && this->WeaponRef->RunBlendSpace)
	{
		this->RunBlendSpace = this->WeaponRef->RunBlendSpace;
	}
	else
	{
		this->RunBlendSpace = DefaultRunBlendSpace;
	}

	this->bShouldPlayWeaponHitAnimation = Bot->ShouldPlayWeaponHitAnimation();

	this->bShouldPlayReloadingAnimation = Bot->ShouldPlayWeaponReloadingAnimation();

	this->CombatAction = Bot->CombatAction;

	if (this->CombatAction == ECombatAction::IDLE)
	{
		this->bShouldApplyGunAnimation = false;
	}
	else
	{
		this->bShouldApplyGunAnimation = Bot->bShouldApplyGunAnimation;
	}

	if (WeaponInstance)
	{
		HitAnimation = WeaponInstance->HitAnimationSequence;
	}

	bFalling = Bot->IsFalling();
}

ABot* UBotAnimInstance::GetBotRef()
{
	if (!this->BotRef)
	{
		auto* PawnOwner = TryGetPawnOwner();

		this->BotRef = Cast<ABot>(PawnOwner);
	}

	return this->BotRef;
}
