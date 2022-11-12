#include <stdio.h>

int main(int argc, char** argv)
{
    puts("Hello, World!");
    puts("Here are some of the arguments passed to this program:");
    
    for (int i = 0; i < argc; i++)
    {
        printf("\t%d\n", argv[i]);
    }
    
    puts("That's all.");
    
    return 0;
}