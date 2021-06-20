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
#include "../UI/Widgets/KillFeedElement.h"
#include "../Player/PlayerPawn.h"
#include "../Core/ChattersGameSession.h"

DECLARE_STATS_GROUP(TEXT("BOTS_Game"), STATGROUP_BOTS, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("CombatTick Time"), STAT_StatsCombatTick, STATGROUP_BOTS);
DECLARE_CYCLE_STAT(TEXT("FirearmCombatTick Time"), STAT_StatsFirearmCombatTick, STATGROUP_BOTS);
DECLARE_CYCLE_STAT(TEXT("MeleeCombatTick Time"), STAT_StatsMeleeCombatTick, STATGROUP_BOTS);


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

	this->MeleeCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("MeleeCollsion"));;
	this->MeleeCollision->SetupAttachment(this->WeaponMesh);
	this->MeleeCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	//this->MeleeCollision->OnComponentBeginOverlap.AddDynamic(this, &ABot::MeleeCollisionBeginOverlap);

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
				//this->CombatTickTimeout.Add(DeltaTime);

				//if (this->CombatTickTimeout.IsEnded())
				//{
					this->CombatTick(DeltaTime);
				//	this->CombatTickTimeout.Reset();
				//}
			}

		}

		auto* NameWidgetObject = this->GetNameWidget();

		if (NameWidgetObject)
		{
			NameWidgetObject->Tick(DeltaTime);
		}

		/** If not moving */
		if (this->bReady && !this->bShouldApplyGunAnimation && this->GetActorLocation() == this->LastTickLocation)
		{
			this->SecondsWithoutMoving.Add(DeltaTime);

			if (this->SecondsWithoutMoving.IsEnded())
			{
				FString OldTargetName = TEXT("nullptr");
				float OldTargetDist = 0.0f;

				if (this->Target.Actor)
				{
					OldTargetName = this->Target.Actor->GetName();
					OldTargetDist = FVector::Dist(this->Target.Actor->GetActorLocation(), this->GetActorLocation());
				}

				UE_LOG(LogTemp, Warning, TEXT("[ABot] Bot %s stuck, finding new target. Combat Action was %d. Old target name is %s"), *this->DisplayName, int(this->CombatAction), *OldTargetName);
				this->SecondsWithoutMoving.Reset();
				this->FindNewEnemyTarget();
			}
		}
		else
		{
			this->SecondsWithoutMoving.Current = 0.0f;
		}

		this->LastTickLocation = this->GetActorLocation();
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

		if (this->SecondsAfterDeath >= 4.0f && !this->bCheckedBloodDecalCreation)
		{
			this->CreateFloorBloodDecal();
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

			if (!Bot || Bot->ID == this->ID || !Bot->bAlive || !this->IsEnemy(Bot))
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
	this->SecondsAimingWithoutHitting.Reset();
	this->DefenderSecondsWithoutMoving.Reset();
	this->bMovingToRandomLocation = false;
	this->TimeSinceStartedMovingInCombat = 0.0f;

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

	if (this->Target.Bot)
	{
		EWeaponType EnemyWeaponType = this->Target.Bot->GetWeaponType();

		if (this->Target.Bot->CombatStyle == ECombatStyle::Defense || EnemyWeaponType == EWeaponType::Melee)
		{
			this->CombatStyle = ECombatStyle::Attack;
		}
		else
		{
			this->CombatStyle = ECombatStyle::Defense;

		}
	}
}

void ABot::UpdateEquipmentTeamColors()
{
	if (this->WeaponInstance && this->WeaponInstance->WeaponRef)
	{
		auto* WeaponRef = this->WeaponInstance->WeaponRef;

		/** Set random projectile trace colors */
		if (WeaponRef->Type == EWeaponType::Firearm)
		{
			auto* FirearmInstance = Cast<UFirearmWeaponInstance>(this->WeaponInstance);
			if (!FirearmInstance || !FirearmInstance->GetFirearmRef())
			{
				return;
			}

			auto* FirearmRef = FirearmInstance->GetFirearmRef();
			auto TeamProjectileColors = FirearmRef->TeamProjectileColors;


			if (this->Team != EBotTeam::White && TeamProjectileColors.Num() > 1)
			{
				FirearmInstance->TraceColor = this->Team == EBotTeam::Blue ? TeamProjectileColors[0] : TeamProjectileColors[1];
			}
			else
			{
				FirearmInstance->TraceColor = FirearmRef->GetRandomProjectileColor();
			}

		}
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

	if (this->Target.TargetType == ETargetType::ExplodingBarrel)
	{
		UE_LOG(LogTemp, Display, TEXT("[ABot] Moving bot %s to exploding barrel. "), *this->DisplayName);
	}

	AIController->MoveToNewLocation(TargetLocation);

	this->AimingAngle = 50.0f;
}

void ABot::CombatTick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_StatsCombatTick);

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

				float MaxDist = 150.0f;
				float RealMaxDist = MaxDist;

				if (WeaponType == EWeaponType::Firearm)
				{
					auto* FirearmInstance = Cast<UFirearmWeaponInstance>(this->WeaponInstance);
					if (FirearmInstance && FirearmInstance->GetFirearmRef())
					{
						MaxDist = FirearmInstance->GetFirearmRef()->MaxDistance;
						RealMaxDist = MaxDist;

						if (this->CombatAction == ECombatAction::Moving)
						{
							MaxDist -= 150.0f;
						}
					}
				}

				//this->bShouldApplyGunAnimation = (TargetDist <= MaxDist + 100.0f);

				if (WeaponType == EWeaponType::Melee)
				{
					this->bShouldApplyGunAnimation = (TargetDist <= 400.0f);
					MaxDist = 400.0f;
					RealMaxDist = MaxDist;
				}

				bool bActivateCombatTick = false;

				if (TargetDist <= MaxDist)
				{
					//bActivateCombatTick = true;
					if (TargetDist <= 150.0f)
					{
						bActivateCombatTick = true;
					}
					else
					{
						bool bObstacleBetweenBots = !this->TraceToTargetResult(true);
						if (!bObstacleBetweenBots)
						{
							bActivateCombatTick = true;
						}
					}
				}

				if (WeaponType == EWeaponType::Firearm)
				{
					this->bShouldApplyGunAnimation = bActivateCombatTick;
				}

				if (bActivateCombatTick)
				{
					if (WeaponType == EWeaponType::Firearm)
					{
						this->FirearmCombatTick(DeltaTime, TargetDist);
					}
					else if (WeaponType == EWeaponType::Melee)
					{
						this->MeleeCombatTick(DeltaTime, TargetDist);
					}
				}
				else
				{
					if (CharacterMovementComponent)
					{
						CharacterMovementComponent->MaxWalkSpeed = 600.0f;

						if (this->bShouldApplyGunAnimation)
						{
							CharacterMovementComponent->MaxWalkSpeed = this->WeaponInstance->WeaponRef->MaxWalkSpeed;
						}
					}

					//this->bUseControllerRotationYaw = true;

					if (this->CombatAction == ECombatAction::Moving)
					{
						if (this->SmoothRotation.bActive)
						{
							this->SmoothRotatingTick(DeltaTime);
						}
						else
						{
							this->bUseControllerRotationYaw = true;
						}

						this->UpdateMovingTargetTimeout -= DeltaTime;

						if (this->UpdateMovingTargetTimeout <= 0.0f)
						{
							this->UpdateMovingTargetTimeout = 0.0f;
							this->MoveToTarget();
						}
					}
					else
					{
						if (!this->ShouldPlayWeaponReloadingAnimation() && !this->ShouldPlayWeaponHitAnimation())
						{
							this->AimAt(this->Target.Actor->GetActorLocation());
							this->bUseControllerRotationYaw = false;
							this->SmoothRotatingTick(DeltaTime);
							this->MoveToTarget();
						}
					}
				}

				if (this->bShouldApplyGunAnimation)
				{
					this->AimingTime.Add(DeltaTime);
				}
				else
				{
					this->AimingTime.Current = 0.0f;
				}
			}
		}
	}

}

void ABot::FirearmCombatTick(float DeltaTime, float TargetDist)
{
	SCOPE_CYCLE_COUNTER(STAT_StatsFirearmCombatTick);

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
		//UE_LOG(LogTemp, Display, TEXT("[ABot] Stop moving"));

		if (AIController)
		{
			AIController->StopMovement();
		}
	}

	bool bReloading = FirearmInstance->Phase == EFirearmPhase::Reloading;


	AExplodingBarrel* TargetBarrel = nullptr;
	bool bCanShoot = FirearmInstance->CanShoot();

	if (!bReloading)
	{
		const float MinTimeToFindNewBarrel = 5.0f;

		if (this->Target.TargetType == ETargetType::ExplodingBarrel)
		{
			auto* Barrel = Cast<AExplodingBarrel>(this->Target.Actor);
			bool bFindNewTarget = false;

			if (!Barrel || !Barrel->bCanExplode)
			{
				bFindNewTarget = true;
			}
			else
			{
				bFindNewTarget = false;

				this->SecondsSinceLastBarrelsCheck += DeltaTime;

				if (this->SecondsSinceLastBarrelsCheck >= MinTimeToFindNewBarrel)
				{
					this->SecondsSinceLastBarrelsCheck = 0.0f;

					bool bCanExplode = this->CanExplodeBarrel(Barrel);
					if (!bCanExplode)
					{
						bFindNewTarget = true;
					}
				}
			}

			if (bFindNewTarget)
			{
				this->Target.TargetType = ETargetType::None;
				this->Target.Actor = nullptr;
				this->FindNewEnemyTarget();
				this->SecondsSinceLastBarrelsCheck = 0.0f;
			}
			else
			{
				TargetBarrel = Barrel;
			}
		}
		else
		{
			auto* GameSessionObject = this->GetGameSession();

			this->SecondsSinceLastBarrelsCheck += DeltaTime;


			if (GameSessionObject && this->SecondsSinceLastBarrelsCheck >= MinTimeToFindNewBarrel)
			{
				this->SecondsSinceLastBarrelsCheck = 0.0f;
				int32 ExplodingBarrelsNum = GameSessionObject->AvailableExplodingBarrels.Num();

				if (ExplodingBarrelsNum)
				{
					for (int32 i = 0; i < ExplodingBarrelsNum; i++)
					{
						auto* Barrel = GameSessionObject->AvailableExplodingBarrels[i];

						if (!Barrel)
						{
							continue;
						}

						FVector BarrelLocation = Barrel->GetActorLocation();

						float Dist = FVector::Dist(BarrelLocation, ActorLocation);

						if (Dist <= FirearmRef->MaxDistance && Dist > Barrel->Radius)
						{
							bool bCanExplode = this->CanExplodeBarrel(Barrel);

							if (bCanExplode)
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
	}

	if (this->Target.TargetType == ETargetType::None || !this->Target.Actor)
	{
		return;
	}

	FVector AimTargetLocation = FVector(0.0f);

	if (this->Target.TargetType == ETargetType::Bot)
	{
		AimTargetLocation = this->Target.Bot->GetMesh()->GetSocketTransform(TEXT("spine_5")).GetLocation();
	}
	else
	{
		AimTargetLocation = this->Target.Actor->GetActorLocation();
	}

	if (!bReloading)
	{
		this->AimAt(AimTargetLocation);

		this->SmoothRotatingTick(DeltaTime);
	}


	AActor* HitActor = nullptr;


	if (bCanShoot)
	{
		FBulletHitResult BulletHitResult = this->LineTraceFromGun(FirearmInstance->GetFirearmRef(), false, false);

		HitActor = BulletHitResult.HitResult.GetActor();
	}

	bool bCanActuallyShoot = bCanShoot && (HitActor == this->Target.Actor || HitActor == this->Target.Bot) && this->AimingTime.IsEnded();

	/** If aiming at barrel */
	if (TargetBarrel && HitActor == this->Target.Actor)
	{
		float Dist = FVector::Dist(TargetBarrel->GetActorLocation(), this->GetActorLocation());

		if (Dist <= TargetBarrel->Radius)
		{
			bCanActuallyShoot = false;
		}
	}

	bool bObstacleBetweenBots = false;

	///** Check if there's an obstacle between bots */
	//if (bCanActuallyShoot)
	//{
	//	bObstacleBetweenBots = !this->TraceToTargetResult();
	//	bCanActuallyShoot = !bObstacleBetweenBots;
	//}

	if (HitActor != this->Target.Actor || bObstacleBetweenBots)
	{
		this->SecondsAimingWithoutHitting.Add(DeltaTime);
	}
	else
	{
		this->SecondsAimingWithoutHitting.Reset();
	}

	bool bCanFindNewPlace = this->CombatStyle == ECombatStyle::Attack;

	if (this->SecondsAimingWithoutHitting.IsEnded() && this->CombatStyle == ECombatStyle::Defense)
	{
		/** Allow defenders find new place if, for example, they aiming at wall  */
		bCanFindNewPlace = true;
		this->SecondsAimingWithoutHitting.Reset();
	}

	if (this->CombatStyle == ECombatStyle::Defense)
	{
		if (this->bMovingToRandomCombatLocation)
		{
			this->DefenderSecondsWithoutMoving.Reset();
		}
		else
		{
			this->DefenderSecondsWithoutMoving.Add(DeltaTime);

			/** Allow defenders to move after 20 seconds */
			if (this->DefenderSecondsWithoutMoving.IsEnded())
			{
				bCanFindNewPlace = true;
			}
		}
	}

	/** Moving around target */		

	float DistToRandomLocation = FVector::Dist(this->CombatRandomLocation, this->GetActorLocation());

	if (this->bMovingToRandomCombatLocation && (DistToRandomLocation < 150.0f || this->TimeSinceStartedMovingInCombat >= 7.0f))
	{
		this->bMovingToRandomCombatLocation = false;
		
		if (AIController)
		{
			AIController->StopMovement();
		}
	}

	if (this->bMovingToRandomCombatLocation)
	{
		bCanFindNewPlace = false;
	}

	/** If the bot can shoot and not reloading, find new location for it */
	if (bCanFindNewPlace && !bCanActuallyShoot && !bReloading)
	{
		float MaxDistance = FirearmRef->MaxDistance - 150.0f;
		if (MaxDistance < 150.0f)
		{
			MaxDistance = 150.0f;
		}
		FVector NewRandomLocation;
		bool bFoundRandomLocation = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this->GetWorld(), this->Target.Actor->GetActorLocation(), NewRandomLocation, MaxDistance);

		this->TimeSinceStartedMovingInCombat = 0.0f;

		if (bFoundRandomLocation)
		{
			if (CharacterMovementComponent)
			{
				CharacterMovementComponent->MaxWalkSpeed = FirearmRef->MaxWalkSpeed;
			}
			this->bMovingToRandomCombatLocation = true;
			this->bUseControllerRotationYaw = false;
			this->CombatRandomLocation = NewRandomLocation;
			this->DefenderSecondsWithoutMoving.Reset();

			if (AIController)
			{
				AIController->MoveToNewLocation(NewRandomLocation);
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

	if (this->bMovingToRandomCombatLocation)
	{
		this->TimeSinceStartedMovingInCombat += DeltaTime;
	}

	this->CombatAction = ECombatAction::Shooting;


	float BotSpeed = this->GetSpeed();

	if (bCanActuallyShoot && BotSpeed < 5.0f)
	{
		this->Shoot(true);
	}
}

void ABot::MeleeCombatTick(float DeltaTime, float TargetDist)
{
	SCOPE_CYCLE_COUNTER(STAT_StatsMeleeCombatTick);

	EWeaponType WeaponType = EWeaponType::Melee;

	auto* MeleeInstance = Cast<UMeleeWeaponInstance>(this->WeaponInstance);

	if (!MeleeInstance)
	{
		return;
	}

	auto* MeleeRef = MeleeInstance->GetMeleeRef();

	FVector AimTargetLocation = this->Target.Bot->GetMesh()->GetSocketTransform(TEXT("spine_5")).GetLocation();

	this->AimAt(AimTargetLocation);
	this->SmoothRotatingTick(DeltaTime);

	auto* AIController = this->GetAIController();

	const float MaxDist = MeleeRef->MaxDistance;

	if (this->bMovingToRandomCombatLocation)
	{
		this->TimeSinceStartedMovingInCombat += DeltaTime;

		if (TargetDist <= MaxDist)
		{
			if (AIController)
			{
				//AIController->StopMovement();
				//this->bMovingToRandomCombatLocation = false;
			}
		}
	}


	if ((!this->bMovingToRandomCombatLocation || this->TimeSinceStartedMovingInCombat >= 1.0f) && !MeleeInstance->bShouldPlayHitAnimation)
	{
		
		if (AIController)
		{
			FVector EndLocation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f).RotateVector(FVector(MaxDist, 0.0f, 0.0f));
			EndLocation += this->Target.Actor->GetActorLocation();
			AIController->MoveToNewLocation(EndLocation);
			this->bMovingToRandomCombatLocation = true;
			this->TimeSinceStartedMovingInCombat = 0.0f;
			this->bUseControllerRotationYaw = false;
		}

		auto* CharacterMovementComponent = this->GetCharacterMovementComponent();

		if (CharacterMovementComponent)
		{
			CharacterMovementComponent->MaxWalkSpeed = MeleeInstance->WeaponRef->MaxWalkSpeed;
		}
	}


	this->CombatAction = ECombatAction::Shooting;

	//UE_LOG(LogTemp, Display, TEXT("[MeleeCombatTick] Can melee hit: %d. bShouldPlayHitAnimation: %d"),  MeleeInstance->CanHit(), MeleeInstance->bShouldPlayHitAnimation);

	if (!MeleeInstance->CanHit())
	{
		return;
	}

	FVector StartTraceLocation = this->GetMesh()->GetSocketTransform(TEXT("spine_5"), ERelativeTransformSpace::RTS_Actor).GetLocation();

	FVector EndTraceLocation = StartTraceLocation + this->GetGunRotation().RotateVector(FVector(MaxDist, 0.0f, 0.0f));

	StartTraceLocation += this->GetActorLocation();
	EndTraceLocation += this->GetActorLocation();

	FHitResult HitResult;

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(HitResult, StartTraceLocation, EndTraceLocation, ECollisionChannel::ECC_GameTraceChannel3, TraceParams);

	//DrawDebugLine(GetWorld(), StartTraceLocation, EndTraceLocation, FColor(255, 0, 0), false, -1.0f, 0, 1.0f);

	float BotSpeed = this->GetSpeed();

	if (HitResult.bBlockingHit && this->IsEnemy(Cast<ABot>(HitResult.GetActor())))
	{
		if (BotSpeed < 5.0f)
		{
			this->MeleeHit();
		}
		else
		{
			this->bMovingToRandomCombatLocation = false;
			if (AIController)
			{
				AIController->StopMovement();
			}
		}
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

			FRotator GunRotation = this->GetGunRotation();

			FVector OutBulletLocation = this->GetFirearmOutBulletWorldPosition(GunRotation, false);

			FBulletHitResult BulletHitResult = this->LineTraceFromGun(FirearmRef, bBulletOffset, this->bTestAiming);

			FTransform ProjectileTransform;
			ProjectileTransform.SetLocation(OutBulletLocation);

			FActorSpawnParameters ProjectileSpawnParams;
			ProjectileSpawnParams.Name = AFirearmProjectile::GenerateName();

			auto ProjectileClass = FirearmRef->FirearmProjectileSubClass;
		
			auto* FirearmProjectile = World->SpawnActor<AFirearmProjectile>(ProjectileClass, ProjectileTransform, ProjectileSpawnParams);

			FVector EndLocation = BulletHitResult.HitResult.bBlockingHit ? BulletHitResult.HitResult.ImpactPoint : BulletHitResult.HitResult.TraceEnd;

			FirearmProjectile->Init(OutBulletLocation, EndLocation, BulletHitResult, FirearmInstance, this->GetActorForwardVector());
			FirearmProjectile->BotCauser = this;
			FirearmProjectile->SetColor(FirearmInstance->TraceColor);

			APlayerPawn* PlayerPawn = APlayerPawn::Get();

			float DistanceFromCamera = PlayerPawn ? PlayerPawn->GetDistanceFromCamera(OutBulletLocation) :  0.0f;

			FirearmProjectile->bSimplified = DistanceFromCamera > 5000.0f;

			if (DistanceFromCamera <= 7000.0f)
			{
				if (FirearmRef->ShootSound)
				{
					UGameplayStatics::PlaySoundAtLocation(World, FirearmRef->ShootSound, OutBulletLocation, FMath::RandRange(0.7f, 0.85f));
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
}

void ABot::MeleeHit()
{
	if (this->WeaponInstance)
	{
		auto* MeleeInstance = Cast<UMeleeWeaponInstance>(this->WeaponInstance);

		if (!MeleeInstance)
		{
			return;
		}

		auto* MeleeRef = MeleeInstance->GetMeleeRef();

		if (!MeleeRef)
		{
			return;
		}

		if (!MeleeInstance->CanHit())
		{
			return;
		}

		this->SetMeleeCollisionEnabled(false);

		MeleeInstance->OnHit();

		if (MeleeRef->HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), MeleeRef->HitSound, this->WeaponMesh->GetComponentLocation(), FMath::RandRange(0.7f, 0.85f));
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

	FRotator TempPitchRotation = UKismetMathLibrary::FindLookAtRotation(FVector(0.0f, 0.0f, SocketLocation.Z + this->GunAnimationRotationPoint.Z), AimLocationRotated);

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


void ABot::MeleeCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
 	if (OtherActor == this)
	{
		return;
	}

	auto* MeleeInstance = Cast<UMeleeWeaponInstance>(this->WeaponInstance);

	if (!MeleeInstance)
	{
		return;
	}


	if (!MeleeInstance->bShouldPlayHitAnimation)
	{
		return;
	}

	auto* MeleeRef = MeleeInstance->GetMeleeRef();

	auto* BotHit = Cast<ABot>(OtherActor);

	if (!BotHit)
	{
		return;
	}

	//if (BotHit->GetMesh() != OtherComp || BotHit->HeadMesh != OtherComp)
	//{
	//	return;
	//}

	if (MeleeInstance->BotsHit.Contains(BotHit) || MeleeInstance->BotsHit.Num())
	{
		return;
	}

	MeleeInstance->BotsHit.Add(BotHit);

	int32 RandNumber = FMath::RandRange(0, 9);
	bool bCritical = false;

	if (RandNumber == 0)
	{
		bCritical = true;
	}


	FTransform CollisionTransform = OverlappedComponent->GetComponentTransform();
	FVector CollisionSize = (CollisionTransform.GetScale3D() * 32.0f);

	if (this->MeleeCollision)
	{
		CollisionSize = this->MeleeCollision->GetScaledBoxExtent();
	}

	TArray<AActor*> TraceActorsToIgnore;
	TraceActorsToIgnore.Add(this);
	FHitResult HitResult;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesCollision;

	ObjectTypesCollision.Add(UEngineTypes::ConvertToObjectType(this->GetMesh()->GetCollisionObjectType()));

	UKismetSystemLibrary::BoxTraceSingleForObjects(GetWorld(), CollisionTransform.GetLocation(), OtherActor->GetActorLocation(), CollisionSize, FRotator(CollisionTransform.GetRotation()), ObjectTypesCollision, true, TraceActorsToIgnore, EDrawDebugTrace::Type::None, HitResult, true, FLinearColor::Red, FLinearColor::Green, 3.0f);

	APlayerPawn* PlayerPawn = APlayerPawn::Get();

	float DistanceFromCamera = PlayerPawn ? PlayerPawn->GetDistanceFromCamera(this->GetActorLocation()) : 0.0f;

	if (DistanceFromCamera <= 7000.0f && BotHit->BloodParticle && HitResult.bBlockingHit && HitResult.GetActor() == BotHit)
	{
		FTransform BloodParticleTransform;
		BloodParticleTransform.SetLocation(HitResult.ImpactPoint);
		FRotator TestRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), HitResult.ImpactPoint);
		TestRot.Pitch += 90.0f;
		BloodParticleTransform.SetRotation(FQuat(TestRot));
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BotHit->BloodParticle, BloodParticleTransform, true);
	}

	BotHit->ApplyDamage(MeleeInstance->GetDamage(), this, EWeaponType::Melee, FVector(), FVector(), NAME_None, bCritical);

	if (MeleeInstance->BotsHit.Num() == 1 && MeleeRef->DamageSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), MeleeRef->DamageSound, this->WeaponMesh->GetComponentLocation(), FMath::RandRange(0.7f, 0.85f));
	}

	//MeleeInstance->bShouldPlayHitAnimation = false;
	//MeleeInstance->HitAnimationTime = 0.0f;
}

bool ABot::TraceToTargetResult(bool bIgnoreBots)
{
	if (!this->Target.Actor)
	{
		return false;
	}


	//FVector StartLocation = this->GetMesh()->GetSocketLocation(TEXT("spine_5"));
	FVector StartLocation = this->GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);
	
	FVector EndLocation = this->Target.Actor->GetActorLocation();

	if (this->Target.Bot && this->Target.TargetType == ETargetType::Bot)
	{
		EndLocation = this->Target.Bot->GetMesh()->GetSocketLocation(TEXT("spine_5"));
	}

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	FHitResult HitResult;
	
	ECollisionChannel Channel = bIgnoreBots ? ECollisionChannel::ECC_GameTraceChannel5 : ECollisionChannel::ECC_GameTraceChannel3;

	this->GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, Channel, TraceParams);

	auto* HitActor = HitResult.GetActor();

	if (bIgnoreBots && (!HitResult.bBlockingHit || HitActor == this->Target.Actor))
	{
		return true;
	}
	else if (!bIgnoreBots && HitResult.bBlockingHit && HitActor == this->Target.Actor)
	{
		return true;
	}

	//FVector DebugEndLocation = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd;

	//DrawDebugLine(GetWorld(), StartLocation, DebugEndLocation, FColor::Red, false, 0.5f);
	
	return false;
}

void ABot::CreateFloorBloodDecal()
{
	this->bCheckedBloodDecalCreation = true;

	if (!this->FloorBloodDecalSubclass)
	{
		return;
	}

	UWorld* WorldObject = this->GetWorld();
	FVector PivotLocation = this->GetMesh()->GetSocketLocation(TEXT("spine_3"));
	FVector DecalLocation = PivotLocation + FVector(0.0f, 0.0f, -50.0f);

	
	FCollisionObjectQueryParams CollisionParams;

	CollisionParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);

	FHitResult HitResult;
	WorldObject->LineTraceSingleByObjectType(HitResult, PivotLocation, DecalLocation, CollisionParams);

	if (!HitResult.bBlockingHit)
	{
		return;
	}

	FTransform DecalSpawnTransform;
	DecalSpawnTransform.SetLocation(HitResult.ImpactPoint);
	DecalSpawnTransform.SetRotation(FQuat(FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f)));

	this->FloorBloodDecalActor = WorldObject->SpawnActor<ABloodDecal>(this->FloorBloodDecalSubclass, DecalSpawnTransform);
	this->FloorBloodDecalActor->BotOwner = this;
}

ABot* ABot::CreateBot(UWorld* World, FString NameToSet, int32 IDToSet, TSubclassOf<ABot> Subclass, UChattersGameSession* GameSessionObject)
{
	GameSessionObject = UChattersGameSession::Get();
	if (!GameSessionObject)
	{
		return nullptr;
	}

	FTransform SpawnPoint = GameSessionObject->GetAvailableSpawnPoint();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*FString::Printf(TEXT("Bot_%d"), IDToSet));
	SpawnParams.bNoFail = true;
	ABot* Bot = World->SpawnActor<ABot>(Subclass, SpawnPoint.GetLocation(), FRotator(SpawnPoint.GetRotation()), SpawnParams);


	if (Bot)
	{
		Bot->Team = GameSessionObject->GameModeType == ESessionGameMode::Teams ? (IDToSet % 2 ? EBotTeam::Blue : EBotTeam::Red) : EBotTeam::White;

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
	CollisionParams.bTraceComplex = false;

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
		auto* EquipmentList = GameSessionObject->EquipmentListLevel;

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

			UWeaponItem* RandomWeapon = EquipmentList->GetRandomWeapon(GameSessionObject->AvailableWeapons);

			if (RandomWeapon)
			{
				if (this->WeaponMesh)
				{
					this->WeaponMesh->SetStaticMesh(RandomWeapon->StaticMesh);
					this->WeaponMesh->SetRelativeTransform(RandomWeapon->GetTransform());

					UClass* WeaponInstanceClass = UWeaponInstance::StaticClass();

					EWeaponType WeaponType = RandomWeapon->Type;

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
						this->WeaponInstance->WeaponRef = RandomWeapon;
						this->WeaponInstance->BotOwner = this;
						this->WeaponInstance->Init();

						if (WeaponType == EWeaponType::Firearm)
						{
							UFirearmWeaponItem* FirearmRef = Cast<UFirearmWeaponItem>(RandomWeapon);
							if (FirearmRef)
							{
								this->GunSocketRelativeLocation = FirearmRef->SocketRelativeLocation - this->GunAnimationRotationPoint;
							}
						}
						else if (WeaponType == EWeaponType::Melee)
						{
							auto* MeleeRef = Cast<UMeleeWeaponItem>(RandomWeapon);
							if (MeleeRef)
							{
								this->GunSocketRelativeLocation = FVector(0.0f, 0.0f, 50.0f) - this->GunAnimationRotationPoint;

								if (this->MeleeCollision)
								{
									this->MeleeCollision->SetRelativeTransform(MeleeRef->CollisionTransform);
								}
							}
						}
					}
				}
			}

			if (RandomEquipment.Costume && this->GetMesh() && RandomEquipment.Costume->SkeletalMesh)
			{
				this->GetMesh()->SetSkeletalMeshWithoutResettingAnimation(RandomEquipment.Costume->SkeletalMesh);

				TArray<UMaterialInterface*> Materials = RandomEquipment.Costume->GetRandomMaterials();

				for (int32 i = 0; i < Materials.Num(); i++)
				{
					this->GetMesh()->SetMaterial(i, Materials[i]);
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
	this->UpdateEquipmentTeamColors();
}

void ABot::MoveToRandomLocation()
{
	auto* AIController = this->GetAIController();

	if (AIController)
	{
		float XPos = FMath::RandRange(-7400, 7400);
		float YPos = FMath::RandRange(-7400, 7400);

		this->RandomLocationTarget = FVector(XPos, YPos, 97);

		AIController->MoveToNewLocation(this->RandomLocationTarget);
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

	if (BoneHit == TEXT("head_"))
	{
		bCritical = true;
	}

	if (bCritical)
	{
		Damage = FMath::RoundToInt(Damage * 1.5f);
	}

	auto OldHP = this->HealthPoints;

	this->HealthPoints -= Damage;

	if (this->HealthPoints <= 0)
	{
		this->HealthPoints = 0;
		this->OnDead(ByBot, WeaponType, ImpulseVector, ImpulseLocation, BoneHit);
	}
	else
	{
		/** If damage by enemy */
		if (this->IsEnemy(ByBot) && ByBot->bAlive)
		{
			/** If bot is not target already */
			if (!this->Target.Bot || this->Target.Bot != ByBot)
			{
				/** If the current target is not aiming at us */
				if (this->Target.Bot->Target.Bot != this)
				{
					/** Set the damager as new target */
					this->SetNewEnemyTarget(ByBot);
				}
			}
		}
	}

	//UE_LOG(LogTemp, Display, TEXT("[ABot] Applying %d damage to bot. Old hp: %d. New HP: %d"), Damage, OldHP, this->HealthPoints);

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

EWeaponType ABot::GetWeaponType()
{
	auto* WeaponRef = this->GetWeaponRef();

	if (!WeaponRef)
	{
		return EWeaponType::None;
	}

	return WeaponRef->Type;
}

bool ABot::IsEnemy(ABot* BotToCheck)
{
	if (!BotToCheck)
	{
		return false;
	}

	if (BotToCheck == this)
	{
		return false;
	}

	if (this->Team == EBotTeam::White)
	{
		return true;
	}
	else
	{
		return this->Team != BotToCheck->Team;
	}
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

	if (Killer && this->IsEnemy(Killer))
	{
		Killer->Kills++;
		auto* NameWidgetRef = Killer->GetNameWidget();
		if (NameWidgetRef)
		{
			NameWidgetRef->UpdateKillsNumber(Killer->Kills);
		}
	}

	if (Killer && Killer != this && this->bPlayerAttached)
	{
		APlayerPawn* PlayerPawn = APlayerPawn::Get();

		if (PlayerPawn)
		{
			PlayerPawn->AttachToBot(Killer);
		}
	}

	auto* GameSessionObject = this->GetGameSession();

	if (GameSessionObject)
	{
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

			FKillFeedIcon KillFeedIcon;

			if (WeaponType == EWeaponType::Explosion)
			{
				KillFeedIcon.IconType = EKillFeedIconType::Explosion;
			}
			else
			{
				if (Killer->WeaponInstance && Killer->WeaponInstance->WeaponRef)
				{
					KillFeedIcon = Killer->WeaponInstance->WeaponRef->KillFeedIcon;
				}
			}

			SessionWidget->OnKill(KillerName, this->DisplayName, Killer->GetTeamColor(), this->GetTeamColor(), KillFeedIcon);
		}

		GameSessionObject->OnBotDied(this->ID);
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

float ABot::GetGunPitchRotation()
{
	return this->GetGunRotation().Pitch;
}

bool ABot::ShouldPlayWeaponHitAnimation()
{
	if (!this->WeaponInstance)
	{
		return false;
	}

	return this->WeaponInstance->bShouldPlayHitAnimation;
}

bool ABot::ShouldPlayWeaponReloadingAnimation()
{
	if (!this->WeaponInstance)
	{
		return false;
	}

	return this->WeaponInstance->bShouldPlayReloadingAnimation;
}

void ABot::ResetOnNewRound()
{
	this->HealthPoints = this->MaxHealthPoints;

	this->AimingAngle = 50.0f;
	
	this->CombatAction = ECombatAction::IDLE;
	this->bShouldApplyGunAnimation = false;
	this->Target.Actor = nullptr;
	this->Target.Bot = nullptr;
	this->Target.TargetType = ETargetType::None;

	auto* AIController = this->GetAIController();

	if (AIController)
	{
		AIController->StopMovement();
	}

	this->bMovingToRandomCombatLocation = false;
	this->bMovingToRandomLocation = false;
	this->bAlive = true;
	this->Kills = 0;

	auto* NameWidgetRef = this->GetNameWidget();

	if (NameWidgetRef)
	{
		NameWidgetRef->UpdateKillsNumber(this->Kills);
		NameWidgetRef->UpdateHealth(this->GetHeathValue());
	}

	this->UpdateNameColor();
	
	this->SmoothRotation.bActive = false;

	if (this->WeaponInstance && this->WeaponInstance->WeaponRef)
	{
		this->WeaponInstance->TimeoutValue = 0.0f;
		this->WeaponInstance->HitAnimationTime = 0.0f;
		this->WeaponInstance->bShouldPlayHitAnimation = false;
		this->WeaponInstance->bShouldPlayReloadingAnimation = false;

		if (this->WeaponInstance->WeaponRef->Type == EWeaponType::Firearm)
		{
			auto* FirearmInstance = Cast<UFirearmWeaponInstance>(this->WeaponInstance);
			if (FirearmInstance)
			{
				auto* FireamRef = FirearmInstance->GetFirearmRef();
				if (FireamRef)
				{
					FirearmInstance->NumberOfBullets = FireamRef->MaxNumberOfBullets;
					FirearmInstance->Phase = EFirearmPhase::IDLE;
				}
			}
		}
	}
}

void ABot::OnGameSessionStarted(ESessionMode SessionMode)
{
	this->bReady = true;

	if (SessionMode == ESessionMode::TestAiming)
	{
		this->TestAimAt();
	}
	else
	{
		this->FindNewEnemyTarget();
	}

	//this->MoveToRandomLocation();
}

void ABot::TryDetachHat()
{
	if (this->bHatAttached && this->HatMesh && this->GetMesh())
	{
		FRotator HeadRotation = this->GetMesh()->GetSocketRotation(FName(TEXT("head_")));

		if (HeadRotation.Roll > 150.0f || HeadRotation.Roll < 75.0f || HeadRotation.Pitch > 50.0f || HeadRotation.Pitch < -50.0f)
		{
			this->bHatAttached = false;
			this->HatMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			this->HatMesh->SetCollisionProfileName(FName(TEXT("OufitPhysics")), true);
			this->HatMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
			this->HatMesh->SetSimulatePhysics(true);

			const FVector HeadLocation = this->GetMesh()->GetSocketLocation(TEXT("head_"));
			FVector HatLocation = HeadLocation - this->HatMesh->GetComponentLocation();

			HatLocation.Normalize();
			
			const FVector HatPhysicsImpulse = HatLocation * -1000.0f;

			//this->HatMesh->AddImpulseAtLocation(HatPhysicsImpulse, this->HatMesh->GetComponentLocation());

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

		const float YawSpeed = 270.0f;
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
			AIController->MoveToNewLocation(this->RandomPointToMoveWhileAiming);
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

FLinearColor ABot::GetTeamColor()
{
	switch (this->Team)
	{
	case EBotTeam::Blue:
		return FLinearColor(0.121f, 0.635f, 0.796f);
	case EBotTeam::Red:
		return FLinearColor(0.533f, 0.054f, 0.054f);
	default:
		return FLinearColor(1.0f, 1.0f, 1.0f);
	}
}

void ABot::UpdateNameColor()
{
	auto* NameWidgetRef = this->GetNameWidget();

	if (NameWidgetRef)
	{
		NameWidgetRef->NicknameColor = this->GetTeamColor();
	}
}

void ABot::StopMovementAfterRound()
{
	this->StopMovement();
	this->CombatAction = ECombatAction::IDLE;
	
	this->Target.Actor = nullptr;
	this->Target.Bot = nullptr;
	this->Target.TargetType = ETargetType::None;
	this->bReady = false;
}

void ABot::StopMovement()
{
	auto* AiController = this->GetAIController();

	if (AiController)
	{
		AiController->StopMovement();
	}

	this->bMovingToRandomCombatLocation = false;
	this->bMovingToRandomLocation = false;
}

bool ABot::CanExplodeBarrel(AExplodingBarrel* Barrel)
{
	auto BotsInRadius = Barrel->GetBotsInRadius();

	if (!BotsInRadius.Num())
	{
		return false;
	}


	if (this->Team == EBotTeam::White)
	{
		return true;
	}


	for (auto Bot : BotsInRadius)
	{
		/** Do not explode barrel if there's an ally nearby */
		if (Bot->Team == this->Team)
		{
			return false;
		}
	}


	return true;
}

void ABot::SetMeleeCollisionEnabled(bool bEnabled)
{
	if (this->MeleeCollision)
	{
		this->MeleeCollision->SetCollisionEnabled(bEnabled ? ECollisionEnabled::Type::QueryOnly : ECollisionEnabled::Type::NoCollision);
	}
}

void ABot::Clear()
{
	this->RemoveBloodDecal();
}

void ABot::RemoveBloodDecal()
{
	if (this->FloorBloodDecalActor)
	{
		this->FloorBloodDecalActor->Destroy();
		this->FloorBloodDecalActor = nullptr;
	}
}

FEyesRotation ABot::GetEyesRotation()
{
	FEyesRotation DefaultRotation;

	if (!this->bAlive)
	{
		return DefaultRotation;
	}

	if (this->CombatAction == ECombatAction::Shooting)
	{
		return DefaultRotation;
	}

	APlayerPawn* PlayerPawn = APlayerPawn::Get();

	if (!PlayerPawn)
	{
		return DefaultRotation;
	}
	
	float DistanceFromCamera = PlayerPawn->GetDistanceFromCamera(this->GetActorLocation());

	if (DistanceFromCamera > 500.0f)
	{
		return DefaultRotation;
	}

	FVector LeftEyeLocation = this->HeadMesh->GetSocketLocation(TEXT("eye_L"));
	FVector RightEyeLocation = this->HeadMesh->GetSocketLocation(TEXT("eye_R"));

	const FVector CameraLocation = PlayerPawn->GetCameraLocation();

	auto GetEyeRotation = [this](FVector EyeLocation, FVector CameraLocation)
	{
		FVector RelativeCameraLocation = EyeLocation - CameraLocation;

		RelativeCameraLocation = FRotator(0.0f, this->GetActorRotation().Yaw * -1.0f, 0.0f).RotateVector(RelativeCameraLocation);

		FRotator EyeRotation = UKismetMathLibrary::FindLookAtRotation(FVector(0.0), RelativeCameraLocation);

		EyeRotation.Yaw += 180.0f;
		EyeRotation.Yaw = FMath::Fmod(EyeRotation.Yaw + 180.0f, 360.0f) - 180.0f;

		EyeRotation.Roll = FMath::Clamp(EyeRotation.Pitch, -50.0f, 50.0f);
		EyeRotation.Yaw = FMath::Clamp(EyeRotation.Yaw, -50.0f, 50.0f);
		EyeRotation.Pitch = 0.0f;

		return EyeRotation;
	};

	FEyesRotation EyesRotation;

	EyesRotation.LeftEye = GetEyeRotation(LeftEyeLocation, CameraLocation);
	EyesRotation.RightEye = GetEyeRotation(RightEyeLocation, CameraLocation);

	return EyesRotation;
}