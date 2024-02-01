// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Flight.h"
#include "BaseStats.h"
#include "KryptonianStats.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MANOFSTEEL_API UKryptonianStats : public UBaseStats {
	GENERATED_BODY()

public:
	UKryptonianStats();
	TWeakObjectPtr<UFlight> Flight;

protected:
	virtual void BeginPlay() override;
	virtual void SetAimingEvent(bool Value) override;
};
