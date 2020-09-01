#include <iostream>
#include <random>
#include <ctime>


float* arrWeightAdd(const float *array1, const float array1Weight,
				  const float *array2, const float array2Weight,
				  const int len)
{
	float *arrayResult = new float[len];
	for (int i = 0; i<len; i++)
	{
		arrayResult[i] = array1[i] * array1Weight + array2[i] * array2Weight;
	}

	return arrayResult;
}

float* arrWeightAddIntrinsic(const float *array1, const float array1Weight,
				             const float *array2, const float array2Weight,
				             const int len)
{
	float *arrayResult = new float[len];
	float *arr1Ptr = array1;
	float *arr2Ptr = array2;

	int neonLen = len >> 2;
	int remainLen = len - (neonLen<<2);

	float32x4_t arr1WF4 = vdupq_n_f32(array1Weight);
	float32x4_t arr2WF4 = vdupq_n_f32(array2Weight);


	for (int i = 0; i<neonLen; i++)
	{
		float32x4_t arr1f4 = vldlq_f32(arr1Ptr);
		float32x4_t arr2f4 = vldlq_f32(arr2Ptr);

		arr1f4 = vmulq_f32(arr1f4, arr1WF4);
		arr2f4 = vmulq_f32(arr2f4, arr2WF4);

		float32x4_t resultf4 = vaddq_f32(arr1f4, arr2f4);

		vstlq_f32(arrayResult, resultf4);

		arr1Ptr += 4;
		arr2Ptr += 4;
		arrayResult += 4;
	}

	for(; remainLen>0; remainLen--)
	{
		*arrayResult = (*arr1Ptr) * array1Weight + (*arr2Ptr) * array2Weight;
		arrayResult++;
		arr1Ptr++;
		arr2Ptr++;
	}

	return arrayResult;
}



float* arrayGenerate(const int n, const int rangeL, const int rangeR)
{
	float *arr = new float[n];
	srand(time(NULL));
	for(int i=0; i<n; i++)
	{
		arr[i] = rand()%(rangeR - rangeL + 1) + rangeL + 0.5;
	}
	return arr;
}

void printArray(const float *array, const int n)
{
	for (int i = 0; i < n; ++i)
	{
		printf("%0.2f  ", array[i]);
	}
	printf("\n");
}


int main(int argc, char const *argv[])
{
	clock_t t1, t2;

	float *array1 = arrayGenerate(10000000, 1, 10);
	float *array2 = arrayGenerate(10000000, 5, 30);


	t1 = clock();
	float *arrayResult = arrWeightAdd(array1, 0.3, array2, 0.52, 10000000);
	t2 = clock();
	printf("Time: %0.6f\n", double(t2 -t1)/CLOCKS_PER_SEC);
	//printArray(arrayResult, 1000);
	return 0;
}