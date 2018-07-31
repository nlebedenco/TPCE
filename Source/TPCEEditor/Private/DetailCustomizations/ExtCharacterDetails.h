#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "IPropertyChangeListener.h"

class FExtCharacterDetails : public IDetailCustomization 
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of IDetailCustomization interface


};