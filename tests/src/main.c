#include "stdio.h"

#include <skidjvminterface.h>

int main(int argc, void* argv)
{
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

        jobject mcInstance = jvm->getStaticFieldObject(mcClass, instanceField);
        jfieldID worldField = jvm->findField(jvm->getObjectClass(mcInstance), "field_1687", "Lnet/minecraft/class_638;");
        jclass mcInstanceClass = jvm->getObjectClass(mcInstance);

        printf("INSTANCE = %p ; WORLD_ID = %d, WORLD = %p\n", mcInstance, worldField, jvm->getObjectField(mcInstance, worldField));

        if (!mcInstance) {
            printf("MinecraftClient instance is null\n");
            break;
        }

        jfieldID id = jvm->findField(mcClass, "field_1738", "I");
    }

}