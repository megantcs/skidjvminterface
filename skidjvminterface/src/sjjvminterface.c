#include "../includes/skidjvminterface.h"

SJStatus ApiNewJvmInterface(In_ PHotspotContext Context, 
							Out_ PIJVMINTERFACE* Interface)
{
	PIJVMINTERFACE OutInterface = malloc(sizeof(IJVMINTERFACE));

	*Interface = OutInterface;
	return SJSuccess;
}