#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define N 8

/*复数结构体类型*/
typedef struct{
double realPart;
double imaginaryPart;
}complexNumber;

complexNumber x[N], W[N/2]; //序列数组以及旋转因子数组
int size_x=N;
double PI=atan(1)*4;        //圆周率
void fastFourierOperation();//快速傅里叶变换算法 
void initRotationFactor();  //生成旋转因子数组 
void changePosition();      //偶奇变换算法 
void outputArray();         //遍历输出数组
void add(complexNumber ,complexNumber ,complexNumber*); //复数加法 
void mul(complexNumber ,complexNumber ,complexNumber*); //复数乘法 
void sub(complexNumber ,complexNumber ,complexNumber*); //复数减法 
/*复数加法的定义*/ 
void add(complexNumber a,complexNumber b,complexNumber *c)
{
	c->realPart=a.realPart+b.realPart;
	c->imaginaryPart=a.imaginaryPart+b.imaginaryPart;
}

/*复数乘法的定义*/ 
void mul(complexNumber a,complexNumber b,complexNumber *c)
{
	c->realPart=a.realPart*b.realPart - a.imaginaryPart*b.imaginaryPart;
	c->imaginaryPart=a.realPart*b.imaginaryPart + a.imaginaryPart*b.realPart;
}

/*复数减法的定义*/ 
void sub(complexNumber a,complexNumber b,complexNumber *c)
{
	c->realPart=a.realPart-b.realPart;
	c->imaginaryPart=a.imaginaryPart-b.imaginaryPart;
}
/*长度为1024的冲激序列 */
void init1()                
{
	printf("Example：度为1024的冲激序列\n");
	x[0].realPart=1;
	x[0].imaginaryPart=0;
	for(int i=1;i<N;i++)
	{
		x[i].realPart=0;
		x[i].imaginaryPart=0;
	}
}

/*长度为1024的矩形序列 */
void init2()               
{               
	printf("Example：长度为1024的矩形序列\n"); 
	for(int i=0;i<N;i++)
	{
		x[i].realPart=1;
		x[i].imaginaryPart=0;
	}
}

/*长度为1024的sin[2πk/1024]序列 */
void init3()               
{             
	printf("Example：长度为1024的sin[2πk/1024]序列\n"); 
	for(int i=0;i<N;i++)
	{
		x[i].realPart=sin(2*PI*i/N);
		x[i].imaginaryPart=0;
	}
}

/*长度为1024的cos[2πk/1024]序列 */
void init4()              
{             
	printf("Example：长度为8的cos[2πk/1024]序列\n");
	for(int i=0;i<N;i++)
	{
		x[i].realPart=cos(2*PI*i/N);
		x[i].imaginaryPart=0;
	}
}


/*快速傅里叶变换*/
void fastFourierOperation()
{
	int l=0;
	complexNumber up,down,product;
	changePosition();                            //偶奇变换算法
	for(int i=0;i<log(size_x)/log(2);i++)        //一级蝶形运算
	{   
		l=1<<i;
		for(int j=0;j<size_x;j+= 2*l )          //一组蝶形运算
		{            
			for(int k=0;k<l;k++)                //一个蝶形运算
			{       
				mul(x[j+k+l],W[size_x*k/2/l],&product);
				add(x[j+k],product,&up);
				sub(x[j+k],product,&down);
				x[j+k]=up;
				x[j+k+l]=down;
			}
		}
	}
}


/*生成旋转因子数组*/
void initRotationFactor() 
{
	for(int i=0;i<size_x/2;i++)
	{
		W[i].realPart=cos(2*PI/size_x*i);//用欧拉公式计算旋转因子
		W[i].imaginaryPart=-1*sin(2*PI/size_x*i);
	}
}


/*偶奇变换算法*/
void changePosition()      
{
	int j=0,k;//待会儿进行运算时需要用到的变量 
	complexNumber temp;
	for (int i=0;i<size_x-1;i++) 
	{ 
		if(i<j) 
		{
			//若倒序数大于顺序数，进行变址（换位）；否则不变，防止重复交换，变回原数
			temp=x[i];
			x[i]=x[j];
			x[j]=temp;
		}
		k=size_x/2;    //判断j的最高位是否为0
		while(j>=k) 
		{              //最高位为1
			j=j-k;
			k=k/2;
		}
		j=j+k;        //最高位为0
	}
	printf("\n\n=============================================================================================================================================================================================\n");
	printf("输出倒序后的结果：\n");
	outputArray();
}


/*遍历输出数组*/
void outputArray()
{
	int num=0;
	for(int i=0;i<size_x;i++)
	{		
	
		if(x[i].imaginaryPart<0)
		{
			printf("%.4f-j%.4f     ",x[i].realPart,fabs(x[i].imaginaryPart));
		}
		else
		{
			printf("%.4f+j%.4f     ",x[i].realPart,x[i].imaginaryPart);
		}
		num++;
		if(num==10){
			printf("\n");
			num=0;
		}
	}
}
int main()
{
    init4();               //调用不同的初始化函数，使用不同的例程进行测试 
	printf("=============================================================================================================================================================================================\n");
	printf("输出原序列：\n");
	outputArray();
	initRotationFactor();  //生成旋转因子数组 
	fastFourierOperation();//调用快速傅里叶变换
	printf("\n\n=============================================================================================================================================================================================\n");
	printf("输出FFT后的结果：\n");
	outputArray();         //遍历输出数组
	return 0;
}
