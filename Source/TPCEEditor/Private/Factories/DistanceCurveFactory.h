// Fill copyright 

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "ExtraEditorTypes.h"

#include "DistanceCurveFactory.generated.h"

class UAnimSequence;

/**
 * This is basically a ripoff of UCurveFactory and UCurveFloatFactory.
 * It has to derive from UFactory instead of UCurveFloatFactory because some "genius" in Epic decided that Curve Factories had be flagged with MinimalAPI
 * rendering their virtual methods impossible to override.
 */
UCLASS()
class UDistanceCurveFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	virtual bool ConfigureProperties() override { return true; }
	virtual bool ShouldShowInNewMenu() const override { return false; }
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	virtual float GetValue(const float Time);

public:

	/** Animation Sequence to use */
	UPROPERTY(EditAnywhere, Category = DistanceCurveFactory)
	UAnimSequence* AnimSequence;

	/** Distance curve type to generate. */
	UPROPERTY(EditAnywhere, Category = DistanceCurveFactory)
	EDistanceCurveType DistanceCurveType;
};
