// Fill out your copyright notice in the Description page of Project Settings.

#include "SZipline.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"

ASZipline::ASZipline()
{
	StartComp = CreateDefaultSubobject<USceneComponent>(TEXT("StartComp"));
	SetRootComponent(StartComp);

	ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
	ArrowComp->SetupAttachment(StartComp);

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->SetupAttachment(StartComp);

	SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComp"));
	SplineComp->SetupAttachment(StartComp);

	EndComp = CreateDefaultSubobject<USceneComponent>(TEXT("EndComp"));
	EndComp->SetupAttachment(StartComp);
}


void ASZipline::BeginPlay()
{
	Super::BeginPlay();

	SetupArrowComp();
	ConstructSpline();
	ConstructCollider();
}

void ASZipline::ConstructCollider()
{
	float HalfLength = SplineComp->GetSplineLength()/2;

	CapsuleComp->SetCapsuleHalfHeight(HalfLength);


	FTransform TempT = SplineComp->GetTransformAtDistanceAlongSpline(HalfLength, ESplineCoordinateSpace::World);

	FRotator NewRot = TempT.GetRotation().Rotator();
	NewRot.Pitch += 90;

	TempT.SetRotation(NewRot.Quaternion());
	CapsuleComp->SetWorldTransform(TempT);


}

void ASZipline::ConstructSpline()
{
	FOccluderVertexArray Positions = FOccluderVertexArray();
	Positions.Add(FVector::ZeroVector);
	Positions.Add(EndComp->RelativeLocation);

	SplineComp->SetSplinePoints(Positions, ESplineCoordinateSpace::Type::Local, true);
}

void ASZipline::SetupArrowComp()
{
	ArrowComp->SetWorldRotation(EndComp->RelativeLocation.ToOrientationQuat());
}

bool ASZipline::GetDirectionIsForward(FVector TargetForward)
{
	FVector Forward = ArrowComp->GetForwardVector();

	float DP = FVector::DotProduct(Forward, TargetForward);

	if (DP > 0) 
	{
		return true;
	}
	else 
	{
		return false;
	}

}

FVector ASZipline::GetDirection(bool DirectionIsForward)
{
	if (DirectionIsForward)
	{
		return ArrowComp->GetForwardVector();
	}
	else 
	{
		return (-ArrowComp->GetForwardVector());
	}
}

FVector ASZipline::GetTargetLocation(bool DirectionIsForward)
{
	if (DirectionIsForward)
	{
		return SplineComp->GetLocationAtSplinePoint(0,ESplineCoordinateSpace::World);
	}
	else
	{
		return SplineComp->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
	}
}

bool ASZipline::DestinationReached(FVector Target, bool DirectionIsForward, float ReachedDistance)
{
	float Distance = FVector::Distance(Target, GetTargetLocation(DirectionIsForward)) < ReachedDistance;
	UE_LOG(LogTemp, Log, TEXT("Distance to destination: %f"), Distance);
	return Distance;
}

