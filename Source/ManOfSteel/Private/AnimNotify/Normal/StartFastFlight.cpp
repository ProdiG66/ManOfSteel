// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/Normal/StartFastFlight.h"

#include "Components/Flight.h"
#include "Components/BaseStats.h"

void UStartFastFlight::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference) {
	UFlight* Flight = MeshComp->GetOwner()->GetComponentByClass<UFlight>();
	UBaseStats* Stats = MeshComp->GetOwner()->GetComponentByClass<UBaseStats>();
	if (IsValid(Flight) && IsValid(Stats)) {
		Flight->StartFlight();
		Stats->SetSprintEvent(true);
	}
}
