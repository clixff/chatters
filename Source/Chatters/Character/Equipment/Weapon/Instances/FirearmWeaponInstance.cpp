// Fill out your copyright notice in the Description page of Project Settings.


#include "FirearmWeaponInstance.h"

UFirearmWeaponInstance::UFirearmWeaponInstance()
{
	
}

UFirearmWeaponInstance::~UFirearmWeaponInstance()
{

}

UFirearmWeaponItem* UFirearmWeaponInstance::GetFirearmRef()
{
	UFirearmWeaponItem* Ref = Cast<UFirearmWeaponItem>(this->WeaponRef);

	return Ref;
}

void UFirearmWeaponInstance::Tick(float DeltaTime)
{
	UWeaponInstance::Tick(DeltaTime);
	if (this->TimeoutValue > 0.0f && this->Phase != EFirearmPhase::IDLE)
	{
		this->TimeoutValue -= DeltaTime;
		if (this->TimeoutValue <= 0.0f)
		{
			this->TimeoutValue = 0.0f;

			if (this->Phase == EFirearmPhase::Shooting)
			{
				if (this->NumberOfBullets <= 0)
				{
					this->StartReloading();
				}
				else
				{
					this->Phase = EFirearmPhase::IDLE;
				}
			}
			else if (this->Phase == EFirearmPhase::Reloading)
			{
				this->Phase = EFirearmPhase::IDLE;
				auto* FirearmRef = this->GetFirearmRef();

				if (FirearmRef)
				{
					this->NumberOfBullets = FirearmRef->MaxNumberOfBullets;
				}
			}
		}
	}
}

void UFirearmWeaponInstance::Init()
{
	auto* FirearmRef = this->GetFirearmRef();

	if (FirearmRef)
	{
		this->NumberOfBullets = FirearmRef->MaxNumberOfBullets;
	}

	this->Phase = EFirearmPhase::IDLE;

}

bool UFirearmWeaponInstance::CanShoot()
{
	return this->Phase == EFirearmPhase::IDLE;
}

void UFirearmWeaponInstance::StartReloading()
{
	auto* FirearmRef = this->GetFirearmRef();

	if (FirearmRef)
	{
		this->Phase = EFirearmPhase::Reloading;
		this->TimeoutValue = FirearmRef->ReloadingTime;
		this->NumberOfBullets = 0;
	}
}

void UFirearmWeaponInstance::OnShoot()
{
	auto* FirearmRef = this->GetFirearmRef();

	if (FirearmRef)
	{
		this->Phase = EFirearmPhase::Shooting;
		this->TimeoutValue = FirearmRef->ShootTime;
		this->NumberOfBullets -= 1;
	}

	this->bShouldPlayHitAnimation = true;
	this->HitAnimationTime = 0.0f;
}