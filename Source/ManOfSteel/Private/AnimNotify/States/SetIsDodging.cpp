// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/States/SetIsDodging.h"

#include "Components/BaseStats.h"

void USetIsDodging::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                const FAnimNotifyEventReference& EventReference) {
	UBaseStats* Stats = MeshComp->GetOwner()->GetComponentByClass<UBaseStats>();
	if (IsValid(Stats)) {
		Stats->SetIsDodging(true);
	}
}

void USetIsDodging::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference) {
	UBaseStats* Stats = MeshComp->GetOwner()->GetComponentByClass<UBaseStats>();
	if (IsValid(Stats)) {
		Stats->SetIsDodging(false);
	}
}
