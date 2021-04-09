// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot.h"


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
			this->ApplyDamage(15);
			this->bMovingToRandomLocation = false;
			this->MoveToRandomLocation();
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
	if (!this->NameWidgetComponent)
	{
		return nullptr;
	}

	auto* NameWidgetObject = this->NameWidgetComponent->GetWidget();

	if (!NameWidgetObject)
	{
		return nullptr;
	}

	return Cast<UBotNameWidget>(NameWidgetObject);
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

	auto* NameWidget = this->GetNameWidget();

	if (NameWidget)
	{
		NameWidget->Nickname = this->DisplayName;
		//NameWidget->UpdateHealth(this->GetHeathValue());
	}

	this->MoveToRandomLocation();
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
	if (Damage < 1)
	{
		return;
	}

	auto OldHP = this->HealthPoints;

	this->HealthPoints -= Damage;

	if (this->HealthPoints < 0)
	{
		this->HealthPoints = 0;
		this->bAlive = false;
	}

	UE_LOG(LogTemp, Display, TEXT("[ABot] Applying %d damage to bot. Old hp: %d. New HP: %d"), Damage, OldHP, this->HealthPoints);

	auto* NameWidget = this->GetNameWidget();

	if (NameWidget)
	{
		float HealthValue = this->GetHeathValue();
		NameWidget->UpdateHealth(HealthValue);
	}
}

float ABot::GetHeathValue()
{
	return (float(this->HealthPoints) / float(this->MaxHealthPoints));
}