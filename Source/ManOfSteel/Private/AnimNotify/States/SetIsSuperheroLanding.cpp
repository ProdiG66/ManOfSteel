// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/States/SetIsSuperheroLanding.h"

#include "Components/Flight.h"

void USetIsSuperheroLanding::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                         float TotalDuration,
                                         const FAnimNotifyEventReference& EventReference) {
	UFlight* Flight = MeshComp->GetOwner()->GetComponentByClass<UFlight>();
	if (IsValid(Flight)) {
		Flight->SetIsSuperheroLanding(true);
	}
}

void USetIsSuperheroLanding::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       const FAnimNotifyEventReference& EventReference) {
	UFlight* Flight = MeshComp->GetOwner()->GetComponentByClass<UFlight>();
	if (IsValid(Flight)) {
		Flight->SetIsSuperheroLanding(false);
	}
}
