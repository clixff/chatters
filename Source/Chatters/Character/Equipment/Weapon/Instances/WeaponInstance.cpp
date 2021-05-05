// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponInstance.h"

UWeaponInstance::UWeaponInstance()
{

}

UWeaponInstance::~UWeaponInstance()
{

}

void UWeaponInstance::Tick(float DeltaTime)
{
    if (this->WeaponRef)
    {
        if (this->bShouldPlayHitAnimation)
        {
            this->HitAnimationTime += DeltaTime;
            if (this->HitAnimationTime >= this->WeaponRef->TimeToPlayHitAnimation)
            {
                this->HitAnimationTime = 0.0f;
                this->bShouldPlayHitAnimation = false;
            }
        }
    }
}

void UWeaponInstance::Init()
{
}

int32 UWeaponInstance::GetDamage()
{
    if (!this->WeaponRef)
    {
        return 0;
    }

    return FMath::RandRange(this->WeaponRef->MinDamage, this->WeaponRef->MaxDamage);
}
