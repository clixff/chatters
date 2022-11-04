// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawnController.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "../Core/ChattersGameInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "../Character/Bot.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/KismetMathLibrary.h"
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
	this->InputComponent->BindAxis("MoveUp", this, &APlayerPawnController::MoveUp);

	this->InputComponent->BindAxis("TurnX", this, &APlayerPawnController::TurnX);
	this->InputComponent->BindAxis("TurnY", this, &APlayerPawnController::TurnY);

	this->InputComponent->BindAction("Shift", IE_Pressed, this, &APlayerPawnController::OnShiftPressed);
	this->InputComponent->BindAction("Shift", IE_Released, this, &APlayerPawnController::OnShiftReleased);

	this->InputComponent->BindAction("Space", IE_Pressed, this, &APlayerPawnController::OnSpacePressed);

	this->InputComponent->BindAction("ZoomUp", IE_Pressed, this, &APlayerPawnController::OnMouseWheelUp);
	this->InputComponent->BindAction("ZoomDown", IE_Pressed, this, &APlayerPawnController::OnMouseWheelDown);

	this->InputComponent->BindAction("LeftMouseClick", IE_Pressed, this, &APlayerPawnController::OnLeftMouseClick);
	this->InputComponent->BindAction("RightMouseClick", IE_Pressed, this, &APlayerPawnController::OnRightMouseClick);

	this->InputComponent->BindAction("Slomo", IE_Pressed, this, &APlayerPawnController::OnSlowmoStart);
	this->InputComponent->BindAction("Slomo", IE_Released, this, &APlayerPawnController::OnSlowmoEnd);

	this->InputComponent->BindAction("Esc", IE_Pressed, this, &APlayerPawnController::OnEscPressed);
	this->InputComponent->BindAction("GameJoin", IE_Pressed, this, &APlayerPawnController::OnGameJoinPressed);
	this->InputComponent->BindAction("Respawn", IE_Pressed, this, &APlayerPawnController::OnRespawnBotPressed);

	this->InputComponent->BindAction("CameraChange", IE_Pressed, this, &APlayerPawnController::OnCameraButtonPressed);

	this->InputComponent->BindAction("CinematicCamera", IE_Pressed, this, &APlayerPawnController::ToggleCinematicCamera);



	for (int32 i = 0; i < 5; i++)
	{
		FString KeyID = FString::Printf(TEXT("Key_%d"), i+1);

		this->InputComponent->BindAction<FSelectLeaderboardBotDelegate>(*KeyID, IE_Pressed, this, &APlayerPawnController::SelectLeaderboardBot, i);
	}

;}

void APlayerPawnController::BeginPlay()
{
	APlayerController::BeginPlay();

	this->UpdateMaxMovementSpeed(this->MaxMovementSpeed);
}

void APlayerPawnController::MoveForward(float Value)
{
	MovePawn(EAxis::Type::X, Value);
}

void APlayerPawnController::MoveRight(float Value)
{
	MovePawn(EAxis::Type::Y, Value);
}

void APlayerPawnController::MoveUp(float Value)
{
	MovePawn(EAxis::Type::Z, Value);
}

void APlayerPawnController::TurnX(float Value)
{
	if (!this->bCanControl)
	{
		return;
	}
	auto* PlayerPawnActor = this->GetPlayerPawn();

	if (PlayerPawnActor && Value != 0.0f)
	{
		if (PlayerPawnActor->IsCinematicCameraEnabled())
		{
			return;
		}

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
	if (!this->bCanControl)
	{
		return;
	}

	auto* PlayerPawnActor = this->GetPlayerPawn();

	if (PlayerPawnActor && Value != 0.0f)
	{
		if (PlayerPawnActor->IsCinematicCameraEnabled())
		{
			return;
		}

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
	if (!this->bCanControl)
	{
		return;
	}
	auto* PlayerPawnActor = this->GetPlayerPawn();
	if (PlayerPawnActor && Value != 0.0f)
	{
		if (PlayerPawnActor->bAttachedToBot)
		{
			PlayerPawnActor->DetachFromBot();
		}

		if (PlayerPawnActor->IsCinematicCameraEnabled())
		{
			if (!PlayerPawnActor->BlockControlsOnCinematicCameraTimer.IsEnded())
			{
				return;
			}
			PlayerPawnActor->DeactivateCinematicCamera();
		}

		FVector DirectionVector;

		if (Axis != EAxis::Type::Z)
		{
			FRotator const ControlSpaceRotation = this->GetControlRotation();
			DirectionVector = FRotationMatrix(ControlSpaceRotation).GetScaledAxis(Axis);
		}
		else
		{
			DirectionVector = FVector::UpVector;
		}

		PlayerPawnActor->AddMovementInput(DirectionVector, Value, true);
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
	if (!this->bCanControl)
	{
		return;
	}

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
	if (!this->bCanControl)
	{
		return;
	}

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance && !GameInstance->GetIsInMainMenu())
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

	if (!this->bCanControl)
	{
		return;
	}

	auto* PlayerPawnActor = this->GetPlayerPawn();

	if (PlayerPawnActor && PlayerPawnActor->bAttachedToBot)
	{
		if (PlayerPawnActor->bFirstPersonCamera)
		{
			if (Value > 0.0f)
			{
				PlayerPawnActor->LastZoomValue = PlayerPawnActor->MinAttachedZoom;
				PlayerPawnActor->SetThirdPersonCamera();
			}

			return;
		}
		else
		{
			if (Value < 0.0f && PlayerPawnActor->LastZoomValue <= PlayerPawnActor->MinAttachedZoom)
			{
				PlayerPawnActor->SetFirstPersonCamera();
				return;
			}
		}

		this->ZoomValue = Value;
		this->ZoomSeconds = this->SecondsForZoom;
	}
}

void APlayerPawnController::ZoomTick(float DeltaTime)
{
	auto* PlayerPawnActor = this->GetPlayerPawn();

	if (PlayerPawnActor && PlayerPawnActor->bAttachedToBot && !PlayerPawnActor->bFirstPersonCamera)
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

			PlayerPawnActor->LastZoomValue = CameraBoom->TargetArmLength;
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

	if (!this->bCanControl)
	{
		return;
	}

	auto* PlayerPawnActor = this->GetPlayerPawn();

	if (PlayerPawnActor)
	{
		auto* CameraBoom = PlayerPawnActor->CameraBoom;
		if (PlayerPawnActor->bFirstPersonCamera)
		{
			return;
		}

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

void APlayerPawnController::OnLeftMouseClick()
{
	this->AttachPlayerToAliveBot(EAttachCameraToBotType::NextBot);
}

void APlayerPawnController::OnRightMouseClick()
{
	this->AttachPlayerToAliveBot(EAttachCameraToBotType::PrevBot);
}

void APlayerPawnController::AttachPlayerToAliveBot(EAttachCameraToBotType Type)
{
	if (!this->bCanControl)
	{
		return;
	}

	auto* GameInstance = UChattersGameInstance::Get();

	if (!GameInstance || GameInstance->GetIsInMainMenu())
	{
		return;
	}

	auto* PlayerActor = this->GetPlayerPawn();

	if (PlayerActor)
	{
		int32 AttachedToBotID = -1;

		if (PlayerActor->bAttachedToBot && PlayerActor->BotToAttach)
		{
			AttachedToBotID = PlayerActor->BotToAttach->ID;
		}

		if (PlayerActor->IsCinematicCameraEnabled())
		{
			PlayerActor->DeactivateCinematicCamera();
		}

		auto* GameSession = GameInstance->GetGameSession();

		if (GameSession)
		{
			GameSession->AttachPlayerToAliveBot(Type, AttachedToBotID);
		}
	}
}

void APlayerPawnController::OnSlowmoStart()
{
	auto* GameSession = UChattersGameSession::Get();

	GameSession->SetSlomoEnabled(true);
}

void APlayerPawnController::OnSlowmoEnd()
{
	auto* GameSession = UChattersGameSession::Get();

	GameSession->SetSlomoEnabled(false);
}

void APlayerPawnController::OnEscPressed()
{
	auto* GameInstance = UChattersGameInstance::Get();

	if (!GameInstance)
	{
		return;
	}

	if (GameInstance->GetIsInMainMenu())
	{
		//
	}
	else
	{
		auto* GameSession = GameInstance->GetGameSession();

		if (GameSession)
		{
			auto* PauseMenuWidget = GameSession->GetPauseMenuWidget();

			if (!PauseMenuWidget || PauseMenuWidget->bAnimationPlaying)
			{
				return;
			}

			if (GameInstance->GetIsGamePaused())
			{
				PauseMenuWidget->OnEscPressed();
			}
			else
			{
				GameSession->PauseGame();
			}
		}
	}
}

void APlayerPawnController::OnGameJoinPressed()
{
	if (!this->bCanControl)
	{
		return;
	}

	auto* GameSession = UChattersGameSession::Get();

	if (!GameSession)
	{
		return;
	}

	if (GameSession->SessionType != ESessionType::Twitch || !GameSession->bCanViewersJoin)
	{
		return;
	}

	auto* GameInstance = UChattersGameInstance::Get();

	if (!GameInstance)
	{
		return;
	}

	auto AuthData = GameInstance->TwitchAuthData;

	if (!AuthData.bSignedIn || AuthData.DisplayName.IsEmpty())
	{
		return;
	}


	ABot* Bot = GameSession->OnViewerJoin(AuthData.DisplayName);

	auto* PlayerPawnRef = this->GetPlayerPawn();

	if (Bot && PlayerPawnRef && GameSession->Bots.Num() > 1)
	{
		if (PlayerPawnRef->IsCinematicCameraEnabled())
		{
			PlayerPawnRef->ActivateCinematicCamera(Bot, true);
		}
		else
		{
			PlayerPawnRef->AttachToBot(Bot);
		}
	}
	
	auto* SessionWidget = GameSession->GetSessionWidget();

	if (SessionWidget)
	{
		SessionWidget->SetStreamerJoinTipVisible(false);
	}
}

void APlayerPawnController::OnRespawnBotPressed()
{
	if (!this->bCanControl)
	{
		return;
	}

	auto* PlayerPawnRef = this->GetPlayerPawn();

	if (!PlayerPawnRef)
	{
		return;
	}

	PlayerPawnRef->RespawnAttachedBot();
}

void APlayerPawnController::SelectLeaderboardBot(int32 Index)
{
	if (!bCanControl)
	{
		return;
	}

	auto* GameSession = UChattersGameSession::Get();

	if (GameSession)
	{
		if (GameSession->GameModeType == ESessionGameMode::Deathmatch || GameSession->GameModeType == ESessionGameMode::Zombie)
		{
			GameSession->SelectDeathmatchLeader(Index);
		}
	}
}

void APlayerPawnController::OnCameraButtonPressed()
{
	if (!bCanControl)
	{
		return;
	}

	auto* PlayerPawnRef = GetPlayerPawn();
	if (PlayerPawnRef->bFirstPersonCamera)
	{
		PlayerPawnRef->SetThirdPersonCamera();
	}
	else
	{
		if (PlayerPawnRef->IsCinematicCameraEnabled())
		{
			PlayerPawnRef->DeactivateCinematicCamera(true);
		}
		PlayerPawnRef->SetFirstPersonCamera();
	}
}

void APlayerPawnController::ToggleCinematicCamera()
{
	if (!bCanControl)
	{
		return;
	}

	auto* PlayerPawnRef = GetPlayerPawn();

	if (PlayerPawnRef)
	{
		if (PlayerPawnRef->IsCinematicCameraEnabled())
		{
			PlayerPawnRef->DeactivateCinematicCamera(true);
		}
		else
		{
			AActor* TargetActor = PlayerPawnRef->BotToAttach;

			if (!TargetActor)
			{
				FVector CameraLocation = PlayerPawnRef->GetActorLocation();
				float MinDistance = -1.0f;
				
				auto* GameSession = UChattersGameSession::Get();

				if (GameSession)
				{
					for (auto* Bot : GameSession->AliveBots)
					{
						if (Bot)
						{
							float Dist = FVector::Dist(CameraLocation, Bot->GetActorLocation());

							if (MinDistance == -1.0f || Dist < MinDistance)
							{
								MinDistance = Dist;
								TargetActor = Bot;
							}
						}
					}
				}
			}

			PlayerPawnRef->ActivateCinematicCamera(TargetActor);
		}
	}
}

void APlayerPawnController::SetMouseSensitivity(int32 Sensitivity)
{
	const float MinSensitivityFloat = 0.0001f;
	const float MaxSensitivityFloat = 0.45f;

	const int32 MinSensitivity = 1;
	const int32 MaxSensitivity = 100;

	Sensitivity = FMath::Clamp(Sensitivity, MinSensitivity, MaxSensitivity);

	float SensitivityScale = UKismetMathLibrary::NormalizeToRange(Sensitivity, 1, 100);

	float NewSensitivity = FMath::Lerp(MinSensitivityFloat, MaxSensitivityFloat, SensitivityScale);

	UE_LOG(LogTemp, Display, TEXT("[APlayerPawnController] Set mouse sensivitiy to %f"), NewSensitivity);

	if (this->PlayerInput)
	{
		this->PlayerInput->SetMouseSensitivity(NewSensitivity);
	}
}
