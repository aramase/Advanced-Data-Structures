//perform binary search on disk

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main()
{
  	FILE *fp,*fpseek;
	unsigned long sizeofkey,sizeofseek,totkeys,totseek,i,j;
	struct timeval starttime, endtime;
	long sec,msec;
	
	//loading seek.db into array

	fpseek=fopen("seek.db","rb");
	fseek(fpseek,0L,SEEK_END);
	sizeofseek=ftell(fpseek);
	totseek=(sizeofseek/sizeof(int));
	int seekarray[totseek];
	fseek(fpseek,0L,SEEK_SET);

	for(i=0;i<totseek;i++)
	{
		fseek(fpseek,i*sizeof(int),SEEK_SET);
		fread(&seekarray[i],sizeof(int),1,fpseek);
	}

	fclose(fpseek);

	//loading key.db into array

	gettimeofday(&starttime,NULL);

	fp=fopen("key.db","rb");
	fseek(fp,0L,SEEK_END);//seek to the end
	sizeofkey=ftell(fp);//get file size in bytes
	totkeys=(sizeofkey/sizeof(int));
	//int keyarray[totkeys];
	fseek(fp,0L,SEEK_SET);//seek back to beginning

	/*for(i=0;i<totkeys;i++)
	{
		fseek(fp,i*sizeof(int),SEEK_SET);
		fread(&keyarray[i],sizeof(int),1,fp);
	}

	fclose(fp);*/

	int hit[totseek];

	//perform binary search
	//develop for disk binary search

	for(i=0;i<totseek;i++){
		long first=0;
		long last=totkeys;
		long middle;
		int currentKey;

		fseek(fp,0L,SEEK_SET);
		clearerr(fp);
		
		do
		{
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

	sec=(endtime.tv_sec)-(starttime.tv_sec);
	msec=(endtime.tv_usec)-(starttime.tv_usec);

	//check for negative time
	if(msec<0){
		msec+=1000000;
		sec-=1;
	}

	//printf("Seconds: %ld\n",(endtime.tv_sec)-(starttime.tv_sec));
	//printf("Microseconds: %d\n",(endtime.tv_usec)-(starttime.tv_usec));

	for(i=0;i<totseek;i++)
	{
		if(hit[i]==1)
		{
			printf("\n%12d: Yes",seekarray[i]);
		}
		else
			printf("\n%12d: No",seekarray[i]);
	}

	printf( "\nTime: %ld.%06ld\n",sec, msec );
}