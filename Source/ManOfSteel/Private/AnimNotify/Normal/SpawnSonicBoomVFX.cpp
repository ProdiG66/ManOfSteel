// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/Normal/SpawnSonicBoomVFX.h"

#include "Components/Flight.h"

void USpawnSonicBoomVFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                const FAnimNotifyEventReference& EventReference) {
	UFlight* Flight = MeshComp->GetOwner()->GetComponentByClass<UFlight>();
	if (IsValid(Flight)) {
		Flight->SpawnSonicBoomVFXEvent(true);
	}
}
