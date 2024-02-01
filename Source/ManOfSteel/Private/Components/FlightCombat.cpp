// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "Components/FlightCombat.h"

#include "GameFramework/Character.h"

void UFlightCombat::BeginPlay() {
	Super::BeginPlay();
	Flight = GetOwner()->GetComponentByClass<UFlight>();
	Flight->StopFlightDelegate.AddDynamic(this, &UFlightCombat::StopAttackOnLand);
}

void UFlightCombat::StopAttackOnLand() {
	IsAttacking = false;
}

void UFlightCombat::Attack(UAnimMontage* Montage) {
	AddStrikeWarpTarget();
	if (!GetIsAttacking()) {
		IsAttacking = true;
		Character->GetMesh()->AnimScriptInstance->Montage_Play(Montage);
		FOnMontageBlendingOutStarted BlendingOutDelegate;
		BlendingOutDelegate.BindUObject(this, &UFlightCombat::StrikeBlendOut);
		Character->GetMesh()->AnimScriptInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Montage);
	}
}


void UFlightCombat::StrikeBlendOut(UAnimMontage* AnimMontage, bool Interrupted) {
	Super::StrikeBlendOut(AnimMontage, Interrupted);
	if (!Interrupted) {
		if (CheckMovementMode(MOVE_Falling)) {
			Flight->StartFlight();
		}
		else {
			if (TargetUpDown) {
				Flight->StopFlight();
			}
		}
	}
}

bool UFlightCombat::CanAttack() {
	return !(Flight->GetIsFlying() && Stats->GetIsSprint()) && !Stats->GetIsAiming();
}
