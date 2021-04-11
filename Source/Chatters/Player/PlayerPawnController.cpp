// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawnController.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "../Core/ChattersGameInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "../Core/ChattersGameSession.h"

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

	if (this->ZoomSeconds > 0.0f)
	{
		this->ZoomTick(DeltaTime);
	}
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

	this->InputComponent->BindAction("Space", IE_Pressed, this, &APlayerPawnController::OnSpacePressed);

	this->InputComponent->BindAction("ZoomUp", IE_Pressed, this, &APlayerPawnController::OnMouseWheelUp);
	this->InputComponent->BindAction("ZoomDown", IE_Pressed, this, &APlayerPawnController::OnMouseWheelDown);
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
	auto* PlayerPawnActor = this->GetPlayerPawn();

	if (PlayerPawnActor && Value != 0.0f)
	{
		if (!PlayerPawnActor->bAttachedToBot)
		{
			this->AddYawInput(Value);
		}
		else
		{
			this->RotateAttachedCamera(ERotationType::Yaw, Value);
		}
	}
}

void APlayerPawnController::TurnY(float Value)
{
	auto* PlayerPawnActor = this->GetPlayerPawn();

	if (PlayerPawnActor && Value != 0.0f)
	{
		if (!PlayerPawnActor->bAttachedToBot)
		{
			this->AddPitchInput(Value);
		}
		else
		{
			this->RotateAttachedCamera(ERotationType::Pitch, Value);
		}
	}
}

void APlayerPawnController::MovePawn(EAxis::Type Axis, float Value)
{
	auto* PlayerPawnActor = this->GetPlayerPawn();
	if (PlayerPawnActor && Value != 0.0f)
	{
		if (PlayerPawnActor->bAttachedToBot)
		{
			PlayerPawnActor->DetachFromBot();
		}

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

void APlayerPawnController::OnSpacePressed()
{
	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		auto* GameSession = GameInstance->GetGameSession();
		
		if (GameSession && !GameSession->bStarted)
		{
			GameSession->Start();
		}
	}
}

void APlayerPawnController::OnMouseWheelUp()
{
	this->Zoom(-1.0f);
}

void APlayerPawnController::OnMouseWheelDown()
{
	this->Zoom(1.0f);
}

void APlayerPawnController::Zoom(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	auto* PlayerPawnActor = this->GetPlayerPawn();

	if (PlayerPawnActor && PlayerPawnActor->bAttachedToBot)
	{
		this->ZoomValue = Value;
		this->ZoomSeconds = this->SecondsForZoom;
	}
}

void APlayerPawnController::ZoomTick(float DeltaTime)
{
	auto* PlayerPawnActor = this->GetPlayerPawn();

	if (PlayerPawnActor && PlayerPawnActor->bAttachedToBot)
	{
		auto* CameraBoom = PlayerPawnActor->CameraBoom;

		if (CameraBoom)
		{
			CameraBoom->TargetArmLength += this->ZoomScale * this->ZoomValue * DeltaTime;

			if (CameraBoom->TargetArmLength < PlayerPawnActor->MinAttachedZoom)
			{
				CameraBoom->TargetArmLength = PlayerPawnActor->MinAttachedZoom;
				this->ZoomSeconds = 0.0f;
			}
			else if (CameraBoom->TargetArmLength > PlayerPawnActor->MaxAttachedZoom)
			{
				CameraBoom->TargetArmLength = PlayerPawnActor->MaxAttachedZoom;
				this->ZoomSeconds = 0.0f;
			}
			else
			{
				this->ZoomSeconds -= DeltaTime;
				if (this->ZoomSeconds < 0.0f)
				{
					this->ZoomSeconds = 0.0f;
				}
			}
		}
	}
	else
	{
		this->ZoomSeconds = 0.0f;
	}
}

void APlayerPawnController::RotateAttachedCamera(ERotationType Type, float Value)
{
	if (Value == 0)
	{
		return;
	}

	auto* PlayerPawnActor = this->GetPlayerPawn();

	if (PlayerPawnActor)
	{
		auto* CameraBoom = PlayerPawnActor->CameraBoom;
		if (CameraBoom)
		{
			FRotator CameraBoomRotation = CameraBoom->GetRelativeRotation();

			float RotationToAdd = Value * this->AttachedCameraRotationScale;

			CameraBoomRotation.Roll = 0.0f;

			if (Type == ERotationType::Yaw)
			{
				CameraBoomRotation.Yaw += RotationToAdd;
			}
			else if (Type == ERotationType::Pitch)
			{
				CameraBoomRotation.Pitch -= RotationToAdd;

				if (CameraBoomRotation.Pitch < this->AttachedCameraMinPitchRotation)
				{
					CameraBoomRotation.Pitch = this->AttachedCameraMinPitchRotation;
				}
				else if (CameraBoomRotation.Pitch > this->AttachedCameraMaxPitchRotation)
				{
					CameraBoomRotation.Pitch = this->AttachedCameraMaxPitchRotation;
				}
			}

			CameraBoom->SetRelativeRotation(CameraBoomRotation);
		}
	}
}