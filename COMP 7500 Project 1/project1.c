#include<stdio.h>
#include<math.h>
int main()
{
	/* Function to take ten numbers from user and sum the square roots of the numbers and find the average of the square roots */
	int array[10];
	int max=10;
	double sum=0;
	double avg;
	int i;
	printf("\n Enter the 10 numbers:\n");
	for ( i=0;i<max;i++)
	{
	scanf("%d", &array[i]);
        }
	for(i=0;i<max;i++)
	{
	sum+=sqrt(array[i]);
	}
        avg=sum/10.0;
        printf("\n The average of the sum of the square roots =%f\n ", avg);
        return 0;
}	
