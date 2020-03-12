// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UCameraShake;
class USoundCue;
class UDataTable;

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RateOfFire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrowForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HitMaxDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int BulletsPerMagazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int DefaultMagazineCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpreadInDegrees;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadshotMultiplyer;

};

USTRUCT(BlueprintType)
struct FWeaponSoundData : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* Shot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* NoAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* Reload;

};


USTRUCT()
struct FMulticastShotData 
{
	GENERATED_BODY()

public:

	UPROPERTY()
	bool HitTarget;

	UPROPERTY()
	FVector_NetQuantize TraceEndPoint;

	UPROPERTY()
	FVector_NetQuantize ImpactPoint;

	UPROPERTY()
	FVector_NetQuantizeNormal ImpactNormal;

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	bool NoShot;
};

UCLASS()
class COOPLEARNING_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();


protected:

	virtual void BeginPlay() override;

	UDataTable* WeaponsDataTable;

	UDataTable* WeaponsSoundDataTable;

	FWeaponData* WeaponsData;

	FWeaponSoundData* WeaponsSoundData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponsDataName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	void PlayFireEffects(FVector TracerEndPoint);

	void PlayImpactEffects(FVector ImpactPoint, FVector ImpactNormal, EPhysicalSurface SurfaceType);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName CenterSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshVulnerableImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UMaterialInterface* BulletHitDecal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FVector BulletHitDecalSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float BulletHitDecalLifetime;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;

	virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	UFUNCTION(NetMulticast, Reliable)
	void MultiCastFire(FMulticastShotData MulticastData);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTimeStamp;

	float TimeBetweenShots;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	int CurrentBulletCount;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	int CurrentMagazineCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float DespawnTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float SpeedEqualToMaxSpread;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReloadSound();

public:
	void StartFire();

	void StopFire();

	void GetEquippedBy(AActor* NewOwner);

	void Unequip();

	void Reload();

};