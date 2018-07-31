#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "IPropertyChangeListener.h"

class FExtCharacterMovementDetails : public IDetailCustomization 
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of IDetailCustomization interface


};