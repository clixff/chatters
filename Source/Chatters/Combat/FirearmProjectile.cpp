// Fill out your copyright notice in the Description page of Project Settings.


#include "FirearmProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "../Player/PlayerPawn.h"
#include "../Character/Bot.h"
#include "../Character/Equipment/Weapon/Instances/FirearmWeaponInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "../Misc/BulletHolesManager.h"
#include "Kismet/KismetMathLibrary.h"

uint32 AFirearmProjectile::TotalNumberOfProjectiles = 0;

// Sets default values
AFirearmProjectile::AFirearmProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->Trace = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Trace"));
	this->SetRootComponent(this->Trace);

}

AFirearmProjectile::~AFirearmProjectile()
{

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

	if (this->bPendingDestroying)
	{
		this->DestroyActor();
		return;
	}

	if (this->bActive)
	{
		this->Time += DeltaTime;
		if (this->Time >= this->MaxTime)
		{
			this->Time = this->MaxTime;
			this->OnEnd();
		}


		if (!this->bSimplified)
		{
			this->DistanceScale = this->Time / this->MaxTime;

			this->SetTraceLocation();
		}
	}

	if (!this->bSimplified)
	{
		if (this->TraceLengthAction == ETraceLengthAction::Increase)
		{
			this->TraceLength += this->TraceLengthSpeed * DeltaTime;

			this->TraceLength = this->DistanceScale * this->Distance;

			if (this->TraceLength >= this->TraceMaxLength)
			{
				this->TraceLength = this->TraceMaxLength;
				this->TraceLengthAction = ETraceLengthAction::Ignore;
			}
		}
		else if (this->TraceLengthAction == ETraceLengthAction::Reduce && this->BulletHitResult.HitResult.bBlockingHit)
		{
			this->TraceLength -= this->TraceLengthSpeed * DeltaTime;
			if (this->TraceLength <= 0.0f)
			{
				this->TraceLength = 0.0f;
				this->DestroyActor();
			}
		}

		if (this->BulletHitResult.HitResult.bBlockingHit || (this->TraceLengthAction == ETraceLengthAction::Increase && this->DistanceScale < 0.8f))
		{
			this->Opacity = FMath::Clamp(this->TraceLength / this->TraceLengthMaxOpacity, 0.0f, 1.0f);
		}
		else
		{
			if (this->DistanceScale >= 0.8f)
			{
				this->Opacity = ((1.0f - this->DistanceScale) / (1.0f - 0.8f));
			}
		}

		this->UpdateTraceLength();
		this->UpdateTraceOpacity();
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

	FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(this->EndLocation, this->StartLocation);
	this->SetActorRotation(NewRotation);
	this->SetTraceLocation();
	this->TraceLengthAction = ETraceLengthAction::Increase;

	if (HitResult.BotToDamage)
	{
		FVector BoneLocationInWorld = HitResult.BotToDamage->GetMesh()->GetSocketLocation(HitResult.HitResult.BoneName);

		this->RelativeImpactLocation = EndLocation - BoneLocationInWorld;
	}

	if (HitResult.HitResult.bBlockingHit && !HitResult.BotToDamage && !HitResult.ExplodingBarrel)
	{
		bShotAtWall = true;
	}
}

void AFirearmProjectile::OnEnd()
{
	this->bActive = false;
	this->TraceLengthAction = ETraceLengthAction::Reduce;

	if (!this->BulletHitResult.HitResult.bBlockingHit || this->bSimplified)
	{
		this->DestroyActor();
	}


	if (!this->BotCauser || !this->FirearmInstance || !this->FirearmRef || (BulletHitResult.BotToDamage && !BulletHitResult.BotToDamage->bAlive))
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
					if (FirearmRef && FirearmRef->ProjectileStaticMesh && FirearmRef->bCanProjectileMeshBeAttachedToEnemy)
					{
						/** Do line trace again to check collision */
						FHitResult NewHitResult;
						FCollisionQueryParams Params;
						Params.AddIgnoredActor(BotCauser);
						GetWorld()->LineTraceSingleByChannel(NewHitResult, StartLocation, RealEndLocation, ECollisionChannel::ECC_GameTraceChannel3, Params);

						FName BoneName = NewHitResult.BoneName;

						if (NewHitResult.bBlockingHit && NewHitResult.GetActor() == BotToDamage)
						{
							BotToDamage->AttachProjectileMeshToBody(FirearmRef->ProjectileStaticMesh, NewHitResult.ImpactPoint, GetActorRotation(), BoneName);
						}
					}

					int32 CriticalHitChance = FMath::RandRange(0, 9);
					bool bCriticalHit = false;

					if (CriticalHitChance < 1)
					{
						bCriticalHit = true;
					}


					FVector ImpulseVector = this->CauserForwardVector * this->FirearmRef->ImpulseForce;

					if (BulletHitResult.bHatDamage)
					{
						if (BotToDamage->CanHatBeDetached())
						{
							BotToDamage->DetachHatAfterShot(BulletHitResult, FirearmRef->ImpulseForce, RealEndLocation);
						}
					}
					else
					{
						BotToDamage->ApplyDamage(this->FirearmInstance->GetDamage(), this->BotCauser, EWeaponType::Firearm, ImpulseVector, RealEndLocation, BulletHitResult.HitResult.BoneName, bCriticalHit);

						APlayerPawn* PlayerPawn = APlayerPawn::Get();

						float DistanceFromCamera = PlayerPawn ? PlayerPawn->GetDistanceFromCamera(RealEndLocation) : 0.0f;

						if (DistanceFromCamera < 5000.0f)
						{
							BotToDamage->SpawnBloodParticle(RealEndLocation, this->GetActorLocation());

							if (FirearmRef && FirearmRef->DamageSound)
							{
								UGameplayStatics::PlaySoundAtLocation(GetWorld(), FirearmRef->DamageSound, RealEndLocation, FMath::RandRange(0.7f, 0.85f));
							}
						}
					}
				}
			}
		}
		else if (BulletHitResult.ExplodingBarrel)
		{
			BulletHitResult.ExplodingBarrel->Explode(this->BotCauser);
		}
		else if (bShotAtWall && !bSimplified)
		{
			FRotator ShotRotation = UKismetMathLibrary::FindLookAtRotation(RealEndLocation, StartLocation);

			if (WallParticle)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WallParticle, RealEndLocation, ShotRotation);
			}

			if (BulletHoleMaterial)
			{
				ShotRotation = BulletHitResult.HitResult.ImpactNormal.Rotation();
				ABulletHolesManager::AddDecal(RealEndLocation, ShotRotation, BulletHoleMaterial);
			}
		}
	}
}

void AFirearmProjectile::SetTraceLocation()
{
	RealEndLocation = this->EndLocation;

	if (!bSimplified && BulletHitResult.BotToDamage)
	{
		FVector BoneLocation = BulletHitResult.BotToDamage->GetMesh()->GetSocketLocation(BulletHitResult.HitResult.BoneName);
		RealEndLocation = BoneLocation + RelativeImpactLocation;
	}

	FVector NewLocation = FMath::Lerp(this->StartLocation, RealEndLocation, this->DistanceScale);
	this->SetActorLocation(NewLocation);
}

void AFirearmProjectile::UpdateTraceLength()
{
	if (this->Trace)
	{
		this->Trace->SetVariableVec3(TEXT("SizeVector"), FVector(this->TraceLength, 0.0f, 0.0f));
	}
}

void AFirearmProjectile::UpdateTraceOpacity()
{
	if (this->Trace)
	{
		this->Trace->SetVariableFloat(TEXT("TraceOpacity"), this->Opacity);
	}
}

FName AFirearmProjectile::GenerateName()
{
	FString NewNameString = FString::Printf(TEXT("Firearm_Projectile_%d"), AFirearmProjectile::TotalNumberOfProjectiles);
	AFirearmProjectile::TotalNumberOfProjectiles++;
	return FName(*NewNameString);
}

void AFirearmProjectile::DestroyActor()
{
	if (this->bDestroyed)
	{
		return;
	}


	//if (this->Trace && this->Trace->IsValidLowLevel())
	//{
	//	this->Trace->DestroyComponent();
	//}

	if (this->Trace)
	{
		auto* TraceSystemInstance = this->Trace->GetSystemInstance();

		if (!TraceSystemInstance || TraceSystemInstance->IsPendingSpawn())
		{
			this->bPendingDestroying = true;
			return;
		}

		this->Trace->DestroyInstance();
	}

	this->bDestroyed = true;
	this->bPendingDestroying = false;

	this->Destroy();
}

void AFirearmProjectile::SetColor(FLinearColor Color)
{
	if (this->Trace)
	{
		this->Trace->SetVariableLinearColor(TEXT("Color"), Color);
	}
}