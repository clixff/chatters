// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawnController.h"
#include "GameFramework/FloatingPawnMovement.h"

APlayerPawnController::APlayerPawnController()
{

}

APlayerPawnController::~APlayerPawnController()
{

}

APlayerPawn* APlayerPawnController::GetPlayerPawn()
{
	if (this->PlayerPawn == nullptr)
	{
		this->PlayerPawn = Cast<APlayerPawn>(this->GetPawn());
	}

	return this->PlayerPawn;
}

void APlayerPawnController::PlayerTick(float DeltaTime)
{
	APlayerController::PlayerTick(DeltaTime);
}

void APlayerPawnController::SetupInputComponent()
{
	APlayerController::SetupInputComponent();

	UE_LOG(LogTemp, Display, TEXT("[APlayerPawnController] SetupInputComponent"));

	this->InputComponent->BindAxis("MoveForward", this, &APlayerPawnController::MoveForward);
	this->InputComponent->BindAxis("MoveRight", this, &APlayerPawnController::MoveRight);

	this->InputComponent->BindAxis("TurnX", this, &APlayerPawnController::TurnX);
	this->InputComponent->BindAxis("TurnY", this, &APlayerPawnController::TurnY);

	this->InputComponent->BindAction("Shift", IE_Pressed, this, &APlayerPawnController::OnShiftPressed);
	this->InputComponent->BindAction("Shift", IE_Released, this, &APlayerPawnController::OnShiftReleased);
;}

void APlayerPawnController::BeginPlay()
{
	APlayerController::BeginPlay();

	this->UpdateMaxMovementSpeed(this->MaxMovementSpeed);
}

void APlayerPawnController::MoveForward(float Value)
{
	this->MovePawn(EAxis::Type::X, Value);
}

void APlayerPawnController::MoveRight(float Value)
{
	this->MovePawn(EAxis::Type::Y, Value);
}

void APlayerPawnController::TurnX(float Value)
{
	if (Value != 0.0f)
	{
		//UE_LOG(LogTemp, Display, TEXT("[APlayerPawnController] Turn X. Value: %f"), Value);
		this->AddYawInput(Value);
	}
}

void APlayerPawnController::TurnY(float Value)
{
	if (Value != 0.0f)
	{
		//UE_LOG(LogTemp, Display, TEXT("[APlayerPawnController] Turn Y. Value: %f"), Value);
		this->AddPitchInput(Value);
	}
}

void APlayerPawnController::MovePawn(EAxis::Type Axis, float Value)
{
	auto* PlayerPawnActor = this->GetPlayerPawn();
	if (PlayerPawnActor && Value != 0.0f)
	{
		FString AxisString = Axis == EAxis::Type::X ? TEXT("X") : TEXT("Y");
		FRotator const ControlSpaceRotation = this->GetControlRotation();
		FVector WorldDirection = FRotationMatrix(ControlSpaceRotation).GetScaledAxis(Axis);

		//UE_LOG(LogTemp, Display, TEXT("[APlayerPawnController] Move player pawn on %s. Value: %f. Direction: %s. Rotation: %s"), *AxisString, Value, *(WorldDirection.ToString()), *(ControlSpaceRotation.ToString()));

		PlayerPawnActor->AddMovementInput(WorldDirection, Value, true);
	}
}

void APlayerPawnController::OnShiftPressed()
{
	this->bShiftPressed = true;
	this->UpdateMaxMovementSpeed(this->MaxMovementSpeedWithShift);
}

void APlayerPawnController::OnShiftReleased()
{
	this->bShiftPressed = false;
	this->UpdateMaxMovementSpeed(this->MaxMovementSpeed);
}

void APlayerPawnController::UpdateMaxMovementSpeed(float MaxSpeed)
{
	auto* PlayerPawnActor = this->GetPlayerPawn();

	if (PlayerPawnActor)
	{
		auto* MovementComponent = Cast<UFloatingPawnMovement>(PlayerPawnActor->GetMovementComponent());

		if (MovementComponent)
		{
			MovementComponent->MaxSpeed = MaxSpeed;
		}
	}
}
