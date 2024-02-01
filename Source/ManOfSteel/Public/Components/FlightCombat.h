// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat.h"
#include "Flight.h"
#include "FlightCombat.generated.h"

UCLASS()
class MANOFSTEEL_API UFlightCombat : public UCombat {
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	TWeakObjectPtr<UFlight> Flight;

public:
	UFUNCTION()
	virtual void StrikeBlendOut(UAnimMontage* AnimMontage, bool Interrupted) override;
	virtual bool CanAttack() override;
	virtual void Attack(UAnimMontage* Montage) override;
	UFUNCTION()
	void StopAttackOnLand();
};
