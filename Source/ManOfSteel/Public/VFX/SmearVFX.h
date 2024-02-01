// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Combat.h"
#include "Components/Flight.h"
#include "Components/ActorComponent.h"
#include "SmearVFX.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MANOFSTEEL_API USmearVFX : public UActorComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USmearVFX();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void CreateDMI();
	TArray<TWeakObjectPtr<UMaterialInstanceDynamic>> MaterialInstances;
	TWeakObjectPtr<ACharacter> Character;
	TWeakObjectPtr<USkeletalMeshComponent> Mesh;
	TWeakObjectPtr<UCombat> Combat;
	TWeakObjectPtr<UFlight> Flight;
	TWeakObjectPtr<UBaseStats> Stats;

public:
	void Update(float DeltaTime);
};
