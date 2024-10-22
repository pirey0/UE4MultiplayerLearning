// Fill out your copyright notice in the Description page of Project Settings.

#include "SExplosiveBarrel.h"
#include "Components/SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundCue.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHeathChanged);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);

	RootComponent = MeshComp;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->SetRelativeLocation(FVector::ZeroVector);
	RadialForceComp->Radius = 250;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;
	RadialForceComp->bIgnoreOwningActor = true;

	ExplosionImpulse = 400;

	SetReplicates(true);
}

void ASExplosiveBarrel::OnHeathChanged(USHealthComponent * SourceHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{

	if (bExploded)
	{
		return;
	}

	if (Health <= 0)
	{
		bExploded = true;


		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;

		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), RadialForceComp->Radius, ExplosionDamageType, IgnoredActors, DamageCauser, InstigatedBy, true);

		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);
		RadialForceComp->FireImpulse();
		MulticastExplode();
	}
}

void ASExplosiveBarrel::MulticastExplode_Implementation()
{
	bExploded = true;

	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), FRotator::ZeroRotator);
	}

	if (MeshComp)
	{
		MeshComp->SetMaterial(0, ExplodedMaterial);
	}

	if (ExplosionSound) 
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation(), 1, 1, 0, SoundAttenuation);
	}
}