#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

void InMemorySequential(const char*,const char*);
void InMemoryBinary(const char*,const char*);
void OnDiskSequential(const char*,const char*);
void OnDiskBinary(const char*,const char*);
void printResults(long,int[],int[],struct timeval,struct timeval);

FILE *fp,*fpseek;
unsigned long sizeofkey,sizeofseek,totkeys,totseek,i,j;
struct timeval starttime, endtime;
long sec,msec;

int main(int argc,char* argv[])
{
	if(argc!=4){
		exit(-1);
	}
	else
	{
		if(strcmp(argv[1],"--mem-lin")==0){
			InMemorySequential(argv[2],argv[3]);
		} 
		else if(strcmp(argv[1],"--mem-bin")==0){
			InMemoryBinary(argv[2],argv[3]);
		}
		else if(strcmp(argv[1],"--disk-lin")==0){
			OnDiskSequential(argv[2],argv[3]);
		}
		else if(strcmp(argv[1],"--disk-bin")==0){
			OnDiskBinary(argv[2],argv[3]);
		}
		else
			exit(-1);
	}
}

void InMemorySequential(const char* keyFile,const char* seekFile)
{	
	fpseek=fopen(seekFile,"rb");
	fseek(fpseek,0L,SEEK_END);
	sizeofseek=ftell(fpseek);//determine no of int in file
	totseek=(sizeofseek/sizeof(int));
	int seekarray[totseek];//seek array
	fseek(fpseek,0L,SEEK_SET);//pointer back to beginning

	for(i=0;i<totseek;i++){
		fseek(fpseek,i*sizeof(int),SEEK_SET);
		fread(&seekarray[i],sizeof(int),1,fpseek);
	}

	fclose(fpseek);

	gettimeofday(&starttime,NULL);

	fp=fopen(keyFile,"rb");
	fseek(fp,0L,SEEK_END);//seek to the end
	sizeofkey=ftell(fp);//get file size in bytes
	totkeys=(sizeofkey/sizeof(int));
	int keyarray[totkeys];
	fseek(fp,0L,SEEK_SET);//seek back to beginning

	for(i=0;i<totkeys;i++){
		fseek(fp,i*sizeof(int),SEEK_SET);
		fread(&keyarray[i],sizeof(int),1,fp);
	}

	fclose(fp);

	int hit[totseek];

	//sequential search in key array
	for(i=0;i<totseek;i++){
		for(j=0;j<totkeys;j++){
			if(keyarray[j]==seekarray[i]){
				hit[i]=1;
			}
		}
	}

	gettimeofday(&endtime,NULL);
	printResults(totseek,hit,seekarray,starttime,endtime);
}

void InMemoryBinary(const char* keyFile,const char* seekFile)
{	
	fpseek=fopen(seekFile,"rb");
	fseek(fpseek,0L,SEEK_END);
	sizeofseek=ftell(fpseek);
	totseek=(sizeofseek/sizeof(int));
	int seekarray[totseek];
	fseek(fpseek,0L,SEEK_SET);

	for(i=0;i<totseek;i++){
		fseek(fpseek,i*sizeof(int),SEEK_SET);
		fread(&seekarray[i],sizeof(int),1,fpseek);
	}

	fclose(fpseek);

	gettimeofday(&starttime,NULL);

	fp=fopen(keyFile,"rb");
	fseek(fp,0L,SEEK_END);//seek to the end
	sizeofkey=ftell(fp);//get file size in bytes
	totkeys=(sizeofkey/sizeof(int));
	int keyarray[totkeys];
	fseek(fp,0L,SEEK_SET);//seek back to beginning

	for(i=0;i<totkeys;i++){
		fseek(fp,i*sizeof(int),SEEK_SET);
		fread(&keyarray[i],sizeof(int),1,fp);
	}

	fclose(fp);

	int hit[totseek];

	//perform binary search

	for(i=0;i<totseek;i++){
		long first=0;
		long last=totkeys-1;
		long middle;
		do{
			middle=(first+last)/2;
			if(seekarray[i]<keyarray[middle]){
				last=middle-1;
			}
			else if(seekarray[i]>keyarray[middle]){
				first=middle+1;
			}
		}while(seekarray[i]!=keyarray[middle] && first<=last);

		if(seekarray[i]==keyarray[middle]){
			hit[i]=1;
		}
	}

	gettimeofday(&endtime,NULL);
	printResults(totseek,hit,seekarray,starttime,endtime);
}

void OnDiskSequential(const char* keyFile,const char* seekFile)
{
	fpseek=fopen(seekFile,"rb");
	fseek(fpseek,0L,SEEK_END);
	sizeofseek=ftell(fpseek);
	totseek=(sizeofseek/sizeof(int));
	int seekarray[totseek];
	fseek(fpseek,0L,SEEK_SET);

	for(i=0;i<totseek;i++){
		fseek(fpseek,i*sizeof(int),SEEK_SET);
		fread(&seekarray[i],sizeof(int),1,fpseek);
	}

	fclose(fpseek);

	gettimeofday(&starttime,NULL);

	fp=fopen(keyFile,"rb");
	int hit[totseek];

	//sequential search on disk
	for(i=0;i<totseek;i++){

		//resetting the pointer to first byte
		fseek(fp,0L,SEEK_SET);
		clearerr(fp);
		hit[i]=0;

		while(1){
			int currentkey;
			//reached EOF
			if(!fread(&currentkey,sizeof(int),1,fp)){
				break;
			}
			else{
				if(currentkey==seekarray[i]){
					hit[i]=1;
				}
			}
		}
	}

	//determining end time
	gettimeofday(&endtime,NULL);
	printResults(totseek,hit,seekarray,starttime,endtime);
}

void OnDiskBinary(const char* keyFile,const char* seekFile)
{
	fpseek=fopen(seekFile,"rb");
	fseek(fpseek,0L,SEEK_END);
	sizeofseek=ftell(fpseek);
	totseek=(sizeofseek/sizeof(int));
	int seekarray[totseek];
	fseek(fpseek,0L,SEEK_SET);

	for(i=0;i<totseek;i++){
		fseek(fpseek,i*sizeof(int),SEEK_SET);
		fread(&seekarray[i],sizeof(int),1,fpseek);
	}

	fclose(fpseek);

	gettimeofday(&starttime,NULL);

	fp=fopen(keyFile,"rb");
	fseek(fp,0L,SEEK_END);//seek to the end
	sizeofkey=ftell(fp);//get file size in bytes
	totkeys=(sizeofkey/sizeof(int));
	//int keyarray[totkeys];
	fseek(fp,0L,SEEK_SET);//seek back to beginning

	int hit[totseek];

	//perform binary search

	for(i=0;i<totseek;i++){
		long first=0;
		long last=totkeys-1;
		long middle;
		int currentKey;

		fseek(fp,0L,SEEK_SET);
		clearerr(fp);
		hit[i]=0;
		
		do{
			middle=(first+last)/2;
			fseek(fp,middle*sizeof(int),SEEK_SET);
			if(!fread(&currentKey,sizeof(int),1,fp)){
				break;
			}
			else if(seekarray[i]<currentKey){
				last=middle-1;
			}
			else if(seekarray[i]>currentKey){
				first=middle+1;
			}
		}while(seekarray[i]!=currentKey && first<=last);

		if(seekarray[i]==currentKey){
			hit[i]=1;
		}
	}

	gettimeofday(&endtime,NULL);
	printResults(totseek,hit,seekarray,starttime,endtime);
}

void printResults(long totseek,int hit[],int seekarray[],struct timeval starttime,struct timeval endtime){

	sec=(endtime.tv_sec)-(starttime.tv_sec);
	msec=(endtime.tv_usec)-(starttime.tv_usec);

	//check for negative time
	if(msec<0){
		msec+=1000000;
		sec-=1;
	}

	long i;
	for(i=0;i<totseek;i++){
		if(hit[i]==1){
			printf("%12d: Yes\n",seekarray[i]);
		}
		else
			printf("%12d: No\n",seekarray[i]);
	}

	printf( "Time: %ld.%06ld\n",sec, msec );
}