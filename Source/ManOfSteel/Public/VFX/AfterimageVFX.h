// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/Combat.h"
#include "GameFramework/Character.h"
#include "AfterimageVFX.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MANOFSTEEL_API UAfterimageVFX : public UActorComponent {
	GENERATED_BODY()

public:
	UAfterimageVFX();
	bool IsStartAfterimage;
	TWeakObjectPtr<ACharacter> Character;
	TWeakObjectPtr<UCombat> Combat;
	void Update(float DeltaTime);

protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero/VFX")
	UClass* AfterimageVFX;

private:
	void SpawnAfterimage();
};
