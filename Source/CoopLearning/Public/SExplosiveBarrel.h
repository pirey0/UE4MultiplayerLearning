// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class UStaticMeshComponent;
class URadialForceComponent;
class USoundCue;
class USoundAttenuation;

UCLASS()
class COOPLEARNING_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ExplosiveBarrel")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ExplosiveBarrel")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ExplosiveBarrel")
	URadialForceComponent* RadialForceComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExplosiveBarrel")
	UMaterialInterface* ExplodedMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExplosiveBarrel")
		UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExplosiveBarrel")
	float ExplosionImpulse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExplosiveBarrel")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExplosiveBarrel")
	TSubclassOf<UDamageType> ExplosionDamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExplosiveBarrel")
	USoundCue* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExplosiveBarrel")
	USoundAttenuation* SoundAttenuation;

	bool bExploded;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastExplode();

	UFUNCTION()
	void OnHeathChanged(USHealthComponent* SourceHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);



};
