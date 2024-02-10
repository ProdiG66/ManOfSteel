// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "Actors/BaseHero.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/Combat.h"
#include "Components/TargetLock.h"
#include "Enumerators/EInputDirection.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/SlateWrapperTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABaseHero::ABaseHero() {
	this->GetCapsuleComponent()->SetCapsuleHalfHeight(96);
	this->GetCapsuleComponent()->SetCapsuleRadius(36);
	GetCharacterMovement()->BrakingSubStepTime = 0.030303;
	GetCharacterMovement()->JumpZVelocity = 1000;
	GetCharacterMovement()->AirControl = 0.5;
	GetCharacterMovement()->MaxFlySpeed = 1500;
	GetCharacterMovement()->BrakingDecelerationFlying = 3500;
	GetCharacterMovement()->RotationRate = FRotator(0, 540, 0);
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bPushForceScaledToMass = true;
	bUseControllerRotationYaw = false;
	GetMesh()->SetRelativeLocation(FVector(0, 0, -100));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABaseHero::BeginPlay() {
	Super::BeginPlay();
}

void ABaseHero::CheckHasMovementInput() {
	const FVector LastInputVector = GetMovementComponent()->GetLastInputVector();
	Stats->SetHasMovementInput(UKismetMathLibrary::NotEqual_VectorVector(LastInputVector, FVector::Zero(), 0.01));
}


FVector ABaseHero::TargetSocket() {
	return Stats->GetIsAiming()
		       ? AimingSocketOffset
		       : (Stats->GetIsSprint() ? SprintSocketOffset : DefaultSocketOffset);
}

float ABaseHero::TargetArmLength() {
	return Stats->GetIsAiming() ? AimingArmLength : DefaultArmLength;
}

bool ABaseHero::CheckCameraPosition() {
	const bool NearlyEqualToArmLength = UKismetMathLibrary::NearlyEqual_FloatFloat(
		SpringArm->TargetArmLength, TargetArmLength(), 0.01
	);

	const bool IsEqualToSocket = UKismetMathLibrary::EqualEqual_VectorVector(
		SpringArm->SocketOffset, TargetSocket(), 0.01
	);
	return NearlyEqualToArmLength && IsEqualToSocket;
}

void ABaseHero::UpdateSpringArmSocketOffset() {
	SpringArm->SocketOffset = UKismetMathLibrary::VInterpTo(
		SpringArm->SocketOffset, TargetSocket(), GetWorld()->DeltaTimeSeconds, 4);
}

void ABaseHero::ResetCameraPosition() {
	SpringArm->SocketOffset = TargetSocket();
	SpringArm->TargetArmLength = TargetArmLength();
	DidResetCameraPosition = true;
}

void ABaseHero::UpdateCamera() {
	//Setting the camera position when aiming
	if (CheckCameraPosition()) {
		//Reset camera position.
		if (!DidResetCameraPosition) {
			ResetCameraPosition();
		}
	}
	else {
		UpdateSpringArmSocketOffset();
		SpringArm->TargetArmLength = UKismetMathLibrary::FInterpTo(
			SpringArm->TargetArmLength, TargetArmLength(), GetWorld()->DeltaTimeSeconds, 4);
		DidResetCameraPosition = false;
	}

	//"Look at Location" setting during aiming
	const FVector Location = GetActorLocation();
	const FVector CharacterForward = GetActorForwardVector();
	const FVector AdjustedForward = Location + DefaultLookAtLocationZ + (CharacterForward * 5000);
	const FVector SocketLocation = GetMesh()->GetSocketLocation(FName("head"));
	const FVector CameraForward = Camera->GetForwardVector();
	const FVector AdjustedSocket = SocketLocation + (CameraForward * 5000);

	const FVector TargetForward = Stats->GetIsAiming() ? AdjustedSocket : AdjustedForward;
	if (UKismetMathLibrary::EqualEqual_VectorVector(Stats->GetLookAtLocation(), TargetForward, 0.01)) {
		if (!DidResetLookAtLocation) {
			ResetLookAtLocation();
		}
	}
	else {
		Stats->SetLookAtLocation(TargetForward);
		DidResetLookAtLocation = false;
	}
}


void ABaseHero::ResetLookAtLocation() {
	const FVector Location = GetActorLocation();
	const FVector CharacterForward = GetActorForwardVector();
	const FVector AdjustedForward = Location + DefaultLookAtLocationZ + (CharacterForward * 5000);
	Stats->SetLookAtLocation(AdjustedForward);
	DidResetLookAtLocation = true;
}

// Called every frame
void ABaseHero::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	CheckGround();
	UpdateHUD();
	CheckHasMovementInput();
	CheckInputDirection();
	UpdateCamera();
	Combat->MoveAxis = MoveAxis;
	Combat->Update(DeltaTime);
	TargetLock->Update(DeltaTime);
}

void ABaseHero::UpdateHUD() {
	if (IsValid(CrosshairWidget.Get())) {
		CrosshairWidget->SetVisibility(Stats->GetIsAiming() ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void ABaseHero::CheckGround() {
	FHitResult HitResult;
	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0, 0, 100);
	IsGrounded = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
}

void ABaseHero::CheckInputDirection() {
	const bool IsLooking = UKismetMathLibrary::EqualEqual_Vector2DVector2D(LookAxis, FVector2d::Zero(), 0.1);
	const float VerticalAxis = IsLooking ? MoveAxis.Y : MoveAxis.Y + (-LookAxis.Y);
	const float HorizontalAxis = IsLooking ? MoveAxis.X : MoveAxis.X + LookAxis.X;

	if (UKismetMathLibrary::InRange_FloatFloat(VerticalAxis, 0.1f, 1.0f)) {
		InputDirection = EInputDirection::Up;
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(VerticalAxis, -1.0f, -0.1f)) {
		InputDirection = EInputDirection::Down;
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(HorizontalAxis, -1.0f, -0.1f)) {
		InputDirection = EInputDirection::Left;
	}
	else if (UKismetMathLibrary::InRange_FloatFloat(HorizontalAxis, 0.1f, 1.0f)) {
		InputDirection = EInputDirection::Right;
	}
	else {
		InputDirection = EInputDirection::None;
	}
}

void ABaseHero::MoveCharacter() {
	AddMovementInput(GetInputAngles(true), MoveAxis.Y, false);
	AddMovementInput(GetInputAngles(false), MoveAxis.X, false);
}

FVector ABaseHero::GetInputAngles(bool XY) {
	const FRotator InRot = FRotator(0, this->GetControlRotation().Yaw, 0);
	return XY ? UKismetMathLibrary::GetForwardVector(InRot) : UKismetMathLibrary::GetRightVector(InRot);
}

void ABaseHero::MoveTriggered(const FInputActionValue& Value) {
	if (!IsAttacking) {
		MoveAxis = Value.Get<FVector2d>();
		MoveCharacter();
	}
}

void ABaseHero::MoveCompleted(const FInputActionValue& Value) {
	MoveAxis = FVector2d::Zero();
	if (!CheckMovementMode(MOVE_Flying) && IsSprintBtnPressed) {
		IsSprintBtnPressed = false;
	}
	if (CheckMovementMode(MOVE_Walking) || CheckMovementMode(MOVE_Falling)) {
		Stats->SetSprintEvent(false);
	}
}

void ABaseHero::LookTriggered(const FInputActionValue& Value) {
	const FVector2d TempLook = Value.Get<FVector2d>();
	LookAxis = FVector2d(TempLook.X, -TempLook.Y);
	this->AddControllerYawInput(LookAxis.X);
	this->AddControllerPitchInput(LookAxis.Y);
}

void ABaseHero::JumpStarted() {
	if (!CheckMovementMode(MOVE_Falling)) {
		if (!CheckMovementMode(MOVE_Flying)) {
			this->Jump();
		}
	}
}

void ABaseHero::JumpCompleted() {
	this->StopJumping();
}

void ABaseHero::AimTriggered() {
	Stats->SetAimingEvent(true);
}

void ABaseHero::AimCompleted() {
	Stats->SetAimingEvent(false);
}

void ABaseHero::DodgeTriggered() {}

void ABaseHero::SprintStarted() {
	if (IsSprintBtnPressed) {
		IsSprintBtnPressed = false;
	}
	else {
		IsSprintBtnPressed = true;
	}
	Stats->SetSprintEvent(IsSprintBtnPressed);
}

void ABaseHero::TargetLockStarted() {
	TargetLock->TargetLock();
}

void ABaseHero::StrikeStarted() {
	if (Combat->CanAttack()) {
		Combat->CachedTarget = Combat->GetCurrentTarget(Combat->CachedObjectType);
		if (IsValid(Combat->CachedTarget.Get())) {
			Combat->TargetUpDown = (Combat->CachedTarget->GetActorLocation().Z - GetOwner()->GetActorLocation().Z) < 0;
			if (Combat->IsWithinStrikeDistance()) {
				if (Combat->IsBeatdownPressed) {
					Combat->Beatdown();
				}
				else {
					Combat->Strike();
				}
				Stats->SetSprintEvent(true);
			}
			else {
				TargetLock->SetTargetLock(Combat->CachedTarget.Get());
				Stats->SetSprintEvent(IsValid(TargetLock->HitActor));
				Combat->HasPendingAttack = true;
				Combat->IsDashingToAttack = true;
			}
		}
	}
}

bool ABaseHero::CheckMovementMode(EMovementMode MovementMode) {
	return GetCharacterMovement()->MovementMode == MovementMode;
}

void ABaseHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		SetupActions(EnhancedInputComponent);
		const ULocalPlayer* LocalPlayer = GetController<APlayerController>()->GetLocalPlayer();
		UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
}

void ABaseHero::SetupActions(UEnhancedInputComponent* EnhancedInputComponent) {
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseHero::MoveTriggered);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ABaseHero::MoveCompleted);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaseHero::LookTriggered);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABaseHero::JumpStarted);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABaseHero::JumpCompleted);
	EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ABaseHero::DodgeTriggered);
	EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ABaseHero::SprintStarted);
	EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ABaseHero::AimTriggered);
	EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABaseHero::AimCompleted);
	EnhancedInputComponent->BindAction(TargetLockAction, ETriggerEvent::Started, this,
	                                   &ABaseHero::TargetLockStarted);
	EnhancedInputComponent->BindAction(StrikeAction, ETriggerEvent::Started, this, &ABaseHero::StrikeStarted);
}
