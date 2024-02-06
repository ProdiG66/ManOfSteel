// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Enumerators/EFlightType.h"
#include "NiagaraComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Math/Vector2D.h"
#include "TargetLock.h"
#include "BaseStats.h"
#include "Flight.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStopFlightDelegate);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MANOFSTEEL_API UFlight : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFlight();

	UFUNCTION()
	void SetIsSprint();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FStopFlightDelegate StopFlightDelegate;

private:
	//References
	TWeakObjectPtr<ACharacter> Character;
	TWeakObjectPtr<UCharacterMovementComponent> CharacterMovement;
	TWeakObjectPtr<UAnimInstance> AnimInstance;
	TWeakObjectPtr<USpringArmComponent> SpringArmComponent;
	TWeakObjectPtr<UCameraComponent> CameraComponent;
	TWeakObjectPtr<UTargetLock> TargetLock;
	TWeakObjectPtr<UBaseStats> Stats;
	UPROPERTY(EditAnywhere, Category = "Flight Parameters")
	UCapsuleComponent* CapsuleComponent;

	//Rotators
	FRotator LastVelocityRotation;
	FRotator PreviousVelocityRotation;

	//Floats
	float HighSpeedVerticalSpeed;
	float YawVelocityDifference;
	float PitchVelocityDifference;
	float CurrentSpeed;
	float SuperheroLandingThreshold = 3000;
	float FastFlightTime;
	float FastFlightTimeMapped;
	float FlightTime;
	float MaxFlightTime = 3;

	//Lean
	FVector2D Lean;

	//Parameter Settings
	UPROPERTY(EditAnywhere, Category = "Flight Parameters")
	float SprintFlySpeed = 6000;

	UPROPERTY(EditAnywhere, Category = "Flight Parameters")
	float SprintAcceleration = 65536.0;

	UPROPERTY(EditAnywhere, Category = "Flight Parameters")
	FRotator SprintRotationRate = FRotator(0, 720, 0);

	UPROPERTY(EditAnywhere, Category = "Flight Parameters")
	float HoverFlySpeed = 1500;

	UPROPERTY(EditAnywhere, Category = "Flight Parameters")
	float HoverAcceleration = 4096.0;

	UPROPERTY(EditAnywhere, Category = "Flight Parameters")
	FRotator HoverRotationRate = FRotator(0, 540, 0);

	UPROPERTY(EditAnywhere, Category = "Flight Parameters")
	float FlightBrakingDeceleration = 9000;

	UPROPERTY(EditAnywhere, Category = "Flight Parameters")
	bool UseSonicBoomVFX = true;

	//Animation
	UPROPERTY(EditAnywhere, Category = "Animation")
	TArray<UAnimMontage*> SuperheroLandingMontages;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TArray<UAnimSequence*> HoverStartAnims;

	//VFX
	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* FlightTrail;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* FlightWave;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* SonicBoom;

	UPROPERTY(EditAnywhere, Category = "VFX")
	TArray<UNiagaraSystem*> SuperheroLanding;

	UPROPERTY(EditAnywhere, Category = "VFX")
	TArray<UNiagaraSystem*> FlightUnderDust;

	TWeakObjectPtr<UNiagaraComponent> FlightTrailRef;
	TWeakObjectPtr<UNiagaraComponent> FlightWaveRef;
	TWeakObjectPtr<UNiagaraComponent> SonicBoomRef;
	TArray<TWeakObjectPtr<UNiagaraComponent>> UnderDustRef;

	UPROPERTY(EditAnywhere, Category = "VFX")
	FName FlightTrailAttachPointName = FName("pelvis");

	//Camera
	UPROPERTY(EditAnywhere, Category = "Camera Parameters")
	FVector FlightSocketOffset = FVector(0, 0, 20);

	//State
	bool IsSuperheroLanding;
	bool IsFlying;
	bool IsAscending;
	bool IsDescending;
	bool GoUp;

	void Land();

	//DoOnceMethods
	void ResetLandingEventDoOnce();
	void DoSuperheroLandingOnce();
	void DoSoftLandingOnce();
	bool DidSuperheroLanding;
	bool DidSoftLanding;

	void PlayAnimMontage(UAnimMontage* AnimMontage);
	void SetCamLag(bool CameraLag);
	void StopDodgeMontage();

	//State
	void SetIsFlying(bool Value);
	void SetIsSuperheroLandingEvent(bool Value);

	//CharacterMovement
	void SetFlightMovementParam();

protected:
	virtual void BeginPlay() override;


	UFUNCTION()
	void OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);

public:
	//Floats
	void Update(float DeltaTime);
	void StopAnimMontage(float InBlendOutTime, const UAnimMontage* Montage);

	UFUNCTION(BlueprintCallable)
	UNiagaraComponent* SpawnNiagaraAtLocationOrAttach(bool IsAttach, USceneComponent* AttachToComponent,
	                                                  UNiagaraSystem* SystemTemplate, FVector Location,
	                                                  FRotator Rotation);


	//Lean
	UFUNCTION(BlueprintPure, BlueprintCallable)
	FVector2D GetLeanParameter();

	//Speed
	UFUNCTION(BlueprintPure, BlueprintCallable)
	float GetHighSpeedVerticalSpeed();

	//Animation
	UFUNCTION(BlueprintPure, BlueprintCallable)
	UAnimMontage* GetSuperheroLandingMontage();
	UFUNCTION(BlueprintPure, BlueprintCallable)
	UAnimSequence* GetHoverStartAnim();

	//VFX
	UFUNCTION(BlueprintPure, BlueprintCallable)
	UNiagaraComponent* GetFlightTrailVfx();
	UFUNCTION(BlueprintPure, BlueprintCallable)
	UNiagaraComponent* GetFlightWaveVFX();
	UFUNCTION(BlueprintPure, BlueprintCallable)
	UNiagaraComponent* GetFlightUnderDustVFX(EPhysicalSurface PhysicalSurfaceType);
	static int PhysicsSurfaceTypeToInt(EPhysicalSurface PhysicalSurfaceType);
	UFUNCTION(BlueprintPure, BlueprintCallable)
	UNiagaraSystem* GetSuperheroLandingVFX(EPhysicalSurface PhysicalSurfaceType);
	UFUNCTION(BlueprintCallable)
	void SpawnSonicBoomVFXEvent(bool Value);
	UFUNCTION(BlueprintCallable)
	void SetActiveSonicBoom(bool NewActive);


	//Utility
	UFUNCTION(BlueprintCallable)
	void LineTraceToTheUpVector(float VectorLength, bool& Hit, FHitResult& OutHit);

	//State
	UFUNCTION(BlueprintCallable)
	void SetIsSuperheroLanding(bool Value);
	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetIsFlying();
	UFUNCTION(BlueprintPure, BlueprintCallable)
	EFlightType GetFlightType();
	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetIsSuperheroLanding();

	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetIsAscending();
	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetIsDescending();
	bool GetGoUp();
	void SetIsAscending(bool Value);
	void SetIsDescending(bool Value);
	void SetGoUp(bool Value);

	//CharacterMovement
	void CheckFlightTime(float Delta);
	UFUNCTION(BlueprintPure, BlueprintCallable)
	float GetFlightTime();
	float GetFastFlightTimeMapped();
	void SetFlightTime(float Value);

	UFUNCTION(BlueprintCallable)
	void StartFlight();
	void StopFlight();
	FVector GetFlightSocketOffset();
};
