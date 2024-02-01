// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#include "Components/TargetLock.h"
#include "Kismet/KismetMathLibrary.h"

UTargetLock::UTargetLock() {
	PrimaryComponentTick.bCanEverTick = false;
}

void UTargetLock::BeginPlay() {
	Super::BeginPlay();
	PlayerController = GetWorld()->GetFirstPlayerController();
	Actor = GetOwner();
	Pawn = Cast<APawn>(Actor);
	CameraComponent = Actor->GetComponentByClass<UCameraComponent>();
}

void UTargetLock::CheckForTarget() {
	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = false;
	TraceParams.AddIgnoredActor(Actor.Get());
	FHitResult Result;

	// Sweep to find the target
	if (GetWorld()->SweepSingleByObjectType(
		Result,
		GetStartLocation(),
		GetEndLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(TargetObjectTypes),
		FCollisionShape::MakeSphere(ScanRadius),
		TraceParams
	)) {
		TargetResult = Result;
		HitActor = Result.GetHitObjectHandle().FetchActor();
		ObjectType = Result.Component->GetCollisionObjectType();
	}
	else {
		TargetResult = FHitResult();
		HitActor = nullptr;
		ObjectType = ECC_Pawn;
	}
}

FVector UTargetLock::GetStartLocation() {
	return Actor->GetActorLocation();
}

FVector UTargetLock::GetEndLocation() {
	const FVector Forward = UKismetMathLibrary::GetForwardVector(CameraComponent->GetComponentRotation()) *
		ScanDistance;
	return GetStartLocation() + Forward;
}

void UTargetLock::Update(float DeltaTime) {
	if (IsValid(HitActor)) {
		PlayerController->SetControlRotation(
			UKismetMathLibrary::FindLookAtRotation(
				CameraComponent->GetComponentLocation(), HitActor->GetActorLocation()
			)
		);
	}
}

void UTargetLock::TargetLock() {
	// Toggle target lock
	if (IsValid(HitActor) && IsTargetLock) {
		TurnOffTargetLock();
	}
	else {
		CheckForTarget();
		IsTargetLock = IsValid(HitActor);
	}
}

void UTargetLock::TurnOffTargetLock() {
	HitActor = nullptr;
	IsTargetLock = false;
}

void UTargetLock::SetTargetLock(AActor* ActorToSet) {
	if (IsValid(ActorToSet)) {
		HitActor = ActorToSet;
		IsTargetLock = IsValid(ActorToSet);
	}
	else {
		TurnOffTargetLock();
	}
}
