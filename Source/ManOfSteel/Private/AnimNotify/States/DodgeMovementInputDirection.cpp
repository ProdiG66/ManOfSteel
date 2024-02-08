// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/States/DodgeMovementInputDirection.h"

#include "Components/BaseStats.h"

void UDodgeMovementInputDirection::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                              float FrameDeltaTime,
                                              const FAnimNotifyEventReference& EventReference) {
	UBaseStats* Stats = MeshComp->GetOwner()->GetComponentByClass<UBaseStats>();
	if (IsValid(Stats)) {
		Stats->DodgeMovementInputDirection(InputDirection);
	}
}
