// Fill out your copyright notice in the Description page of Project Settings.


#include "SGranade.h"
#include "Components/SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

// Sets default values
ASGranade::ASGranade()
{
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASGranade::OnHeathChanged);

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
	ExplosionDefaultTime = 5;

	SetReplicates(true);

}

// Called when the game starts or when spawned
void ASGranade::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorldTimerManager().SetTimer(TimerHandle_DefaultExplosion, this, &ASGranade::DefaultExplode, ExplosionDefaultTime, true);

}

void ASGranade::OnHeathChanged(USHealthComponent * SourceHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Health <= 0) 
	{
		Explode(InstigatedBy);
	}
}

void ASGranade::DefaultExplode()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());

	if (Character)
	{
		Explode(Character->GetController());
	}
	else 
	{
		Explode(nullptr);
		UE_LOG(LogTemp, Warning, TEXT("Granade has no Owner, might not be tracked properly"));
	}

}

void ASGranade::Explode(AController* InstigatedBy)
{
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), RadialForceComp->Radius, ExplosionDamageType, IgnoredActors, this, InstigatedBy, true);

	RadialForceComp->FireImpulse();
	MulticastExplode();

	SetLifeSpan(0.05f);
}

UStaticMeshComponent * ASGranade::GetMeshComp()
{
	return MeshComp;
}


void ASGranade::MulticastExplode_Implementation()
{
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), FRotator::ZeroRotator);
	}

	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation(), 1, 1, 0, SoundAttenuation);
	}
}