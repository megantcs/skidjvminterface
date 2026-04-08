#include "stdio.h"

#include <skidjvminterface.h>

int main(int argc, void* argv) 
{
    JvmProccess Proc;

    if (!ApiFindFirstProcessByTitle(&Proc, "javaw.exe", "Minecraft")) {
        printf("Game not found!");
        abort();
    }

    HotspotContext Context;
    SJStatus Status = ApiNewHotspotContext(Proc, &Context);
    if (Status != SJSuccess) {
        printf("Error create hotspot context: %d\n", Status);
        abort();
    }

    PIJVMINTERFACE JvmInterface;
    Status = ApiNewJvmInterface(&Context, &JvmInterface);

    if (Status != SJSuccess) {
        printf("Error create jvm interface: %d\n", Status);
        abort();
    }

    jclass MinecraftClass = JvmInterface->findClass("net/minecraft/class_310");
    jfieldID MinecraftInstanceId = JvmInterface->findField(MinecraftClass, "field_1700", "Lnet/minecraft/class_310;");

    jobject Minecraft = JvmInterface->getStaticFieldObject(MinecraftClass, MinecraftInstanceId);

    printf("Found Minecraft instance: %p\n", Minecraft);

	JvmProccess proc;
	SJStatus status = ApiFindFirstProcessByTitle(&proc, "javaw.exe", "Minecraft");

	HotspotContext context;
	ApiNewHotspotContext(proc, &context);

	PIJVMINTERFACE jvm;
	ApiNewJvmInterface(&context, &jvm);
   
    while (1) {
        jclass mcClass = jvm->findClass("net/minecraft/class_310");
        if (!mcClass) {
            printf("MinecraftClient class not found\n");
            break;
        }

        jfieldID instanceField = jvm->findField(mcClass, "field_1700", "Lnet/minecraft/class_310;");
        if (!instanceField) {
            printf("Instance field not found\n");
            break;
        }

        jobject mcInstance = NULL;
        void* result = jvm->getStaticFieldObject(mcClass, instanceField);
        if (result) {
            mcInstance = result;
        }

        if (!mcInstance) {
            printf("MinecraftClient instance is null\n");
            break;
        }

        jfieldID id = jvm->findField(mcClass, "field_1738", "I");
        printf("FPS: %d\n", jvm->getStaticFieldInt(mcClass, id));

        Sleep(1000); 
    }

}