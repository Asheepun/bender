#include "engine/array.h"

#include "stdbool.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static size_t availableID = 0;

void Array_init(Array *array_p,  unsigned int itemSize){

	//array_p->items = malloc(maxLength * itemSize);
	array_p->maxLength = 16;
	array_p->itemSize = itemSize;
	array_p->items = malloc(array_p->maxLength * array_p->itemSize);
	array_p->length = 0;

}

void Array_free(Array *array_p){
	free(array_p->items);
}

void EntityHeader_init(EntityHeader *entityHeader_p){

	entityHeader_p->ID = availableID;
	availableID++;

}

void *Array_addItem(Array *array_p){
	
	array_p->length++;

	if(array_p->length > array_p->maxLength){
		array_p->maxLength += 16;
		array_p->items = realloc(array_p->items, (array_p->maxLength) * array_p->itemSize);
	}

	return Array_getItemPointerByIndex(array_p, array_p->length - 1);

}

void *Array_getItemPointerByIndex(Array *array_p, unsigned int index){

	if(index >= array_p->length){
		printf("Index is bigger or equal to array length!\n");
		printf("index: %i\n", index);
		printf("array length: %i\n", array_p->length);
		return NULL;
	}

	return (void *)((char *)array_p->items + index * array_p->itemSize);

}

void *Array_getItemPointerByID(Array *array_p, size_t ID){

	for(int i = 0; i < array_p->length; i++){

		void *itemPointer = Array_getItemPointerByIndex(array_p, i);

		if(((EntityHeader *)itemPointer)->ID == ID){
			return itemPointer;
		}

	}

	//printf("***\n");
	//printf("Array_getItemPointerByID\n");
	//printf("COULD NOT FIND ARRAY ITEM WITH ID: %i\n", ID);
	//printf("***\n");

	return NULL;

}

unsigned int Array_getItemIndexByID(Array *array_p, size_t ID){

	for(int i = 0; i < array_p->length; i++){

		void *itemPointer = Array_getItemPointerByIndex(array_p, i);

		if(((EntityHeader *)itemPointer)->ID == ID){
			return i;
		}

	}

	printf("***\n");
	printf("Array_getItemIndexByID\n");
	printf("COULD NOT FIND ARRAY ITEM WITH ID: %i\n", ID);
	printf("***\n");

}

void Array_removeItemByIndex(Array *array_p, unsigned int index){

	if(index >= array_p->length){
		printf("***\n");
		printf("TRIED REMOVING ITEM AT AN INDEX BIGGER THAN THE ARRAY LENGTH\n");
		printf("array pointer: %p\n", array_p);
		printf("index: %i\n", index);
		printf("array length: %i\n", array_p->length);
		printf("***\n");
		return;
	}

	if(index < 0){
		printf("***\n");
		printf("TRIED REMOVING ITEM WITH INDEX SMALLER THAN 0\n!!!");
		printf("array pointer: %p\n", array_p);
		printf("index: %i\n", index);
		printf("array length: %i\n", array_p->length);
		printf("***\n");
		return;
	}
	
	if(index < array_p->length - 1){
		memcpy(
			(char *)array_p->items + index * array_p->itemSize,
			(char *)array_p->items + (index + 1) * array_p->itemSize,
			(array_p->length - index - 1) * array_p->itemSize
		);
	}

	array_p->length--;
}

void Array_removeItemByID(Array *array_p, size_t ID){

	unsigned int index = Array_getItemIndexByID(array_p, ID);

	Array_removeItemByIndex(array_p, index);

}

void Array_clear(Array *array_p){

	array_p->length = 0;
	//while(array_p->length > 0){
		//Array_removeItemByIndex(array_p, 0);
	//}

}

bool compareFloatToFloat(float a, float b){
	return fabs(a - b) < 0.000001;
}

void IndexSafeArray_init(IndexSafeArray *indexSafeArray_p, unsigned int itemSize, unsigned int maxLength){

	indexSafeArray_p->itemSize = itemSize;
	indexSafeArray_p->indexSize = itemSize + sizeof(bool);
	indexSafeArray_p->deepestIndex = 0;
	//indexSafeArray_p->spaces = 0;
	indexSafeArray_p->maxLength = maxLength;
	
	indexSafeArray_p->items = malloc(maxLength * indexSafeArray_p->indexSize);

	memset(indexSafeArray_p->items, 0, maxLength * indexSafeArray_p->indexSize);
	//for(int i = 0; i < maxLength; i++){
		//bool *flag = indexSafeArray_p->items + i * indexSafeArray_p->indexSize;
		//*flag = false;
	//}

}

unsigned int IndexSafeArray_addItem(IndexSafeArray *indexSafeArray_p){

	for(int i = 0; i < indexSafeArray_p->maxLength; i++){

		bool *flag = indexSafeArray_p->items + i * indexSafeArray_p->indexSize;

		if(!*flag){

			*flag = true;

			if(i > indexSafeArray_p->deepestIndex){
				indexSafeArray_p->deepestIndex = i;
			}
			//indexSafeArray_p->length++;

			return i;
			//return (void *)flag + sizeof(bool);

		}
		
	}

	printf("ARRAY IS FULL! COULD NOT ADD ITEM!\n");
	printf("ArrayPointer: %p\n", indexSafeArray_p);

}

void *IndexSafeArray_getItemPointer(IndexSafeArray *indexSafeArray_p, unsigned int index){

	bool *flag = indexSafeArray_p->items + index * indexSafeArray_p->indexSize;

	if(*flag){
		return (void *)((char *)flag + sizeof(bool));
	}

	return NULL;

}

void IndexSafeArray_removeItem(IndexSafeArray *indexSafeArray_p, unsigned int index){

	bool *flag = indexSafeArray_p->items + index * indexSafeArray_p->indexSize;

	*flag = false;
	
}

void IndexSafeArray_clear(IndexSafeArray *indexSafeArray_p){
	memset(indexSafeArray_p->items, 0, indexSafeArray_p->maxLength * indexSafeArray_p->indexSize);
	//indexSafeArray_p->length = 0;
	//for(int i = 0; i < indexSafeArray_p->length; i++){
		//IndexSafeArray_removeItem(indexSafeArray_p, i);
	//}
}

void IndexSafeArray_free(IndexSafeArray *indexSafeArray_p){
	free(indexSafeArray_p->items);
	indexSafeArray_p->items = NULL;
}
