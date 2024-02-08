// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/States/SetCameraLag.h"

#include "Components/BaseStats.h"

void USetCameraLag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                const FAnimNotifyEventReference& EventReference) {
	UBaseStats* Stats = MeshComp->GetOwner()->GetComponentByClass<UBaseStats>();
	if (IsValid(Stats)) {
		Stats->SetCameraLagEvent(BeginCameraLagSpeed, BeginCameraRotationLagSpeed, BeginLerpSpeed);
	}
}

void USetCameraLag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference) {
	UBaseStats* Stats = MeshComp->GetOwner()->GetComponentByClass<UBaseStats>();
	if (IsValid(Stats)) {
		Stats->SetCameraLagEvent(EndLerpSpeed, EndCameraRotationLagSpeed, EndLerpSpeed);
	}
}
