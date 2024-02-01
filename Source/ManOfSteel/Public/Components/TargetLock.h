// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/ActorComponent.h"
#include "TargetLock.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MANOFSTEEL_API UTargetLock : public UActorComponent {
	GENERATED_BODY()

public:
	/** Default constructor */
	UTargetLock();

private:
	/** Weak reference to the player controller */
	TWeakObjectPtr<APlayerController> PlayerController;

	/** Weak reference to the actor */
	TWeakObjectPtr<AActor> Actor;

	/** Weak reference to the pawn */
	TWeakObjectPtr<APawn> Pawn;

	/** Weak reference to the camera component */
	TWeakObjectPtr<UCameraComponent> CameraComponent;

	/** Hit result of the target */
	FHitResult TargetResult;

protected:
	/** Called when the game starts */
	virtual void BeginPlay() override;

	/** Radius used for scanning targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Lock")
	float ScanRadius;

	/** Checks for available targets */
	void CheckForTarget();

	/** Retrieves the start location for scanning */
	FVector GetStartLocation();

	/** Retrieves the end location for scanning */
	FVector GetEndLocation();

public:
	/** Called every frame */
	void Update(float DeltaTime);

	/** Indicates whether target lock is active */
	bool IsTargetLock;

	/** Toggles target locking */
	void TargetLock();

	/** Deactivates target locking */
	void TurnOffTargetLock();

	/**
	 * Sets the Actor to be Target Locked
	 * @param ActorToSet The actor to be set.
	 */
	void SetTargetLock(AActor* ActorToSet);

	/** Weak reference to the hit actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Lock")
	AActor* HitActor;

	/** Collision channel of the object type */
	ECollisionChannel ObjectType;

	/** Array of target object types */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Lock")
	TArray<TEnumAsByte<EObjectTypeQuery>> TargetObjectTypes;

	/** Distance used for scanning targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Lock")
	float ScanDistance;
};
