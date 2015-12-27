#include <stdlib.h>
#include <stdio.h>  
#include <string.h>

int order; /* B-tree order */ 
int array[1000];
long childArray[1000];
FILE *fp;
long root=0;
int check=0,leaf=0;
long current=0,curr=0;
char* fileName;

typedef struct { 
	int n; /* Number of keys in node */ 
	int *key; /* Node's keys */ 
	long *child; /* Node's child subtree offsets */ 
} btree_node; 

//Queue for printing and splitting nodes
struct Node {
	long data;
	struct Node* next;
	struct Node* before;
};

struct Node* front=NULL;
struct Node* rear=NULL;

void add(int);
int search(int,long,int,int);
int posToInsert(long,int,int,int);
void insert(long,int,int,int,int);
long findNode(long,int,int);
void splitNode(long,int);
void enqueue(long);
void dequeue(int);
long peek(int);
void emptyQueue();
void insertOffset(long,long,int,int,int);
int childSize(long);
void insertArray(int,int,int);
void insertChildArray(long,int,int);
void print(int);

int main(int argc,char** argv)
{
	if(argc!=3)
	{
		printf("Invalid number of arguments\n");
		exit(-1);
	}
	else
	{
		fileName=argv[1];//filename of the student file
		order=atoi(argv[2]);//order of tree
	}

	//taken from my previous assignment - assn_2.c
	char *op,*mydata,*key,*temp,command[150];
	const char token2[3]=" \n";
	const char token1[2]=" ";

	while(1)
	{
		fgets(command,150,stdin);
		temp=strtok(command,token2);
		op=temp;

		if(strcmp(op,"add")==0){
			temp=strtok(NULL,token1);
			key=atoi(temp);
			add(key);
		}
		else if(strcmp(op,"find")==0){
			temp=strtok(NULL,token1);
			key=atoi(temp);
			fp=fopen(fileName,"r+b");
			if(fp==NULL)
			{
				printf("Entry with key=%d does not exist\n",key);
			}
			else
			{
				long newOff=0;
				fseek(fp,0L,SEEK_SET);
				fread(&newOff,sizeof(long),1,fp);
				search(key,newOff,order,1);
				fclose(fp);
			}
		}
		else if(strcmp(op,"print")==0){
			fp=fopen(fileName,"r+b");
			if(fp!=NULL)
			{
				long tempOff=0;
				fseek(fp,0L,SEEK_SET);
				fread(&tempOff,sizeof(long),1,fp);
				emptyQueue();
				enqueue(tempOff);
				print(order);
				fclose(fp);
			}
		}
		else if(strcmp(op,"end")==0){
			break;
		}
		else{
			printf("Invalid Command\n");
		}
	}
}

void add(int k)
{
	root=0;
	int position=0,i=0;
	fp=fopen(fileName,"r+b");

	btree_node node; /* Single B-tree node */ 
	node.n = 0; 
	node.key = (int *) calloc( order - 1, sizeof( int ) ); 
	node.child = (long *) calloc( order, sizeof( long ) );

	if(fp==NULL)
	{
		root=-1;//initialize to unknown value
		fp=fopen(fileName,"w+b");

		node.key[node.n]=k;
		node.n++;
		root=sizeof(long);
		fwrite(&root,sizeof(long),1,fp);
		fseek(fp,0L,SEEK_END);

		fwrite( &node.n, sizeof( int ), 1, fp ); 
		fwrite( node.key, sizeof( int ), order - 1, fp ); 
		fwrite( node.child, sizeof( long ), order, fp );
	}
	else	
	{
		fread(&root,sizeof(long),1,fp);
		fseek(fp,root,SEEK_SET);

		fread( &node.n, sizeof( int ), 1, fp ); 
		fread( node.key, sizeof( int ), order - 1, fp ); 
		fread( node.child, sizeof( long ), order, fp );

		//search if key already present
		check=search(k,root,order,0);
		if(check==1)
		{
			printf("Entry with key=%d already exists\n",k);
			free(node.key);
			free(node.child);
		}
		else
		{
			fseek(fp,root,SEEK_SET);

			fread( &node.n, sizeof( int ), 1, fp ); 
			fread( node.key, sizeof( int ), order - 1, fp ); 
			fread( node.child, sizeof( long ), order, fp );

			long nodeOff=findNode(root,k,order);

			fseek(fp,nodeOff,SEEK_SET);

			fread( &node.n, sizeof( int ), 1, fp ); 
			fread( node.key, sizeof( int ), order - 1, fp ); 
			fread( node.child, sizeof( long ), order, fp );

			int x=node.n;

			free(node.key);
			free(node.child);

			if(x!=order-1)
			{
				position=posToInsert(nodeOff,k,x,order-1);
				insert(nodeOff,k,position,x,order-1);
				emptyQueue();
			}
			else
			{
				if(x==order-1)
				{
					splitNode(nodeOff,k);
				}
			}
		}
	}

	fclose(fp);
}

int search(int k,long off,int order,int flag)
{
	btree_node temp; /* Single B-tree node */ 
	temp.n = 0; 
	temp.key = (int *) calloc( order - 1, sizeof( int ) ); 
	temp.child = (long *) calloc( order, sizeof( long ) );

	fseek(fp,off,SEEK_SET);

	fread( &temp.n, sizeof( int ), 1, fp ); 
	fread( temp.key, sizeof( int ), order - 1, fp ); 
	fread( temp.child, sizeof( long ), order, fp );

	int i=0,s=0;
	while(s<order-1 && temp.key[s]!=NULL)
	{
		if(k==temp.key[s])
		{
			free(temp.key);
			free(temp.child);
			if(flag==0)
				return 1;
			else
			{
				printf("Entry with key=%d exists\n",k);
				return 1;
			}

		}
		else if(k<temp.key[s])
		{
			break;
		}
		else
			s++;
	}

	if(temp.child[s]!=NULL)
	{
		long tempOffset=temp.child[s];
		free(temp.key);
		free(temp.child);
		return search(k,tempOffset,order,flag);
	}
	else
	{
		free(temp.key);
		free(temp.child);
		if(flag==0)
			return -1;//no key found
		else
		{
			printf("Entry with key=%d does not exist\n",k);
			return -1;
		}
	}
}

int posToInsert(long off,int k,int limit,int size)
{
	int i=0;

	btree_node temp; /* Single B-tree node */ 
	temp.n = 0; 
	temp.key = (int *) calloc( size, sizeof( int ) ); 
	temp.child = (long *) calloc( order, sizeof( long ) );

	fseek(fp,off,SEEK_SET);

	fread( &temp.n, sizeof( int ), 1, fp ); 
	fread( temp.key, sizeof( int ), size, fp ); 
	fread( temp.child, sizeof( long ), order, fp );

	for(i=0;i<limit;i++)
	{
		if(temp.key[i]>k)
		{
			free(temp.key);
			free(temp.child);
			return i;
		}
	}

	free(temp.key);
	free(temp.child);
	return limit;
}

//insert and re-arrange key 
void insert(long off,int k,int pos,int start,int size)
{
	int j=0;

	btree_node temp; /* Single B-tree node */ 
	temp.n = 0; 
	temp.key = (int *) calloc( size, sizeof( int ) ); 
	temp.child = (long *) calloc( order, sizeof( long ) );

	fseek(fp,off,SEEK_SET);

	fread( &temp.n, sizeof( int ), 1, fp ); 
	fread( temp.key, sizeof( int ), size, fp ); 
	fread( temp.child, sizeof( long ), order, fp );

	for(j=start;j>pos;j--)
	{
		temp.key[j]=temp.key[j-1];
	}

	temp.key[pos]=k;
	temp.n++;

	fseek(fp,off,SEEK_SET);

	fwrite( &temp.n, sizeof( int ), 1, fp ); 
	fwrite( temp.key, sizeof( int ), size, fp ); 
	fwrite( temp.child, sizeof( long ), order, fp );
	free(temp.key);
	free(temp.child);
	return;
}

//find the node to add element
long findNode(long off,int k,int order)
{
	int s=0;
	long current=0;
	btree_node temp; /* Single B-tree node */ 
	temp.n = 0; 
	temp.key = (int *) calloc( order - 1, sizeof( int ) ); 
	temp.child = (long *) calloc( order, sizeof( long ) );

	fseek(fp,off,SEEK_SET);
	current=off;

	fread( &temp.n, sizeof( int ), 1, fp ); 
	fread( temp.key, sizeof( int ), order - 1, fp ); 
	fread( temp.child, sizeof( long ), order, fp );

	s=0;
	while(s<order-1 && temp.key[s]!=NULL)
	{
		if(k<temp.key[s])
		{
			break;
		}
		else
			s++;
	}

	if(temp.child[s]!=NULL)
	{
		enqueue(off);
		long findOff=temp.child[s];
		free(temp.key);
		free(temp.child);
		return findNode(findOff,k,order);
	}
	else
	{
		free(temp.key);
		free(temp.child);
		return current;//no key found
	}
}

void splitNode(long off,int k)
{
	int median=order/2,i=0,temp;
	long currentOff=0;

	btree_node tempNode; /* Single B-tree node */ 
	tempNode.n = 0; 
	tempNode.key = (int *) calloc( order-1, sizeof( int ) ); 
	tempNode.child = (long *) calloc( order, sizeof( long ) );

	fseek(fp,off,SEEK_SET);
	currentOff=off;

	fread( &tempNode.n, sizeof( int ), 1, fp ); 
	fread( tempNode.key, sizeof( int ), order-1, fp ); 
	fread( tempNode.child, sizeof( long ), order, fp );

	int position=posToInsert(currentOff,k,tempNode.n,order);

	for(i=0;i<tempNode.n;i++)
	{
		array[i]=tempNode.key[i];
	}

	insertArray(k,position,tempNode.n+1);

	int count=0;
	for(i=0;i<order;i++)
	{
		childArray[i]=tempNode.child[i];
		if(childArray[i]==NULL)
			count++;
	}

	if(count==order)
		leaf=1;//leaf node
	else
		leaf=0;

	btree_node splitnode; /* Single B-tree node */ 
	splitnode.n = 0; 
	splitnode.key = (int *) calloc( order-1, sizeof( int ) ); 
	splitnode.child = (long *) calloc( order, sizeof( long ) );

	//if it is a leaf node
	if(leaf==1)
	{
		k=0;
		for(i=0;i<median;i++)
		{
			splitnode.key[i]=array[i];
			k++;
		}

		temp=array[median];
		splitnode.n=median;

		fseek(fp,off,SEEK_SET);
		current=off;

		fwrite( &splitnode.n, sizeof( int ), 1, fp ); 
		fwrite( splitnode.key, sizeof( int ), order - 1, fp ); 
		fwrite( splitnode.child, sizeof( long ), order, fp );

		free(splitnode.key);
		free(splitnode.child);

		btree_node newNode; /* Single B-tree node */ 
		newNode.n = 0; 
		newNode.key = (int *) calloc( order-1, sizeof( int ) ); 
		newNode.child = (long *) calloc( order, sizeof( long ) );

		int j=0;
		for(i=median+1;i<order;i++)
		{
			newNode.key[j]=array[i];
			newNode.child[j]=tempNode.child[i-1];
			newNode.n++;
			j++;
		}

		newNode.child[j]=tempNode.child[order-1];

		free(tempNode.key);
		free(tempNode.child);

		fseek(fp,0L,SEEK_END);
		curr=ftell(fp);

		fwrite( &newNode.n, sizeof( int ), 1, fp ); 
		fwrite( newNode.key, sizeof( int ), order - 1, fp ); 
		fwrite( newNode.child, sizeof( long ), order, fp );

		free(newNode.key);
		free(newNode.child);
	}
	else
	{
		//not a leaf node case
		if(position==order-1)
			childArray[position+1]=curr;
		else
		{
			int start=childSize(off);
			insertChildArray(curr,position+1,start);
		}

		k=0;
		for(i=0;i<median;i++)
		{
			splitnode.key[i]=array[i];
			splitnode.child[i]=childArray[i];
			k++;
		}

		splitnode.child[k]=childArray[k];
		temp=array[median];
		splitnode.n=median;

		fseek(fp,off,SEEK_SET);
		current=off;

		fwrite( &splitnode.n, sizeof( int ), 1, fp ); 
		fwrite( splitnode.key, sizeof( int ), order - 1, fp ); 
		fwrite( splitnode.child, sizeof( long ), order, fp );

		free(splitnode.key);
		free(splitnode.child);

		btree_node newNode; /* Single B-tree node */ 
		newNode.n = 0; 
		newNode.key = (int *) calloc( order-1, sizeof( int ) ); 
		newNode.child = (long *) calloc( order, sizeof( long ) );

		int j=0,k=median+1;
		for(i=median+1;i<order;i++)
		{
			newNode.key[j]=array[i];
			newNode.child[j]=childArray[i];
			newNode.n++;
			j++;
			k++;
		}

		newNode.child[j]=childArray[k];

		free(tempNode.key);
		free(tempNode.child);

		fseek(fp,0L,SEEK_END);
		curr=ftell(fp);

		fwrite( &newNode.n, sizeof( int ), 1, fp ); 
		fwrite( newNode.key, sizeof( int ), order - 1, fp ); 
		fwrite( newNode.child, sizeof( long ), order, fp );

		free(newNode.key);
		free(newNode.child);
	}

	long parentOff=peek(1);
	dequeue(1);
	memset(array,0,1000);
	memset(childArray,0,1000);

	if(parentOff==-1)
	{
		btree_node newNode;
		newNode.n = 0; 
		newNode.key = (int *) calloc( order-1, sizeof( int ) ); 
		newNode.child = (long *) calloc( order, sizeof( long ) );

		newNode.n++;
		newNode.key[0]=temp;
		newNode.child[0]=current;
		newNode.child[1]=curr;

		fseek(fp,0L,SEEK_END);
		long newRoot;
		newRoot=ftell(fp);

		fwrite( &newNode.n, sizeof( int ), 1, fp ); 
		fwrite( newNode.key, sizeof( int ), order - 1, fp ); 
		fwrite( newNode.child, sizeof( long ), order, fp );

		fseek(fp,0L,SEEK_SET);
		fwrite(&newRoot,sizeof(long),1,fp);

		free(newNode.key);
		free(newNode.child);
	}
	else
	{
		fseek(fp,parentOff,SEEK_SET);

		btree_node newNode;
		newNode.n = 0; 
		newNode.key = (int *) calloc( order-1, sizeof( int ) ); 
		newNode.child = (long *) calloc( order, sizeof( long ) );

		fread( &newNode.n, sizeof( int ), 1, fp ); 
		fread( newNode.key, sizeof( int ), order-1 , fp ); 
		fread( newNode.child, sizeof( long ), order, fp );

		if(newNode.n!=order-1)
		{
			int promotePos=posToInsert(parentOff,temp,newNode.n,order-1);
			insert(parentOff,temp,promotePos,newNode.n,order-1);

			free(newNode.key);
			free(newNode.child);

			int sizeOfChild=childSize(parentOff);
			insertOffset(parentOff,curr,promotePos+1,sizeOfChild,order-1);
		}
		else //if(newNode.n==order-1)
		{
			free(newNode.key);
			free(newNode.child);
			splitNode(parentOff,temp);
		}
	}

	emptyQueue();
	return;
}

//Queue implementation for book keeping - parent offset
void enqueue(long x)
{
	struct Node* n=(struct Node*)malloc(sizeof(struct Node));
	n->data=x;
	n->next=NULL;
	n->before=NULL;
	if(front==NULL && rear==NULL)
	{
		front=rear=n;
		return;
	}

	rear->next=n;
	rear->next->before=rear;
	rear=n;
}

void dequeue(int flag)
{
	if(flag==0)
	{
		struct Node* n=front;
		if(front==NULL)
			return;
		if(front==rear)
			front=rear=NULL;
		else
			front=front->next;
	}
	else
	{
		struct Node* n=rear;
		if(rear==NULL)
			return;
		if(front==rear)
			front=rear=NULL;
		else
			rear=rear->before;
	}
}

long peek(int flag)
{
	if(flag==0)
	{
		if(front==NULL)
			return -1;
		else
			return front->data;
	}
	else
	{
		if(rear==NULL)
			return -1;
		else
			return rear->data;
	}
}

void emptyQueue()
{
	long mypeek=peek(0);
	if(mypeek==-1)
		return;
	else
	{
		dequeue(0);
		return emptyQueue();
	}
}


int childSize(long off)
{
	int i=0;
	btree_node temp; /* Single B-tree node */ 
	temp.n = 0; 
	temp.key = (int *) calloc( order-1, sizeof( int ) ); 
	temp.child = (long *) calloc( order, sizeof( long ) );

	fseek(fp,off,SEEK_SET);

	fread( &temp.n, sizeof( int ), 1, fp ); 
	fread( temp.key, sizeof( int ), order-1, fp ); 
	fread( temp.child, sizeof( long ), order, fp );

	for(i=0;i<order;i++)
	{
		if(temp.child[i]==NULL)
		{
			free(temp.key);
			free(temp.child);
			return i+1;
		}
	}

	free(temp.key);
	free(temp.child);
	return order;
}

//insert and re-arrange child 
void insertOffset(long off,long offset,int pos,int start,int size)
{
	int j=0;
	btree_node temp; /* Single B-tree node */ 
	temp.n = 0; 
	temp.key = (int *) calloc( size, sizeof( int ) ); 
	temp.child = (long *) calloc( order, sizeof( long ) );

	fseek(fp,off,SEEK_SET);

	fread( &temp.n, sizeof( int ), 1, fp ); 
	fread( temp.key, sizeof( int ), size, fp ); 
	fread( temp.child, sizeof( long ), order, fp );

	for(j=start;j>pos;j--)
	{
		temp.child[j]=temp.child[j-1];
	}

	temp.child[pos]=offset;

	fseek(fp,off,SEEK_SET);

	fwrite( &temp.n, sizeof( int ), 1, fp ); 
	fwrite( temp.key, sizeof( int ), size, fp ); 
	fwrite( temp.child, sizeof( long ), order, fp );

	free(temp.key);
	free(temp.child);
	return;
}

void insertArray(int k,int pos,int start)
{
	int j=0;
	for(j=start;j>pos;j--)
	{
		array[j]=array[j-1];
	}

	array[pos]=k;
}

void insertChildArray(long off,int pos,int start)
{
	int j=0;
	for(j=start;j>pos;j--)
	{
		childArray[j]=childArray[j-1];
	}

	childArray[pos]=off;
}

void print(int order)
{
	int childLength[1000];
	int i=0;
	int level=1;
	int count=0;

	for(i=0;i<1000;i++)
	{
		childLength[i]=0;
	}

	childLength[1]=1;

	while(peek(0)!=-1)
	{
		printf("%2d: ", level);
		count=0;

		btree_node node;
		node.n = 0; 
		node.key = (int *) calloc( order - 1, sizeof( int ) ); 
		node.child = (long *) calloc( order, sizeof( long ) );

		do{

			fseek(fp,peek(0),SEEK_SET);

			fread( &node.n, sizeof( int ), 1, fp ); 
			fread( node.key, sizeof( int ), order - 1, fp ); 
			fread( node.child, sizeof( long ), order, fp );

			for( i = 0; i < node.n - 1; i++ ) 
			{ 
				printf( "%d,", node.key[ i ] ); 
			} 

			printf( "%d ", node.key[ node.n - 1 ] );

			childLength[level]-=1;

			for(i=0;i<order;i++)
			{
				if(node.child[i]!=NULL)
				{
					enqueue(node.child[i]);
					childLength[level+1]+=1;
				}
			}
			dequeue(0);
		}while(childLength[level]!=0);
		printf("\n");
		level++;

		free(node.key);
		free(node.child);
	}	
}