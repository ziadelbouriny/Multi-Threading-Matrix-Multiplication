#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>

int **createMat(int rows, int cols);
void *matMulPerEle(void *params);
void *matMulPerRow(void *params);
void *matMul();
void readArgs(int argc, char *argv[]);
void readFromFile(char *path1, char *path2);
void setOutFile(int argc, char *argv[]);
void writeInFile(char *path, int method);
void threadPerEle();
void threadPerRow();
void threadPerMat();

int **mat1;
int **mat2;
int **res_mul;
int **res_row;
int **res_ele;

int r1, c1, r2, c2;

typedef struct str
{
	int i;
	int j;
}param;



int main(int argc, char *argv[])
{
    readArgs(argc, argv);
    
    threadPerEle();
    
    threadPerRow();
    
    threadPerMat();
    
    setOutFile(argc, argv);	
    
    
    //GARBAGE COLLECTOR
    for(int i = 0 ; i < r1 ; i++)
    {
        free(mat1[i]);
        free(res_mul[i]);
        free(res_row[i]);
        free(res_ele[i]);
    }
    
    free(mat1);
    free(res_mul);
    free(res_row);
    free(res_ele);    
        
    for(int i = 0 ; i < r2 ; i++)
    	free(mat2[i]); 
    	
    free(mat2);	
    	
    return 0;
}

void readArgs(int argc, char *argv[])
{
    if(argc == 1)
    {
     	readFromFile("a.txt", "b.txt");
    }
    else
    {
        char* in1 = (char*)malloc(sizeof(char) * 20);
        char* in2 = (char*)malloc(sizeof(char) * 20);
        
        strcpy(in1 , argv[1]);
        strcpy(in2 , argv[2]);
        
        strcat(in1 , ".txt");
        strcat(in2 , ".txt");
  
        readFromFile(in1, in2);
    
        free(in1);
        free(in2);
    }
}

void readFromFile(char *path1, char *path2)
{
	FILE *f1;
        
        f1 = fopen(path1, "r");
        
        if (f1 == NULL)
    	{
        	printf("File name entered doesn't exist in this directory!\n");
        	exit(0);
    	}
    	
    	if (fscanf(f1, "row=%d col=%d", &r1, &c1) != 2)
    	{
        	printf("Error reading dimensions\n");
        	fclose(f1);
        	return;
    	}
        
    	mat1 = createMat(r1, c1);
    
    	for (int i = 0; i < r1; i++)
    	{
        	for (int j = 0; j < c1; j++)
        	    fscanf(f1, "%d", &mat1[i][j]);
    	}
    
    	fclose(f1);
    	
    	// READING 2ND MATRIX
    	
    	FILE *f2;
    	
    	f2 = fopen(path2, "r");
     
    	if (f2 == NULL)
    	{
        	printf("File name entered doesn't exist in this directory!\n");
        	exit(0);
    	}
    	
    	if (fscanf(f2, "row=%d col=%d", &r2, &c2) != 2)
    	{
    	    printf("Error reading dimensions\n");
    	    fclose(f2);
    	    return;
    	}
    	
    	mat2 = createMat(r2, c2);
    
    	for (int i = 0; i < r2; i++)
    	{
    	   for (int j = 0; j < c2; j++) 
    	        fscanf(f2, "%d", &mat2[i][j]);
    	}
    	
    	if(c1 != r2)
    	{
        	printf("Error in dimensions\n");
        	exit(0);
    	}
    
    	res_mul = createMat(r1, c2);
    	res_row = createMat(r1, c2);
    	res_ele = createMat(r1, c2);
    	
    	fclose(f2);	
}

int **createMat(int rows, int cols)
{
	int **arr = (int **)malloc(rows * sizeof(int *));
	
	for (int i = 0; i < rows; i++)
		arr[i] = (int *)malloc(cols * sizeof(int));
		
	return arr;	
}

void threadPerEle()
{
	struct timeval stop , start;
    	gettimeofday(&start , NULL);
    	
	pthread_t threads[r1 * c2]; 
	int index = 0;
    
    	for (int i = 0; i < r1; i++)
    	{
        	for (int j = 0; j < c2; j++)
        	{
        	    param *p = malloc(sizeof(param));
        	    p -> i = i;
        	    p -> j = j;
    
        	    pthread_create(&threads[index++], NULL, &matMulPerEle, p);
        	}
    	}
    
    	for (int i = 0 ; i < r1 * c2; i++)
        	pthread_join(threads[i], NULL);
        	
        gettimeofday(&stop , NULL);
    	printf("Time taken per element method: %lu microsecond\n", stop.tv_usec - start.tv_usec);
    	printf("Number of threads created in per element method: %d\n\n", r1 * c2);	
}  

void threadPerRow()
{
	struct timeval stop , start ;
    	gettimeofday(&start , NULL);
    
	pthread_t threads[r1]; 
	int index = 0;
    
    	for (int i = 0; i < r1; i++)
    	{
        	param *p = malloc(sizeof(param));
        	p -> i = i;
    
        	pthread_create(&threads[i], NULL, &matMulPerRow, p);
    	}
    
    	for (int i = 0 ; i < r1; i++)
        	pthread_join(threads[i], NULL);
        	
        gettimeofday(&stop , NULL);
    	printf("Time taken per row method: %lu microsecond\n", stop.tv_usec - start.tv_usec);
    	printf("Number of threads created in per row method: %d\n\n", r1);	
}

void threadPerMat()
{
	struct timeval stop , start;
    	gettimeofday(&start , NULL);
    	
	pthread_t threads ;
    
    	pthread_create(&threads , NULL , &matMul , NULL);	
   	pthread_join(threads , NULL);
   	
   	gettimeofday(&stop , NULL);
    	printf("Time taken per matrix method: %lu microsecond\n", stop.tv_usec - start.tv_usec);
    	printf("Number of threads created in per matrix method: 1\n\n");
}    

void *matMulPerRow(void *params)
{
	param resPar = *(param*)params;
	
	int element = 0;
	
	for(int j = 0 ; j < c2 ; j++)
	{
        	for(int k = 0 ; k < c1 ; k++)
            		element+= mat1[resPar.i][k] * mat2[k][j];
            		
        	res_row[resPar.i][j] = element;
        	element = 0;
        }
    	free(params);
}	         	

void *matMulPerEle(void *params)			
{
	param resPar = *(param*)params;
	
	int element = 0;
	
        for (int k = 0; k < c1; k++)
        {
        	element += mat1[resPar.i][k] * mat2[k][resPar.j];
        }
        
        res_ele[resPar.i][resPar.j] = element;
        
        free(params);		
}

void *matMul()
{
	int element = 0;
	
    	for(int i = 0 ; i < r1 ; i++)
    	{
        	for(int j = 0 ; j < c2 ; j++)
        	{
        	  	for(int k = 0 ; k < c1 ; k++)
        	    		element += mat1[i][k] * mat2[k][j];
        	    		
        	  	res_mul[i][j] = element;
        	  	element = 0;
        	}
    	}	
}

void writeInFile(char *path, int method)
{
	FILE * f = fopen(path , "w");
	if(f == NULL)
	{
		printf("Error opening file\n");
		exit(0);
	} 
	
	switch(method)
	{
		case 1:
			fprintf(f, "Method: A thread per matrix\nrow=%d col=%d\n", r1, c2);
        		for(int i = 0 ; i < r1 ; i++)
        		{
        	    		for(int j = 0 ; j < c2 ; j++)
        	        		fprintf(f, "%d  ", res_mul[i][j]);
        	    
        	    		fprintf(f, "\n");
        		}
        		break;
			
		case 2:
			fprintf(f, "Method: A thread per matrix\nrow=%d col=%d\n", r1, c2);
        		for(int i = 0 ; i < r1 ; i++)
        		{
        	    		for(int j = 0 ; j < c2 ; j++)
        	        		fprintf(f, "%d  ", res_mul[i][j]);
        	    
        	    		fprintf(f, "\n");
        		}
        		break;
	
        	case 3:
        		fprintf(f, "Method: A thread per element\nrow=%d col=%d\n", r1, c2);
        		for(int i = 0 ; i < r1 ; i++)
        		{
        	    		for(int j = 0 ; j < c2 ; j++)
        	    			fprintf(f, "%d  ", res_ele[i][j]);
        	    	
        	    		fprintf(f, "\n");
        		}
    		        break;
    		        
    		default:
        		exit(0);
        }		
    	
    	fclose(f);
}    

void setOutFile(int argc, char *argv[])
{
	if (argc == 1)
	{
		writeInFile("c_per_matrix.txt", 1);
        	writeInFile("c_per_row.txt", 2);
        	writeInFile("c_per_element.txt", 3);	
        }
        else
        {
        	char *file_1 = malloc(sizeof(char) * 16);
        	char *file_2 = malloc(sizeof(char) * 16);
        	char *file_3 = malloc(sizeof(char) * 17);
        	
        	strcpy(file_1 , argv[3]);
        	strcat(file_1 , "_per_matrix.txt");
        	writeInFile(file_1 , 1);
       		free(file_1);
       		
        	strcpy(file_2 , argv[3]);
        	strcat(file_2 , "_per_row.txt");
        	writeInFile(file_2 , 2);
        	free(file_2);
        	
        	strcpy(file_3 , argv[3]);
        	strcat(file_3 , "_per_element.txt");
        	writeInFile(file_3 , 3);
        	free(file_3);	
        }	
}			
