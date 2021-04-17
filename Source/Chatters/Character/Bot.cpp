// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot.h"
#include "Components/CapsuleComponent.h"
#include "../Core/ChattersGameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BotSpawnPoint.h"
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

	this->NameWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidget"));
	this->NameWidgetComponent->SetupAttachment(this->GetMesh());
	this->NameWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	this->NameWidgetComponent->SetWidgetClass(UBotNameWidget::StaticClass());
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
		if (this->bMovingToRandomLocation)
		{
			float DistToTarget = FVector::Dist(this->GetActorLocation(), this->RandomLocationTarget);

			if (DistToTarget <= 150.0f)
			{
				this->ApplyDamage(50);
				this->bMovingToRandomLocation = false;
				this->SayRandomMessage();
				this->MoveToRandomLocation();
			}
		}

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
	if (!this->GameSession)
	{
		auto* GameInstance = UChattersGameInstance::Get();
		if (GameInstance)
		{
			this->GameSession = GameInstance->GetGameSession();
		}
	}

	return this->GameSession;
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
		Bot->GameSession = GameSessionObject;
		Bot->Init(NameToSet, IDToSet);
	}

	return Bot;
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

void ABot::ApplyDamage(int32 Damage)
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
		this->OnDead();
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

void ABot::OnDead()
{
	this->bAlive = false;
	this->HealthPoints = 0;

	this->SetActorLocation(this->GetActorLocation());

	if (this->GetMesh())
	{
		this->GetMesh()->SetSimulatePhysics(true);
		this->GetMesh()->SetCollisionProfileName(FName(TEXT("DeadBody")), true);
	}

	if (this->GetCapsuleComponent())
	{
		this->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
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

	this->MoveToRandomLocation();
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