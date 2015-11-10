#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIRSTFIT "--first-fit"
#define BESTFIT "--best-fit"
#define WORSTFIT "--worst-fit"

void add(int,char*);
void find(int);
void del(int);
long binarySearch(int,int,int);
int binarySearchforIndex(int,int,int);
long availableOff(int);
int comparatorAscenAvail(const void *,const void *);
int comparatorDescenAvail(const void *,const void *);
int comparatorPrim(const void *,const void *);
void write_primary();
void write_availability();
void read_primary();
void read_availability();
void end();

typedef struct{
	int key;
	long off;
}index_S;

typedef struct{
	int siz;
	long off;
}avail_S;

index_S primary[10000];// primary key index
avail_S available[10000]; //availability list
FILE *fp;
char *listOrder,*fileName;//Mode of operation for availability lists
int primary_end,available_end=0; 

int main(int argc,char* argv[])
{
	if(argc!=3)
	{
		printf("ERROR\n");
		exit(-1);
	}
	else
	{
		if(strcmp(argv[1],"--first-fit")==0){
			listOrder=FIRSTFIT;
		} 
		else if(strcmp(argv[1],"--best-fit")==0){
			listOrder=BESTFIT;
		}
		else if(strcmp(argv[1],"--worst-fit")==0){
			listOrder=WORSTFIT;
		}

		fileName=argv[2];//filename of the student file
	}

	char *op,*mydata,*key,*temp,command[150];
	const char token2[3]=" \n";
	const char token1[2]=" ";

	if((fp=fopen(fileName,"r+b"))!=NULL){
		read_primary();
		read_availability();
	}

	while(1)
	{
		fgets(command,150,stdin);
		temp=strtok(command,token2);
		op=temp;

		if(strcmp(op,"add")==0){
			temp=strtok(NULL,token1);
			key=atoi(temp);
			temp=strtok(NULL,token1);
			mydata=temp;
			add(key,mydata);
		}
		else if(strcmp(op,"find")==0){
			temp=strtok(NULL,token1);
			key=atoi(temp);
			find(key);
		}
		else if(strcmp(op,"del")==0){
			temp=strtok(NULL,token1);
			key=atoi(temp);
			del(key);
		}
		else if(strcmp(op,"end")==0){
			break;
		}
		else{
			printf("Invalid Command\n");
		}
	}

	end();
}

void add(int key,char* myRecord)
{
	//pass in primary_end for finish in binary search
	long result,positionOff,newOff=0;
	int recSize=strlen(myRecord)-1;
	
	//to check if key already in primary index
	result=binarySearchforIndex(key,0,primary_end);	

	//key already present in index
	if(result!=-1){
		printf("Record with SID=%d exists\n",key);
		return;
	}

	//not present in index, need to add
	else
	{
		if ( ( fp = fopen(fileName, "r+b" ) ) == NULL ) 
		{ 
			fp = fopen(fileName, "w+b" ); 
		}
		else
		{
			fp=fopen(fileName,"r+b");
		} 

		positionOff=availableOff(recSize);
		if(positionOff==-1){
			fseek(fp,0,SEEK_END);
			newOff=ftell(fp);
		}
		else
		{
			fseek(fp,positionOff,SEEK_SET);
			newOff=positionOff;
		}

		fwrite(&recSize,1,sizeof(int),fp);
		fwrite(myRecord,recSize,1,fp);

		primary[primary_end].key=key;
		primary[primary_end].off=newOff;
		primary_end+=1;
		qsort(primary,primary_end,sizeof(index_S),comparatorPrim);

		fclose(fp);
	}
}

void find(int key)
{
	long matchOff;
	int recSize;
	char *myOut;
	matchOff=binarySearch(key,0,primary_end);

	if(matchOff==-1){
		printf("No record with SID=%d exists\n",key);
	}
	else
	{
		fp = fopen(fileName, "r+b" );
		fseek(fp,matchOff,SEEK_SET);
		fread(&recSize,sizeof(int),1,fp);
		myOut=malloc(recSize+1);
		fread(myOut,1,recSize,fp);
		//myOut[recSize]="\0";
		printf("%s\n",myOut);//print the output after finding using primary key
		fclose(fp);
	}
}

void del(int key)
{
	long delOff;
	int recSize;

	delOff=binarySearch(key,0,primary_end);

	if(delOff==-1){
		printf("No record with SID=%d exists\n",key);
	}
	else
	{
		fp = fopen(fileName, "r+b" );
		fseek(fp,delOff,SEEK_SET);
		fread(&recSize,1,sizeof(int),fp);//found the record size

		//add entry to availability list
		available[available_end].siz=recSize+sizeof(int);
		available[available_end].off=delOff;
		available_end+=1;

		if(listOrder==BESTFIT){
			qsort(available,available_end,sizeof(avail_S),comparatorAscenAvail);
		}
		else if(listOrder==WORSTFIT){
			qsort(available,available_end,sizeof(avail_S),comparatorDescenAvail);
		}

		//remove entry from primary key index
		int myIndex,j;
		myIndex=binarySearchforIndex(key,0,primary_end);
		for(j=myIndex;j<primary_end;j++){
			primary[j].key=primary[j+1].key;
			primary[j].off=primary[j+1].off;
		}
		primary_end-=1;

		fclose(fp);
	}
}

//close the program and transfer to disk
void end()
{
	write_primary();
	write_availability();

	int i,j,hole_siz=0;

	printf("Index:\n");

	for(i=0;i<primary_end;i++){
		printf( "key=%d: offset=%ld\n", primary[i].key, primary[i].off );
	}

	printf("Available:\n");

	for(i=0;i<available_end;i++){
		printf( "size=%d: offset=%ld\n", available[i].siz, available[i].off );
		hole_siz+=available[i].siz;
	}

	printf( "Number of holes: %d\n", available_end);
	printf( "Hole space: %d\n", hole_siz );
}

//write primary index to on disk
void write_primary()
{
	FILE *out; /* Output file stream */ 
	out = fopen( "index.bin", "w+b" ); 
	fwrite( primary, sizeof( index_S ), primary_end, out ); 
	fclose( out );
}

//write availability list to on disk
void write_availability()
{
	FILE *out; /* Output file stream */ 
	out = fopen( "availability.bin", "w+b" ); 
	fwrite( available, sizeof( avail_S ), available_end, out ); 
	fclose( out );
}

void read_primary()
{
	FILE *out;
	out=fopen("index.bin","r+b");
	fseek(out,0,SEEK_END);
	primary_end=ftell(out)/sizeof(index_S);
	fseek(out,0,SEEK_SET);
	fread(primary,primary_end,sizeof(index_S),out);
	fclose(out);
}

void read_availability()
{
	FILE *out;
	out=fopen("availability.bin","r+b");
	fseek(out,0,SEEK_END);
	available_end=ftell(out)/sizeof(avail_S);
	fseek(out,0,SEEK_SET);
	fread(available,available_end,sizeof(avail_S),out);
	fclose(out);
}

//use this only for the find method
long binarySearch(int element,int start,int finish)
{
	int first=start;
	int last=finish-1;
	int middle;
	do{
		middle=(first+last)/2;
		//printf("Middle:%d\n",middle);
		if(element<primary[middle].key){
			last=middle-1;
		}
		else if(element>primary[middle].key){
			first=middle+1;
		}
	}while(element!=primary[middle].key && first<=last);

	if(element==primary[middle].key){
		return primary[middle].off;
	}
	else
	{
		return -1;
	}
}

//use this to determine if element present in primary key before you add
int binarySearchforIndex(int element,int start,int finish)
{
	int first=start;
	int last=finish-1;
	int middle;
	do{
		middle=(first+last)/2;
		if(element<primary[middle].key){
			last=middle-1;
		}
		else if(element>primary[middle].key){
			first=middle+1;
		}
	}while(element!=primary[middle].key && first<=last);

	if(element==primary[middle].key){
		return middle;
	}
	else
	{
		return -1;
	}
}

long availableOff(int length)
{
	
	int i,j;
	for(i=0;i<available_end;i++)
	{
		if(length<available[i].siz)
		{
			int j;
			int remainSize=available[i].siz-length-sizeof(int);
			long remainOff=available[i].off+length+sizeof(int);
			long matchOff=available[i].off;

			if(listOrder==FIRSTFIT)
			{
				for(j=i;j<available_end;j++)
				{
					available[j].siz=available[j+1].siz;
					available[j].off=available[j+1].off;
				}

				if(remainSize>0)
				{
					available[available_end-1].siz=remainSize;
					available[available_end-1].off=remainOff;
					return matchOff;
				}
				else
				{
					available_end-=1;
					return matchOff;
				}
			}

			else if(listOrder==BESTFIT)
			{
				if(remainSize>0)
				{
					available[i].siz=remainSize;
					available[i].off=remainOff;
					qsort(available,i+1,sizeof(avail_S),comparatorAscenAvail);
					return matchOff;
				}
				else if(remainSize==0)
				{
					for(j=i;j<available_end;j++)
					{
						available[j].siz=available[j+1].siz;
						available[j].off=available[j+1].off;
					}
					available_end-=1;
					return matchOff;
				}
			}

			else if(listOrder==WORSTFIT)
			{
				if(remainSize>0)
				{
					available[i].siz=remainSize;
					available[i].off=remainOff;
					qsort(available,available_end,sizeof(avail_S),comparatorDescenAvail);
					return matchOff;
				}

				else if(remainSize==0)
				{
					for(j=i;j<available_end;j++)
					{
						available[j].siz=available[j+1].siz;
						available[j].off=available[j+1].off;
					}
					available_end-=1;
					return matchOff;
				}
			}
		}
	}

	return -1;
}

int comparatorAscenAvail(const void *a,const void *b)
{
	avail_S *ava=(avail_S *)a;
	avail_S *avb=(avail_S *)b;
	int av_size= (ava->siz-avb->siz);

	//check if size=0, then sort using offset
	if(av_size==0){
		return (int)(ava->off-avb->off);
	}
	else{
		return av_size;
	}
}

int comparatorDescenAvail(const void *a,const void *b)
{
	avail_S *ava=(avail_S *)a;
	avail_S *avb=(avail_S *)b;
	int av_size= (avb->siz-ava->siz);
	//check if size=0, then sort using offset

	if(av_size==0){
		return (int)(avb->off-ava->off);
	}
	else{
		return av_size;
	}
}

int comparatorPrim(const void *a,const void *b)
{
	index_S *ina=(index_S *)a;
	index_S *inb=(index_S *)b;
	return (int)(ina->key-inb->key);//unique primary keys
}