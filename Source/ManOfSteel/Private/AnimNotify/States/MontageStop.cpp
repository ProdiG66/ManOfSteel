// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/States/MontageStop.h"

#include "Components/BaseStats.h"

void UMontageStop::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              float FrameDeltaTime,
                              const FAnimNotifyEventReference& EventReference) {
	UBaseStats* Stats = Cast<UBaseStats>(MeshComp->GetOwner());
	if (IsValid(Stats)) {
		if (Stats->GetHasMovementInput()) {
			UAnimMontage* CurrentMontage = MeshComp->GetAnimInstance()->GetCurrentActiveMontage();
			if (IsValid(CurrentMontage)) {
				if (!DidOnce) {
					Stats->MontageStop(BlendTime, CurrentMontage);
					DidOnce = true;
				}
			}
		}
	}
}
