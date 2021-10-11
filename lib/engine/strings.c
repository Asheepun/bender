#include "engine/strings.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void String_set(char *destString, char *sourceString, int size){

	memset(destString, 0, size);

	memcpy(destString, sourceString, strlen(sourceString));
	
}

void String_append(char *baseString, char *appendString){

	memcpy(baseString + strlen(baseString), appendString, strlen(appendString));

}

void String_clearRange(char *string, int startIndex, int stopIndex){

	memset(string + startIndex, 0, stopIndex - startIndex);

}
