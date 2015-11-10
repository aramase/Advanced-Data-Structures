#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char *op,*mydata,*key;
	char command[150];
	const char token1[2]=" ";
	const char token2[2]="\n";

	while(1)
	{
		fgets(command,150,stdin);
		op=strtok(command,"\n");

		if(strcmp(op,"add")==0)
		{
			printf("add\n");
			key=strtok(NULL,token1);
			mydata=strtok(NULL,token1);
			printf("key:%d",atoi(key));
		}
		else if(strcmp(op,"find")==0)
		{
			printf("find\n");
		}
		else if(strcmp(op,"del")==0)
		{
			printf("del\n");
		}
		else if(strcmp(op,"end")==0)
		{
			printf("end\n");
			break;
		}
		else
		{
			printf("Invalid Command\n");
		}
	}
}