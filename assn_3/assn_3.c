#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>

#define INPUT_BUF 1000
#define OUTPUT_BUF 1000
#define HEAP_SIZE 750
#define REP_INPUT_BUF 250

int inputBuffer[INPUT_BUF];
int outputBuffer[OUTPUT_BUF];
struct timeval starttime, endtime;
long sec,msec;
int primaryheap[750];
int secondaryheap[250];
int minElement=INT_MAX,elementPos,indexOut=0,i=0,runNumber;

//int inputBuffer[10]={16,47,5,12,67,21,7,17,14,58};

void basicMergeSort(const char*,const char*);
void multiStepMerge(const char*,const char*);
void replacementMerge(const char*,const char*);
int comparatorAscen(const void*,const void *);
int minimumElement();
void printResults(struct timeval,struct timeval);
void heapify(int,int);
void sift(int,int);
void swap(int,int);

int main(int argc,char* argv[])
{
	if(argc!=4){
		exit(-1);
	}
	else
	{
		if(strcmp(argv[1],"--basic")==0){
			basicMergeSort(argv[2],argv[3]);
		} 
		else if(strcmp(argv[1],"--multistep")==0){
			multiStepMerge(argv[2],argv[3]);
		}
		else if(strcmp(argv[1],"--replacement")==0){
			replacementMerge(argv[2],argv[3]);
		}
		else
			exit(-1);
	}
}

//basic merge sort
void basicMergeSort(const char* inFile,const char* outFile)
{
	FILE *fp,*out,*temp;//pointer for the input file
	FILE **run;
	int totkeys,intRead,bufRun=0;

	gettimeofday(&starttime,NULL);

	fp=fopen(inFile,"r+b");

	//get the number of integer keys in the file
	fseek(fp,0L,SEEK_END);
	totkeys=ftell(fp)/sizeof(int);
	fseek(fp,0L,SEEK_SET);

	if(totkeys<1000)
	{
		char buf[5]="",fileName[40]="";
		strcat(fileName,inFile);
		strcat(fileName,".");
		sprintf(buf,"%0.03d",i);
		strcat(fileName,buf);

		fread(inputBuffer,totkeys,sizeof(int),fp);
		qsort(inputBuffer,totkeys,sizeof(int),comparatorAscen);
		temp=fopen(fileName,"w+b");
		out=fopen(outFile,"w+b");
		fwrite(inputBuffer,sizeof(int),totkeys,temp);
		fwrite(inputBuffer,sizeof(int),totkeys,out);
		fclose(temp);
	}
	else
	{
		intRead=totkeys/INPUT_BUF;

		//printf("keys: %d\n",totkeys);
		run=malloc(intRead * sizeof(FILE*));
		//read from file,sort and put into run files
		for(i=0;i<intRead;i++)
		{
			fread(inputBuffer,INPUT_BUF,sizeof(int),fp);
			qsort(inputBuffer,INPUT_BUF,sizeof(int),comparatorAscen);

			//creating the file names for the runs
			char buf[5]="",fileName[40]="";
			strcat(fileName,inFile);
			strcat(fileName,".");
			sprintf(buf,"%0.03d",i);
			strcat(fileName,buf);

			run[i]=fopen(fileName,"w+b");
			fwrite(inputBuffer,sizeof(int),INPUT_BUF,run[i]);
			fseek(run[i],0L,SEEK_SET);
		}

		fclose(fp);

		//resetting input buffer to avoid conflicts
		memset(inputBuffer,INT_MAX,INPUT_BUF);

		//need to merge runs now
		bufRun=INPUT_BUF/intRead;//number of elements to buffer from each run

		for(i=0;i<intRead;i++)
		{
			fread(inputBuffer+(i*bufRun),bufRun,sizeof(int),run[i]);
		}

		out=fopen(outFile,"w+b");

		do{

			elementPos=minimumElement();
			minElement=inputBuffer[elementPos];

			//all elements have been exhausted
			if(minElement==INT_MAX)
			{
				fwrite(outputBuffer,sizeof(int),indexOut,out);
				break;
			}
			
			outputBuffer[indexOut]=inputBuffer[elementPos];//send it to output buffer
			indexOut++;
			inputBuffer[elementPos]=INT_MAX; //need to reset to max value

			if((elementPos+1)%bufRun==0)
			{
				runNumber=elementPos/bufRun;
				fread(inputBuffer+(runNumber*bufRun),bufRun,sizeof(int),run[runNumber]);
			}

			//if my output buffer is full
			if(indexOut==OUTPUT_BUF)
			{
				fwrite(outputBuffer,sizeof(int),OUTPUT_BUF,out);
				indexOut=0;//reset the index in output buffer
			}

		}while(minElement!=INT_MAX);

		//close all run files
		for(i=0;i<intRead;i++)
		{
			fclose(run[i]);
		}
	}//end of else
	fclose(out);

	gettimeofday(&endtime,NULL);
	printResults(starttime,endtime);
}

//multi-step merge 
void multiStepMerge(const char* inFile,const char* outFile)
{
	FILE *fp,*out,*temp;//pointer for the input file
	FILE **run,**super;
	int totkeys,intRead,bufRun=0,superRuns=0,superBufRun=0;

	gettimeofday(&starttime,NULL);

	fp=fopen(inFile,"r+b");

	//get the number of integer keys in the file
	fseek(fp,0L,SEEK_END);
	totkeys=ftell(fp)/sizeof(int);
	fseek(fp,0L,SEEK_SET);

	if(totkeys<1000)
	{
		char buf[5]="",fileName[40]="";
		strcat(fileName,inFile);
		strcat(fileName,".");
		sprintf(buf,"%0.03d",i);
		strcat(fileName,buf);

		fread(inputBuffer,totkeys,sizeof(int),fp);
		qsort(inputBuffer,totkeys,sizeof(int),comparatorAscen);
		temp=fopen(fileName,"w+b");
		out=fopen(outFile,"w+b");
		fwrite(inputBuffer,sizeof(int),totkeys,temp);
		fwrite(inputBuffer,sizeof(int),totkeys,out);
		fclose(temp);

		char superbuf[5]="",superfileName[40]="";
		strcat(superfileName,inFile);
		strcat(superfileName,".super.");
		sprintf(superbuf,"%0.03d",i);
		strcat(superfileName,superbuf);

		temp=fopen(superfileName,"w+b");
		fwrite(inputBuffer,sizeof(int),totkeys,temp);
		fclose(temp);
	}
	else
	{
		intRead=totkeys/INPUT_BUF;

		//printf("keys: %d\n",totkeys);
		run=malloc(intRead * sizeof(FILE*));

		//read from file,sort and put into run files
		for(i=0;i<intRead;i++)
		{
			fread(inputBuffer,INPUT_BUF,sizeof(int),fp);
			qsort(inputBuffer,INPUT_BUF,sizeof(int),comparatorAscen);

			//creating the file names for the runs
			char buf[5]="",fileName[40]="";
			strcat(fileName,inFile);
			strcat(fileName,".");
			sprintf(buf,"%0.03d",i);
			strcat(fileName,buf);

			run[i]=fopen(fileName,"w+b");
			fwrite(inputBuffer,sizeof(int),INPUT_BUF,run[i]);
			fseek(run[i],0L,SEEK_SET);
		}

		fclose(fp);
	
		memset(inputBuffer,INT_MAX,INPUT_BUF);

		//calculate the value of superruns
		superRuns=(intRead%15==0)?(intRead/15):((intRead/15)+1);
		superBufRun=(INPUT_BUF/15);//number of elements to buffer from each run for super-run

		super=malloc(superRuns * sizeof(FILE*));

		//start merging super runs

		int j;
		for(j=0;j<superRuns;j++)
		{
			int temp=((j*15+15)>intRead)?intRead:(j*15+15);

			for(i=j*15;i<temp;i++)
			{
				fread(inputBuffer+((i-j*15)*superBufRun),superBufRun,sizeof(int),run[i]);
			}

			char superbuf[5]="",superfileName[40]="";
			strcat(superfileName,inFile);
			strcat(superfileName,".super.");
			sprintf(superbuf,"%0.03d",j);
			strcat(superfileName,superbuf);

			super[j]=fopen(superfileName,"w+b");

			do{

				elementPos=minimumElement();
				minElement=inputBuffer[elementPos];

				//all elements have been exhausted
				if(minElement==INT_MAX)
				{
					fwrite(outputBuffer,sizeof(int),indexOut,super[j]);
					indexOut=0;
					int k;

					for(k=j*15;k<temp;k++)
					{
						fclose(run[k]);
					}

					break;
				}

				outputBuffer[indexOut]=inputBuffer[elementPos];//send it to output buffer
				indexOut++;
				inputBuffer[elementPos]=INT_MAX; //need to reset to max value

				if((elementPos+1)%superBufRun==0)
				{
					runNumber=elementPos/superBufRun;
					fread(inputBuffer+(runNumber*superBufRun),superBufRun,sizeof(int),run[runNumber+(j*15)]);
				}

				//if my output buffer is full
				if(indexOut==OUTPUT_BUF)
				{
					fwrite(outputBuffer,sizeof(int),OUTPUT_BUF,super[j]);
					indexOut=0;//reset the index in output buffer
				}

			}while(minElement!=INT_MAX);
		}

		//reset the seek pointer in all super run files
		for(i=0;i<superRuns;i++)
		{
			fseek(super[i],0L,SEEK_SET);
		}

		//super runs created - now need to merge super runs
		int readFromSuper=INPUT_BUF/superRuns;

		for(i=0;i<superRuns;i++)
		{
			fread(inputBuffer+(i*readFromSuper),readFromSuper,sizeof(int),super[i]);
		}

		out=fopen(outFile,"w+b");

		do{

			elementPos=minimumElement();
			minElement=inputBuffer[elementPos];

			//all elements have been exhausted
			if(minElement==INT_MAX)
			{
				fwrite(outputBuffer,sizeof(int),indexOut,out);
				break;
			}
		
			outputBuffer[indexOut]=inputBuffer[elementPos];//send it to output buffer
			indexOut++;
			inputBuffer[elementPos]=INT_MAX; //need to reset to max value

			if((elementPos+1)%readFromSuper==0)
			{
				runNumber=elementPos/readFromSuper;
				fread(inputBuffer+(runNumber*readFromSuper),readFromSuper,sizeof(int),super[runNumber]);
			}

			//if my output buffer is full
			if(indexOut==OUTPUT_BUF)
			{
				fwrite(outputBuffer,sizeof(int),OUTPUT_BUF,out);
				indexOut=0;//reset the index in output buffer
			}

		}while(minElement!=INT_MAX);

		for(i=0;i<superRuns;i++)
		{
			fclose(super[i]);
		}
	}//end of else

	fclose(out);

	gettimeofday(&endtime,NULL);
	printResults(starttime,endtime);
}

void replacementMerge(const char* inFile,const char* outFile)
{
	FILE *fp,*out,**run;
	int heapsize=HEAP_SIZE-1;
	int totkeys=0,bufferpos=0,newheapsize=0,counter=0,bufRun=0,intRead;
	int newheapindex=heapsize;
	int flag=0;

	char buf[5]="",fileName[40]="";
	strcat(fileName,inFile);
	strcat(fileName,".");
	sprintf(buf,"%0.03d",counter);
	strcat(fileName,buf);

	run=malloc(1000 * sizeof(FILE *));

	gettimeofday(&starttime,NULL);

	fp=fopen(inFile,"r+b");

	//number of keys in file
	fseek(fp,0L,SEEK_END);
	totkeys=ftell(fp)/sizeof(int);
	fseek(fp,0L,SEEK_SET);

	//if input file contains keys lesser than 750
	if(totkeys<=750)
	{
		fread(primaryheap,sizeof(int),totkeys,fp);
		heapify(totkeys,0);
		while(totkeys>0)
		{
			outputBuffer[indexOut]=primaryheap[0];
			primaryheap[0]=primaryheap[totkeys-1];
			totkeys--;
			sift(0,totkeys);
			indexOut++;
		}

		run[counter]=fopen(fileName,"w+b");
		out=fopen(outFile,"w+b");
		fwrite(outputBuffer,sizeof(int),indexOut,run[counter]);
		fwrite(outputBuffer,sizeof(int),indexOut,out);
	}

	else if(totkeys>750 && totkeys<1000)
	{	
		//total keys less than 1000
		flag=1;
		fread(primaryheap,sizeof(int),HEAP_SIZE,fp);
		totkeys-=HEAP_SIZE;

		heapify(HEAP_SIZE,0);

		fread(secondaryheap,sizeof(int),totkeys,fp);

		while(bufferpos!=totkeys)
		{
			outputBuffer[indexOut]=primaryheap[0];
			indexOut++;	

			if(primaryheap[0]<=secondaryheap[bufferpos])
			{
				primaryheap[0]=secondaryheap[bufferpos];
				bufferpos++;
				sift(0,heapsize+1);
			}
			else if(primaryheap[0]>secondaryheap[bufferpos])
			{
				primaryheap[0]=primaryheap[heapsize];
				primaryheap[heapsize]=secondaryheap[bufferpos];
				heapsize--;
				bufferpos++;
				newheapsize++;
				sift(0,heapsize+1);
			}	
		}

		run[counter]=fopen(fileName,"w+b");
		//printf("indexOut %d\n",indexOut);
		if(indexOut>0)
		{
			fwrite(outputBuffer,sizeof(int),indexOut,run[counter]);
			indexOut=0;
		}

		newheapindex=heapsize+1;
	
		if(heapsize!=0)
		{
			heapify(heapsize+1,0);
			while(heapsize>=0)
			{
				outputBuffer[indexOut]=primaryheap[0];
				primaryheap[0]=primaryheap[heapsize];
				sift(0,heapsize);
				heapsize--;
				indexOut++;
			}

			fwrite(outputBuffer,sizeof(int),indexOut,run[counter]);
			indexOut=0;
		}

		if(newheapsize>=0)
		{
			int n=HEAP_SIZE-1;
			counter++;
			char buf[5]="",fileName[40]="";
			strcat(fileName,inFile);
			strcat(fileName,".");
			sprintf(buf,"%0.03d",counter);
			strcat(fileName,buf);
			run[counter]=fopen(fileName,"w+b");
			indexOut=0;

			int j=0,k=0;
			for(i=n;i>=newheapindex;i--)
			{
				primaryheap[j]=primaryheap[i];
				j++;
				k++;
			}

			j--;
			k--;

			heapify(j+1,0);
			while(j>=0)
			{
				outputBuffer[indexOut]=primaryheap[0];
				primaryheap[0]=primaryheap[j];
				sift(0,j);
				j--;
				indexOut++;
			}

			fwrite(outputBuffer,sizeof(int),indexOut,run[counter]);
			indexOut=0;
		}
	}
	else
	{
		flag=1;
		fread(primaryheap,sizeof(int),HEAP_SIZE,fp);
		totkeys-=HEAP_SIZE;

		heapify(HEAP_SIZE,0);

		fread(secondaryheap,sizeof(int),REP_INPUT_BUF,fp);
		totkeys-=REP_INPUT_BUF;

		run[counter]=fopen(fileName,"w+b");

		while(1)
		{
			outputBuffer[indexOut]=primaryheap[0];
			indexOut++;

			if(indexOut==OUTPUT_BUF)
			{
				fwrite(outputBuffer,sizeof(int),OUTPUT_BUF,run[counter]);
				indexOut=0;
			}

			//perform the check
			if(primaryheap[0]<=secondaryheap[bufferpos])
			{
				primaryheap[0]=secondaryheap[bufferpos];
				bufferpos++;
				sift(0,heapsize+1);
			}
			else if(primaryheap[0]>secondaryheap[bufferpos])
			{
				primaryheap[0]=primaryheap[heapsize];
				primaryheap[heapsize]=secondaryheap[bufferpos];
				heapsize--;
				bufferpos++;
				newheapsize++;
				sift(0,heapsize+1);
				//printf("%d\n",newheapsize);
			}
		
			if(bufferpos==REP_INPUT_BUF)
			{
				//printf("newheapsize %d\n",newheapsize);
				int ret=fread(secondaryheap,sizeof(int),REP_INPUT_BUF,fp);
				if(feof(fp) || ret==0 || totkeys==0)
				{
					break;
				}
				else
				{
					totkeys-=ret;
					bufferpos=0;
				}
			}

			if(newheapsize==HEAP_SIZE)
			{
				fwrite(outputBuffer,sizeof(int),indexOut,run[counter]);
 				indexOut=0;
				counter++;
				newheapsize=0;
				heapsize=HEAP_SIZE-1;
				heapify(HEAP_SIZE,0);
				char buf[5]="",fileName[40]="";
				strcat(fileName,inFile);
				strcat(fileName,".");
				sprintf(buf,"%0.03d",counter);
				strcat(fileName,buf);
				run[counter]=fopen(fileName,"w+b");
			}
		}

		//printf("indexOut %d\n",indexOut);
		if(indexOut>0)
		{
			fwrite(outputBuffer,sizeof(int),indexOut,run[counter]);
			indexOut=0;
		}

		//printf("heapsize %d\n",heapsize);
		//printf("newheapsize %d\n",newheapsize);
		newheapindex=heapsize+1;
	
		if(heapsize!=0)
		{
			heapify(heapsize+1,0);
			while(heapsize>=0)
			{
				outputBuffer[indexOut]=primaryheap[0];
				primaryheap[0]=primaryheap[heapsize];
				sift(0,heapsize);
				heapsize--;
				indexOut++;
			}

			fwrite(outputBuffer,sizeof(int),indexOut,run[counter]);
			indexOut=0;
		}

		if(newheapsize>=0)
		{
			int n=HEAP_SIZE-1;
			counter++;
			char buf[5]="",fileName[40]="";
			strcat(fileName,inFile);
			strcat(fileName,".");
			sprintf(buf,"%0.03d",counter);
			strcat(fileName,buf);
			run[counter]=fopen(fileName,"w+b");
			indexOut=0;

			int j=0,k=0;
			for(i=n;i>=newheapindex;i--)
			{
				primaryheap[j]=primaryheap[i];
				j++;
				k++;
			}

			j--;
			k--;

			heapify(j+1,0);
			while(j>=0)
			{
				outputBuffer[indexOut]=primaryheap[0];
				primaryheap[0]=primaryheap[j];
				sift(0,j);
				j--;
				indexOut++;
			}

			fwrite(outputBuffer,sizeof(int),indexOut,run[counter]);
			indexOut=0;
		}
	}
	
	counter++;	
	fclose(fp);

	if(flag==1)
	{
		for(i=0;i<counter;i++)
		{
			fseek(run[i],0L,SEEK_SET);
		}

		//printf("totkeys %d\n",runs);
		//printf("totalwritten %d\n",totalwritten);
		memset(inputBuffer,INT_MAX,INPUT_BUF);
		intRead=counter;

		//need to merge runs now
		bufRun=INPUT_BUF/counter;//number of elements to buffer from each run
		
		for(i=0;i<intRead;i++)
		{
			fread(inputBuffer+(i*bufRun),sizeof(int),bufRun,run[i]);
		}

		out=fopen(outFile,"w+b");
		
		do{

			elementPos=minimumElement();
			minElement=inputBuffer[elementPos];

			//all elements have been exhausted
			if(minElement==INT_MAX)
			{
				fwrite(outputBuffer,sizeof(int),indexOut,out);
				break;
			}

			outputBuffer[indexOut]=inputBuffer[elementPos];//send it to output buffer
			indexOut++;
			inputBuffer[elementPos]=INT_MAX; //need to reset to max value

			if((elementPos+1)%bufRun==0)
			{
				runNumber=elementPos/bufRun;
				fread(inputBuffer+(runNumber*bufRun),sizeof(int),bufRun,run[runNumber]);
			}

			//if my output buffer is full
			if(indexOut==OUTPUT_BUF)
			{
				fwrite(outputBuffer,sizeof(int),OUTPUT_BUF,out);
				indexOut=0;//reset the index in output buffer
			}

		}while(minElement!=INT_MAX);	
		
		/*int runs=0,temp=0;
		for(i=0;i<counter;i++)
		{
			fseek(run[i],0L,SEEK_END);
			temp=ftell(run[i])/sizeof(int);
			//printf("runs value %d\n",temp);
			runs+=temp;
			fseek(run[i],0L,SEEK_SET);
		}*/

		for(i=0;i<intRead;i++)
		{
			fclose(run[i]);
		}
	}
	
	fclose(out);

	gettimeofday(&endtime,NULL);
	printResults(starttime,endtime);
}

int comparatorAscen(const void *a,const void *b)
{
	return (*(int *)a - *(int *)b);
}

int minimumElement()
{
	//cannot sort array - need to know if run complete
	int minKey=INT_MAX,i,minKeyPos=0;
	for(i=0;i<INPUT_BUF;i++)
	{
		if(inputBuffer[i]<=minKey)
		{
			minKey=inputBuffer[i];
			minKeyPos=i;
		}
	}

	return minKeyPos;
}

void printResults(struct timeval starttime,struct timeval endtime)
{
	sec=(endtime.tv_sec)-(starttime.tv_sec);
	msec=(endtime.tv_usec)-(starttime.tv_usec);

	//check for negative time
	if(msec<0){
		msec+=1000000;
		sec-=1;
	}

	printf( "Time: %ld.%06ld\n",sec, msec );
}

void heapify(int sizeofArray,int index)
{
	int i=sizeofArray/2;
	while(i>=index)
	{
		sift(i,sizeofArray);
		i--;
	}
}

//http://www.studytonight.com/data-structures/heap-sort
void sift(int i,int heapsize)
{
	int left,right,smallest,temp;
	left=2*i;
	right=left+1;
	if(left<heapsize && primaryheap[left]<=primaryheap[i])
		smallest=left;
	else
		smallest=i;
	if(right<heapsize && primaryheap[right] <= primaryheap[smallest])
		smallest=right;
	if(smallest!=i)
	{
		swap(i,smallest);
		sift(smallest,heapsize);
	}
}

void swap(int i,int lg)
{
	int temp;
	temp=primaryheap[i];
	primaryheap[i]=primaryheap[lg];
	primaryheap[lg]=temp;
}