// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "Components/Combat.h"

#include "Components/TargetLock.h"
#include "MotionWarpingComponent.h"
#include "RootMotionModifier.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UCombat::UCombat() {
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}


// Called when the game starts
void UCombat::BeginPlay() {
	Super::BeginPlay();

	Character = Cast<ACharacter>(GetOwner());
	CharacterMovement = Character->GetCharacterMovement();
	AnimInstance = Character->GetMesh()->GetAnimInstance();
	CameraComponent = Character->GetComponentByClass<UCameraComponent>();
	TargetLock = Character->GetComponentByClass<UTargetLock>();
	Stats = Character->GetComponentByClass<UBaseStats>();
	MotionWarping = Character->GetComponentByClass<UMotionWarpingComponent>();
}

void UCombat::EndHitStop() {
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1);
	HitStopEnabled = false;
	HitStopTimer = 0;
}

void UCombat::Update(float DeltaTime) {
	CheckForPendingAttack();
	if (HitStopEnabled) {
		if (HitStopTimer < 0.16) {
			HitStopTimer += GetWorld()->DeltaRealTimeSeconds;
		}
		else {
			EndHitStop();
		}
	}
}

void UCombat::CheckForPendingAttack() {
	if (StopWhenNearTarget(HasPendingAttack)) {
		Strike();
	}
}

bool UCombat::StopWhenNearTarget(bool PendingAction) {
	if (PendingAction && IsWithinStrikeDistance()) {
		IsDashingToAttack = false;
		Stats->SetSprintEvent(false);
		TargetLock->TurnOffTargetLock();
		return IsValid(CachedTarget.Get());
	}
	return false;
}

bool UCombat::GetIsAttacking() {
	return IsAttacking;
}

bool UCombat::IsWithinStrikeDistance() {
	if (IsValid(CachedTarget.Get())) {
		const bool IsNear = GetOwner()->GetDistanceTo(CachedTarget.Get()) < 1600;
		const float DistanceHeight = UKismetMathLibrary::Abs(
			CachedTarget->GetActorLocation().Z - GetOwner()->GetActorLocation().Z);
		return IsNear || DistanceHeight < 140;
	}
	return true;
}

bool UCombat::CheckMovementMode(EMovementMode MovementMode) {
	return CharacterMovement->MovementMode == MovementMode;
}

void UCombat::StartHitStop() {
	HitStopEnabled = true;
	GetWorld()->GetWorldSettings()->SetTimeDilation(0.0f);
}

UAnimMontage* UCombat::GetLeftOrRightStrike(bool LeftOrRight) {
	if (LeftOrRight) {
		return GetRandomMontageFromArray(PreviousR, StrikeR);
	}
	return GetRandomMontageFromArray(PreviousL, StrikeL);
}

UAnimMontage* UCombat::GetRandomMontageFromArray(int& Previous, TArray<UAnimMontage*> Montages) {
	if (Montages.Num() > 1) {
		int Index = UKismetMathLibrary::RandomInteger(Montages.Num());
		while (Index == Previous) {
			Index = UKismetMathLibrary::RandomInteger(Montages.Num());
		}
		UAnimMontage* Montage = Montages[Index];
		Previous = Index;
		return Montage;
	}
	return Montages[0];
}

UAnimMontage* UCombat::GetStrikeMontage() {
	ArmUsed = !ArmUsed;
	const int BeatdownProbability = UKismetMathLibrary::RandomInteger(100);
	return BeatdownProbability < BeatdownChance && !ArmUsed ? BeatdownStartMontage : GetLeftOrRightStrike(ArmUsed);
}


void UCombat::Strike() {
	Attack(GetStrikeMontage());
}

void UCombat::Attack(UAnimMontage* Montage) {
	AddStrikeWarpTarget();
	if (!IsAttacking && !AnimInstance->Montage_IsPlaying(AnimInstance->GetCurrentActiveMontage())) {
		IsAttacking = true;
		Character->GetMesh()->AnimScriptInstance->Montage_Play(Montage);
		FOnMontageBlendingOutStarted BlendingOutDelegate;
		BlendingOutDelegate.BindUObject(this, &UCombat::StrikeBlendOut);
		Character->GetMesh()->AnimScriptInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Montage);
	}
}

void UCombat::StrikeBlendOut(UAnimMontage* AnimMontage, bool Interrupted) {
	if (!Interrupted) {
		IsAttacking = false;
		HasPendingAttack = false;
		IsBeatdown = false;
		Stats->SetSprintEvent(false);
	}
}

void UCombat::Beatdown() {
	IsBeatdown = true;
	Attack(BeatdownLoopMontage);
}

AActor* UCombat::GetCurrentTarget(ECollisionChannel& ObjectType) {
	ECollisionChannel ToObjectType = ECC_Pawn;
	AActor* Result = TargetLock->IsTargetLock ? TargetLock->HitActor : GetNearestTarget(ToObjectType);
	if (TargetLock->IsTargetLock) {
		ToObjectType = TargetLock->ObjectType;
	}
	ObjectType = ToObjectType;
	return Result;
}

AActor* UCombat::GetNearestTarget(ECollisionChannel& ObjectType) {
	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = false;
	TraceParams.AddIgnoredActor(GetOwner());
	FHitResult Result;

	if (GetWorld()->SweepSingleByObjectType(
		Result,
		GetOwner()->GetActorLocation(),
		GetInputDirection(),
		FQuat::Identity,
		FCollisionObjectQueryParams(TargetLock->TargetObjectTypes),
		FCollisionShape::MakeSphere(246),
		TraceParams
	)) {
		ObjectType = Result.Component->GetCollisionObjectType();
		return Result.GetHitObjectHandle().FetchActor();
	}
	ObjectType = ECC_WorldStatic;
	return nullptr;
}

FVector UCombat::GetInputDirection() {
	FVector CamInput = (MoveAxis.Y * CameraComponent->GetForwardVector()) + (MoveAxis.X * CameraComponent->
		GetRightVector());
	CamInput.Normalize(0.0001f);
	const bool IsMoving = UKismetMathLibrary::NotEqual_Vector2DVector2D(MoveAxis, FVector2D::Zero());
	const FVector Direction = IsMoving ? CamInput : CameraComponent->GetForwardVector();
	return GetOwner()->GetActorLocation() + (Direction * TargetLock->ScanDistance);
}

bool UCombat::CanAttack() {
	return !(Stats->GetIsSprint()) && !Stats->GetIsAiming();
}

void UCombat::AddStrikeWarpTarget() {
	FMotionWarpingTarget WarpTarget;
	WarpTarget.Name = FName("Strike");
	WarpTarget.Location = CachedTarget->GetActorLocation() - FVector(0, 0, 130);
	WarpTarget.Rotation = CachedTarget->GetActorRotation();
	MotionWarping->AddOrUpdateWarpTarget(WarpTarget);
}
