// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "VFX/SmearVFX.h"

#include "Components/Combat.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

USmearVFX::USmearVFX() {
	PrimaryComponentTick.bCanEverTick = true;
}

void USmearVFX::BeginPlay() {
	Super::BeginPlay();
	Character = Cast<ACharacter>(GetOwner());
	Mesh = Character->GetMesh();
	Combat = GetOwner()->GetComponentByClass<UCombat>();
	Flight = GetOwner()->GetComponentByClass<UFlight>();
	Stats = GetOwner()->GetComponentByClass<UBaseStats>();
	CreateDMI();
}

void USmearVFX::CreateDMI() {
	MaterialInstances.Add(Mesh->CreateDynamicMaterialInstance(0));
	MaterialInstances.Add(Mesh->CreateDynamicMaterialInstance(1));
}

void USmearVFX::Update(float DeltaTime, bool IsOn, float TargetAmount) {
	for (const TWeakObjectPtr<UMaterialInstanceDynamic> MaterialInstancePtr : MaterialInstances) {
		if (UMaterialInstanceDynamic* MaterialInstance = MaterialInstancePtr.Get()) {
			if (Combat->GetIsAttacking()) {
				MaterialInstance->SetVectorParameterValue("SmearLocation", -Character->GetMesh()->GetForwardVector());
			}
			float SmearAmount;
			if (MaterialInstance->GetScalarParameterValue(TEXT("SmearAmount"), SmearAmount)) {
				MaterialInstance->SetScalarParameterValue(
					"SmearAmount",
					UKismetMathLibrary::FInterpTo(
						SmearAmount,
						IsOn ? TargetAmount : 0,
						IsOn ? 0.16 : 0.9,
						1
					)
				);
			}
		}
	}
}
