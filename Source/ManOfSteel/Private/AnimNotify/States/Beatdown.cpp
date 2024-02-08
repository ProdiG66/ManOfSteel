// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/States/Beatdown.h"

#include "Components/Combat.h"

void UBeatdown::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                            const FAnimNotifyEventReference& EventReference) {
	UCombat* Combat = MeshComp->GetOwner()->GetComponentByClass<UCombat>();
	if (IsValid(Combat)) {
		Combat->IsBeatdownPressed = true;
	}
}

void UBeatdown::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                          const FAnimNotifyEventReference& EventReference) {
	UCombat* Combat = MeshComp->GetOwner()->GetComponentByClass<UCombat>();
	if (IsValid(Combat)) {
		Combat->IsBeatdownPressed = false;
	}
}
