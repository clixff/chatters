// Fill out your copyright notice in the Description page of Project Settings.


#include "FirearmWeaponInstance.h"
#include "../../../Bot.h"

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

	if (this->WeaponRef->bLoopingHitAnimation)
	{
		switch (this->Phase)
		{
		case EFirearmPhase::Shooting:
			this->bShouldPlayHitAnimation = true;
			break;
		default:
			this->bShouldPlayHitAnimation = false;
			break;
		}
	}

	if (this->TimeoutValue > 0.0f && this->Phase != EFirearmPhase::IDLE)
	{
		this->TimeoutValue -= DeltaTime;

		auto* BotOwnerRef = Cast<ABot>(this->BotOwner);

		if (this->Phase == EFirearmPhase::Reloading && !this->bSpawnedReloadingParticle)
		{
			auto* FirearmRef = this->GetFirearmRef();
			if (FirearmRef)
			{
				float TimeoutValueReversed = FirearmRef->ReloadingTime - this->TimeoutValue;

				if (TimeoutValueReversed >= FirearmRef->ReloadingParticleStartSecond)
				{
					this->bSpawnedReloadingParticle = true;
					
					if (BotOwnerRef)
					{
						BotOwnerRef->SpawnReloadingParticle(FirearmRef->ReloadingParticle, FirearmRef->ReloadingParticleTransform);
					}
				}
			}
		}

		if (this->Phase == EFirearmPhase::Shooting)
		{
			ProjectileMeshRespawnTimer.Add(DeltaTime);

			if (ProjectileMeshRespawnTimer.IsEnded())
			{
				SetProjectileMeshVisibility(true);
			}

			if (BowstringAnimInstance)
			{
				if (!BowstringDetachTimer.IsEnded())
				{
					BowstringDetachTimer.Add(DeltaTime);

					if (BowstringDetachTimer.IsEnded())
					{
						bShouldAttachBowstring = false;
					}
				}

				if (!BowstringAttachTimer.IsEnded())
				{
					BowstringAttachTimer.Add(DeltaTime);

					if (BowstringAttachTimer.IsEnded())
					{
						bShouldAttachBowstring = true;
					}
				}
			}
		}

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
					SetProjectileMeshVisibility(true);
					this->SecondsWithoutHit.Reset();

					if (this->BowstringAnimInstance)
					{
						bShouldAttachBowstring = true;
					}
				}
			}
			else if (this->Phase == EFirearmPhase::Reloading)
			{
				this->Phase = EFirearmPhase::IDLE;
				this->bShouldPlayReloadingAnimation = false;
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
	Super::Init();
	auto* FirearmRef = this->GetFirearmRef();

	if (FirearmRef)
	{
		this->NumberOfBullets = FirearmRef->MaxNumberOfBullets;

		auto* BotOwnerRef = Cast<ABot>(this->BotOwner);

		if (FirearmRef->ProjectileStaticMesh)
		{
			ProjectileMeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(0.0f), FRotator(0.0f), FActorSpawnParameters());
			if (ProjectileMeshActor)
			{
				ProjectileMeshActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
				ProjectileMeshActor->GetStaticMeshComponent()->SetStaticMesh(FirearmRef->ProjectileStaticMesh);
				ProjectileMeshActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

				if (BotOwnerRef && BotOwnerRef->WeaponMesh)
				{
					ProjectileMeshActor->AttachToComponent(BotOwnerRef->WeaponMesh, FAttachmentTransformRules::KeepRelativeTransform);
					ProjectileMeshActor->SetActorRelativeTransform(FirearmRef->ProjectileMeshTransform);
					BotOwnerRef->bProjectileMeshExists = true;
				}

				SetProjectileMeshVisibility(false);
			}
		}

		if (FirearmRef->BowstringMesh)
		{
			BowstringComponent = NewObject<USkeletalMeshComponent>(BotOwnerRef, TEXT("Bowstring"));
			if (BowstringComponent)
			{
				BowstringComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				BowstringComponent->AttachTo(BotOwnerRef->WeaponMesh);
				BowstringComponent->SetSkeletalMesh(FirearmRef->BowstringMesh);
				BowstringComponent->RegisterComponent();
				BotOwnerRef->AddInstanceComponent(BowstringComponent);

				if (FirearmRef->BowstringAnimation)
				{
					BowstringComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);
					BowstringComponent->SetAnimInstanceClass(FirearmRef->BowstringAnimation);

					this->BowstringAnimInstance = Cast<UBowstringAnimation>(BowstringComponent->GetAnimInstance());

					if (BowstringAnimInstance)
					{
						BowstringAnimInstance->Bot = BotOwnerRef;
						BowstringAnimInstance->FirearmInstance = this;
					}
				}
			}
		}
	}

	this->Phase = EFirearmPhase::IDLE;


}

bool UFirearmWeaponInstance::CanShoot()
{
	return this->Phase == EFirearmPhase::IDLE;
}

void UFirearmWeaponInstance::SetProjectileMeshVisibility(bool bVisible)
{
	if (this->ProjectileMeshActor)
	{
		auto* BotOwnerRef = Cast<ABot>(this->BotOwner);

		if (bVisible && this->Phase == EFirearmPhase::Shooting)
		{
			if (ProjectileMeshRespawnTimer.IsEnded())
			{
				ProjectileMeshRespawnTimer.Reset();
			}
			else
			{
				bVisible = false;
			}
		}

		if (BotOwnerRef)
		{
			BotOwnerRef->bProjectileMeshVisibility = bVisible;
		}

		ProjectileMeshActor->SetActorHiddenInGame(!bVisible);
	}
}

void UFirearmWeaponInstance::StartReloading()
{
	auto* FirearmRef = this->GetFirearmRef();

	if (FirearmRef)
	{
		this->Phase = EFirearmPhase::Reloading;
		this->TimeoutValue = FirearmRef->ReloadingTime;
		this->NumberOfBullets = 0;
		this->bShouldPlayReloadingAnimation = true;
		/** If ReloadingParticle reference is null, set as already spawned, so there will be no attempts to spawn new particle */
		this->bSpawnedReloadingParticle = (FirearmRef->ReloadingParticle == nullptr);
	}

	if (this->BotOwner)
	{
		ABot* Bot = Cast<ABot>(this->BotOwner);

		if (Bot)
		{
			Bot->StopMovement();
		}
	}
}

void UFirearmWeaponInstance::OnShoot()
{
	auto* FirearmRef = this->GetFirearmRef();

	if (FirearmRef)
	{
		this->Phase = EFirearmPhase::Shooting;
		this->TimeoutValue = FirearmRef->ShootTime + FMath::RandRange(0.0f, 0.1f);
		this->NumberOfBullets -= 1;
		this->bHitPrevTick = true;
		ProjectileMeshRespawnTimer.Max = FirearmRef->ProjectileMeshRespawnTime;

		BowstringDetachTimer.Max = FirearmRef->BowstringDetachTime;
		BowstringAttachTimer.Max = FirearmRef->BowstringAttachTime;
	}

	ProjectileMeshRespawnTimer.Reset();
	BowstringDetachTimer.Reset();
	BowstringAttachTimer.Reset();

	if (this->BowstringAnimInstance)
	{
		bShouldAttachBowstring = true;
	}

	this->bShouldPlayHitAnimation = true;
	this->HitAnimationTime = 0.0f;

	SetProjectileMeshVisibility(false);

	//UE_LOG(LogTemp, Display, TEXT("[UFirearmWeaponInstance] Shoot. Seconds without shooting: %f"), this->SecondsWithoutHit.Current);
}