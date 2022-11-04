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
#include "NiagaraFunctionLibrary.h"
#include "../Core/ChattersGameSession.h"

DECLARE_CYCLE_STAT(TEXT("Bot Tick Time"), STAT_StatsBotTick, STATGROUP_BOTS);
DECLARE_CYCLE_STAT(TEXT("Bot Tick Alive Time"), STAT_StatsBotAliveTick, STATGROUP_BOTS);
DECLARE_CYCLE_STAT(TEXT("Bot Tick Dead Time"), STAT_StatsBotDeadTick, STATGROUP_BOTS);
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

	this->MeleeHitbox = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeHitbox"));
	this->MeleeHitbox->SetupAttachment(GetMesh());

}

ABot::~ABot()
{
	FVector Position = this->GetActorLocation();

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

	this->UpdateHeadAnimationType(-1.0f, true);
}

// Called every frame
void ABot::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_StatsBotTick);

	Super::Tick(DeltaTime);


	if (this->bAlive)
	{
		{
			SCOPE_CYCLE_COUNTER(STAT_StatsBotAliveTick);

			if (HitBoneRotationTimer.Current != 0)
			{
				HitBoneRotationTimer.Add(-DeltaTime);
				if (HitBoneRotationTimer.Current <= 0.0f)
				{
					HitBoneRotationTimer.Current = 0.0f;
				}
			}

			if (IsInRobot())
			{
				SetActorRelativeRotation(RobotInstance->CharacterTransform.GetRotation().Rotator());
				SetActorRelativeLocation(RobotInstance->CharacterTransform.GetLocation());
			}

			if (GetGameSession()->GameModeType == ESessionGameMode::Zombie && CombatAction == ECombatAction::Moving)
			{
				if (!IsZombie())
				{
					FindNewTargetAliveTimer.Max = 25.0f;
				}

				FindNewTargetAliveTimer.Add(DeltaTime);

				if (FindNewTargetAliveTimer.IsEnded())
				{
					FindNewTargetAliveTimer.Reset();

					if (IsZombie() || FMath::RandRange(0, 1) == 0)
					{
						FindNewEnemyTarget();
					}
				}
			}

			///** Enable collision for revived players after 3 seconds */
			//if (this->bReviveCollisionTimerActive)
			//{
			//	ReviveCollisionTimer.Add(DeltaTime);

			//	if (ReviveCollisionTimer.IsEnded())
			//	{
			//		this->bReviveCollisionTimerActive = false;
			//		ReviveCollisionTimer.Reset();

			//		this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			//		this->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			//	}
			//}

			if (this->WeaponInstance)
			{
				this->WeaponInstance->Tick(DeltaTime);

				//if (this->bPlayerAttached)
				//{
				//	UE_LOG(LogTemp, Display, TEXT("[ABot] %s bShouldPlayHitAnimation: %d. No hit time: %f s"), *this->DisplayName, this->ShouldPlayWeaponHitAnimation(), this->WeaponInstance->SecondsWithoutHit.Current);
				//}

				if (this->bTestAiming)
				{
					this->TestAimingTick(DeltaTime);
				}
				else
				{
					this->CombatTick(DeltaTime);
				}

			}

			auto* NameWidgetObject = this->GetNameWidget();

			if (NameWidgetObject)
			{
				NameWidgetObject->Tick(DeltaTime);
			}

			/** If not moving */
			if (this->bReady && !this->bShouldApplyGunAnimation && this->GetActorLocation() == this->LastTickLocation && IsInRobot() == false)
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

					this->StuckCount++;

					if (this->StuckCount >= 10)
					{
						this->StuckCount = 0;
						this->RespawnAtRandomPlace();
					}

					this->FindNewEnemyTarget();
				}
			}
			else
			{
				this->SecondsWithoutMoving.Reset();
				this->StuckCount = 0;
			}
		
			if (this->bReady)
			{
				bool bFalling = this->GetMovementComponent()->IsFalling();

				/** When falling starts */
				if (bFalling && !this->bFallingLastTick)
				{
					this->FallingStartZLocation = this->GetActorLocation().Z;
				}
				/** When falling ends */
				else if (!bFalling && this->bFallingLastTick)
				{
					float FallDistance = FMath::Abs(this->FallingStartZLocation - this->GetActorLocation().Z);

					const float MinFallDistanceToDamage = 250.0f;
					const float MaxFallDistanceToDamage = 600.0f;

					const float MinFallDamage = 5.0f;
					const float MaxFallDamage = 100.0f;

					if (FallDistance >= MinFallDistanceToDamage)
					{
						FallDistance = FMath::Clamp(FallDistance, MinFallDistanceToDamage, MaxFallDistanceToDamage);
					
						float FallDistanceScale = UKismetMathLibrary::NormalizeToRange(FallDistance, MinFallDistanceToDamage, MaxFallDistanceToDamage);
					
						FallDistanceScale = FMath::Clamp(FallDistanceScale, 0.0f, 1.0f);

						int32 Damage = FMath::FloorToInt(FMath::Lerp(MinFallDamage, MaxFallDamage, FallDistanceScale));

						this->ApplyDamage(Damage, this, EWeaponType::Fall);

						if (this->FallDamageSound)
						{
							UGameplayStatics::PlaySoundAtLocation(GetWorld(), this->FallDamageSound, this->GetActorLocation(), FMath::RandRange(0.7f, 0.85f));
						}
					}

				}

				this->bFallingLastTick = bFalling;

				/** Kill the character if it fell out of world  */
				if (GetActorLocation().Z < -10000.0f)
				{
					ApplyDamage(1000, this, EWeaponType::Fall);
				}
			}

			this->LastTickLocation = this->GetActorLocation();
		}
	}
	else
	{
		{
			if (IsZombie())
			{
				ZombieDestroyTimer.Add(DeltaTime);

				if (ZombieDestroyTimer.IsEnded())
				{
					Destroy();
					return;
				}
			}
			SCOPE_CYCLE_COUNTER(STAT_StatsBotDeadTick);
			if (this->SecondsAfterDeath < 100.0f)
			{
				this->SecondsAfterDeath += DeltaTime;
				if (this->SecondsAfterDeath > 100.0f)
				{
					this->SecondsAfterDeath = 100.0f;
				}
			}

			if (this->SecondsAfterDeath >= 2.0f && !this->bCheckedBloodDecalCreation && !IsInRobot())
			{
				this->CreateFloorBloodDecal();
			}

			if (!this->bShouldReviveBot && !this->bRigidBodiesSleep && this->SecondsAfterDeath >= 4.0f)
			{
				this->bRigidBodiesSleep = true;
				//this->GetMesh()->SetSimulatePhysics(false);
				//this->GetMesh()->SetAllBodiesSimulatePhysics(false);

				if (!IsInRobot())
				{
					this->GetMesh()->PutAllRigidBodiesToSleep();
					this->HeadMesh->PutAllRigidBodiesToSleep();
					this->GetMesh()->bPauseAnims = true;
					this->HeadMesh->bPauseAnims = true;
				}
			}

			if (!bHatAttached && bCanHatBeDetached)
			{
				if (!HatDetachedTimer.IsEnded())
				{
					HatDetachedTimer.Add(DeltaTime);

					if (HatDetachedTimer.IsEnded())
					{
						this->HatMesh->SetSimulatePhysics(false);
						this->HatMesh->PutAllRigidBodiesToSleep();
					}
				}
			}

			if (!WeaponDetachTimer.IsEnded())
			{
				WeaponDetachTimer.Add(DeltaTime);
				
				if (WeaponDetachTimer.IsEnded())
				{
					this->WeaponMesh->SetSimulatePhysics(false);
					this->WeaponMesh->PutAllRigidBodiesToSleep();
				}
			}

			if (!DropEquipmentAfterDeathTimer.IsEnded() && (bInstantFallAfterDeath || RagdollAfterDeathTimer.IsEnded()))
			{
				DropEquipmentAfterDeathTimer.Add(DeltaTime);

				if (DropEquipmentAfterDeathTimer.IsEnded())
				{
					if (bHatAttached && bCanHatBeDetached)
					{
						TryDetachHat();
					}

					if (WeaponMesh)
					{
						DeatachWeapon();
					}
				}
			}

			if (this->bShouldReviveBot)
			{
				this->BotReviveDeathmatchTimer.Add(DeltaTime);

				if (this->BotReviveDeathmatchTimer.IsEnded())
				{
					this->BotReviveDeathmatchTimer.Reset();
					this->ReviveBotDeatchmatch();
				}
			}

			if (!RemoveProjectileMeshesTimeout.IsEnded())
			{
				RemoveProjectileMeshesTimeout.Add(DeltaTime);
				if (RemoveProjectileMeshesTimeout.IsEnded())
				{
					RemoveAllAttachedProjectileMeshes();
				}
			}

			if (!RagdollAfterDeathTimer.IsEnded())
			{
				RagdollAfterDeathTimer.Add(DeltaTime);

				if (RagdollAfterDeathTimer.IsEnded())
				{
					if (!IsInRobot())
					{
						GetMesh()->SetSimulatePhysics(true);
						GetMesh()->SetCollisionProfileName(FName(TEXT("DeadBody")), true);
						GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

						GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
					}
				}
			}
		}
	}
}

// Called to bind functionality to input
void ABot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABot::Destroyed()
{
	Super::Destroyed();

	//FVector Position = this->GetActorLocation();

	//UE_LOG(LogTemp, Display, TEXT("[ABot] Bot %s destroyed."), *this->DisplayName);
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

		if (GetGameSession()->GameModeType == ESessionGameMode::Zombie)
		{
			if (IsZombie() == false && FMath::RandRange(0, 6) != 0)
			{
				AliveBots = GameSessionObject->Zombies;
			}
		}

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
		SetWeaponProjectileMeshVisibility(false);
		this->bShouldApplyGunAnimation = false;
		return;
	}

	this->Target.TargetType = ETargetType::Bot;
	this->Target.Actor = TargetBot;
	this->Target.Bot = TargetBot;
	
	if (TargetBot->IsInRobot())
	{
		Target.Actor = TargetBot->GetRobotInstance();
	}

	this->CombatAction = ECombatAction::IDLE;
	SetWeaponProjectileMeshVisibility(false);

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
		else if (WeaponRef->Type == EWeaponType::Melee)
		{
			auto* MeleeInstance = Cast<UMeleeWeaponInstance>(this->WeaponInstance);
			if (!MeleeInstance || !MeleeInstance->GetMeleeRef())
			{
				return;
			}

			auto* MeleeRef = MeleeInstance->GetMeleeRef();

			TArray<UMaterialInterface*> RandomMaterials;

			if (this->Team != EBotTeam::White && MeleeRef->TeamMaterials.Num() > 1)
			{
				RandomMaterials = this->Team == EBotTeam::Blue ? MeleeRef->TeamMaterials[0].Slots : MeleeRef->TeamMaterials[1].Slots;
			}
			else
			{
				RandomMaterials = MeleeRef->GetRandomMaterials();
			}

			if (RandomMaterials.Num() && this->WeaponMesh)
			{
				for (int32 i = 0; i < RandomMaterials.Num(); i++)
				{
					this->WeaponMesh->SetMaterial(i, RandomMaterials[i]);
				}
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
	UpdateMovingTargetTimeout = 1.0f;
	this->CombatAction = ECombatAction::Moving;
	SetWeaponProjectileMeshVisibility(false);
	FVector TargetLocation = this->Target.Actor->GetActorLocation();

	if (IsInRobot())
	{
		auto* Robot = Cast<ARobot>(this->Target.Actor);

		if (Robot)
		{
			TargetLocation = Robot->GetMesh()->GetComponentLocation();
		}
	}

	if (this->Target.TargetType == ETargetType::ExplodingBarrel)
	{
		UE_LOG(LogTemp, Display, TEXT("[ABot] Moving bot %s to exploding barrel. "), *this->DisplayName);
	}

	if (GetGameSession()->GameModeType == ESessionGameMode::Zombie && Team == EBotTeam::White && FMath::RandRange(0, 5) != 0)
	{
		UNavigationSystemV1::K2_GetRandomReachablePointInRadius(GetWorld(), GetActorLocation(), TargetLocation, 5000.0f);
		UpdateMovingTargetTimeout = 4.0f;
	}

	MoveToNewLocation(TargetLocation);

	this->AimingAngle = 50.0f;
}

void ABot::CombatTick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_StatsCombatTick);

	if (this->WeaponInstance->WeaponRef)
	{
		EWeaponType WeaponType = this->WeaponInstance->WeaponRef->Type;

		if (this->Target.Actor && this->Target.TargetType != ETargetType::None)
		{
			if (this->Target.TargetType == ETargetType::Bot && this->Target.Bot && !this->Target.Bot->bAlive)
			{
				bMovingToRandomCombatLocation = false;

				FindNewTargetTimer.Add(DeltaTime);

				if (FindNewTargetTimer.IsEnded())
				{
					FindNewTargetTimer.Reset();
					FindNewEnemyTarget();
				}
			}
			else
			{
				float TargetDist = FVector::Dist(this->GetActorLocation(), this->Target.Actor->GetActorLocation());

				auto* CharacterMovementComponent = this->GetCharacterMovementComponent();

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

				if (IsInRobot())
				{
					auto* Robot = GetRobotInstance();
					Robot->GetCharacterMovement()->MaxWalkSpeed = Robot->RunSpeed;
					if (bActivateCombatTick)
					{
						Robot->GetCharacterMovement()->MaxWalkSpeed = WeaponInstance->WeaponRef->MaxWalkSpeed;
					}
				}

				if (bActivateCombatTick)
				{
					SetOrientRotationToMovement(false);
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
						if (IsInRobot())
						{

						}
						else
						{
							CharacterMovementComponent->MaxWalkSpeed = GetMaxSpeedForBot(GetDefaultSpeed());

							if (this->bShouldApplyGunAnimation)
							{
								CharacterMovementComponent->MaxWalkSpeed = GetMaxSpeedForBot(WeaponInstance->WeaponRef->MaxWalkSpeed);
							}
						}
					}

					if (this->CombatAction == ECombatAction::Moving)
					{
						this->bUseControllerRotationYaw = false;
						SetOrientRotationToMovement(true);

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
							FVector AimLoc = Target.Actor->GetActorLocation();
							this->AimAt(AimLoc);
							this->bUseControllerRotationYaw = false;
							SetOrientRotationToMovement(false);
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

	/** Temporarily do not find new barrels because of UE5 PhysX no support */
	bool bFindBarrels = true;

	if (IsZombie())
	{
		bFindBarrels = false;
	}

	if (bFindBarrels && !bReloading)
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
								CombatStyle = ECombatStyle::Attack;
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
		if (Target.Bot->CanHatBeDetached() && Target.Bot->HatMesh->GetStaticMesh())
		{
			//AimTargetLocation = Target.Bot->HatMesh->GetComponentLocation();
		}
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

	/** If aiming at a barrel */
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

	bool bCanMoveWhenShooting = FirearmRef->bCanMoveWhenShooting;

	//if (!bCanMoveWhenShooting && FirearmInstance->Phase != EFirearmPhase::IDLE)
	//{
	//	bCanFindNewPlace = false;
	//}

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

	float MaxTimeToFindNewPlace = 2.0f;

	if (TargetBarrel)
	{
		MaxTimeToFindNewPlace = 5.0f;
	}

	if (this->bMovingToRandomCombatLocation && (DistToRandomLocation < 150.0f || TimeSinceStartedMovingInCombat >= MaxTimeToFindNewPlace))
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

		bool bRandomPlaceIsValid = false;
		float MinDistanceForRandomLocation = 0.0f;
		static const int32 MaxTriesToFindRandomLocation = 15;

		if (Target.TargetType == ETargetType::ExplodingBarrel && TargetBarrel)
		{
			MinDistanceForRandomLocation = TargetBarrel->Radius;
		}

		TArray<FVector> RandomLocations;
		FVector NewRandomLocation;

		for (int32 i = 0; i < MaxTriesToFindRandomLocation; i++)
		{
			FVector TempRandomLocation;
			bool bFoundRandomLocation = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(GetWorld(), Target.Actor->GetActorLocation(), TempRandomLocation, MaxDistance);

			if (bFoundRandomLocation)
			{
				float Dist = FVector::Dist(GetActorLocation(), TempRandomLocation);
				if (!TargetBarrel || Dist >= MinDistanceForRandomLocation)
				{
					NewRandomLocation = TempRandomLocation;
					bRandomPlaceIsValid = true;
					break;
				}
			}
		}


		if (!bRandomPlaceIsValid)
		{
			FVector MaxDistLocation;
			float MaxDist = 0.0f;
			FVector Location = GetActorLocation();
			for (auto Vector : RandomLocations)
			{
				float Dist = FVector::Dist(Vector, Location);

				if (Dist > MaxDist)
				{
					MaxDistLocation = Vector;
					MaxDist = Dist;
				}
			}

			NewRandomLocation = MaxDistLocation;

			if (RandomLocations.Num())
			{
				bRandomPlaceIsValid = true;
			}
		}

		this->TimeSinceStartedMovingInCombat = 0.0f;

		if (bRandomPlaceIsValid)
		{
			if (CharacterMovementComponent)
			{
				CharacterMovementComponent->MaxWalkSpeed = GetMaxSpeedForBot(FirearmRef->MaxWalkSpeed);
			}
			this->bMovingToRandomCombatLocation = true;
			//this->bUseControllerRotationYaw = false;
			SetOrientRotationToMovement(false);
			this->CombatRandomLocation = NewRandomLocation;
			this->DefenderSecondsWithoutMoving.Reset();

			MoveToNewLocation(NewRandomLocation);
		}
		else
		{
			this->bMovingToRandomCombatLocation = false;
		}
	}


	/** If the bot can shoot or reloading and still moving, stop it */
	if ((bReloading || (!bCanMoveWhenShooting && bCanActuallyShoot)) && this->bMovingToRandomCombatLocation)
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

	if (bProjectileMeshExists && !bProjectileMeshVisibility)
	{
		SetWeaponProjectileMeshVisibility(true);
	}

	float BotSpeed = this->GetSpeed();

	if (bCanActuallyShoot && (bCanMoveWhenShooting || BotSpeed < 5.0f))
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
			MoveToNewLocation(EndLocation);
			this->bMovingToRandomCombatLocation = true;
			this->TimeSinceStartedMovingInCombat = 0.0f;
			this->bUseControllerRotationYaw = false;
			SetOrientRotationToMovement(false);
		}

		auto* CharacterMovementComponent = this->GetCharacterMovementComponent();

		if (CharacterMovementComponent)
		{
			CharacterMovementComponent->MaxWalkSpeed = GetMaxSpeedForBot(MeleeInstance->WeaponRef->MaxWalkSpeed);
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

			if (IsInRobot())
			{
				RobotGunPositionCached = GetRobotInstance()->GetGunPosition();
			}

			FRotator GunRotation = this->GetGunRotation();

			FVector OutBulletLocation = this->GetFirearmOutBulletWorldPosition(GunRotation, false);

			FBulletHitResult BulletHitResult = this->LineTraceFromGun(FirearmRef, bBulletOffset, false);

			if (IsInRobot())
			{
				OutBulletLocation = RobotGunPositionCached;
			}

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
			//FirearmProjectile->bSimplified = false;

			if (DistanceFromCamera <= 7000.0f)
			{
				if (FirearmRef->ShootSound)
				{
					float MinSoundPitch = FirearmRef->HitSoundPitchRange.GetLowerBoundValue();
					float ManSoundPitch = FirearmRef->HitSoundPitchRange.GetUpperBoundValue();
					float SoundPitch = FMath::RandRange(MinSoundPitch, ManSoundPitch);
					UGameplayStatics::PlaySoundAtLocation(World, FirearmRef->ShootSound, OutBulletLocation, FMath::RandRange(0.7f, 0.85f), SoundPitch);
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

			auto* GameSessionRef = GetGameSession();

			if (GameSessionRef && !IsZombie())
			{
				auto& Stat = GameSessionRef->PlayerStats[this->ID];
				Stat.Shots++;
				if ((BulletHitResult.ExplodingBarrel || BulletHitResult.BotToDamage || IsInRobot()) && BulletHitResult.HitResult.GetActor() == Target.Actor)
				{
					Stat.Hits++;
				}
				
				if (BulletHitResult.bHatDamage)
				{
					Stat.HatsDropped++;
				}

				if (BulletHitResult.ExplodingBarrel)
				{
					Stat.BarrelsExploded++;
				}

				Stat.Accuracy = (float(Stat.Hits) / float(Stat.Shots)) * 100.0f;
			}

			if (bCinematicCameraAttached && !FirearmProjectile->bSimplified)
			{
				if (!PlayerPawn->CinematicCameraData.ProjectileActor && FirearmProjectile->Distance >= 50.0f && FMath::RandRange(0, 2) == 0)
				{
					if (PlayerPawn->CinematicCameraData.ProjectileTimeout.Current == 0.0f)
					{
						PlayerPawn->CinematicCameraData.ProjectileActor = FirearmProjectile;

						GameSessionRef->SetSlomoEnabled(true);
					}
				}
			}

			if (BulletHitResult.ExplodingBarrel)
			{
				if (PlayerPawn->IsCinematicCameraEnabled())
				{
					bool bAttachCameraToExplodingBarrel = true;

					if (PlayerPawn->CinematicCameraData.ProjectileActor)
					{
						auto* OldProjectile = Cast<AFirearmProjectile>(PlayerPawn->CinematicCameraData.ProjectileActor);
						/** If the camera is already attached to another exploding barrel */
						if (OldProjectile && OldProjectile->BulletHitResult.ExplodingBarrel)
						{
							bAttachCameraToExplodingBarrel = false;
						}
					}

					if (bAttachCameraToExplodingBarrel)
					{
						FirearmProjectile->bSimplified = false;
						PlayerPawn->ActivateCinematicCamera(this);
						PlayerPawn->CinematicCameraData.ProjectileActor = FirearmProjectile;
						GameSessionRef->SetSlomoEnabled(true);
					}
				}
			}

			if (FirearmProjectile->bSimplified)
			{
				FirearmProjectile->SetActorHiddenInGame(true);
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

		auto* GameSessionRef = GetGameSession();

		if (GameSessionRef && !IsZombie())
		{
			auto& Stat = GameSessionRef->PlayerStats[this->ID];
			Stat.Shots++;
			Stat.Accuracy = (float(Stat.Hits) / float(Stat.Shots)) * 100.0f;
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

	if (HitResult.bBlockingHit && HitResult.GetActor() == BotHit && DistanceFromCamera < 5000.0f)
	{
		BotHit->SpawnBloodParticle(HitResult.ImpactPoint, this->GetActorLocation());
	}

	FVector ImpulseVector = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), BotHit->GetActorLocation()).Vector() * MeleeRef->ImpulseForce;
	FVector ImpulseLocation = HitResult.bBlockingHit ? HitResult.ImpactPoint : BotHit->GetActorLocation();

	BotHit->ApplyDamage(MeleeInstance->GetDamage(), this, EWeaponType::Melee, ImpulseVector, ImpulseLocation, HitResult.bBlockingHit ? HitResult.BoneName : NAME_None, bCritical);

	if (MeleeInstance->BotsHit.Num() == 1 && MeleeRef->DamageSound && DistanceFromCamera < 5000.0f)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), MeleeRef->DamageSound, this->WeaponMesh->GetComponentLocation(), FMath::RandRange(0.7f, 0.85f));
	}

	auto* GameSessionRef = GetGameSession();

	if (GameSessionRef && !IsZombie())
	{
		auto& Stat = GameSessionRef->PlayerStats[this->ID];
		Stat.Shots++;
		if (IsEnemy(BotHit))
		{
			Stat.Hits++;
			Stat.Accuracy = (float(Stat.Hits) / float(Stat.Shots)) * 100.0f;
		}
	}

	BotHit->TryAddWallBloodDecal(HitResult.TraceStart, HitResult.bBlockingHit ? HitResult.ImpactPoint : BotHit->GetActorLocation());

}

bool ABot::TraceToTargetResult(bool bIgnoreBots)
{
	if (!this->Target.Actor)
	{
		return false;
	}

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

	return false;
}

void ABot::CreateFloorBloodDecal()
{
	if (!bCanSpawnBlood || IsZombie())
	{
		return;
	}


	this->bCheckedBloodDecalCreation = true;

	if (!this->FloorBloodDecalSubclass)
	{
		return;
	}

	UWorld* WorldObject = this->GetWorld();
	FName BoneName = DeathBoneName == NAME_None ? TEXT("spine_3") : DeathBoneName; 
	FVector PivotLocation = this->GetMesh()->GetSocketLocation(BoneName) + FVector(0.0f, 0.0f, 10.0f);
	FVector DecalLocation = PivotLocation + FVector(0.0f, 0.0f, -60.0f);

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
	
	FRotator DecalRotation = HitResult.ImpactNormal.Rotation();
	DecalRotation.Roll += FMath::RandRange(0.0f, 360.0f);

	DecalSpawnTransform.SetRotation(FQuat(DecalRotation));

	this->FloorBloodDecalActor = WorldObject->SpawnActor<ABloodDecal>(this->FloorBloodDecalSubclass, DecalSpawnTransform);
	this->FloorBloodDecalActor->BotOwner = this;
}

void ABot::SpawnBloodParticle(FVector ImpactPoint, FVector CauserLocation)
{
	APlayerPawn* PlayerPawn = APlayerPawn::Get();

	if (!PlayerPawn)
	{
		return;
	}

	float DistanceFromCamera = PlayerPawn->GetDistanceFromCamera(ImpactPoint);

	UNiagaraSystem* NiagaraParticle = nullptr;

	if (RobotInstance)
	{
		NiagaraParticle = RobotInstance->DamageNiagaraParticle;
	}
	else
	{
		NiagaraParticle = BloodNiagaraParticle;
	}

	if (DistanceFromCamera <= 7000.0f && NiagaraParticle)
	{
		FRotator ParticleRotation = UKismetMathLibrary::FindLookAtRotation(ImpactPoint, CauserLocation);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraParticle, ImpactPoint, ParticleRotation);
	}
}

void ABot::RespawnAtRandomPlace()
{
	FVector NewRandomLocation;

	FVector OldLocation = GetBotParentActor()->GetActorLocation();

	bool bFoundRandomLocation = UNavigationSystemV1::K2_GetRandomLocationInNavigableRadius(GetWorld(), OldLocation, NewRandomLocation, 1500.0f);

	if (bFoundRandomLocation)
	{
		GetBotParentActor()->SetActorLocation(NewRandomLocation + FVector(0.0f, 0.0f, 150.0f));
	}
	else
	{
		auto* GameSession = UChattersGameSession::Get();

		if (!GameSession)
		{
			return;
		}

		GameSession->RespawnBotAfterStuck(this);
	}
}

bool ABot::CanHatBeDetached()
{
	return bHatAttached && bCanHatBeDetached;
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
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
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

	if (IsZombie())
	{
		if (FMath::RandRange(0, 10) != 0)
		{
			return;
		}
	}

	UWorld* World = this->GetWorld();

	if (!World)
	{
		return;
	}

	FHitResult HitResult;
	FVector StartLocation = this->GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0.0f, 0.0f, 1500.0f);
	FCollisionQueryParams CollisionParams;
	CollisionParams.bReturnPhysicalMaterial = true;
	CollisionParams.bTraceComplex = false;
	CollisionParams.AddIgnoredActor(this);

	if (RobotInstance)
	{
		CollisionParams.AddIgnoredActor(RobotInstance);
	}

	World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, CollisionParams);


	if (HitResult.bBlockingHit)
	{
		UPhysicalMaterial* PhysMaterial = HitResult.PhysMaterial.Get();
		if (PhysMaterial)
		{
			if (RobotInstance)
			{
				RobotInstance->PlayFootstepSound(HitResult.Location, PhysMaterial->SurfaceType);
			}
			else
			{
				this->PlayFootstepSound(HitResult.Location, PhysMaterial->SurfaceType);
			}
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
			auto* EquipmentSet = EquipmentList->GetEquipmentSet(this->Team, GameSessionObject->AvailableWeapons);

			auto RandomEquipment = EquipmentSet->GetRandomEquipment(this->Team);
			if (this->HatMesh)
			{
				bHatAttached = RandomEquipment.Hat != nullptr;
				if (!RandomEquipment.Hat)
				{
					this->HatMesh->SetStaticMesh(nullptr);
					this->HatMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
				else
				{
					HatTransform = RandomEquipment.Hat->GetTransform();
					bCanHatBeDetached = RandomEquipment.Hat->bCanDetach;
					AttachHat();
					this->HatMesh->SetStaticMesh(RandomEquipment.Hat->StaticMesh);
					this->HatMesh->EmptyOverrideMaterials();

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
					this->BeardMesh->EmptyOverrideMaterials();

					TArray<UMaterialInterface*> Materials = RandomEquipment.BeardStyle->GetRandomMaterials();

					for (int32 i = 0; i < Materials.Num(); i++)
					{
						this->BeardMesh->SetMaterial(i, Materials[i]);
					}
				}
			}

			if (RandomEquipment.FaceMaterial)
			{
				if (this->HeadMesh)
				{
					this->HeadMesh->SetMaterial(0, RandomEquipment.FaceMaterial);
				}
			}

			UWeaponItem* RandomWeapon = EquipmentSet->GetRandomWeapon(GameSessionObject->AvailableWeapons, this->Team);

			if (RandomWeapon)
			{
				if (this->WeaponMesh)
				{
					this->bProjectileMeshExists = false;
					//this->WeaponMesh->ReregisterComponent();
					this->WeaponMesh->SetStaticMesh(RandomWeapon->StaticMesh);
					this->WeaponMesh->SetRelativeTransform(RandomWeapon->GetTransform());

					this->WeaponMesh->EmptyOverrideMaterials();

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

					this->SetMeleeCollisionEnabled(false);

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

				this->GetMesh()->EmptyOverrideMaterials();

				bCanSpawnBlood = RandomEquipment.Costume->bCanSpawnBlood;

				TArray<UMaterialInterface*> Materials = RandomEquipment.Costume->GetRandomMaterials();

				for (int32 i = 0; i < Materials.Num(); i++)
				{
					this->GetMesh()->SetMaterial(i, Materials[i]);
				}

				bIsHeadHidden = RandomEquipment.Costume->bHideHeadMesh;

				this->HeadMesh->SetHiddenInGame(bIsHeadHidden);

				this->BloodNiagaraParticle = RandomEquipment.Costume->BloodParticle ? RandomEquipment.Costume->BloodParticle : this->GetDefaultBloodParticle();
			}

			if (RandomEquipment.Robot)
			{
				SetRobot(RandomEquipment.Robot);
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

	if (IsZombie())
	{
		NameWidgetComponent->SetHiddenInGame(true);
		NameWidgetComponent->SetVisibility(false);
	}

	this->SetEquipment();
	this->UpdateEquipmentTeamColors();
}

void ABot::SpawnReloadingParticle(UNiagaraSystem* Particle, FTransform Transform)
{
	auto* PlayerPawn = APlayerPawn::Get();

	if (!PlayerPawn)
	{
		return;
	}

	auto DistanceFromCamera = PlayerPawn->GetDistanceFromCamera(this->GetActorLocation());

	if (DistanceFromCamera > 5000.0f)
	{
		return;
	}

	if (!Particle)
	{
		return;
	}

	if (!this->WeaponInstance || !this->WeaponMesh)
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAttached(Particle, this->WeaponMesh, TEXT("out"), Transform.GetLocation(), FRotator(Transform.GetRotation()), EAttachLocation::SnapToTarget, true, true);
}

void ABot::AttachHat()
{
	HatMesh->SetSimulatePhysics(false);
	HatMesh->AttachToComponent(HeadMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("head_"));
	HatMesh->SetCollisionProfileName(FName(TEXT("Hat")), true);
	HatMesh->SetCollisionEnabled(bCanHatBeDetached ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	HatMesh->SetRelativeTransform(HatTransform);
}

void ABot::MoveToRandomLocation()
{
	auto* AIController = this->GetAIController();

	if (AIController)
	{
		FVector OutLocation;
		bool bFound = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(GetWorld(), GetActorLocation(), OutLocation, 5000.0f);

		MoveToNewLocation(OutLocation);
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

	if (IsZombie())
	{
		Damage *= 5;
	}

	bool bHeadshot = false;

	if (BoneHit == TEXT("head_") || BoneHit == TEXT("eye_R") || BoneHit == TEXT("eye_L") || BoneHit == TEXT("spine_6"))
	{
		bHeadshot = true;
	}

	if (bHeadshot)
	{
		bCritical = true;
	}

	if (bCritical)
	{
		if (ByBot && ByBot != this)
		{
			auto* ByWeaponRef = ByBot->GetWeaponRef();
			if (ByWeaponRef)
			{
				Damage = FMath::RoundToInt(float(ByWeaponRef->MaxDamage) * FMath::RandRange(1.25f, 1.5f));
			}
		}
		else
		{
			Damage = FMath::RoundToInt(Damage * 1.5f);
		}
	}

	auto OldHP = this->HealthPoints;

	if (OldHP <= 0)
	{
		return;
	}

	this->HealthPoints -= Damage;


	auto* GameSessionObject = this->GetGameSession();

	if (GameSessionObject)
	{
		if (bPlayerAttached)
		{
			auto* SessionWidget = GameSessionObject->GetSessionWidget();
			if (SessionWidget)
			{
				SessionWidget->UpdateSpectatorBotHealth(this->HealthPoints);
			}
		}

		if (ByBot && ByBot != this && !ByBot->IsZombie())
		{
			GameSessionObject->PlayerStats[ByBot->ID].Damage += Damage;
		}
	}

	if (IsInRobot())
	{
		RobotInstance->LastDamageLocation = ImpulseLocation;
	}

	if (HealthPoints <= 0)
	{
		HealthPoints = 0;
		this->OnDead(ByBot, WeaponType, ImpulseVector, ImpulseLocation, BoneHit, bHeadshot);
	}
	else
	{
		/** If damage by enemy */
		if (ByBot && IsEnemy(ByBot) && ByBot->bAlive)
		{
			/** If bot is not target already */
			if (!Target.Bot || Target.Bot != ByBot)
			{
				bool bSetDamagerAsNewTarget = false;

				/** If the current target is not aiming at us */
				if (Target.Bot->Target.Bot != this)
				{
					bSetDamagerAsNewTarget = true;
				}
				else
				{
					auto OldTargetWeaponType = Target.Bot->GetWeaponType();
					auto DamagerWeaponType = ByBot->GetWeaponType();

					if (OldTargetWeaponType == EWeaponType::Firearm && DamagerWeaponType == EWeaponType::Melee)
					{
						bSetDamagerAsNewTarget = true;
					}
				}

				if (ByBot->HealthPoints < Target.Bot->HealthPoints)
				{
					bSetDamagerAsNewTarget = true;
				}

				if (bSetDamagerAsNewTarget)
				{
					/** Set the damager as new target */
					SetNewEnemyTarget(ByBot);
				}
			}
		}

		if (HitBoneRotationTimer.Current == 0)
		{
			HitBoneRotationTimer.Current = HitBoneRotationTimer.Max;
		}

		UpdateBonesDamageData(BoneHit);
	}

	//UE_LOG(LogTemp, Display, TEXT("[ABot] Applying %d damage to bot. Old hp: %d. New HP: %d"), Damage, OldHP, this->HealthPoints);

	auto* NameWidgetObject = this->GetNameWidget();

	if (NameWidgetObject)
	{
		float HealthValue = this->GetHeathValue();
		NameWidgetObject->UpdateHealth(HealthValue);
		NameWidgetObject->ShowDamageNumber(Damage, bCritical);
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

	if (GetGameSession()->GameModeType == ESessionGameMode::Zombie)
	{
		if (Team == EBotTeam::Zombie && BotToCheck->Team == EBotTeam::Zombie)
		{
			return false;
		}

		return true;
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

void ABot::OnDead(ABot* Killer, EWeaponType WeaponType, FVector ImpulseVector, FVector ImpulseLocation, FName BoneHit, bool bHeadshot)
{
	this->bAlive = false;
	this->HealthPoints = 0;

	this->SetActorLocation(this->GetActorLocation());

	this->UpdateHeadAnimationType(-1.0f, true);

	RemoveProjectileMeshesTimeout.Reset();

	DeathBoneName = BoneHit;

	auto* AIController = this->GetAIController();

	auto* RobotInstanceRef = GetRobotInstance();

	if (AIController)
	{
		AIController->StopMovement();

		if (RobotInstanceRef)
		{
			RobotInstanceRef->StopMovement();
		}
	}

	bInstantFallAfterDeath = true;

	if (bInstantFallAfterDeath)
	{
		if (RobotInstanceRef == nullptr)
		{
			this->GetMesh()->SetSimulatePhysics(true);
			this->GetMesh()->SetCollisionProfileName(FName(TEXT("DeadBody")), true);
			this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
			RagdollAfterDeathTimer.Current = RagdollAfterDeathTimer.Max;
		}
	}
	else
	{
		RagdollAfterDeathTimer.Max = 0.5f;
		RagdollAfterDeathTimer.Reset();
		MoveToRandomLocation();
		SetOrientRotationToMovement(true);
		GetCharacterMovementComponent()->MaxWalkSpeed = 200.0f;
	}

	if (this->GetMesh())
	{
		//
		this->GetMesh()->SetGenerateOverlapEvents(false);
	}

	if (this->MeleeCollision)
	{
		this->MeleeCollision->SetGenerateOverlapEvents(false);
	}

	if (this->MeleeHitbox)
	{
		this->MeleeHitbox->SetGenerateOverlapEvents(false);
		this->MeleeHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}


	if (bInstantFallAfterDeath)
	{
		switch (WeaponType)
		{
		case EWeaponType::Firearm:
		case EWeaponType::Explosion:
		case EWeaponType::Melee:
		case EWeaponType::Train:
		case EWeaponType::Walker:
		case EWeaponType::Bomber:
			this->GetMesh()->AddImpulseAtLocation(ImpulseVector, ImpulseLocation, BoneHit);
			break;
		}
	}

	if (this->NameWidgetComponent)
	{
		this->NameWidgetComponent->SetVisibility(false);
	}

	CombatAction = ECombatAction::IDLE;
	bShouldApplyGunAnimation = false;

	auto* CharacterMovementComponent = this->GetCharacterMovementComponent();

	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->bUseRVOAvoidance = false;
	}

	if (RobotInstanceRef)
	{
		RobotInstanceRef->GetCharacterMovement()->bUseRVOAvoidance = false;
	}

	auto* GameSessionObject = this->GetGameSession();

	if (GameSessionObject && GameSessionObject->BotNameDiedFirst.IsEmpty() && !IsZombie())
	{
		GameSessionObject->BotNameDiedFirst = this->DisplayName;
	}

	if (Killer && this->IsEnemy(Killer) && !GameSessionObject->bGameEnded)
	{
		Killer->Kills++;
		auto* NameWidgetRef = Killer->GetNameWidget();
		if (NameWidgetRef)
		{
			NameWidgetRef->UpdateKillsNumber(Killer->Kills);
		}

		if (GameSessionObject && !Killer->IsZombie())
		{
			GameSessionObject->OnBotKill(Killer);
			GameSessionObject->PlayerStats[Killer->ID].Kills++;
		}
	}

	if (bPlayerAttached && !Killer->IsZombie())
	{
		auto* PlayerPawn = APlayerPawn::Get();
		if (PlayerPawn)
		{
			if (Killer && Killer != this)
			{
				PlayerPawn->AttachToBot(Killer);
			}
			else
			{
				if (PlayerPawn->bFirstPersonCamera)
				{
					PlayerPawn->SetThirdPersonCamera();
				}
			}
		}
	}


	if (GameSessionObject)
	{
		if (GameSessionObject->GameModeType == ESessionGameMode::Deathmatch)
		{
			bShouldReviveBot = true;
		}

		if (IsZombie())
		{
			GameSessionObject->OnZombieDied(this);
		}

		auto* SessionWidget = GameSessionObject->GetSessionWidget();
		if (SessionWidget && !IsZombie())
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

			switch (WeaponType)
			{
			case EWeaponType::Explosion:
				KillFeedIcon.IconType = EKillFeedIconType::Explosion;
				break;
			case EWeaponType::Train:
				KillFeedIcon.IconType = EKillFeedIconType::Train;
				break;
			case EWeaponType::Fall:
				KillFeedIcon.IconType = EKillFeedIconType::Fall;
				break;
			case EWeaponType::Walker:
				KillFeedIcon.IconType = EKillFeedIconType::Walker;
				break;
			case EWeaponType::Bomber:
				KillFeedIcon.IconType = EKillFeedIconType::Bomber;
				break;
			default:
				if (Killer->WeaponInstance && Killer->WeaponInstance->WeaponRef)
				{
					KillFeedIcon = Killer->WeaponInstance->WeaponRef->KillFeedIcon;
				}
				break;
			}

			SessionWidget->OnKill(KillerName, this->DisplayName, Killer->GetTeamColor(), this->GetTeamColor(), KillFeedIcon, bHeadshot && WeaponType == EWeaponType::Firearm, (Killer == this || Killer->IsZombie()));
		}


		if (!IsZombie())
		{
			GameSessionObject->OnBotDied(this->ID);
		}
	}

	if (RobotInstanceRef)
	{
		RobotInstanceRef->OnDead();
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
	float GunRotation = this->GetGunRotation().Pitch;

	if (CombatAction == ECombatAction::Moving || CombatAction == ECombatAction::IDLE)
	{
		GunRotation = 0.0f;
	}

	if (HitBoneRotationCurve && HitBoneRotationTimer.Current != 0.0f)
	{
		GunRotation += HitBoneRotationCurve->GetFloatValue(HitBoneRotationTimer.Max - HitBoneRotationTimer.Current);
	}

	return GunRotation;
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
	SetWeaponProjectileMeshVisibility(false);
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

	RemoveAllAttachedProjectileMeshes();

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

	AttachHat();
	ClearDamagedBonesData();

	if (RobotInstance)
	{
		RobotInstance->OnNewRound(Team);
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
		this->bHatAttached = false;
		this->HatMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		this->HatMesh->SetCollisionProfileName(FName(TEXT("OufitPhysics")), true);
		this->HatMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		this->HatMesh->SetSimulatePhysics(true);

		const FVector HeadLocation = this->GetMesh()->GetSocketLocation(TEXT("head_"));
		FVector HatLocation = HeadLocation - this->HatMesh->GetComponentLocation();

		HatLocation.Normalize();
			
		const FVector HatPhysicsImpulse = HatLocation * -1000.0f;

		HatDetachedTimer.Reset();

		//this->HatMesh->AddImpulseAtLocation(HatPhysicsImpulse, this->HatMesh->GetComponentLocation());
	}
}

void ABot::DetachHatAfterShot(FBulletHitResult BulletHitResult, float ImpulseForce, FVector ShotLocation)
{
	TryDetachHat();
	FVector ImpactPoint = ShotLocation;

	FVector StartLocation = BulletHitResult.HitResult.TraceStart;
	FVector ShotDirection = ShotLocation - StartLocation;
	ShotDirection.Normalize();
	FVector ImpulseVector = ShotDirection * ImpulseForce;

	HatMesh->AddImpulseAtLocation(ImpulseVector, ImpactPoint, BulletHitResult.HitResult.BoneName);

	if (HatDamageSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), HatDamageSound, ShotLocation, FMath::RandRange(0.7f, 0.85f));
	}
}

void ABot::DeatachWeapon()
{
	if (this->WeaponMesh && this->WeaponInstance)
	{
		this->WeaponInstance->BotOwner = nullptr;
		if (this->WeaponInstance->IsValidLowLevel())
		{
			this->WeaponInstance->ConditionalBeginDestroy();

			/** Try to cast to fiream instance and destroy projectile mesh */
			auto* FirearmInstance = Cast<UFirearmWeaponInstance>(WeaponInstance);

			if (FirearmInstance && FirearmInstance->ProjectileMeshActor)
			{
				if (FirearmInstance->ProjectileMeshActor)
				{

				}
				FirearmInstance->ProjectileMeshActor->Destroy();

				if (FirearmInstance->BowstringComponent)
				{
					FirearmInstance->BowstringComponent->DetachFromParent(false, false);
					FirearmInstance->BowstringComponent->UnregisterComponent();
					FirearmInstance->BowstringComponent->DestroyComponent();
					FirearmInstance->BowstringComponent = nullptr;
				}
			}

		}
		this->WeaponInstance = nullptr;
		this->WeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		this->WeaponMesh->SetCollisionProfileName(FName(TEXT("OufitPhysics")), true);
		this->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		this->WeaponMesh->SetSimulatePhysics(true);

		WeaponDetachTimer.Reset();
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

		FRotator NewRotation = FRotator(0.0f, this->SmoothRotation.CurrentYaw, 0.0f);

		if (IsInRobot())
		{
			auto* Robot = GetRobotInstance();

			if (Robot)
			{
				Robot->SetActorRotation(NewRotation);
			}
		}
		else
		{
			SetActorRotation(NewRotation);
		}
		

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

		MoveToNewLocation(this->RandomPointToMoveWhileAiming);

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

FVector ABot::GetFirearmBulletTargetWorldPosition(FVector OutBulletWorldPosition, float BulletDistance, FRotator GunRotation, bool bShouldRecalculateGunLocation, bool bBulletOffset, float RecoilFactor)
{
	if (bShouldRecalculateGunLocation)
	{
		GunRotation = this->GetGunRotation();
	}

	if (bBulletOffset)
	{
		const float MaxBulletOffset = 5.0f * RecoilFactor;

		GunRotation.Pitch += FMath::RandRange(-MaxBulletOffset, MaxBulletOffset);
		GunRotation.Yaw += FMath::RandRange(-MaxBulletOffset, MaxBulletOffset);
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

FBulletHitResult ABot::LineTraceFromGun(UFirearmWeaponItem* FirearmRef, bool bBulletOffset, bool bDrawDebugLines, bool bShot)
{
	FBulletHitResult BulletHitResult;

	if (FirearmRef)
	{
		FRotator GunRotation = this->GetGunRotation();
		FVector StartLocation = this->GetFirearmOutBulletWorldPosition(GunRotation, false);
		FVector EndLocation = this->GetFirearmBulletTargetWorldPosition(StartLocation, FirearmRef->MaxDistance, GunRotation, false, bBulletOffset, FirearmRef->RecoilFactor);

		UWorld* World = this->GetWorld();

		if (!World)
		{
			return BulletHitResult;
		}

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (IsInRobot())
		{
			if (bShot)
			{
				StartLocation = RobotGunPositionCached;
			}
			else
			{
				StartLocation = GetActorLocation();
			}

			EndLocation = GetFirearmBulletTargetWorldPosition(StartLocation, FirearmRef->MaxDistance, GunRotation, false, bBulletOffset, FirearmRef->RecoilFactor);
			Params.AddIgnoredActor(GetRobotInstance());

		}

		World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel3, Params);

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
					BulletHitResult.bHatDamage = HitResult.GetComponent() == Bot->HatMesh;
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

	if (RobotInstance)
	{
		RobotInstance->StopMovement();
	}
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
		this->MeleeCollision->SetGenerateOverlapEvents(bEnabled);
	}
}

void ABot::Clear()
{
	this->RemoveBloodDecal();

	for (auto* BloodDecal : WallBloodDecals)
	{
		BloodDecal->Destroy();
	}

	WallBloodDecals.Empty();
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

void ABot::UpdateHeadAnimationType(float CameraDistance, bool bForce)
{
	bool bNewHeadAnimationDetailedMode = false;

	if (!this->bAlive)
	{
		bNewHeadAnimationDetailedMode = false;
	}
	else
	{
		if (this->CombatAction == ECombatAction::IDLE)
		{
			if (CameraDistance == -1.0f)
			{
				auto* PlayerRef = APlayerPawn::Get();
				if (PlayerRef)
				{
					CameraDistance = PlayerRef->GetDistanceFromCamera(GetActorLocation());
				}
			}

			bNewHeadAnimationDetailedMode = CameraDistance <= 3000.0f;
		}
	}

	if (bForce || bNewHeadAnimationDetailedMode != this->bUseDetailedHeadAnimation)
	{
		if (bNewHeadAnimationDetailedMode)
		{
			this->HeadMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			this->HeadMesh->SetAnimInstanceClass(this->HeadAnimationBlueprint);
			this->HeadMesh->SetMasterPoseComponent(nullptr, true);
		}
		else
		{
			this->HeadMesh->SetMasterPoseComponent(this->GetMesh(), true);
		}
	}

	this->bUseDetailedHeadAnimation = bNewHeadAnimationDetailedMode;
}

void ABot::ReviveBotDeatchmatch()
{
	this->bShouldReviveBot = false;

	auto* GameSession = this->GetGameSession();

	if (!GameSession->bDeathmatchRoundEnded)
	{
		this->bAlive = true;

		this->HealthPoints = this->MaxHealthPoints;

		if (!IsInRobot())
		{
			this->GetMesh()->SetSimulatePhysics(false);

			this->GetMesh()->SetCollisionProfileName(FName(TEXT("CharacterMesh")), true);
			//this->GetMesh()->PutAllRigidBodiesToSleep();
			this->GetMesh()->WakeAllRigidBodies();
		}

		this->HeadMesh->SetSimulatePhysics(false);
		this->HatMesh->SetSimulatePhysics(false);
		this->WeaponMesh->SetSimulatePhysics(false);

		this->MeleeHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		this->MeleeHitbox->SetGenerateOverlapEvents(true);

		//this->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

		if (this->NameWidgetComponent)
		{
			this->NameWidgetComponent->SetVisibility(true);
		}

		auto* NameWidgetRef = GetNameWidget();

		if (NameWidgetRef)
		{
			float HealthValue = this->GetHeathValue();
			NameWidgetRef->UpdateHealth(HealthValue);
		}

		if (this->HatMesh)
		{
			this->bHatAttached = true;
			this->HatMesh->WakeAllRigidBodies();
			this->HatMesh->AttachToComponent(this->HeadMesh, FAttachmentTransformRules::KeepRelativeTransform, FName(TEXT("head_")));
			this->HatMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		auto* CharacterMovementComponent = this->GetCharacterMovementComponent();

		if (CharacterMovementComponent && !IsInRobot())
		{
			//CharacterMovementComponent->bUseRVOAvoidance = true;
		}

		SecondsAfterDeath = 0.0f;

		this->bRigidBodiesSleep = false;
		//this->GetMesh()->WakeAllRigidBodies();
		//this->HeadMesh->WakeAllRigidBodies();
		this->GetMesh()->bPauseAnims = false;
		this->HeadMesh->bPauseAnims = false;

		HatDetachedTimer.Reset();
		WeaponDetachTimer.Reset();


		FTransform SpawnPoint = GameSession->GetAvailableSpawnPoint(false, true);

		if (IsInRobot() == false)
		{
			SetActorLocation(SpawnPoint.GetLocation());
			SetActorRotation(SpawnPoint.GetRotation().Rotator());

			this->WeaponMesh->WakeAllRigidBodies();

			this->WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"), true);
			this->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			this->WeaponMesh->AttachToComponent(this->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName(TEXT("R_arm_4")));

			if (this->WeaponInstance && this->WeaponInstance->IsValidLowLevel() && !this->WeaponInstance->IsPendingKill())
			{
				this->WeaponInstance->ConditionalBeginDestroy();
				this->WeaponInstance = nullptr;
			}


			SetEquipment();
			UpdateEquipmentTeamColors();

			this->GetMesh()->AttachToComponent(this->GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		}
		else
		{
			RobotInstance->SetActorLocation(SpawnPoint.GetLocation());
			RobotInstance->SetActorRotation(SpawnPoint.GetRotation().Rotator());
		}

		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f));
		GetMesh()->SetRelativeRotation((FRotator(0.0f, -90.0f, 0.0f)));

		FVector RelativeLocation = this->GetMesh()->GetRelativeLocation();

		this->OnGameSessionStarted(GameSession->SessionMode);

		//this->bReviveCollisionTimerActive = true;
		//this->ReviveCollisionTimer.Reset();

		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		RemoveAllAttachedProjectileMeshes();

		DropEquipmentAfterDeathTimer.Reset();

		ClearDamagedBonesData();

		if (IsInRobot())
		{
			GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			RobotInstance->OnRespawn();
			FindNewEnemyTarget();
		}
	}
}

void ABot::SetWeaponProjectileMeshVisibility(bool bVisible)
{
	if (!bProjectileMeshExists)
	{
		false;
	}

	if (!this->WeaponInstance || !this->WeaponInstance->WeaponRef)
	{
		return;
	}

	if (this->WeaponInstance->WeaponRef->Type != EWeaponType::Firearm)
	{
		return;
	}

	auto* FirearmInstance = Cast<UFirearmWeaponInstance>(WeaponInstance);

	if (!FirearmInstance)
	{
		return;
	}

	FirearmInstance->SetProjectileMeshVisibility(bVisible);
	bProjectileMeshVisibility = bVisible;
}

void ABot::AttachProjectileMeshToBody(UStaticMesh* StaticMesh, FVector Location, FRotator Rotation, FName BoneName)
{
	if (ProjectileMeshesAttached.Num() >= 3)
	{
		if (ProjectileMeshesAttached[0])
		{
			ProjectileMeshesAttached[0]->DetachFromParent(false, false);
			ProjectileMeshesAttached[0]->UnregisterComponent();
			ProjectileMeshesAttached[0]->DestroyComponent();
		}

		ProjectileMeshesAttached.RemoveAt(0);
	}

	if (!StaticMesh)
	{
		return;
	}

	FString ComponentName = FString::Printf(TEXT("Projectile_%d"), ProjectileMeshCounter);
	ProjectileMeshCounter++;

	auto* ProjectileMesh = NewObject<UStaticMeshComponent>(this, *ComponentName);

	if (ProjectileMesh)
	{
		ProjectileMeshesAttached.Add(ProjectileMesh);
		ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		USkeletalMeshComponent* SkeletalMesh = (BoneName == TEXT("spine_6") || BoneName == TEXT("head_")) ? HeadMesh : GetMesh();

		/** If head mesh doesn't exists */
		if (!SkeletalMesh)
		{
			SkeletalMesh = GetMesh();
		}

		ProjectileMesh->AttachTo(SkeletalMesh, BoneName, EAttachLocation::SnapToTarget);
		ProjectileMesh->SetStaticMesh(StaticMesh);
		ProjectileMesh->RegisterComponent();
		ProjectileMesh->SetWorldLocation(Location);
		ProjectileMesh->SetWorldRotation(Rotation);
		this->AddInstanceComponent(ProjectileMesh);
	}

}

void ABot::RemoveAllAttachedProjectileMeshes()
{
	for (auto* MeshRef : ProjectileMeshesAttached)
	{
		if (MeshRef)
		{
			MeshRef->DetachFromParent(false, false);
			MeshRef->UnregisterComponent();
			MeshRef->DestroyComponent();
		}
	}
	
	this->ProjectileMeshesAttached.Empty();
}

void ABot::WinnerTick(float DeltaTime)
{
	if (this->WeaponInstance)
	{
		auto* FirearmInstance = Cast<UFirearmWeaponInstance>(WeaponInstance);

		if (FirearmInstance)
		{
			auto* PlayerPawn = APlayerPawn::Get();

			if (PlayerPawn)
			{
				FVector CameraLocation = PlayerPawn->GetCameraLocation();

				this->AimAt(CameraLocation);
				this->SmoothRotatingTick(DeltaTime);
				bShouldApplyGunAnimation = true;
			}
		}
	}
}

void ABot::AddWallBloodDecal(FHitResult HitResult)
{
	if (!this->FloorBloodDecalSubclass)
	{
		return;
	}

	FTransform DecalSpawnTransform;
	DecalSpawnTransform.SetLocation(HitResult.ImpactPoint);

	FRotator DecalRotation = HitResult.ImpactNormal.Rotation();

	DecalRotation += FRotator(0.0f, 0.0f, FMath::RandRange(0.0f, 360.0f));

	DecalSpawnTransform.SetRotation(FQuat(DecalRotation));

	ABloodDecal* BloodDecal = GetWorld()->SpawnActor<ABloodDecal>(this->FloorBloodDecalSubclass, DecalSpawnTransform);
	BloodDecal->BotOwner = this;
	BloodDecal->bFloorDecal = false;
	float BloodScale = FMath::RandRange(BloodDecal->MinWallDecalScale, BloodDecal->MaxWallDecalScale);
	BloodDecal->SetActorScale3D(FVector(BloodScale));

	WallBloodDecals.Add(BloodDecal);
}

void ABot::RemoveWallBloodDecal(ABloodDecal* Decal)
{
	for (int32 i = 0; i < WallBloodDecals.Num(); i++)
	{
		if (WallBloodDecals[i] == Decal)
		{
			WallBloodDecals[i]->Destroy();
			WallBloodDecals.RemoveAt(i, 1);
			return;
		}
	}
}

void ABot::TryAddWallBloodDecal(FVector StartPoint, FVector EndPoint)
{
	if (!bCanSpawnBlood || IsZombie())
	{
		return;
	}

	if (WallBloodDecals.Num() >= MaxWallBloodDecals)
	{
		return;
	}

	StartPoint.Z = EndPoint.Z;

	APlayerPawn* PlayerPawn = APlayerPawn::Get();

	float DistanceFromCamera = PlayerPawn ? PlayerPawn->GetDistanceFromCamera(EndPoint) : 0.0f;

	if (DistanceFromCamera > 10000.0f)
	{
		return;
	}

	FVector ShotDirection = UKismetMathLibrary::FindLookAtRotation(StartPoint, EndPoint).Vector();

	float Dist = FVector::Dist(StartPoint, EndPoint);

	FVector ShotDirectionToWall = ShotDirection * (Dist + 300.0f);

	FCollisionObjectQueryParams CollisionParams;
	CollisionParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);


	FVector BloodLocation;

	int32 RandNumber = FMath::RandRange(0, 1);

	if (RandNumber == 0)
	{
		BloodLocation = StartPoint + ShotDirectionToWall;
	}
	else if (RandNumber == 1)
	{
		FVector RandVector = FVector(0.2f, 0.0f, -0.8f);
		RandVector.Normalize();

		RandVector = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f).RotateVector(RandVector);

		BloodLocation = EndPoint + (RandVector * 300.0f);
	}

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByObjectType(HitResult, EndPoint, BloodLocation, CollisionParams);

	//FVector ResultPoint = HitResult.bBlockingHit ? HitResult.ImpactPoint : BloodLocation;
	//DrawDebugLine(GetWorld(), EndPoint, ResultPoint, FColor(255, 0, 0), false, 5.0f, 0, 1.0f);

	if (HitResult.bBlockingHit)
	{
		AddWallBloodDecal(HitResult);
	}
}

bool ABot::IsHatAttached()
{
	return bHatAttached;
}

void ABot::SetCharacterDetailsVisible(bool bVisible)
{
	HeadMesh->SetVisibility(bVisible, true);
	WeaponMesh->SetVisibility(bVisible, true);

	bDetailsVisibility = bVisible;
}

void ABot::UpdateDetailsVisibilityByDistance(float CameraDistance)
{
	bool bNewBotVisibility = CameraDistance < 20000.0f;

	if (bNewBotVisibility != bBotVisible)
	{
		GetMesh()->SetVisibility(bNewBotVisibility);
		bBotVisible = bNewBotVisibility;
	}

	bool bNewDetailsVisibility = CameraDistance < 10000.0f;

	if (bNewDetailsVisibility != bDetailsVisibility)
	{
		SetCharacterDetailsVisible(bNewDetailsVisibility);
	}
}

FBotTarget ABot::GetTargetData()
{
	return Target;
}

bool ABot::IsFalling()
{
	return GetMovementComponent()->IsFalling();
}

float ABot::GetMaxSpeedForBot(float RequiredSpeed)
{
	if (IsZombie())
	{
		return FMath::Min(RequiredSpeed, GetDefaultSpeed());
	}

	/** No legs damaged */
	if (!DamagedBonesData.bLeftLegDamaged && !DamagedBonesData.bRightLegDamaged)
	{
		return RequiredSpeed;
	}

	float MaxSpeed = GetDefaultSpeed();

	/** Only one leg damaged */
	if (DamagedBonesData.bLeftLegDamaged || DamagedBonesData.bRightLegDamaged)
	{
		MaxSpeed = 450.0f;
	}
	
	/** All legs damaged */
	if (DamagedBonesData.bLeftLegDamaged && DamagedBonesData.bRightLegDamaged)
	{
		MaxSpeed = 350.0f;
	}

	return FMath::Min(RequiredSpeed, MaxSpeed);
}

void ABot::UpdateBonesDamageData(FName BoneName)
{
	FString BoneHit = BoneName.ToString();
	bool bUpdateMaxWalkSpeed = false;
	if (BoneHit.StartsWith(TEXT("L_leg_")) && BoneHit != TEXT("L_leg_1") && !DamagedBonesData.bLeftLegDamaged)
	{
		DamagedBonesData.bLeftLegDamaged = true;
		bUpdateMaxWalkSpeed = true;
	}
	else if (BoneHit.StartsWith(TEXT("R_leg_")) && BoneHit != TEXT("R_leg_1") && DamagedBonesData.bRightLegDamaged)
	{
		DamagedBonesData.bRightLegDamaged = true;
		bUpdateMaxWalkSpeed = true;
	}

	if (bUpdateMaxWalkSpeed)
	{
		auto* CharacterMovementComponent = this->GetCharacterMovementComponent();

		if (CharacterMovementComponent)
		{
			CharacterMovementComponent->MaxWalkSpeed = GetMaxSpeedForBot(CharacterMovementComponent->MaxWalkSpeed);
		}
	}
}

void ABot::ClearDamagedBonesData()
{
	DamagedBonesData.bLeftLegDamaged = false;
	DamagedBonesData.bRightLegDamaged = false;
}

void ABot::SetUseControllerRotationYaw(bool bUse)
{
	if (bUse)
	{

	}
	else
	{

	}
}

ARobot* ABot::GetRobotInstance()
{
	return RobotInstance;
}

bool ABot::IsInRobot()
{
	return RobotInstance != nullptr;
}

ARobot* ABot::SetRobot(TSubclassOf<ARobot> RobotClass)
{
	if (!RobotClass)
	{
		return nullptr;
	}

	if (IsInRobot())
	{
		RobotInstance->OnNewRound(Team);
		return nullptr;
	}

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HatMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ARobot* Robot = GetWorld()->SpawnActor<ARobot>(RobotClass, GetActorLocation(), GetActorRotation(), SpawnParams);
	AttachToComponent(Robot->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("body"));

	RobotInstance = Robot;

	if (Robot->WeaponClass)
	{
		WeaponInstance = NewObject<UWeaponInstance>(this, UFirearmWeaponInstance::StaticClass());

		if (WeaponInstance)
		{
			WeaponInstance->WeaponRef = Robot->WeaponClass;
			WeaponInstance->BotOwner = this;
			WeaponInstance->Init();
		}
	}

	WeaponMesh->SetVisibility(false);
	WeaponMesh->SetHiddenInGame(true, true);

	if (MeleeCollision)
	{
		MeleeCollision->SetGenerateOverlapEvents(false);
	}

	GetCharacterMovement()->bUseRVOAvoidance = false;

	Robot->GetCharacterMovement()->bOrientRotationToMovement = true;

	SetActorRelativeRotation(Robot->CharacterTransform.GetRotation().Rotator());
	SetActorRelativeLocation(Robot->CharacterTransform.GetLocation());

	Robot->BotOwner = this;

	if (Robot->BotAnimation)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->SetAnimation(Robot->BotAnimation);
	}

	NameWidgetComponent->SetRelativeLocation(Robot->NicknameOffset);

	Robot->OnNewRound(Team);

	if (Robot->HitBoneRotationCurve)
	{
		HitBoneRotationCurve = Robot->HitBoneRotationCurve;
	}

	return Robot;
}

void ABot::MoveToNewLocation(FVector NewLocation)
{
	auto* Robot = GetRobotInstance();

	if (Robot)
	{
		Robot->MoveToNewLocation(NewLocation);
	}
	else
	{
		auto* AIController = GetAIController();

		if (AIController)
		{
			AIController->MoveToNewLocation(NewLocation);
		}
	}

}

void ABot::SetOrientRotationToMovement(bool bNewValue)
{
	if (IsInRobot())
	{
		GetCharacterMovementComponent()->bOrientRotationToMovement = false;
		auto* Robot = GetRobotInstance();

		if (Robot)
		{
			Robot->GetCharacterMovement()->bOrientRotationToMovement = bNewValue;
		}
	}
	else
	{
		GetCharacterMovementComponent()->bOrientRotationToMovement = bNewValue;
	}
}

AActor* ABot::GetBotParentActor()
{
	if (IsInRobot())
	{
		return RobotInstance;
	}

	return this;
}

bool ABot::IsZombie()
{
	return Team == EBotTeam::Zombie;
}

ABot* ABot::CreateZombie(UWorld* World, TSubclassOf<ABot> Subclass)
{
	UChattersGameSession*  GameSessionObject = UChattersGameSession::Get();
	if (!GameSessionObject)
	{
		return nullptr;
	}

	auto& ZombiePortals = GameSessionObject->ZombiePortals;

	FVector SpawnLocation = FVector(0.0f, 0.0f, 150.0f);
	
	if (ZombiePortals.Num())
	{
		auto ZombiePortal = ZombiePortals[FMath::RandRange(0, ZombiePortals.Num() - 1)];
		
		SpawnLocation = ZombiePortal->GetActorLocation();
		SpawnLocation.Z += 50.0f;
	}

	FActorSpawnParameters SpawnParams;
	//SpawnParams.Name = FName(*FString::Printf(TEXT("Zombie_"), IDToSet));
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ABot* Bot = World->SpawnActor<ABot>(Subclass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);


	if (Bot)
	{
		Bot->bReady = true;
		Bot->Team = EBotTeam::Zombie;
		GameSessionObject->Zombies.Add(Bot);
		Bot->Init(TEXT("Zombie_"), -1);
		Bot->FindNewEnemyTarget();
		Bot->bCanSpawnBlood = false;
	}

	return Bot;
}

float ABot::GetDefaultSpeed()
{
	if (IsZombie())
	{
		return 600.0f;
	}

	if (GetGameSession()->GameModeType == ESessionGameMode::Zombie)
	{
		return 450.0f;
	}

	return 600.0f;
}
