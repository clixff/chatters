// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot.h"
#include "Components/CapsuleComponent.h"
#include "../Core/ChattersGameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BotSpawnPoint.h"
#include "Equipment/Weapon/Instances/MeleeWeaponInstance.h"
#include "Equipment/Weapon/Instances/FirearmWeaponInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Misc/MathHelper.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "../Core/ChattersGameSession.h"


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

			this->CombatTick(DeltaTime);
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

void ABot::SetNewEnemyTarget(ABot* Target)
{
	this->TargetTo = Target;
	this->CombatAction = ECombatAction::IDLE;
}

void ABot::MoveToTarget()
{
	if (!this->TargetTo)
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
	FVector TargetLocation = this->TargetTo->GetActorLocation();

	AIController->MoveToLocation(TargetLocation);
}

void ABot::CombatTick(float DeltaTime)
{
	if (this->WeaponInstance->WeaponRef)
	{
		if (this->TargetTo)
		{
			if (!this->TargetTo->bAlive)
			{
				this->FindNewEnemyTarget();
			}
			else
			{
				float TargetDist = FVector::Dist(this->GetActorLocation(), this->TargetTo->GetActorLocation());

				//UE_LOG(LogTemp, Display, TEXT("[ABot] Dist is %f"), TargetDist);

				EWeaponType WeaponType = this->WeaponInstance->WeaponRef->Type;

				if (WeaponType == EWeaponType::Firearm)
				{
					if (TargetDist <= 700.0f)
					{
						if (this->CombatAction == ECombatAction::Moving)
						{
							UE_LOG(LogTemp, Display, TEXT("[ABot] Stop moving"));

							auto* AIController = this->GetAIController();

							if (AIController)
							{
								AIController->StopMovement();
							}
						}
						
						this->CombatAction = ECombatAction::Shooting;
						
						float OldYawRotation = this->GetActorRotation().Yaw;
						float NewYawRotation = UKismetMathLibrary::FindLookAtRotation(this->WeaponMesh->GetComponentLocation(), this->TargetTo->GetActorLocation()).Yaw;

						float YawDiff = FMath::Abs(FMath::Fmod((OldYawRotation - NewYawRotation) + 180.0f, 360.0f) - 180.0f);

						if (YawDiff >= 1.0f)
						{
							this->SetActorRotation(FRotator(0.0f, NewYawRotation, 0.0f));

							UE_LOG(LogTemp, Display, TEXT("[ABot] Set rotation yaw %f. Old yaw is %f. YawDiff is %f."), NewYawRotation, OldYawRotation, YawDiff);
						}

						this->Shoot();

					}
					else
					{
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

}

void ABot::Shoot()
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
			FVector OutLocation;
			FRotator OutRotation;
			this->WeaponMesh->GetSocketWorldLocationAndRotation(TEXT("OutBullet"), OutLocation, OutRotation);

			float YawRotation = this->GetActorRotation().Yaw;
			FRotator GunRotation = FRotator(FMath::RandRange(-5.0f, 5.0f), YawRotation + FMath::RandRange(-5.0f, 5.0f), 0.0f);

			FVector EndLocation = OutLocation + (GunRotation.Vector() * FirearmRef->MaxDistance);

			UWorld* World = this->GetWorld();

			if (!World)
			{
				return;
			}

			FirearmInstance->OnShoot();
			
			//EndLocation = OutLocation + FVector(0.0f, 0.0f, 150.0f);

			FHitResult HitResult;
			World->LineTraceSingleByChannel(HitResult, OutLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel3);


			DrawDebugLine(World, OutLocation, HitResult.TraceEnd, FColor(255, 0, 0), false, 0.5f);

			if (HitResult.bBlockingHit)
			{
				DrawDebugSphere(World, HitResult.ImpactPoint, 10.0f, 2, FColor(0, 255, 0), false, 0.5f);
				auto* Actor = HitResult.Actor.Get();
				if (Actor)
				{
					ABot* BotToDamage = Cast<ABot>(Actor);
					if (BotToDamage && BotToDamage->ID != this->ID)
					{
						FVector ImpulseVector = this->GetActorForwardVector() * FirearmRef->ImpulseForce;
						BotToDamage->ApplyDamage(FirearmInstance->GetDamage(), this, WeaponType, ImpulseVector, HitResult.ImpactPoint, HitResult.BoneName);
					}
				}
			}


			if (FirearmRef->ShootSound)
			{
				UGameplayStatics::PlaySoundAtLocation(World, FirearmRef->ShootSound, OutLocation, FMath::RandRange(0.5f, 0.7f));
			}
		}
	}
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
						this->WeaponInstance->Bot = this;
						this->WeaponInstance->Init();
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

void ABot::ApplyDamage(int32 Damage, ABot* ByBot, EWeaponType WeaponType, FVector ImpulseVector, FVector ImpulseLocation, FName BoneHit)
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

	if (Killer)
	{
		if (WeaponType == EWeaponType::Firearm)
		{
			this->GetMesh()->AddImpulseAtLocation(ImpulseVector, ImpulseLocation, BoneHit);
		}
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

	auto* CharacterMovementComponent = Cast<UCharacterMovementComponent>(this->GetMovementComponent());

	if (CharacterMovementComponent)
	{
		CharacterMovementComponent->bUseRVOAvoidance = false;
	}

	auto* GameSessionObject = this->GetGameSession();

	if (GameSessionObject)
	{
		GameSessionObject->OnBotDied(this->ID);
	}
}

void ABot::OnGameSessionStarted()
{
	this->bReady = true;

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
			//this->HatMesh->DetachFromParent(true, false);
			//this->HatMesh->AttachToComponent(this->HeadMesh, FAttachmentTransformRules::KeepRelativeTransform);
			this->HatMesh->SetCollisionProfileName(FName(TEXT("OufitPhysics")), true);
			this->HatMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
			this->HatMesh->SetSimulatePhysics(true);
		}
	}
}