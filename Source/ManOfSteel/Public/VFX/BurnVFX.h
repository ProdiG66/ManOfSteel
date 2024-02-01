// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structs/FBurnDecalStruct.h"
#include "BurnVFX.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MANOFSTEEL_API UBurnVFX : public UActorComponent {
	GENERATED_BODY()

public:
	/** Constructor */
	UBurnVFX();

	/** Clears all burn decals */
	void ClearBurnDecals();

	/**
	 * Updates burn effects based on hit result.
	 * @param Hit The hit result.
	 * @param IHit True if hit occurred, false otherwise.
	 */
	void UpdateBurn(const FHitResult& Hit, bool IHit);

private:
	/** Retrieves the location of the first decal */
	FVector GetFirstDecalLocation();

	/** Retrieves the normal vector of the first decal */
	FVector GetFirstDecalNormal();

	/** Retrieves the distance between the first two decals */
	FVector GetFirstTwoDecalDistance();

	/** Retrieves the quarter of the distance between the first two decals */
	FVector GetFirstTwoDecalQuarter();

	/** Retrieves the midpoint between the first two decals */
	FVector GetFirstTwoDecalMidpoint();

	/** Retrieves 75% of the distance between the first two decals */
	FVector GetFirstTwoDecal75();

	/** Retrieves 25% of the distance between the first two decals */
	FVector GetFirstTwoDecal25();

	/** Retrieves half-quarter of the distance between the first two decals */
	FVector GetFirstTwoDecalHalfQuarter();

	/** Retrieves the location of the second decal */
	FVector GetSecondDecalLocation();

	/** Adds a location to the burn locations array */
	TArray<FVector> AddToBurnLocations(const FVector& ToAdd);

	/** Adds a created decal to the burn decals array */
	void AddCreatedDecal();

	/** Creates a decal from the current hit */
	FBurnDecalStruct CreateDecalFromCurrentHit();

	/** Sets up hit variables */
	void SetupHitVariables();

	/** Retrieves the rotation of the decal */
	FRotator GetDecalRotation();

	/** Spawns a decal at the specified location */
	void SpawnDecal(const FVector& Loc);

	/** Completes all decals */
	void CompleteDecals();

	/** Compiles burn locations */
	void CompileBurnLocations();

	/** Updates the burn trace based on start and end vectors */
	void UpdateBurnTrace(const FVector& Start, const FVector& End);

	/** Checks if the number of burn decals is greater than 3 */
	bool IsBurnLengthGreaterThan3();

	/** Initializes burn decals */
	void InitializeBurnDecals();

	/** Checks if the distance between the first two decals is greater than 4 */
	bool IsDistanceGreaterThan4();

protected:
	/** Array of burn decals */
	TArray<FBurnDecalStruct> BurnDecals;

	/** Parent material interface */
	UPROPERTY(EditAnywhere, Category = "Burn References")
	UMaterialInterface* ParentMaterial;

private:
	/** Array of burn locations */
	TArray<FVector> BurnLocations;

	/** Indicates if a hit occurred */
	bool IsHit;

	/** Information about the hit result */
	FHitResult OutHit;

	/** Location of the hit */
	FVector Location;

	/** Normal vector of the hit */
	FVector Normal;

	/** Hit component */
	TWeakObjectPtr<USceneComponent> HitComponent;

	/** Hit bone name */
	FName HitBone;

	/** Dynamic instance of the burn mark material */
	TWeakObjectPtr<UMaterialInstanceDynamic> BurnmarkInstance;
};
