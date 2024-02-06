// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Enumerators/EInputDirection.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Math/Vector2D.h"
#include "TargetLock.h"
#include "BaseStats.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSprintDelegate);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MANOFSTEEL_API UBaseStats : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBaseStats();
	TWeakObjectPtr<ACharacter> Character;
	TWeakObjectPtr<UCharacterMovementComponent> CharacterMovement;
	TWeakObjectPtr<UAnimInstance> AnimInstance;
	TWeakObjectPtr<UCapsuleComponent> CapsuleComponent;
	TWeakObjectPtr<USpringArmComponent> SpringArmComponent;
	TWeakObjectPtr<UCameraComponent> CameraComponent;
	TWeakObjectPtr<UTargetLock> TargetLock;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FSprintDelegate SprintDelegate;

protected:
	virtual void BeginPlay() override;
	void SetIsAiming(bool Value);
	virtual void SetIsSprint(bool Value);
	bool IsSprint;

private:
	bool IsAiming;
	bool IsDodging;
	bool HasMovementInput;
	FVector LookAtLocation;

	UPROPERTY(EditAnywhere, Category = "Camera Parameters")
	float LagLerpSpeed = 20;

	UPROPERTY(EditAnywhere, Category = "Camera Parameters")
	float FOVLerpSpeed = 20;

	UPROPERTY(EditAnywhere, Category = "Camera Parameters")
	bool UseCameraLag = true;

	UPROPERTY(EditAnywhere, Category = "Camera Parameters")
	bool UseCameraFOV = true;

	float SavedLagSpeed = 15;
	float SavedRotationLagSpeed = 15;
	float SavedFOV = 20;

public:
	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetIsSprint();
	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetIsDodging();
	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetIsAiming();
	virtual void SetAimingEvent(bool Value);
	UFUNCTION(BlueprintCallable)
	void MontageStop(float Blend, UAnimMontage* Montage);
	UFUNCTION(BlueprintCallable)
	void SetSprintEvent(bool Value);
	UFUNCTION(BlueprintCallable)
	void SetIsDodging(bool Value);
	void DodgeEvent(EInputDirection InputDirection);
	UFUNCTION(BlueprintCallable)
	void DodgeMovementInputDirection(EInputDirection InputDirection);
	UPROPERTY(EditAnywhere, Category = "Animation")
	float DodgeStrength = 1;
	void PlayDodgeMontage(EInputDirection InputDirection);
	UFUNCTION(BlueprintPure, BlueprintCallable)
	UAnimMontage* GetDodgeMontage(EInputDirection InputDirection);
	UPROPERTY(EditAnywhere, Category = "Animation")
	TArray<UAnimMontage*> DodgeMontages;
	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetHasMovementInput();
	void SetHasMovementInput(bool Value);

	UFUNCTION(BlueprintPure, BlueprintCallable)
	FVector GetLookAtLocation();
	void SetLookAtLocation(FVector Value);
	void SetCollisionHeight(EMovementMode TargetMovementMode);
	bool CheckMovementMode(EMovementMode MovementMode);
	void SetMovementMode(EMovementMode MovementMode);

	//Events
	void SetCameraFOVLerp();
	void SetCameraLagLerp();

	UFUNCTION(BlueprintCallable)
	void SetCameraFOVEvent(float FOV, float FOVSpeed);

	UFUNCTION(BlueprintCallable)
	void SetCameraLagEvent(float LagSpeed, float RotLagSpeed, float LagLerp);

	UFUNCTION(BlueprintCallable)
	void SetActiveComponent(bool UseActiveDelay, float DelayDuration, USceneComponent* Component, bool NewActive,
	                        bool Reset);
};
