// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "Actors/Superman.h"

#include "VFX/AfterimageVFX.h"
#include "EnhancedInputComponent.h"
#include "Components/FlightCombat.h"
#include "Components/KryptonianStats.h"
#include "Blueprint/UserWidget.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

class UEnhancedInputLocalPlayerSubsystem;

ASuperman::ASuperman() {
	CapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	CapsuleCollider->SetVisibility(true);
	CapsuleCollider->SetHiddenInGame(true);
	CapsuleCollider->SetCapsuleHalfHeight(100);
	CapsuleCollider->SetCapsuleRadius(40);
	PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Physics Constraint"));
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(this->GetCapsuleComponent());
	SpringArm->bUsePawnControlRotation = true;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	TargetLock = CreateDefaultSubobject<UTargetLock>(TEXT("Target Lock"));
	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("Motion Warping"));
	Stats = CreateDefaultSubobject<UKryptonianStats>(TEXT("Kryptonian Stats"));
	Flight = CreateDefaultSubobject<UFlight>(TEXT("Flight"));
	HeatVision = CreateDefaultSubobject<UHeatVision>(TEXT("Heat Vision"));
	BurnVfx = CreateDefaultSubobject<UBurnVFX>(TEXT("Burn VFX"));
	Combat = CreateDefaultSubobject<UFlightCombat>(TEXT("Flight Combat"));
	AfterimageVfx = CreateDefaultSubobject<UAfterimageVFX>(TEXT("Afterimage VFX"));
	SmearVfx = CreateDefaultSubobject<USmearVFX>(TEXT("Smear VFX"));

	LaserEyeLeft = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Laser Eye Left"));
	LaserEyeRight = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Laser Eye Right"));
	LaserDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("Laser Direction"));
	LaserEyeLeft->SetupAttachment(GetMesh(), "Laser_L");
	LaserEyeRight->SetupAttachment(GetMesh(), "Laser_R");
	LaserDirection->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void ASuperman::BeginPlay() {
	Super::BeginPlay();
	AddCrosshair();
}

void ASuperman::AddCrosshair() {
	if (CrosshairWidgetClass) {
		CrosshairWidget = CreateWidget<UUserWidget>(GetController<APlayerController>(), CrosshairWidgetClass);
		check(CrosshairWidget.Get());
		CrosshairWidget->AddToViewport();
	}
}


void ASuperman::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	Flight->Update(DeltaTime);
	HeatVision->Update(DeltaTime);
	FastFlightMoveCharacter();
	const bool EnableSmear = Combat->GetIsAttacking() && !Flight->GetIsSuperheroLanding();
	const bool IsFastFlying = Flight->GetIsFlying() && Stats->GetIsSprint();
	const float TargetAmount = IsFastFlying ? 300 : 100;
	SmearVfx->Update(DeltaTime, EnableSmear, TargetAmount);
	FlyUp();
	AfterimageVfx->Update(DeltaTime);
}

void ASuperman::FlyUp() {
	if (Flight->GetGoUp()) {
		if (GetActorRotation().Pitch < 76) {
			AddControllerPitchInput(-10);
		}
		else {
			Flight->SetGoUp(false);
		}
	}
}

void ASuperman::FastFlightMoveCharacter() {
	const bool IsMoving = UKismetMathLibrary::NotEqual_Vector2DVector2D(MoveAxis, FVector2d::Zero(), 0.1);
	if (!IsMoving) {
		if (Flight->GetIsFlying() && Stats->GetIsSprint()) {
			const FVector ControlRotation = UKismetMathLibrary::Conv_RotatorToVector(GetControlRotation());
			const float NegRand = UKismetMathLibrary::InRange_FloatFloat(
				                      ControlRotation.Z, -1, -0.1, true, true
			                      )
				                      ? ControlRotation.Z
				                      : 0;
			const float PosRand = UKismetMathLibrary::InRange_FloatFloat(
				                      ControlRotation.Z, 0.1, 1, true, true
			                      )
				                      ? ControlRotation.Z
				                      : 0;
			const FVector WorldDirection(ControlRotation.X, ControlRotation.Y, PosRand + NegRand);
			AddMovementInput(WorldDirection, 1.0, true);
		}
	}
}

void ASuperman::MoveCharacter() {
	FVector FallBack = UKismetMathLibrary::Conv_RotatorToVector(GetControlRotation());
	const bool PosChecker = UKismetMathLibrary::InRange_FloatFloat(FallBack.Z, 0.1f, 1.0f, true, true);
	const bool NegChecker = UKismetMathLibrary::InRange_FloatFloat(
		FallBack.Z, -1.0f, -0.1f, true, true);
	const float Pos = PosChecker ? FallBack.Z : 0;
	const float Neg = NegChecker ? FallBack.Z : 0;
	FallBack.Z = Neg + Pos;
	const FVector WorldDirectionY = CheckMovementMode(MOVE_Flying) ? FallBack : GetInputAngles(true);
	const FVector WorldDirectionX = CanFlyStraight() ? FVector::Zero() : GetInputAngles(false);
	const float ScaleY = CanFlyStraight() ? 1 : MoveAxis.Y;
	AddMovementInput(WorldDirectionY, ScaleY, false);
	AddMovementInput(WorldDirectionX, MoveAxis.X, false);
}

bool ASuperman::CanFlyStraight() {
	return Stats->GetIsSprint() && CheckMovementMode(MOVE_Flying) && !TargetLock->IsTargetLock;
}

void ASuperman::MoveTriggered(const FInputActionValue& Value) {
	if (Flight->GetIsSuperheroLanding()) {
		return;
	}
	Super::MoveTriggered(Value);
	if (!IsAttacking) {
		if (!Flight->GetIsSuperheroLanding()) {
			const float NormalizeToRange = UKismetMathLibrary::NormalizeToRange(
				this->GetControlRotation().Pitch, 45, 90);
			if (UKismetMathLibrary::InRange_FloatFloat(NormalizeToRange, 0, 1)) {
				if (IsSprintBtnPressed && MoveAxis.Y != 0) {
					if (!CheckMovementMode(MOVE_Flying)) {
						Flight->StartFlight();
						Stats->SetSprintEvent(true);
					}
				}
			}
		}
	}
}

void ASuperman::MoveCompleted(const FInputActionValue& Value) {
	Super::MoveCompleted(Value);
	if (!CheckMovementMode(MOVE_Flying) && IsSprintBtnPressed) {
		Stats->SetSprintEvent(false);
	}
}

void ASuperman::JumpStarted() {
	if (Flight->GetIsSuperheroLanding()) {
		return;
	}
	Super::JumpStarted();
}

void ASuperman::DodgeTriggered() {
	Super::DodgeTriggered();
	if (Stats->GetIsSprint() && Flight->GetIsFlying()) {
		Stats->DodgeEvent(InputDirection);
	}
}

void ASuperman::AimCompleted() {
	Super::AimCompleted();
	HeatVision->StopLaser();
}

void ASuperman::StrikeStarted() {
	Super::StrikeStarted();
	if (Combat->CanAttack()) {
		if (IsValid(Combat->CachedTarget.Get())) {
			if (Combat->TargetUpDown) {
				Flight->StopFlight();
			}
			if (!Combat->IsWithinStrikeDistance()) {
				Flight->StartFlight();
			}
		}
	}
}

void ASuperman::LaserTriggered() {
	HeatVision->LaserTriggered();
}

void ASuperman::LaserStarted() {
	HeatVision->LaserStarted();
}

void ASuperman::LaserCompleted() {
	HeatVision->LaserCompleted();
}


void ASuperman::AscendOrDescend(bool UpOrDown) {
	if (Flight->GetIsFlying() && !Stats->GetIsSprint()) {
		Flight->SetIsAscending(UpOrDown);
		Flight->SetIsDescending(!UpOrDown);
		const float AscendForce = Flight->GetIsAscending() ? 1 : 0;
		const float DescendForce = Flight->GetIsDescending() ? -1 : 0;
		const float FlightForce = AscendForce + DescendForce;
		const FVector WorldDirection(0, 0, FlightForce);
		const FVector DeltaLocation = WorldDirection * 600 * GetWorld()->GetDeltaSeconds();
		const FVector NewLocation = GetActorLocation() + DeltaLocation;
		SetActorLocation(NewLocation);
	}
}

void ASuperman::AscendTriggered() {
	if (Flight->GetIsFlying()) {
		AscendOrDescend(true);
	}
	else {
		if (CheckMovementMode(MOVE_Walking)) {
			Flight->StartFlight();
		}
	}
}

void ASuperman::AscendCompleted() {
	Flight->SetIsAscending(false);
}

void ASuperman::DescendTriggered() {
	AscendOrDescend(false);
	if (IsGrounded && GetCharacterMovement()->IsFlying()) {
		Flight->StopFlight();
	}
}

void ASuperman::DescendCompleted() {
	Flight->SetIsDescending(false);
}

void ASuperman::DelayedStopFlight() {
	if (!(Flight->GetIsAscending() || Flight->GetIsDescending())) {
		Flight->StopFlight();
	}
}

void ASuperman::FlyToggleStarted() {
	if (!IsJumpProvidingForce() && CheckMovementMode(MOVE_Falling)) {
		Flight->StartFlight();
	}
	else {
		if (Flight->GetIsFlying()) {
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ASuperman::DelayedStopFlight, 0.11f, false);
		}
	}
}

bool ASuperman::CheckCameraPosition() {
	const bool NearlyEqualToArmLength = UKismetMathLibrary::NearlyEqual_FloatFloat(SpringArm->TargetArmLength,
		Stats->GetIsAiming()
			? AimingArmLength
			: DefaultArmLength,
		0.1);

	const FVector TargetSocket = Stats->GetIsAiming()
		                             ? AimingSocketOffset
		                             : (Stats->CheckMovementMode(MOVE_Flying)
			                                ? Flight->GetFlightSocketOffset()
			                                : DefaultSocketOffset);
	const bool IsEqualToSocket = UKismetMathLibrary::EqualEqual_VectorVector(
		SpringArm->SocketOffset, TargetSocket,
		0.1f);
	return NearlyEqualToArmLength && IsEqualToSocket;
}

void ASuperman::UpdateSpringArmSocketOffset() {
	const FVector TargetSocket = Stats->GetIsAiming()
		                             ? AimingSocketOffset
		                             : (Stats->CheckMovementMode(MOVE_Flying)
			                                ? Flight->GetFlightSocketOffset()
			                                : DefaultSocketOffset);
	SpringArm->SocketOffset = UKismetMathLibrary::VInterpTo(
		SpringArm->SocketOffset, TargetSocket, GetWorld()->DeltaTimeSeconds, 4);
}

void ASuperman::ResetCameraPosition() {
	const FVector TargetSocket = Stats->GetIsAiming()
		                             ? AimingSocketOffset
		                             : (Stats->CheckMovementMode(MOVE_Flying)
			                                ? Flight->GetFlightSocketOffset()
			                                : DefaultSocketOffset);
	SpringArm->SocketOffset = TargetSocket;

	const float TargetArmLength = Stats->GetIsAiming()
		                              ? AimingArmLength
		                              : DefaultArmLength;
	SpringArm->TargetArmLength = TargetArmLength;
	DidResetCameraPosition = true;
}

void ASuperman::CheckInputDirection() {
	if (!Stats->GetIsSprint() || !Flight->GetIsFlying()) {
		InputDirection = EInputDirection::None;
		return;
	}
	Super::CheckInputDirection();
}

void ASuperman::SetupActions(UEnhancedInputComponent* EnhancedInputComponent) {
	Super::SetupActions(EnhancedInputComponent);
	EnhancedInputComponent->BindAction(LaserAction, ETriggerEvent::Triggered, this, &ASuperman::LaserTriggered);
	EnhancedInputComponent->BindAction(LaserAction, ETriggerEvent::Started, this, &ASuperman::LaserStarted);
	EnhancedInputComponent->BindAction(LaserAction, ETriggerEvent::Completed, this, &ASuperman::LaserCompleted);
	EnhancedInputComponent->BindAction(AscendAction, ETriggerEvent::Triggered, this, &ASuperman::AscendTriggered);
	EnhancedInputComponent->BindAction(AscendAction, ETriggerEvent::Completed, this, &ASuperman::AscendCompleted);
	EnhancedInputComponent->BindAction(DescendAction, ETriggerEvent::Triggered, this,
	                                   &ASuperman::DescendTriggered);
	EnhancedInputComponent->BindAction(DescendAction, ETriggerEvent::Completed, this,
	                                   &ASuperman::DescendCompleted);
	EnhancedInputComponent->BindAction(FlyToggleAction, ETriggerEvent::Started, this,
	                                   &ASuperman::FlyToggleStarted);
}
