// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "VFX/AfterimageVFX.h"

#include "Components/Combat.h"
#include "GameFramework/Character.h"

UAfterimageVFX::UAfterimageVFX() {
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UAfterimageVFX::BeginPlay() {
	Super::BeginPlay();
	Character = Cast<ACharacter>(GetOwner());
	Combat = Character->GetComponentByClass<UCombat>();
}

void UAfterimageVFX::Update(float DeltaTime, bool IsOn) {
	if ((Combat->IsBeatdown || Combat->IsDashingToAttack) || IsOn) {
		if (IsStartAfterimage) {
			IsStartAfterimage = false;
			SpawnAfterimage();
		}
		else {
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UAfterimageVFX::SpawnAfterimage, SpawnRate,
			                                       false);
		}
	}
	else {
		IsStartAfterimage = true;
	}
}

void UAfterimageVFX::SpawnAfterimage() {
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined;
	SpawnParameters.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;
	GetWorld()->SpawnActor<AActor>(
		AfterimageVFX, Character->GetMesh()->GetComponentToWorld(), SpawnParameters);
}
