// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/Normal/HitStop.h"
#include "Components/Combat.h"

void UHitStop::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                      const FAnimNotifyEventReference& EventReference) {
	UCombat* Combat = MeshComp->GetOwner()->GetComponentByClass<UCombat>();
	if (IsValid(Combat)) {
		Combat->StartHitStop();
	}
}
