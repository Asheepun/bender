#ifndef ARRAY_H_
#define ARRAY_H_

#include "stdbool.h"
#include "stddef.h"

typedef struct Array{
	void *items;
	unsigned int length;
	unsigned int maxLength;
	unsigned int itemSize;
}Array;

typedef struct EntityHeader{
	size_t ID;
}EntityHeader;

typedef struct IndexSafeArray{
	void *items;
	unsigned int deepestIndex;
	//unsigned int length;
	//unsigned int spaces;
	unsigned int itemSize;
	unsigned int indexSize;
	unsigned int maxLength;
}IndexSafeArray;

typedef struct String{
	char *characters;
}String;

void Array_init(Array *, unsigned int);

void Array_free(Array *);

void EntityHeader_init(EntityHeader *);

void *Array_addItem(Array *);

void *Array_getItemPointerByIndex(Array *, unsigned int);

void *Array_getItemPointerByID(Array *, size_t);

unsigned int Array_getItemIndexByID(Array *, size_t);

void Array_removeItemByIndex(Array *, unsigned int);

void Array_removeItemByID(Array *, size_t);

void Array_clear(Array *);

void initTmpArrays();

void Array_addToTmpArrays(Array *);

void freeTmpArrays();

void String_init(String *, char *);

void String_setText(String *, char *);

void String_concatText(String *, char *);

void String_concatString(String *, String);

void String_concatInt(String *, int);

unsigned int String_getLength(String *);

void String_free(String *);

bool compareFloatToFloat(float, float);

void IndexSafeArray_init(IndexSafeArray *, unsigned int, unsigned int);

unsigned int IndexSafeArray_addItem(IndexSafeArray *);

void *IndexSafeArray_getItemPointer(IndexSafeArray *, unsigned int);

void IndexSafeArray_removeItem(IndexSafeArray *, unsigned int);

void IndexSafeArray_clear(IndexSafeArray *);

void IndexSafeArray_free(IndexSafeArray *);

#endif
