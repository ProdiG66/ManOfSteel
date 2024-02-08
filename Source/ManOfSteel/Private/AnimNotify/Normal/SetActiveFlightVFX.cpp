// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/Normal/SetActiveFlightVFX.h"

#include "Components/Flight.h"
#include "Components/BaseStats.h"

void USetActiveFlightVFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference) {
	UFlight* Flight = MeshComp->GetOwner()->GetComponentByClass<UFlight>();
	UBaseStats* Stats = MeshComp->GetOwner()->GetComponentByClass<UBaseStats>();
	if (IsValid(Flight) && IsValid(Stats)) {
		Stats->SetActiveComponent(false, 0, Flight->GetFlightTrailVfx(), IsActive, true);
		Stats->SetActiveComponent(false, 0, Flight->GetFlightWaveVFX(), IsActive, true);
	}
}
