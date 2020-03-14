// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGranade.generated.h"


class USHealthComponent;
class URadialForceComponent;
class USoundCue;
class USoundAttenuation;

UCLASS()
class COOPLEARNING_API ASGranade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASGranade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Granade")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Granade")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Granade")
	URadialForceComponent* RadialForceComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Granade")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Granade")
	float ExplosionImpulse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Granade")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Granade")
	TSubclassOf<UDamageType> ExplosionDamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Granade")
	USoundCue* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Granade")
	USoundAttenuation* SoundAttenuation;

 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Granade")
	float ExplosionDefaultTime;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastExplode();

	UFUNCTION()
	void OnHeathChanged(USHealthComponent* SourceHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	FTimerHandle TimerHandle_DefaultExplosion;

	void DefaultExplode();

public:

	void Explode(AController* InstigatedBy);

	UStaticMeshComponent* GetMeshComp();
};
