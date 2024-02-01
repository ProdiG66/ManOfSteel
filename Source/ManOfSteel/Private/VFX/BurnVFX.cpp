// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#include "VFX/BurnVFX.h"

#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"

UBurnVFX::UBurnVFX() {}

FVector UBurnVFX::GetFirstDecalLocation() {
	return BurnDecals[1].Location;
}

FVector UBurnVFX::GetFirstDecalNormal() {
	return BurnDecals[1].Normal;
}

FVector UBurnVFX::GetFirstTwoDecalDistance() {
	return GetFirstDecalLocation() - GetSecondDecalLocation();
}

FVector UBurnVFX::GetFirstTwoDecalQuarter() {
	return (GetFirstDecalLocation() - GetFirstTwoDecalMidpoint()) / 2;
}

FVector UBurnVFX::GetFirstTwoDecalMidpoint() {
	return GetFirstDecalLocation() - (GetFirstTwoDecalDistance() / 2);
}

FVector UBurnVFX::GetFirstTwoDecal75() {
	return GetFirstDecalLocation() - GetFirstTwoDecalQuarter();
}

FVector UBurnVFX::GetFirstTwoDecal25() {
	return GetFirstTwoDecalMidpoint() - GetFirstTwoDecalQuarter();
}

FVector UBurnVFX::GetFirstTwoDecalHalfQuarter() {
	return GetFirstDecalLocation() - GetFirstTwoDecal75();
}

FVector UBurnVFX::GetSecondDecalLocation() {
	return BurnDecals[2].Location;
}

TArray<FVector> UBurnVFX::AddToBurnLocations(const FVector& ToAdd) {
	BurnLocations.Add(ToAdd);
	return BurnLocations;
}

void UBurnVFX::AddCreatedDecal() {
	BurnDecals.Add(CreateDecalFromCurrentHit());
}

FBurnDecalStruct UBurnVFX::CreateDecalFromCurrentHit() {
	FBurnDecalStruct NewBurnDecal;
	NewBurnDecal.Location = OutHit.Location;
	NewBurnDecal.Normal = OutHit.Normal;
	NewBurnDecal.HitComponent = OutHit.Component.Get();
	NewBurnDecal.HitBone = OutHit.BoneName;
	return NewBurnDecal;
}

void UBurnVFX::SetupHitVariables() {
	Location = OutHit.Location;
	Normal = OutHit.Normal;
	HitComponent = OutHit.Component;
	HitBone = OutHit.BoneName;
}

FRotator UBurnVFX::GetDecalRotation() {
	return UKismetMathLibrary::Conv_VectorToRotator(GetFirstDecalNormal());
}

void UBurnVFX::SpawnDecal(const FVector& Loc) {
	UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(BurnmarkInstance.Get(), FVector(16),
	                                                              BurnDecals[2].HitComponent,
	                                                              BurnDecals[2].HitBone, Loc, GetDecalRotation(),
	                                                              EAttachLocation::KeepWorldPosition, 6.0);
	Decal->SetFadeScreenSize(0);
}

void UBurnVFX::CompleteDecals() {
	BurnDecals.RemoveAt(0);
	BurnLocations.Empty();
}

void UBurnVFX::CompileBurnLocations() {
	AddToBurnLocations(GetFirstTwoDecalMidpoint());
	AddToBurnLocations(GetFirstTwoDecal75());
	AddToBurnLocations(GetFirstTwoDecal25());
	AddToBurnLocations(GetFirstDecalLocation() - GetFirstTwoDecalHalfQuarter());
	AddToBurnLocations(GetFirstTwoDecal75() - GetFirstTwoDecalHalfQuarter());
	AddToBurnLocations(GetFirstTwoDecal25() - GetFirstTwoDecalHalfQuarter());
	AddToBurnLocations(GetFirstTwoDecal25() - GetFirstTwoDecalHalfQuarter());
}

void UBurnVFX::UpdateBurnTrace(const FVector& Start, const FVector& End) {
	FCollisionQueryParams CollisionParameters;
	CollisionParameters.bTraceComplex = false;
	CollisionParameters.AddIgnoredActor(GetOwner());
	IsHit = GetWorld()->LineTraceSingleByChannel(
		OutHit, Start, End, ECC_Visibility, CollisionParameters
	);
	UpdateBurn(OutHit, IsHit);
}

bool UBurnVFX::IsBurnLengthGreaterThan3() {
	return BurnDecals.Num() > 3;
}

void UBurnVFX::InitializeBurnDecals() {
	SetupHitVariables();
	AddCreatedDecal();
}

bool UBurnVFX::IsDistanceGreaterThan4() {
	return GetFirstTwoDecalDistance().Size() > 1;
}

void UBurnVFX::UpdateBurn(const FHitResult& Hit, const bool IHit) {
	OutHit = Hit;
	IsHit = IHit;
	if (IsHit) {
		InitializeBurnDecals();
		if (IsBurnLengthGreaterThan3()) {
			AddToBurnLocations(GetFirstDecalLocation());
			CompileBurnLocations();
			BurnmarkInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(
				GetWorld(), ParentMaterial
			);
			for (FVector BurnLocation : BurnLocations) {
				SpawnDecal(BurnLocation);
				BurnmarkInstance.Get()->SetScalarParameterValue(
					FName("SpawnTime"),
					UKismetSystemLibrary::GetGameTimeInSeconds(GetWorld())
				);
			}
			CompleteDecals();
		}
	}
	else {
		ClearBurnDecals();
	}
}

void UBurnVFX::ClearBurnDecals() {
	BurnDecals.Empty();
}
