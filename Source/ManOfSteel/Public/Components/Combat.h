// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseStats.h"
#include "Components/ActorComponent.h"
#include "TargetLock.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "Combat.generated.h"

class UCurveFloat;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MANOFSTEEL_API UCombat : public UActorComponent {
	GENERATED_BODY()

public:
	UCombat();

protected:
	virtual void BeginPlay() override;
	bool CheckMovementMode(EMovementMode MovementMode);
	TWeakObjectPtr<ACharacter> Character;
	TWeakObjectPtr<UBaseStats> Stats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	TArray<UAnimMontage*> StrikeL;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	TArray<UAnimMontage*> StrikeR;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	int BeatdownChance = 10;
	bool IsAttacking = false;

private:
	//References
	TWeakObjectPtr<UCharacterMovementComponent> CharacterMovement;
	TWeakObjectPtr<UAnimInstance> AnimInstance;
	TWeakObjectPtr<UCameraComponent> CameraComponent;
	TWeakObjectPtr<UTargetLock> TargetLock;
	TWeakObjectPtr<UMotionWarpingComponent> MotionWarping;

	void EndHitStop();
	int StrikeCount = 0;
	bool ArmUsed = false;
	UAnimMontage* GetLeftOrRightStrike(bool LeftOrRight);
	UAnimMontage* GetRandomMontageFromArray(int& Previous, TArray<UAnimMontage*> Montages);
	int PreviousL;
	int PreviousR;
	bool HitStopEnabled = false;
	float HitStopTimer = 0;

public:
	void Update(float DeltaTime);
	void CheckForPendingAttack();
	bool IsWithinStrikeDistance();
	bool StopWhenNearTarget(bool PendingAction);

	UFUNCTION(BlueprintPure, BlueprintCallable)
	bool GetIsAttacking();
	bool IsDashingToAttack;
	bool TargetUpDown;
	bool IsBeatdown;
	UPROPERTY(BlueprintReadWrite)
	bool IsBeatdownPressed;
	bool HasPendingAttack;
	TWeakObjectPtr<AActor> CachedTarget;
	ECollisionChannel CachedObjectType;

	UFUNCTION(BlueprintCallable)
	void StartHitStop();
	void BeginHitStop();
	UAnimMontage* GetStrikeMontage();
	void Strike();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimMontage* BeatdownLoopMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animations")
	UAnimMontage* BeatdownStartMontage;
	virtual void Attack(UAnimMontage* Montage);
	virtual void StrikeBlendOut(UAnimMontage* AnimMontage, bool Interrupted);
	void Beatdown();
	AActor* GetCurrentTarget(ECollisionChannel& ObjectType);
	AActor* GetNearestTarget(ECollisionChannel& ObjectType);
	FVector GetInputDirection();
	FVector2D MoveAxis;
	virtual bool CanAttack();
	void AddStrikeWarpTarget();
};
