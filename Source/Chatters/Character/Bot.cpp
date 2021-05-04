// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot.h"
#include "Components/CapsuleComponent.h"
#include "../Core/ChattersGameInstance.h"
#include "BotSpawnPoint.h"
#include "Equipment/Weapon/Instances/MeleeWeaponInstance.h"
#include "Equipment/Weapon/Instances/FirearmWeaponInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Misc/MathHelper.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"
#include "../Core/ChattersGameSession.h"

const float ABot::MinAimRotationValue = 0.0f;
const float ABot::MaxAimRotationValue = 100.0f;

// Sets default values
ABot::ABot()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->MaxHealthPoints = 100;
	this->HealthPoints = this->MaxHealthPoints;
	this->ID = 0;
	this->DisplayName = FString(TEXT(""));

	this->HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Head"));
	this->HeadMesh->SetupAttachment(this->GetMesh(), NAME_None);

	this->HatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hat"));
	this->HatMesh->SetupAttachment(this->HeadMesh, FName(TEXT("head_")));
	this->HatMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	this->BeardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Beard"));
	this->BeardMesh->SetupAttachment(this->HeadMesh, FName(TEXT("head_")));
	this->BeardMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	this->WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
	this->WeaponMesh->SetupAttachment(this->GetMesh(), FName(TEXT("R_arm_4")));
	this->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	this->NameWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidget"));
	this->NameWidgetComponent->SetupAttachment(this->GetMesh());
	this->NameWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	this->NameWidgetComponent->SetWidgetClass(UBotNameWidget::StaticClass());
}

ABot::~ABot()
{
	if (this->WeaponInstance != nullptr)
	{
		if (this->WeaponInstance->IsValidLowLevel())
		{
			this->WeaponInstance->ConditionalBeginDestroy();
		}
	}
}

// Called when the game starts or when spawned
void ABot::BeginPlay()
{
	Super::BeginPlay();

	this->MaxHealthPoints = 100;
	this->HealthPoints = this->MaxHealthPoints;
}

// Called every frame
void ABot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (this->bAlive)
	{
		if (this->WeaponInstance)
		{
			this->WeaponInstance->Tick(DeltaTime);

			if (this->bTestAiming)
			{
				this->TestAimingTick(DeltaTime);
			}
			else
			{
				this->CombatTick(DeltaTime);
			}

		}

		//if (this->bMovingToRandomLocation)
		//{
		//	float DistToTarget = FVector::Dist(this->GetActorLocation(), this->RandomLocationTarget);

		//	if (DistToTarget <= 150.0f)
		//	{
		//		this->ApplyDamage(50);
		//		this->bMovingToRandomLocation = false;
		//		this->SayRandomMessage();
		//		this->MoveToRandomLocation();
		//	}
		//}

		auto* NameWidgetObject = this->GetNameWidget();

		if (NameWidgetObject)
		{
			NameWidgetObject->Tick(DeltaTime);
		}

	}
	else
	{
		if (this->SecondsAfterDeath < 100.0f)
		{
			this->SecondsAfterDeath += DeltaTime;
			if (this->SecondsAfterDeath > 100.0f)
			{
				this->SecondsAfterDeath = 100.0f;
			}
		}

		if (this->bHatAttached && this->SecondsAfterDeath <= 25.0f)
		{
			this->TryDetachHat();
		}
	}
}

// Called to bind functionality to input
void ABot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool ABot::GetIsAlive()
{
	return this->bAlive;
}

ABotController* ABot::GetAIController()
{
	auto* BotController = this->GetController();

	if (!BotController)
	{
		return nullptr;
	}

	return Cast<ABotController>(BotController);
}

UBotNameWidget* ABot::GetNameWidget()
{
	if (!this->NameWidget)
	{
		if (!this->NameWidgetComponent)
		{
			return nullptr;
		}

		auto* NameWidgetObject = this->NameWidgetComponent->GetWidget();

		if (!NameWidgetObject)
		{
			return nullptr;
		}

		this->NameWidget = Cast<UBotNameWidget>(NameWidgetObject);
	}

	return this->NameWidget;
}

UChattersGameSession* ABot::GetGameSession()
{
	return UChattersGameSession::Get();
}

void ABot::FindNewEnemyTarget()
{
	float MinDistance = -1.0f;
	ABot* NewTarget = nullptr;

	auto* GameSessionObject = this->GetGameSession();

	if (GameSessionObject)
	{
		auto AliveBots = GameSessionObject->AliveBots;

		for (int32 i = 0; i < AliveBots.Num(); i++)
		{
			ABot* Bot = AliveBots[i];

			if (!Bot || Bot->ID == this->ID || !Bot->bAlive)
			{
				continue;
			}

			float Dist = FVector::Dist(this->GetActorLocation(), Bot->GetActorLocation());

			if (MinDistance == -1.0f || Dist < MinDistance)
			{
				MinDistance = Dist;
				NewTarget = Bot;
			}
			
		}
	}

	this->SetNewEnemyTarget(NewTarget);
}

void ABot::SetNewEnemyTarget(ABot* TargetBot)
{
	if (!TargetBot)
	{
		this->Target.TargetType = ETargetType::None;
		this->Target.Actor = nullptr;
		this->Target.Bot = nullptr;
		this->CombatAction = ECombatAction::IDLE;
		this->bShouldApplyGunAnimation = false;
		return;
	}

	this->Target.TargetType = ETargetType::Bot;
	this->Target.Actor = TargetBot;
	this->Target.Bot = TargetBot;

	this->CombatAction = ECombatAction::IDLE;

	if (this->Target.Bot && this->Target.Bot->CombatStyle == ECombatStyle::Defense)
	{
		this->CombatStyle = ECombatStyle::Attack;
	}
	else
	{
		this->CombatStyle = ECombatStyle::Defense;
	}
}

void ABot::MoveToTarget()
{
	if (!this->Target.Actor)
	{
		return;
	}

	auto* AIController = this->GetAIController();

	if (!AIController)
	{
		return;
	}

	/** Allow to update target location every 1 second */
	this->UpdateMovingTargetTimeout = 1.0f;
	this->CombatAction = ECombatAction::Moving;
	FVector TargetLocation = this->Target.Actor->GetActorLocation();

	AIController->MoveToLocation(TargetLocation);

	this->AimingAngle = 50.0f;
}

void ABot::CombatTick(float DeltaTime)
{
	if (this->WeaponInstance->WeaponRef)
	{
		if (this->Target.Actor && this->Target.TargetType != ETargetType::None)
		{
			if (this->Target.TargetType == ETargetType::Bot && this->Target.Bot && !this->Target.Bot->bAlive)
			{
				this->bMovingToRandomCombatLocation = false;
				this->FindNewEnemyTarget();
			}
			else
			{
				float TargetDist = FVector::Dist(this->GetActorLocation(), this->Target.Actor->GetActorLocation());

				auto* CharacterMovementComponent = this->GetCharacterMovementComponent();

				EWeaponType WeaponType = this->WeaponInstance->WeaponRef->Type;

				float MaxDist = 100.0f;

				if (WeaponType == EWeaponType::Firearm)
				{
					auto* FirearmInstance = Cast<UFirearmWeaponInstance>(this->WeaponInstance);
					if (FirearmInstance && FirearmInstance->GetFirearmRef())
					{
						MaxDist = FirearmInstance->GetFirearmRef()->MaxDistance;
					}
				}

				this->bShouldApplyGunAnimation = (TargetDist <= MaxDist + 100.0f);

				if (TargetDist <= MaxDist)
				{
					if (WeaponType == EWeaponType::Firearm)
					{
						this->FirearmCombatTick(DeltaTime, TargetDist);
					}
				}
				else
				{
					if (CharacterMovementComponent)
					{
						CharacterMovementComponent->MaxWalkSpeed = 600.0f;
					}

					this->bUseControllerRotationYaw = true;

					if (this->CombatAction == ECombatAction::Moving)
					{
						this->UpdateMovingTargetTimeout -= DeltaTime;

						if (this->UpdateMovingTargetTimeout <= 0.0f)
						{
							this->UpdateMovingTargetTimeout = 0.0f;
							this->MoveToTarget();
						}
					}
					else
					{
						this->MoveToTarget();
					}
				}
			}
		}
	}

}

void ABot::FirearmCombatTick(float DeltaTime, float TargetDist)
{
	EWeaponType WeaponType = EWeaponType::Firearm;

	auto* CharacterMovementComponent = this->GetCharacterMovementComponent();

	FVector ActorLocation = this->GetActorLocation();

	auto* FirearmInstance = Cast<UFirearmWeaponInstance>(this->WeaponInstance);

	if (!FirearmInstance)
	{
		return;
	}
	auto* FirearmRef = FirearmInstance->GetFirearmRef();

	auto* AIController = this->GetAIController();

	if (this->CombatAction == ECombatAction::Moving)
	{
		UE_LOG(LogTemp, Display, TEXT("[ABot] Stop moving"));

		if (AIController)
		{
			AIController->StopMovement();
		}
	}

	AExplodingBarrel* TargetBarrel = nullptr;

	if (this->Target.TargetType == ETargetType::ExplodingBarrel)
	{
		auto* Barrel = Cast<AExplodingBarrel>(this->Target.Actor);

		if (!Barrel || !Barrel->bCanExplode)
		{
			this->Target.TargetType = ETargetType::None;
			this->Target.Actor = nullptr;
			this->FindNewEnemyTarget();
		}
		else
		{
			TargetBarrel = Cast<AExplodingBarrel>(this->Target.Actor);
		}
	}
	else
	{
		auto* GameSessionObject = this->GetGameSession();

		if (GameSessionObject)
		{
			int32 ExplodingBarrelsNum = GameSessionObject->ExplodingBarrels.Num();

			if (ExplodingBarrelsNum)
			{
				for (int32 i = 0; i < ExplodingBarrelsNum; i++)
				{
					auto* Barrel = GameSessionObject->ExplodingBarrels[i];

					if (!Barrel)
					{
						continue;
					}

					FVector BarrelLocation = Barrel->GetActorLocation();

					float Dist = FVector::Dist(BarrelLocation, ActorLocation);

					if (Dist <= FirearmRef->MaxDistance && Dist > Barrel->Radius)
					{
						auto BotsInRadius = Barrel->GetBotsInRadius();

						if (BotsInRadius.Num())
						{
							this->Target.Actor = Barrel;
							this->Target.TargetType = ETargetType::ExplodingBarrel;
							TargetBarrel = Barrel;
							break;
						}
					}
				}
			}
		}
	}

	if (this->Target.TargetType == ETargetType::None || !this->Target.Actor)
	{
		return;
	}

	bool bCanShoot = FirearmInstance->CanShoot();
	bool bReloading = FirearmInstance->Phase == EFirearmPhase::Reloading;

	FVector AimTargetLocation = FVector(0.0f);
	
	if (this->Target.TargetType == ETargetType::Bot)
	{
		AimTargetLocation = this->Target.Bot->GetMesh()->GetSocketTransform(TEXT("spine_5")).GetLocation();
	}
	else
	{
		AimTargetLocation = this->Target.Actor->GetActorLocation();
	}

	this->AimAt(AimTargetLocation);

	this->SmoothRotatingTick(DeltaTime);

	AActor* HitActor = nullptr;


	if (bCanShoot)
	{
		FBulletHitResult BulletHitResult = this->LineTraceFromGun(FirearmInstance->GetFirearmRef(), false, false);

		HitActor = BulletHitResult.HitResult.GetActor();
	}

	bool bCanActuallyShoot = bCanShoot && (HitActor == this->Target.Actor || HitActor == this->Target.Bot);

	/** If aiming at barrel */
	if (TargetBarrel && HitActor == this->Target.Actor)
	{
		float Dist = FVector::Dist(TargetBarrel->GetActorLocation(), this->GetActorLocation());

		if (Dist <= TargetBarrel->Radius)
		{
			bCanActuallyShoot = false;
		}
	}

	/** Moving around target */
	float DistToRandomLocation = FVector::Dist(this->CombatRandomLocation, this->GetActorLocation());

	/** If the bot can shoot and not reloading, find new location for it */
	if (this->CombatStyle == ECombatStyle::Attack && !bCanActuallyShoot && !bReloading && (!this->bMovingToRandomCombatLocation || DistToRandomLocation < 150.0f || this->TimeSinceStartedMovingInCombat >= 7.0f))
	{
		FVector NewRandomLocation;
		bool bFoundRandomLocation = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this->GetWorld(), this->Target.Actor->GetActorLocation(), NewRandomLocation, FirearmRef->MaxDistance);

		this->TimeSinceStartedMovingInCombat = 0.0f;

		if (bFoundRandomLocation)
		{
			if (CharacterMovementComponent)
			{
				CharacterMovementComponent->MaxWalkSpeed = 250.0f;
			}
			this->bMovingToRandomCombatLocation = true;
			this->bUseControllerRotationYaw = false;
			this->CombatRandomLocation = NewRandomLocation;

			if (AIController)
			{
				AIController->MoveToLocation(NewRandomLocation);
			}
		}
		else
		{
			this->bMovingToRandomCombatLocation = false;
		}
	}

	/** If the bot can shoot or reloading and still moving, stop it */
	if ((bCanActuallyShoot || bReloading) && this->bMovingToRandomCombatLocation)
	{
		if (AIController)
		{
			AIController->StopMovement();
			this->bMovingToRandomCombatLocation = false;
		}
	}

	if (bMovingToRandomCombatLocation)
	{
		this->TimeSinceStartedMovingInCombat += DeltaTime;
		//DrawDebugSphere(GetWorld(), this->CombatRandomLocation, 20.0f, 12, FColor(0, 0, 255), false, -1.0f);
		//DrawDebugLine(GetWorld(), this->CombatRandomLocation, this->GetActorLocation(), FColor(255, 255, 255), false, -1.0f);

		//if (this->TargetTo)
		//{
		//	DrawDebugLine(GetWorld(), this->CombatRandomLocation, this->TargetTo->GetActorLocation(), FColor(255, 0, 0), false, -1.0f);
		//}
	}

	this->CombatAction = ECombatAction::Shooting;


	float BotSpeed = this->GetSpeed();

	if (bCanActuallyShoot && BotSpeed < 5.0f)
	{
		this->Shoot(true);
	}

}

void ABot::Shoot(bool bBulletOffset)
{
	if (this->WeaponInstance && this->WeaponInstance->WeaponRef)
	{
		EWeaponType WeaponType = this->WeaponInstance->WeaponRef->Type;

		if (WeaponType != EWeaponType::Firearm || !this->WeaponMesh)
		{
			return;
		}

		auto* FirearmInstance = Cast<UFirearmWeaponInstance>(this->WeaponInstance);

		auto* FirearmRef = FirearmInstance->GetFirearmRef();

		if (FirearmInstance->CanShoot())
		{
			UWorld* World = this->GetWorld();

			if (!World)
			{
				return;
			}

			FirearmInstance->OnShoot();

			FVector OutBulletLocation = this->GetFirearmOutBulletWorldPosition();

			FBulletHitResult BulletHitResult = this->LineTraceFromGun(FirearmRef, bBulletOffset, false);

			if (BulletHitResult.HitResult.bBlockingHit)
			{
				if (BulletHitResult.BotToDamage)
				{
					ABot* BotToDamage = BulletHitResult.BotToDamage;

					if (BotToDamage->ID != this->ID)
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
							}

							FVector ImpulseVector = this->GetActorForwardVector() * FirearmRef->ImpulseForce;
							BotToDamage->ApplyDamage(FirearmInstance->GetDamage(), this, WeaponType, ImpulseVector, BulletHitResult.HitResult.ImpactPoint, BulletHitResult.HitResult.BoneName, bCriticalHit);
							this->SayRandomMessage();

							if (BotToDamage->BloodParticle)
							{
								FTransform BloodParticleTransform;
								BloodParticleTransform.SetLocation(BulletHitResult.HitResult.ImpactPoint);
								FRotator TestRot = UKismetMathLibrary::FindLookAtRotation(OutBulletLocation, BulletHitResult.HitResult.ImpactPoint);
								TestRot.Pitch += 90.0f;
								BloodParticleTransform.SetRotation(FQuat(TestRot));
								UGameplayStatics::SpawnEmitterAtLocation(World, BotToDamage->BloodParticle, BloodParticleTransform, true);
							}
						}
					}
				}
				else if (BulletHitResult.ExplodingBarrel)
				{
					BulletHitResult.ExplodingBarrel->Explode(this);
				}
			}

			if (FirearmRef->ShootSound)
			{
				UGameplayStatics::PlaySoundAtLocation(World, FirearmRef->ShootSound, OutBulletLocation, FMath::RandRange(0.5f, 0.7f));
			}

			if (FirearmRef->ShotParticle)
			{
				FTransform ParticleTransform;
				ParticleTransform.SetLocation(OutBulletLocation);
				ParticleTransform.SetRotation(FQuat(this->GetActorRotation()));
				ParticleTransform.SetScale3D(FirearmRef->ParticleScale);
				UGameplayStatics::SpawnEmitterAtLocation(World, FirearmRef->ShotParticle, ParticleTransform, true);
			}
		}
	}
}

void ABot::AimAt(FVector Location)
{
	FVector AimRelativeLocation = Location - this->GetActorLocation();

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(FVector(0.0f), AimRelativeLocation);

	FVector AimLocationRotated = FRotator(0.0f, LookAtRotation.Yaw * -1.0f, 0.0f).RotateVector(AimRelativeLocation);

	FVector SocketLocation = this->GunSocketRelativeLocation;

	/** Gun socket relative location without X */
	FVector SocketYLocation = SocketLocation;
	SocketYLocation.X = 0.0f;

	/** Yaw rotation */

	FVector SocketYLocationRotated = FRotator(0.0f, LookAtRotation.Yaw, 0.0f).RotateVector(SocketYLocation);

	float NewYawRotation = UKismetMathLibrary::FindLookAtRotation(SocketYLocationRotated, AimRelativeLocation).Yaw;

	float OldYawRotation = this->GetActorRotation().Yaw;

	/** Pitch rotation */

	FRotator TempPitchRotation = UKismetMathLibrary::FindLookAtRotation(FVector(0.0f, 0.0f, SocketLocation.Z + this->GunSocketRelativeLocation.Z), AimLocationRotated);

	FVector TempPitchVector = FVector(SocketLocation.Z, 0.0f, 0.0f);
	TempPitchVector = FRotator(0.0f, TempPitchRotation.Pitch, 0.0f).RotateVector(TempPitchVector);

	float Pitch = FMath::Atan2(AimLocationRotated.Z - this->GunAnimationRotationPoint.Z - TempPitchVector.X, AimLocationRotated.X + TempPitchVector.Y);
	Pitch = FMath::RadiansToDegrees(Pitch);

	float PitchClamped = FMath::Clamp(Pitch, this->MinAimingPitchRotation, this->MaxAimingPitchRotation);
	float PitchScale = UKismetMathLibrary::NormalizeToRange(PitchClamped, this->MinAimingPitchRotation, this->MaxAimingPitchRotation);

	this->SmoothRotation.Target.Pitch = FMath::Clamp(FMath::Lerp(ABot::MinAimRotationValue, ABot::MaxAimRotationValue, PitchScale), ABot::MinAimRotationValue, ABot::MaxAimRotationValue);

	if (NewYawRotation != OldYawRotation || this->SmoothRotation.Target.Pitch != this->AimingAngle)
	{
		this->SmoothRotation.bActive = true;

		//float YawDiff = FMath::Fmod((NewYawRotation - OldYawRotation) + 180.0f, 360.0f) - 180.0f;

		float YawDiff = FMath::FindDeltaAngleDegrees(OldYawRotation, NewYawRotation);

		this->SmoothRotation.CurrentYaw = FMath::Fmod((OldYawRotation + 360.0f), 360.0f);

		this->SmoothRotation.Target.Yaw = this->SmoothRotation.CurrentYaw + YawDiff;

		if (this->SmoothRotation.Target.Yaw < this->SmoothRotation.CurrentYaw)
		{
			this->SmoothRotation.YawType = EYawRotatingType::CounterClockwise;
		}
		else
		{
			this->SmoothRotation.YawType = EYawRotatingType::Clockwise;
		}
	}
	else
	{
		this->SmoothRotation.bActive = false;
	}

}

void ABot::TestAimAt()
{
	UWorld* World = this->GetWorld();

	if (World)
	{
		TArray<AActor*> AimTargetActors;
		UGameplayStatics::GetAllActorsWithTag(World, TEXT("Aim_Target"), AimTargetActors);
		if (AimTargetActors.Num())
		{
			this->AimingTarget = AimTargetActors[0];
		}
	}

	this->SetActorLocation(FVector(0.0f, 0.0f, 100.0f));
	this->bTestAiming = true;
}

UCharacterMovementComponent* ABot::GetCharacterMovementComponent()
{
	return Cast<UCharacterMovementComponent>(this->GetMovementComponent());
}


ABot* ABot::CreateBot(UWorld* World, FString NameToSet, int32 IDToSet, TSubclassOf<ABot> Subclass, UChattersGameSession* GameSessionObject)
{
	if (!GameSessionObject)
	{
		return nullptr;
	}

	auto& SpawnPoints = GameSessionObject->BotSpawnPoints;

	if (!SpawnPoints.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("[ABot::CreateBot] No spawn point found for bot ID %d"), IDToSet);
		return nullptr;
	}

	int32 RandNumber = FMath::RandRange(0, SpawnPoints.Num() - 1);
	auto* SpawnPoint = SpawnPoints[RandNumber];

	if (!SpawnPoint)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*FString::Printf(TEXT("Bot_%d"), IDToSet));
	SpawnParams.bNoFail = true;
	ABot* Bot = World->SpawnActor<ABot>(Subclass, SpawnPoint->GetActorLocation(), SpawnPoint->GetRotation(), SpawnParams);

	SpawnPoint->Destroy();
	SpawnPoints.RemoveAt(RandNumber, 1, true);

	if (Bot)
	{
		Bot->Init(NameToSet, IDToSet);
	}

	return Bot;
}


void ABot::OnFootstep()
{
	if (!this->bAlive)
	{
		return;
	}

	UWorld* World = this->GetWorld();

	if (!World)
	{
		return;
	}

	FHitResult HitResult;
	FVector StartLocation = this->GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 150.0f);
	FCollisionQueryParams CollisionParams;
	CollisionParams.bReturnPhysicalMaterial = true;

	World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (HitResult.bBlockingHit)
	{
		UPhysicalMaterial* PhysMaterial = HitResult.PhysMaterial.Get();
		if (PhysMaterial)
		{
			this->PlayFootstepSound(HitResult.Location, PhysMaterial->SurfaceType);
		}
	}
}

void ABot::SetEquipment()
{
	auto* GameSessionObject = this->GetGameSession();
	if (GameSessionObject)
	{
		auto* EquipmentList =  GameSessionObject->EquipmentListLevel;

		if (EquipmentList)
		{
			auto RandomEquipment = EquipmentList->GetRandomEquipment();
			if (this->HatMesh)
			{
				if (!RandomEquipment.Hat)
				{
					this->HatMesh->SetStaticMesh(nullptr);
				}
				else
				{
					this->HatMesh->SetStaticMesh(RandomEquipment.Hat->StaticMesh);
					this->HatMesh->SetRelativeTransform(RandomEquipment.Hat->GetTransform());


					if (RandomEquipment.Hat->StaticMesh)
					{
						TArray<UMaterialInterface*> Materials = RandomEquipment.Hat->GetRandomMaterials();

						for (int32 i = 0; i < Materials.Num(); i++)
						{
							this->HatMesh->SetMaterial(i, Materials[i]);
						}

					}
				}
			}

			if (this->BeardMesh)
			{
				if (!RandomEquipment.BeardStyle)
				{
					this->BeardMesh->SetStaticMesh(nullptr);
				}
				else
				{
					this->BeardMesh->SetStaticMesh(RandomEquipment.BeardStyle->StaticMesh);
					this->BeardMesh->SetRelativeTransform(RandomEquipment.BeardStyle->GetTransform());
				}
			}

			if (RandomEquipment.FaceMaterial)
			{
				if (this->HeadMesh)
				{
					this->HeadMesh->SetMaterial(0, RandomEquipment.FaceMaterial);
				}
			}

			if (RandomEquipment.Weapon)
			{
				if (this->WeaponMesh)
				{
					this->WeaponMesh->SetStaticMesh(RandomEquipment.Weapon->StaticMesh);
					this->WeaponMesh->SetRelativeTransform(RandomEquipment.Weapon->GetTransform());

					UClass* WeaponInstanceClass = UWeaponInstance::StaticClass();

					EWeaponType WeaponType = RandomEquipment.Weapon->Type;

					if (WeaponType == EWeaponType::Melee)
					{
						WeaponInstanceClass = UMeleeWeaponInstance::StaticClass();
					}
					else if (WeaponType == EWeaponType::Firearm)
					{
						WeaponInstanceClass = UFirearmWeaponInstance::StaticClass();
					}

					this->WeaponInstance = NewObject<UWeaponInstance>(this, WeaponInstanceClass);
					if (this->WeaponInstance)
					{
						this->WeaponInstance->WeaponRef = RandomEquipment.Weapon;
						this->WeaponInstance->BotOwner = this;
						this->WeaponInstance->Init();

						if (WeaponType == EWeaponType::Firearm)
						{
							UFirearmWeaponItem* FirearmRef = Cast<UFirearmWeaponItem>(RandomEquipment.Weapon);
							if (FirearmRef)
							{
								this->GunSocketRelativeLocation = FirearmRef->SocketRelativeLocation - this->GunAnimationRotationPoint;
							}
						}
					}
				}
			}
		}
	}
}

void ABot::Init(FString NewName, int32 NewID)
{
	this->SpawnDefaultController();
	this->DisplayName = NewName;
	this->ID = NewID;
	this->HealthPoints = this->MaxHealthPoints;

	auto* NameWidgetObject = this->GetNameWidget();

	if (NameWidgetObject)
	{
		NameWidget->Nickname = this->DisplayName;
		NameWidget->UpdateHealth(this->GetHeathValue());
	}

	this->SetEquipment();
}

void ABot::MoveToRandomLocation()
{
	auto* AIController = this->GetAIController();

	if (AIController)
	{
		float XPos = FMath::RandRange(-7400, 7400);
		float YPos = FMath::RandRange(-7400, 7400);

		this->RandomLocationTarget = FVector(XPos, YPos, 97);

		AIController->MoveToLocation(this->RandomLocationTarget);
		bMovingToRandomLocation = true;
	}
}

void ABot::ApplyDamage(int32 Damage, ABot* ByBot, EWeaponType WeaponType, FVector ImpulseVector, FVector ImpulseLocation, FName BoneHit, bool bCritical)
{
	/** When game session not started */
	if (!this->bReady)
	{
		return;
	}

	if (Damage < 1)
	{
		return;
	}

	if (bCritical)
	{
		Damage *= 2;
	}

	auto OldHP = this->HealthPoints;

	this->HealthPoints -= Damage;

	if (this->HealthPoints <= 0)
	{
		this->HealthPoints = 0;
		this->OnDead(ByBot, WeaponType, ImpulseVector, ImpulseLocation, BoneHit);
	}

	UE_LOG(LogTemp, Display, TEXT("[ABot] Applying %d damage to bot. Old hp: %d. New HP: %d"), Damage, OldHP, this->HealthPoints);

	auto* NameWidgetObject = this->GetNameWidget();

	if (NameWidgetObject)
	{
		float HealthValue = this->GetHeathValue();
		NameWidgetObject->UpdateHealth(HealthValue);
		NameWidgetObject->ShowDamageNumber(Damage, bCritical);
	}
	
	if (this->bPlayerAttached)
	{
		auto* GameSessionObject = this->GetGameSession();
		if (GameSessionObject)
		{
			auto* SessionWidget = GameSessionObject->GetSessionWidget();
			if (SessionWidget)
			{
				SessionWidget->UpdateSpectatorBotHealth(this->HealthPoints);
			}
		}
	}
}

float ABot::GetHeathValue()
{
	return (float(this->HealthPoints) / float(this->MaxHealthPoints));
}

void ABot::Say(FString Message)
{
	UE_LOG(LogTemp, Display, TEXT("[ABot] Say message: %s"), *Message);
	if (Message.IsEmpty())
	{
		return;
	}

	this->ChatBubbleMessage = Message;

	auto* NameWidgetObject = this->GetNameWidget();
	
	if (NameWidgetObject)
	{
		NameWidgetObject->UpdateChatBubbleMessage(this->ChatBubbleMessage);
	}
}

void ABot::SayRandomMessage()
{
	FString Message;

	int32 MessageLength = FMath::RandRange(5, 100);

	TArray<TCHAR> RawMessage;

	RawMessage.Init(L' ', MessageLength);

	//TCHAR* RawMessage = new TCHAR[MessageLength];

	for (int32 i = 0; i < MessageLength; i++)
	{
		int32 RandNum = FMath::RandRange(0, 10);

		int32 RandCharCode = 0;

		/** Use latin symbols */
		if (RandNum < 5)
		{
			RandCharCode = FMath::RandRange(0x41, 0x7a);
		}
		/** Use cyrillic symbols */
		else if (RandNum < 10)
		{
			RandCharCode = FMath::RandRange(0x410, 0x44f);
		}
		else
		{
			RandCharCode = 20;
		}

		RawMessage[i] = TCHAR(RandCharCode);
	}
	
	Message = FString(RawMessage);

	this->Say(Message);
}

void ABot::OnDead(ABot* Killer, EWeaponType WeaponType, FVector ImpulseVector, FVector ImpulseLocation, FName BoneHit)
{
	this->bAlive = false;
	this->HealthPoints = 0;

	this->SetActorLocation(this->GetActorLocation());

	auto* AIController = this->GetAIController();

	if (AIController)
	{
		AIController->StopMovement();
	}

	if (this->GetMesh())
	{
		this->GetMesh()->SetSimulatePhysics(true);
		this->GetMesh()->SetCollisionProfileName(FName(TEXT("DeadBody")), true);
	}

	if (this->GetCapsuleComponent())
	{
		this->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	}

	if (WeaponType == EWeaponType::Firearm || WeaponType == EWeaponType::Explosion || WeaponType == EWeaponType::Bow)
	{
		this->GetMesh()->AddImpulseAtLocation(ImpulseVector, ImpulseLocation, BoneHit);
	}

	this->bMovingToRandomLocation = false;

	if (this->NameWidgetComponent)
	{
		this->NameWidgetComponent->SetVisibility(false);
	}


	if (this->HatMesh)
	{
		this->bHatAttached = true;
	}

	if (this->WeaponInstance)
	{
		this->DeatachWeapon();
	}

	this->CombatAction = ECombatAction::IDLE;
	this->bShouldApplyGunAnimation = false;

	auto* CharacterMovementComponent = this->GetCharacterMovementComponent();

	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->bUseRVOAvoidance = false;
	}

	if (Killer && Killer != this)
	{
		Killer->Kills++;
	}

	auto* GameSessionObject = this->GetGameSession();

	if (GameSessionObject)
	{
		GameSessionObject->OnBotDied(this->ID);
		auto* SessionWidget = GameSessionObject->GetSessionWidget();
		if (SessionWidget)
		{
			FString KillerName = TEXT("");
			if (Killer)
			{
				KillerName = Killer->DisplayName;

				if (Killer->bPlayerAttached && Killer != this)
				{
					SessionWidget->UpdateSpectatorBotKills(Killer->Kills);
				}
			}
			SessionWidget->OnKill(KillerName, this->DisplayName);
		}
	}
}

float ABot::GetSpeed()
{
	return this->GetVelocity().Size();
}

float ABot::GetSpeedForAnimationBlueprint()
{
	float DefaultSpeed = this->GetSpeed();

	return DefaultSpeed;
}

UWeaponItem* ABot::GetWeaponRef()
{
	if (!this->WeaponInstance)
	{
		return nullptr;
	}
	else
	{
		return this->WeaponInstance->WeaponRef;
	}
}

void ABot::OnGameSessionStarted()
{
	this->bReady = true;

	//this->TestAimAt();

	this->FindNewEnemyTarget();

	//this->MoveToRandomLocation();
}

void ABot::TryDetachHat()
{
	if (this->bHatAttached && this->HatMesh && this->GetMesh())
	{
		FRotator HeadRotation = this->GetMesh()->GetSocketRotation(FName(TEXT("head_")));

		if (HeadRotation.Roll > 150.0f || HeadRotation.Roll < 75.0f || HeadRotation.Pitch > 50.0f || HeadRotation.Pitch < 50.0f)
		{
			this->bHatAttached = false;
			this->HatMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			this->HatMesh->SetCollisionProfileName(FName(TEXT("OufitPhysics")), true);
			this->HatMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
			this->HatMesh->SetSimulatePhysics(true);
		}
	}
}

void ABot::DeatachWeapon()
{
	if (this->WeaponMesh && this->WeaponInstance)
	{
		this->WeaponInstance->BotOwner = nullptr;
		this->WeaponInstance = nullptr;
		this->WeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		this->WeaponMesh->SetCollisionProfileName(FName(TEXT("OufitPhysics")), true);
		this->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		this->WeaponMesh->SetSimulatePhysics(true);
	}
}

void ABot::SmoothRotatingTick(float DeltaTime)
{
	if (this->SmoothRotation.bActive)
	{
		bool bYawEnd = false;
		bool bPitchEnd = false;

		const float YawSpeed = 360.0f;
		const float PitchSpeed = 180.0f;

		if (this->SmoothRotation.YawType == EYawRotatingType::Clockwise)
		{
			this->SmoothRotation.CurrentYaw += YawSpeed * DeltaTime;
			
			if (this->SmoothRotation.CurrentYaw >= this->SmoothRotation.Target.Yaw)
			{
				this->SmoothRotation.CurrentYaw = this->SmoothRotation.Target.Yaw;
				bYawEnd = true;
			}
		}
		else
		{
			this->SmoothRotation.CurrentYaw -= YawSpeed * DeltaTime;

			if (this->SmoothRotation.CurrentYaw <= this->SmoothRotation.Target.Yaw)
			{
				this->SmoothRotation.CurrentYaw = this->SmoothRotation.Target.Yaw;
				bYawEnd = true;
			}
		}

		this->SetActorRotation(FRotator(0.0f, this->SmoothRotation.CurrentYaw, 0.0f));

		if (this->AimingAngle < this->SmoothRotation.Target.Pitch)
		{
			this->AimingAngle += PitchSpeed * DeltaTime;

			if (this->AimingAngle >= this->SmoothRotation.Target.Pitch)
			{
				this->AimingAngle = this->SmoothRotation.Target.Pitch;
				bPitchEnd = true;
			}
		}
		else
		{
			this->AimingAngle -= PitchSpeed * DeltaTime;

			if (this->AimingAngle <= this->SmoothRotation.Target.Pitch)
			{
				this->AimingAngle = this->SmoothRotation.Target.Pitch;
				bPitchEnd = true;
			}
		}

		if (bYawEnd && bPitchEnd)
		{
			this->SmoothRotation.bActive = false;
		}
	}
}

void ABot::TestAimingTick(float DeltaTime)
{
	FVector AimAtLocation = this->AimAtTestLocation;

	if (this->AimingTarget)
	{
		AimAtLocation = this->AimingTarget->GetActorLocation();
	}

	//

	FVector BulletOut = this->GetFirearmOutBulletWorldPosition();
	DrawDebugSphere(GetWorld(), BulletOut, 5.0f, 6, FColor(255, 0, 0), false, -1.0f);
	DrawDebugLine(GetWorld(), this->GetActorLocation(), BulletOut, FColor(255, 0, 0), false, -1.0f);
	DrawDebugLine(GetWorld(), this->GetActorLocation() + this->GunAnimationRotationPoint, BulletOut, FColor(0, 255, 0), false, -1.0f);

	this->bShouldApplyGunAnimation = true;
	this->CombatAction = ECombatAction::Shooting;
	const bool bMovingToRandomPointWhileAiming = false;

	float Dist = FVector::Dist(this->GetActorLocation(), this->RandomPointToMoveWhileAiming);

	if (bMovingToRandomPointWhileAiming && (Dist < 130.0f))
	{
		this->bUseControllerRotationYaw = false;
		
		auto* CharacterMovementComponent = this->GetCharacterMovementComponent();

		if (CharacterMovementComponent)
		{
			CharacterMovementComponent->MaxWalkSpeed = 250.0f;
		}

		float XPos = 0.0f;
		float YPos = 0.0f;

		int32 RandNumber = FMath::RandRange(0, 2);
		float RandLocation = 0.0f;


		if (RandNumber == 0)
		{
			RandLocation = 0.0f;
		}
		else if (RandNumber == 1)
		{
			RandLocation = 500.0f;
		}
		else
		{
			RandLocation = -500.0f;
		}

		if (bTestAimingMovingToCenter)
		{
			RandLocation = 0.0f;
			bTestAimingMovingToCenter = false;
		}
		else
		{
			bTestAimingMovingToCenter = true;
		}

		if (FMath::RandRange(0, 1))
		{
			XPos = RandLocation;
		}
		else
		{
			YPos = RandLocation;
		}

		this->RandomPointToMoveWhileAiming = FVector(XPos, YPos, 0.0f);

		auto* AIController = this->GetAIController();

		if (AIController)
		{
			AIController->MoveToLocation(this->RandomPointToMoveWhileAiming);
		}

	}

	this->AimAt(AimAtLocation);
	this->SmoothRotatingTick(DeltaTime);
	this->Shoot(false);
}

FVector ABot::GetFirearmOutBulletWorldPosition(FRotator GunRotation, bool bShouldRecalculateGunLocation)
{
	if (bShouldRecalculateGunLocation)
	{
		GunRotation = this->GetGunRotation();
	}

	FVector TempVector = this->GunSocketRelativeLocation;
	//TempVector.Z = 0;
	FVector OutLocation = GunRotation.RotateVector(TempVector);
	//OutLocation.Z += this->GunSocketRelativeLocation.Z + this->GunAnimationRotationPoint.Z;;
	//OutLocation.Z += this->GunSocketRelativeLocation.Z;
	OutLocation += this->GunAnimationRotationPoint;
	OutLocation += this->GetActorLocation();

	return OutLocation;
}

FVector ABot::GetFirearmBulletTargetWorldPosition(FVector OutBulletWorldPosition, float BulletDistance, FRotator GunRotation, bool bShouldRecalculateGunLocation, bool bBulletOffset)
{
	if (bShouldRecalculateGunLocation)
	{
		GunRotation = this->GetGunRotation();
	}

	if (bBulletOffset)
	{
		GunRotation.Pitch += FMath::RandRange(-5.0f, 5.0f);
		GunRotation.Yaw += FMath::RandRange(-5.0f, 5.0f);
	}

	FVector GunRotationVector = GunRotation.Vector();
	GunRotationVector.Normalize();

	FVector EndLocation = OutBulletWorldPosition + (GunRotationVector * BulletDistance);

	return EndLocation;
}

FRotator ABot::GetGunRotation()
{
	float YawRotation = this->GetActorRotation().Yaw;
	float AimingAngleScale = UKismetMathLibrary::NormalizeToRange(this->AimingAngle, ABot::MinAimRotationValue, ABot::MaxAimRotationValue);

	float PitchRotation = FMath::Clamp(FMath::Lerp(this->MinAimingPitchRotation, this->MaxAimingPitchRotation, AimingAngleScale), this->MinAimingPitchRotation, this->MaxAimingPitchRotation);

	FRotator GunRotation = FRotator(PitchRotation, YawRotation, 0.0f);

	return GunRotation;
}

FBulletHitResult ABot::LineTraceFromGun(UFirearmWeaponItem* FirearmRef, bool bBulletOffset, bool bDrawDebugLines)
{
	FBulletHitResult BulletHitResult;

	if (FirearmRef)
	{
		FRotator GunRotation = this->GetGunRotation();
		FVector StartLocation = this->GetFirearmOutBulletWorldPosition(GunRotation, false);
		FVector EndLocation = this->GetFirearmBulletTargetWorldPosition(StartLocation, FirearmRef->MaxDistance, GunRotation, false, bBulletOffset);

		UWorld* World = this->GetWorld();

		if (!World)
		{
			return BulletHitResult;
		}

		FHitResult HitResult;
		World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel3);

#if !UE_BUILD_SHIPPING

		if (bDrawDebugLines)
		{
			if (HitResult.bBlockingHit)
			{
				DrawDebugSphere(World, HitResult.ImpactPoint, 15.0f, 6, FColor(0, 255, 0), false, 0.5f);
			}

			DrawDebugLine(World, StartLocation, HitResult.bBlockingHit ? HitResult.ImpactPoint : EndLocation, FColor(0, 0, 255), false, 0.5f);
		}
#endif

		BulletHitResult.HitResult = HitResult;


		if (HitResult.bBlockingHit)
		{
			AActor* Actor = HitResult.GetActor();

			if (Actor)
			{
				ABot* Bot = Cast<ABot>(Actor);

				if (Bot)
				{
					BulletHitResult.BotToDamage = Bot;
					return BulletHitResult;
				}

				AExplodingBarrel* Barrel = Cast<AExplodingBarrel>(Actor);
				if (Barrel)
				{
					BulletHitResult.ExplodingBarrel = Barrel;
					return BulletHitResult;
				}
			}
		}

	}

	return BulletHitResult;
}
