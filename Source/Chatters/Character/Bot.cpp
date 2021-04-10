// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot.h"
#include "Components/CapsuleComponent.h"
#include "../Core/ChattersGameInstance.h"
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
	this->HatMesh->SetupAttachment(this->HeadMesh, TEXT("head1"));

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

ABot* ABot::CreateBot(UWorld* World, FString NameToSet, uint32 IDToSet, TSubclassOf<ABot> Subclass)
{
	FVector BotPosition(0, float(IDToSet * 300), 100);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*FString::Printf(TEXT("Bot_%d"), IDToSet));
	SpawnParams.bNoFail = true;
	ABot* Bot = World->SpawnActor<ABot>(Subclass, BotPosition, FRotator(0), SpawnParams);

	if (Bot)
	{
		Bot->Init(NameToSet, IDToSet);
	}

	return Bot;
}


void ABot::SetOutfit()
{

}

void ABot::Init(FString NewName, uint32 NewID)
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

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		auto* GameSession = GameInstance->GetGameSession();

		if (GameSession)
		{
			GameSession->OnBotDied(this->ID);
		}
	}

}

void ABot::OnGameSessionStarted()
{
	this->bReady = true;

	this->MoveToRandomLocation();
}