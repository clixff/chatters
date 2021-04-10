// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "../Core/ChattersGameInstance.h"
#include "PlayerPawnController.h"

// Sets default values
APlayerPawn::APlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	this->SetRootComponent(this->SphereCollision);

	this->CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	this->CameraBoom->SetupAttachment(this->SphereCollision);
	
	this->Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	this->Camera->SetupAttachment(this->CameraBoom);

	this->MovementComponent = CreateDefaultSubobject<UPawnMovementComponent, UFloatingPawnMovement>(TEXT("MovementComponent"));
	this->MovementComponent->UpdatedComponent = this->SphereCollision;

	this->AIControllerClass = APlayerPawnController::StaticClass();
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (this->bReady)
	{
		this->UpdateBotNicknameWidgetsSize();
	}
}

void APlayerPawn::AttachToBot(ABot* Bot)
{
	if (Bot)
	{
		this->BotToAttach = Bot;

		this->bAttachedToBot = true;


		FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true);

		if (this->CameraBoom)
		{
			this->CameraBoom->TargetArmLength = this->DefaultAttachedZoom;
			this->CameraBoom->SetRelativeRotation(FRotator(-30.0f, 0.0f, 0.0f));
		}

		UE_LOG(LogTemp, Display, TEXT("[APlayerPawn] Set pawn rotation %s"), *(this->BotToAttach->GetActorRotation().ToString()));

		this->AttachToActor(this->BotToAttach, TransformRules);

		if (this->Controller)
		{
			this->Controller->SetControlRotation(this->BotToAttach->GetActorRotation());
		}
	}
}

void APlayerPawn::DetachFromBot()
{
	if (this->bAttachedToBot)
	{
		this->bAttachedToBot = false;

		if (this->BotToAttach)
		{
			FVector CameraLocation = this->BotToAttach->GetActorLocation();
			FRotator CameraRotation = this->BotToAttach->GetActorRotation();

			this->BotToAttach = nullptr;

			if (this->Camera)
			{
				CameraLocation = this->Camera->GetComponentLocation();;
				CameraRotation = this->Camera->GetComponentRotation();
			}

			if (this->CameraBoom)
			{
				this->CameraBoom->TargetArmLength = 0.0f;
				this->CameraBoom->SetRelativeRotation(FRotator(0.0f));
			}

			this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

			this->TeleportTo(CameraLocation, CameraRotation);

			if (this->Controller)
			{
				this->Controller->SetControlRotation(CameraRotation);
			}
		}
	}
}

void APlayerPawn::UpdateBotNicknameWidgetsSize()
{
	if (!this->GameSession)
	{
		auto* GameInstance = UChattersGameInstance::Get();
		if (!GameInstance || GameInstance->GetIsInMainMenu())
		{
			return;
		}

		this->GameSession = GameInstance->GetGameSession();

		if (!this->GameSession)
		{
			return;
		}
	}

	TArray<ABot*> Bots = this->GameSession->Bots;

	if (!Bots.Num())
	{
		return;
	}

	if (!this->Camera)
	{
		return;
	}

	for (int32 i = 0; i < Bots.Num(); i++)
	{
		ABot* Bot = Bots[i];

		if (!Bot)
		{
			continue;
		}

		FVector const CameraLocation = this->Camera->GetComponentToWorld().GetLocation();
		FVector const BotLocation = Bot->GetActorLocation();
		float const DistanceFromBot = FVector::Dist(CameraLocation, BotLocation);

		auto* BotNameWidgetComponent = Bot->NameWidgetComponent;

		if (!BotNameWidgetComponent)
		{
			continue;
		}

		auto* BotNameWidget = Cast<UBotNameWidget>(BotNameWidgetComponent->GetWidget());

		if (BotNameWidget)
		{
			float const ClampedDistance = FMath::Clamp(DistanceFromBot, BotNameWidget->MinDistanceToScale, BotNameWidget->MaxDistanceToScale);
			float const DistanceAlpha = (ClampedDistance - BotNameWidget->MinDistanceToScale) / (BotNameWidget->MaxDistanceToScale - BotNameWidget->MinDistanceToScale);

			float NewWidgetSize = FMath::Lerp(BotNameWidget->MaxWrapperScale, BotNameWidget->MinWrapperScale, DistanceAlpha);

			BotNameWidget->UpdateSize(NewWidgetSize);
		}

	}

}


