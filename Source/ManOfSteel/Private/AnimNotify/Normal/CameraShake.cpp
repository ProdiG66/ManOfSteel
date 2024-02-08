// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/Normal/CameraShake.h"

#include "GameFramework/Character.h"


void UCameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                          const FAnimNotifyEventReference& EventReference) {
	const ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (IsValid(Character)) {
		APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
		if (IsValid(PlayerController)) {
			PlayerController->ClientStartCameraShake(CameraShake, Scale);
		}
	}
}
