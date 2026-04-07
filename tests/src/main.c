#include "stdio.h"

#include <skidjvminterface.h>

int main(int argc, void* argv) 
{
	JvmProccess proc;
	SJStatus status = ApiFindFirstProcessByTitle(&proc, "javaw.exe", "Minecraft");


	HotspotContext context;
	status = ApiNewHotspotContext(&proc, &context);
	printf("Loaded structs %d\n", context._vmStructs.size);
	for (int i = 0; i < context._vmStructs.size; i++) {
		printf("%s\n", context._vmStructs.data[i].fieldName);
	}
}