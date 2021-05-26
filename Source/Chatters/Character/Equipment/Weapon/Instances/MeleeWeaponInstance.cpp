// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponInstance.h"
#include "../../../Bot.h"

void UMeleeWeaponInstance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (this->Phase == EMeleePhase::Hit)
	{
		this->TimeoutValue -= DeltaTime;

		if (this->TimeoutValue <= this->TimeToDisableCollision)
		{
			if (this->bCollisionEnabled)
			{
				this->SetCollisionEnabled(false);
			}
		}
		else if (this->TimeoutValue <= this->TimeToEnableCollision)
		{
			if (!this->bCollisionEnabled)
			{
				this->SetCollisionEnabled(true);
			}
		}

		if (this->TimeoutValue <= 0.0f)
		{
			this->TimeoutValue = 0.0f;
			this->Phase = EMeleePhase::IDLE;
			this->bShouldPlayHitAnimation = false;
			this->SetCollisionEnabled(false);
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

		float EnableCollisionTimePercent = Ref->AnimationTimeToDamage.GetLowerBoundValue();
		float DisableCollisionTimePercent = Ref->AnimationTimeToDamage.GetUpperBoundValue();

		this->TimeToEnableCollision = (1.0f - EnableCollisionTimePercent) * Ref->HitTimeout;
		this->TimeToDisableCollision = (1.0f - DisableCollisionTimePercent) * Ref->HitTimeout;
	}
}

bool UMeleeWeaponInstance::CanHit()
{
	return this->Phase == EMeleePhase::IDLE;
}

void UMeleeWeaponInstance::SetCollisionEnabled(bool bEnabled)
{
	this->bCollisionEnabled = bEnabled;

	auto* Bot = Cast<ABot>(this->BotOwner);

	if (Bot)
	{
		Bot->SetMeleeCollisionEnabled(bEnabled);
	}
}
