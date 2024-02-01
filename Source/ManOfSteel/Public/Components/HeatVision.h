// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "TargetLock.h"
#include "Flight.h"
#include "VFX/BurnVFX.h"
#include "BaseStats.h"
#include "Components/ActorComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Math/Vector2D.h"
#include "HeatVision.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MANOFSTEEL_API UHeatVision : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHeatVision();
	TWeakObjectPtr<ACharacter> Character;
	TWeakObjectPtr<UCharacterMovementComponent> CharacterMovement;
	TWeakObjectPtr<UAnimInstance> AnimInstance;
	TWeakObjectPtr<USpringArmComponent> SpringArmComponent;
	TWeakObjectPtr<UCameraComponent> CameraComponent;
	TWeakObjectPtr<UTargetLock> TargetLock;
	TWeakObjectPtr<UFlight> Flight;
	TWeakObjectPtr<UBurnVFX> BurnVfx;
	TWeakObjectPtr<UBaseStats> Stats;
	TWeakObjectPtr<UNiagaraComponent> LaserEyeLeft;
	TWeakObjectPtr<UNiagaraComponent> LaserEyeRight;
	TWeakObjectPtr<UArrowComponent> LaserDirection;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero/VFX/Particles")
	UParticleSystem* LaserSparksEmitter;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero/VFX/Particles")
	UParticleSystem* LaserSmokeEmitter;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	bool LaserEyesActive;
	bool IsPressingLaserEyes;
	void LaserDistanceCalculation(const FHitResult& TargetHitResult, bool IsTargetGotHit);

public:
	void Update(float DeltaTime);

	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool IsLaserEyesPressed();
	void LaserTriggered();
	void LaserStarted();
	void LaserCompleted();
	void StopLaser();
	void SpawnLaserHitSparks(const FVector& Scale);
	void SpawnLaserSmoke(const FVector& Scale);
	void SpawnLaserEyes(float Delta);
	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetLaserEyesActive();
	UFUNCTION(BlueprintCallable)
	void SetLaserEyesActive(bool Value);
	void SetActiveLaserEyes(const FVector& Param, bool NewActive);
	bool LaserEyeVisible;
	bool CanActivateLaserEyes();
	float LaserDistance;
	float LaserSpeed = 3000;
	FVector LaserEyeStart;
	FVector LaserEyeEnd;
	FHitResult OutHit;
	bool IsHit;
	bool BlockingHit;
	FVector ImpactPoint;
	FVector ImpactNormal;
	TWeakObjectPtr<UPrimitiveComponent> HitComponent;
	void SetupTraceComponents(const FHitResult& HitResult, bool Hit);
	void SetLaserDistance();
	void SetMeshPhysics(UStaticMeshComponent* Target, bool Enable);
};
