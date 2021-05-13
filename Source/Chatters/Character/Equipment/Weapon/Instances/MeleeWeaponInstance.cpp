// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponInstance.h"

void UMeleeWeaponInstance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (this->Phase == EMeleePhase::Hit)
	{
		this->TimeoutValue -= DeltaTime;

		if (this->TimeoutValue <= 0.0f)
		{
			this->TimeoutValue = 0.0f;
			this->Phase = EMeleePhase::IDLE;
			this->bShouldPlayHitAnimation = false;
			
		}
	}
}

UMeleeWeaponItem* UMeleeWeaponInstance::GetMeleeRef()
{
	return Cast<UMeleeWeaponItem>(this->WeaponRef);
}

void UMeleeWeaponInstance::OnHit()
{
	this->BotsHit.Empty();
	this->Phase = EMeleePhase::Hit;

	auto* Ref = this->GetMeleeRef();

	if (Ref)
	{
		this->TimeoutValue = Ref->HitTimeout;
		this->bShouldPlayHitAnimation = true;
		this->HitAnimationTime = 0.0f;
	}
}

bool UMeleeWeaponInstance::CanHit()
{
	return this->Phase == EMeleePhase::IDLE;
}
