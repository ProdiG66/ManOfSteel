﻿// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/States/SetCameraFOV.h"

#include "Components/BaseStats.h"


void USetCameraFOV::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                float TotalDuration,
                                const FAnimNotifyEventReference& EventReference) {
	UBaseStats* Stats = MeshComp->GetOwner()->GetComponentByClass<UBaseStats>();
	if (IsValid(Stats)) {
		Stats->SetCameraFOVEvent(BeginFOV, BeginLerpSpeed);
	}
}

void USetCameraFOV::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference) {
	UBaseStats* Stats = MeshComp->GetOwner()->GetComponentByClass<UBaseStats>();
	if (IsValid(Stats)) {
		Stats->SetCameraFOVEvent(EndFOV, EndLerpSpeed);
	}
}
