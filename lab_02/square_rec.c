#include <stdio.h>

int main(void)
{
    double length, width;
    printf("Enter length: ");
    if (scanf("%lf", &length) != 1)
    {
        printf("Error length\n");
        return -1;
    }
    printf("Enter width: ");
    if (scanf("%lf", &width) != 1)
    {
        printf("Error width\n");
        return -1;
    }
    printf("Square of rectangle (length = %.3lf, width = %.3lf)  = %.3lf", length, width, length * width);
    return 0;
}