#include <stdio.h>
#define N 10

int input_n(int *n)
{
    printf("Enter number of elements: ");
    if(scanf("%d", n) != 1)
    {
        printf("Error number of elements\n");
        return -1;
    }
    return 0;
}

int input_array(int *arr, const int n)
{
    for (int i = 0; i < n; i++)
    {
        printf("Enter %d-th element(interger): ", i + 1);
        if (scanf("%d", &arr[i]) != 1)
        {
            printf("Error number\n");
            return -1;
        }
    }
    return 0;
}

int sum_array(int *arr, const int n)
{
    int sum = 0;
    for(int i = 0; i < n; i++)
        sum += arr[i];
    return sum;
}
int main(void)
{
    int a[N];
    int n;
    int rc;
    rc = input_n(&n);
    if (rc)
        return -1;
    rc = input_array(a, n);
    if (rc)
        return -1;
    printf("Sum = : %d\n", sum_array(a, n));
    return 0;
}