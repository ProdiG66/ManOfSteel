// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "Components/KryptonianStats.h"

#include "GameFramework/Character.h"


UKryptonianStats::UKryptonianStats() {}

void UKryptonianStats::BeginPlay() {
	Super::BeginPlay();
	Flight = Character->GetComponentByClass<UFlight>();
}

void UKryptonianStats::SetAimingEvent(bool Value) {
	SetIsAiming(Value && !Flight->GetIsSuperheroLanding());
}

bool UKryptonianStats::OverrideStatus() {
	return Flight->GetIsFlying() && GetIsSprint();
}
