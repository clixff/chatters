// Fill out your copyright notice in the Description page of Project Settings.


#include "FirearmProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AFirearmProjectile::AFirearmProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFirearmProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFirearmProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (this->bActive)
	{
		this->Time += DeltaTime;
		if (this->Time >= this->MaxTime)
		{
			this->Time = this->MaxTime;
			this->OnEnd();
		}
		
		this->SetActorLocation(FMath::Lerp(this->StartLocation, this->EndLocation, this->Time / this->MaxTime));
	}
	
}

void AFirearmProjectile::Init(FVector InitStartLocation, FVector InitEndLocation, FBulletHitResult HitResult, UFirearmWeaponInstance* FirearmInstanceRef, FVector BotForwardVector)
{
	this->StartLocation = InitStartLocation;
	this->EndLocation = InitEndLocation;
	this->Distance = FVector::Dist(StartLocation, EndLocation);
	float SpeedCantimetersInSecond = this->Speed * 100.0f;
	this->MaxTime = Distance / SpeedCantimetersInSecond;
	this->Time = 0.0f;
	this->bActive = true;

	this->BulletHitResult = HitResult;
	this->FirearmInstance = FirearmInstanceRef;
	this->FirearmRef = this->FirearmInstance->GetFirearmRef();
	this->CauserForwardVector = BotForwardVector;
}

void AFirearmProjectile::OnEnd()
{
	this->bActive = false;

	this->Destroy();

	if (!this->BotCauser || !this->FirearmRef || (BulletHitResult.BotToDamage && !BulletHitResult.BotToDamage->bAlive))
	{
		return;
	}

	if (BulletHitResult.HitResult.bBlockingHit)
	{
		if (BulletHitResult.BotToDamage)
		{
			ABot* BotToDamage = BulletHitResult.BotToDamage;

			if (BotToDamage->ID != this->BotCauser->ID)
			{
				int32 ChanceOfMiss = FMath::RandRange(0, 9);
				bool bMiss = false;

				if (ChanceOfMiss < 1)
				{
					bMiss = true;
				}

				if (bMiss)
				{
					auto* TargetNameWidget = BotToDamage->GetNameWidget();
					if (TargetNameWidget)
					{
						TargetNameWidget->ShowDamageNumber(-1, false);
					}
				}
				else
				{
					int32 CriticalHitChance = FMath::RandRange(0, 9);
					bool bCriticalHit = false;

					if (CriticalHitChance < 1)
					{
						bCriticalHit = true;
						this->BotCauser->SayRandomMessage();
					}

					FVector ImpulseVector = this->CauserForwardVector * FirearmRef->ImpulseForce;
					BotToDamage->ApplyDamage(this->FirearmInstance->GetDamage(), this->BotCauser, EWeaponType::Firearm, ImpulseVector, BulletHitResult.HitResult.ImpactPoint, BulletHitResult.HitResult.BoneName, bCriticalHit);

					if (BotToDamage->BloodParticle)
					{
						FTransform BloodParticleTransform;
						BloodParticleTransform.SetLocation(BulletHitResult.HitResult.ImpactPoint);
						FRotator TestRot = UKismetMathLibrary::FindLookAtRotation(this->StartLocation, BulletHitResult.HitResult.ImpactPoint);
						TestRot.Pitch += 90.0f;
						BloodParticleTransform.SetRotation(FQuat(TestRot));
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BotToDamage->BloodParticle, BloodParticleTransform, true);
					}
				}
			}
		}
		else if (BulletHitResult.ExplodingBarrel)
		{
			BulletHitResult.ExplodingBarrel->Explode(this->BotCauser);
		}
	}
}

