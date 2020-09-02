#include <iostream>
#include <random>
#include <ctime>
#include <arm_neon.h>
#include <chrono>

using namespace std;

//for array weight addition
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


//for arry weight addition using arm intrinsic
float* arrWeightAddIntrinsic(const float *array1, const float array1Weight,
				             const float *array2, const float array2Weight,
				             const int len)
{
	float *arrayResult = new float[len];
	const float *arr1Ptr = array1;
	const float *arr2Ptr = array2;

	int neonLen = len >> 2;
	int remainLen = len - (neonLen<<2);

	float32x4_t arr1WF4 = vdupq_n_f32(array1Weight);
	float32x4_t arr2WF4 = vdupq_n_f32(array2Weight);


	for (int i = 0; i<neonLen; i++)
	{
		float32x4_t arr1f4 = vld1q_f32(arr1Ptr);
		float32x4_t arr2f4 = vld1q_f32(arr2Ptr);

		arr1f4 = vmulq_f32(arr1f4, arr1WF4);
		arr2f4 = vmulq_f32(arr2f4, arr2WF4);

		float32x4_t resultf4 = vaddq_f32(arr1f4, arr2f4);

		vst1q_f32(arrayResult, resultf4);

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


//for arry weight addition using arm assembly
float* arrWeightAddAssembly(const float *array1, const float array1Weight,
				             const float *array2, const float array2Weight,
				             const int len)
{
	float *arrayResult = new float[len];
	const float *arr1Ptr = array1;
	const float *arr2Ptr = array2;

	int neonLen = len >> 2;
	int remainLen = len - (neonLen<<2);

	__asm__ volatile(

	    //float32x4_t arr1WF4 = vdupq_n_f32(array1Weight);
	    //float32x4_t arr2WF4 = vdupq_n_f32(array2Weight);
		"vdup.f32      q0, %[array1Weight]       \n"
		"vdup.f32      q1, %[array2Weight]       \n"
		//start do-while loop
		"0:                                      \n"
		//what's the meaning of pld
		"pld           [%[arr1Ptr], #128]        \n"
		//float32x4_t arr1f4 = vld1q_f32(arr1Ptr);
		//{d4-d5} = q2
		"vld1.f32      {d4-d5}, [%[arr1Ptr]]!       \n"
		//float32x4_t arr2f4 = vld1q_f32(arr2Ptr);
		"pld           [%[arr2Ptr], #128]        \n"
		//float32x4_t arr1f4 = vld1q_f32(arr1Ptr);
		//{d6-d7} = q3
		"vld1.f32      {d6-d7}, [%[arr2Ptr]]!       \n"

		//arr1f4 = vmulq_f32(arr1f4, arr1WF4);
		//arr2f4 = vmulq_f32(arr2f4, arr2WF4);
		"vmul.f32      q4, q0, q2       \n"
		"vmul.f32      q5, q1, q3       \n"
		//float32x4_t resultf4 = vaddq_f32(arr1f4, arr2f4);
		"vadd.f32      q6, q4, q5       \n"
		


			)



	for (int i = 0; i<neonLen; i++)
	{
		float32x4_t arr1f4 = vld1q_f32(arr1Ptr);
		float32x4_t arr2f4 = vld1q_f32(arr2Ptr);

		arr1f4 = vmulq_f32(arr1f4, arr1WF4);
		arr2f4 = vmulq_f32(arr2f4, arr2WF4);

		float32x4_t resultf4 = vaddq_f32(arr1f4, arr2f4);

		vst1q_f32(arrayResult, resultf4);

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


//random generate float array
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
	clock_t t1, t2, t3;
	const int size = 80000000;

	float *array1 = arrayGenerate(size, 1, 10);
	float *array2 = arrayGenerate(size, 5, 30);


	t1 = clock();
	auto start_time = chrono::steady_clock::now();
	float *arrayResult = arrWeightAdd(array1, 0.3, array2, 0.52, size);
	t2 = clock();
	auto middle_time = chrono::steady_clock::now();
	float *arrayResult2 = arrWeightAddIntrinsic(array1, 0.3, array2, 0.52, size);
	t3 = clock();
	auto end_time = chrono::steady_clock::now();
	printf("normal time: %0.6f\n", double(t2 -t1)/CLOCKS_PER_SEC);
	printf("instrinsic time: %0.6f\n", double(t3 -t2)/CLOCKS_PER_SEC);
	printf("normal time: %d\n", (middle_time -start_time).count());
	printf("instrinsic time: %d\n", (end_time - middle_time).count());

	//printArray(arrayResult, 1000);
	return 0;
}