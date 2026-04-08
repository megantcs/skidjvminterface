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
        jclass mcClass = jvm->findClass("flk");
        if (!mcClass) {
            printf("MinecraftClient class not found\n");
            break;
        }
        printf("MCKLASS without opbjectclass %p \n", mcClass);

        jfieldID instanceField = jvm->findField(mcClass, "F", "Lflk;");
        if (!instanceField) {
            printf("Instance field not found\n");
            break;
        }
        jobject mcInstance = jvm->getStaticFieldObject(mcClass, instanceField);
        jfieldID worldField = jvm->findField(jvm->getObjectClass(mcInstance), "s", "Lgga;");
        jclass mcInstanceClass = jvm->getObjectClass(mcInstance);

        printf("INSTANCE = %p ; WORLD_ID = %d, WORLD = %p\n", mcInstance, worldField, jvm->getObjectField(mcInstance, worldField));

        if (!mcInstance) {
            printf("MinecraftClient instance is null\n");
            break;
        }

        jclass mc_klass = jvm->getObjectClass(mcInstance);
        printf("Pointer mcklass %p \n", mc_klass);


        jfieldID id = jvm->findField(mcClass, "bf", "I");
    }

}