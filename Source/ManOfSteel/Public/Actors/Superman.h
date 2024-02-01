// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Flight.h"
#include "Components/HeatVision.h"
#include "VFX/BurnVFX.h"
#include "VFX/AfterimageVFX.h"
#include "VFX/SmearVFX.h"
#include "BaseHero.h"
#include "InputAction.h"
#include "Superman.generated.h"

UCLASS()
class MANOFSTEEL_API ASuperman : public ABaseHero {
	GENERATED_BODY()

public:
	ASuperman();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupActions(UEnhancedInputComponent* EnhancedInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "Hero/Components")
	UFlight* Flight;
	UPROPERTY(EditAnywhere, Category = "Hero/Components")
	UHeatVision* HeatVision;
	UPROPERTY(EditAnywhere, Category = "Hero/Components VFX")
	UBurnVFX* BurnVfx;
	UPROPERTY(EditAnywhere, Category = "Hero/Components VFX")
	USmearVFX* SmearVfx;
	UPROPERTY(EditAnywhere, Category = "Hero/Components")
	UNiagaraComponent* LaserEyeLeft;
	UPROPERTY(EditAnywhere, Category = "Hero/Components")
	UNiagaraComponent* LaserEyeRight;
	UPROPERTY(EditAnywhere, Category = "Hero/Components")
	UArrowComponent* LaserDirection;

	UPROPERTY(EditAnywhere, Category = "Hero/Components VFX")
	UAfterimageVFX* AfterimageVfx;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero/Enhanced Input")
	UInputAction* LaserAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero/Enhanced Input")
	UInputAction* AscendAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero/Enhanced Input")
	UInputAction* DescendAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hero/Enhanced Input")
	UInputAction* FlyToggleAction;

protected:
	void AddCrosshair();
	virtual void BeginPlay() override;

	bool CanFlyStraight();
	virtual void CheckInputDirection() override;
	virtual void MoveTriggered(const FInputActionValue& Value) override;
	virtual void MoveCompleted(const FInputActionValue& Value) override;
	virtual void JumpStarted() override;
	virtual void DodgeTriggered() override;
	virtual void AimCompleted() override;
	virtual void StrikeStarted() override;
	virtual void MoveCharacter() override;
	void LaserStarted();
	void LaserTriggered();
	void LaserCompleted();
	void AscendTriggered();
	void AscendCompleted();
	void DescendTriggered();
	void DescendCompleted();
	void FlyToggleStarted();

	void FastFlightMoveCharacter();

	void FlyUp();
	void AscendOrDescend(bool UpOrDown);
	void DelayedStopFlight();
};
