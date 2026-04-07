#include "stdio.h"

#include <skidjvminterface.h>

int main(int argc, void* argv) 
{
	JvmProccess proc;
	SJStatus status = ApiFindFirstProcessByTitle(&proc, "javaw.exe", "Minecraft");


	printf("%d \n", status);
}