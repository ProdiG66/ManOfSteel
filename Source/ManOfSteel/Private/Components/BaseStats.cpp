// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "Components/BaseStats.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UBaseStats::UBaseStats() {
	PrimaryComponentTick.bCanEverTick = false;
}

void UBaseStats::BeginPlay() {
	Super::BeginPlay();
	Character = Cast<ACharacter>(GetOwner());
	CharacterMovement = Character->GetCharacterMovement();
	AnimInstance = Character->GetMesh()->GetAnimInstance();
	SpringArmComponent = Character->GetComponentByClass<USpringArmComponent>();
	CameraComponent = Character->GetComponentByClass<UCameraComponent>();
	TargetLock = Character->GetComponentByClass<UTargetLock>();
	TSet<UActorComponent*> NiagaraComponents = Character->GetComponents();
	for (UActorComponent* Component : NiagaraComponents) {
		if (UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(Component)) {
			FString ComponentName = CapsuleComp->GetName();
			if (ComponentName == "Collision") {
				CapsuleComponent = CapsuleComp;
			}
		}
	}
}

bool UBaseStats::GetIsSprint() {
	return IsSprint;
}

bool UBaseStats::GetIsDodging() {
	return IsDodging;
}

bool UBaseStats::GetIsAiming() {
	return IsAiming;
}

void UBaseStats::SetAimingEvent(bool Value) {
	SetIsAiming(Value);
}

void UBaseStats::MontageStop(float Blend, UAnimMontage* Montage) {
	AnimInstance->Montage_Stop(Blend, Montage);
}

void UBaseStats::SetIsAiming(bool Value) {
	// Set IsAiming flag
	IsAiming = Value;

	// Determine movement mode
	const bool IsWalking = CheckMovementMode(MOVE_Walking);
	const bool IsFastFlying = CheckMovementMode(MOVE_Flying) && GetIsSprint();
	const bool IsNotTargetLockedButFastFlying = IsFastFlying && !TargetLock->IsTargetLock;

	// Update CharacterMovement settings
	CharacterMovement->bOrientRotationToMovement = Value ? (IsWalking || IsNotTargetLockedButFastFlying) : true;

	// Update SpringArmComponent settings
	SpringArmComponent->bEnableCameraLag = Value ? false : !TargetLock->IsTargetLock;
	SpringArmComponent->bEnableCameraRotationLag = Value ? false : !TargetLock->IsTargetLock;
}

bool UBaseStats::CheckMovementMode(EMovementMode MovementMode) {
	return CharacterMovement->MovementMode == MovementMode;
}

void UBaseStats::PlayDodgeMontage(EInputDirection InputDirection) {
	Character->PlayAnimMontage(GetDodgeMontage(InputDirection));
	DodgeStrength = 1;
}

void UBaseStats::DodgeEvent(EInputDirection InputDirection) {
	if (CheckMovementMode(MOVE_Flying) && IsSprint && !IsDodging) {
		PlayDodgeMontage(InputDirection);
	}
}

UAnimMontage* UBaseStats::GetDodgeMontage(EInputDirection InputDirection) {
	switch (InputDirection) {
	case EInputDirection::Up:
		return DodgeMontages[1];
	case EInputDirection::Down:
		return DodgeMontages[2];
	case EInputDirection::Left:
		return DodgeMontages[3];
	case EInputDirection::Right:
		return DodgeMontages[4];
	case EInputDirection::None:
	default:
		return DodgeMontages[0];
	}
}

void UBaseStats::SetSprintEvent(bool Value) {
	SetIsSprint(Value);
}

void UBaseStats::SetIsDodging(bool Value) {
	IsDodging = Value;
}

void UBaseStats::SetHasMovementInput(bool Value) {
	HasMovementInput = Value;
}

bool UBaseStats::GetHasMovementInput() {
	return HasMovementInput;
}

void UBaseStats::SetIsSprint(bool Value) {
	IsSprint = Value;
	if (IsValid(CharacterMovement.Get())) {
		SetCollisionHeight(CharacterMovement->MovementMode);
		if (!CharacterMovement->bOrientRotationToMovement) {
			CharacterMovement->bOrientRotationToMovement = true;
		}
		CharacterMovement->MaxWalkSpeed = GetIsSprint() ? 1500 : 600;
	}
	SprintDelegate.Broadcast();
	// SprintDelegate.Execute();
}

void UBaseStats::SetCollisionHeight(EMovementMode TargetMovementMode) {
	const bool IsFastFlight = TargetMovementMode == MOVE_Flying && GetIsSprint();
	const float RotY = IsFastFlight ? -90 : 0;
	const FRotator NewRotation(0, RotY, 0);
	if (IsValid(CapsuleComponent.Get())) {
		CapsuleComponent->SetRelativeRotation(
			NewRotation, true, nullptr, ETeleportType::TeleportPhysics);
	}
}

void UBaseStats::SetMovementMode(EMovementMode MovementMode) {
	if (CharacterMovement.Get()) {
		CharacterMovement->MovementMode = MovementMode;
		SetCollisionHeight(MovementMode);
	}
}

void UBaseStats::DodgeMovementInputDirection(EInputDirection InputDirection) {
	if (CheckMovementMode(MOVE_Flying) && IsSprint) {
		DodgeStrength = UKismetMathLibrary::FInterpTo(DodgeStrength, 0, GetWorld()->DeltaTimeSeconds, 1.5);

		FVector WorldVector;
		switch (InputDirection) {
		case EInputDirection::Up:
			WorldVector = UKismetMathLibrary::GetUpVector(Character->GetControlRotation()) * 0.75f;
			break;
		case EInputDirection::Down:
			WorldVector = UKismetMathLibrary::GetUpVector(Character->GetControlRotation()) * -0.75f;
			break;
		case EInputDirection::Left:
			WorldVector = UKismetMathLibrary::GetRightVector(Character->GetControlRotation()) * -0.75f;
			break;
		case EInputDirection::Right:
			WorldVector = UKismetMathLibrary::GetRightVector(Character->GetControlRotation()) * 0.75f;
			break;
		default:
			WorldVector = FVector::Zero();
			break;
		}
		CharacterMovement->AddInputVector(WorldVector * DodgeStrength, false);
	}
}
