/*******************************************************************************


  This is a hash table API  


*******************************************************************************/
#if defined(DEBUG)
#undef DEBUG
#endif


#include "hash_table.h"


UINT32 ComputeBucketIndex( CADDR_T key, UINT32 key_size, UINT32 bucket_size );




HASH_TABLE *CreateHashTable( UINT32 bucket_size )
{
	UINT32 i = 0;
	HASH_TABLE *table = NULL;

	START_FUNCTION("CreateHashTable");
	
	if( bucket_size < 1 ) {
		END_FUNCTION( "CreateHashTable", FAILING );
		return NULL;
	}

	table = (HASH_TABLE*)CALLOC( 1, sizeof(HASH_TABLE) );
	if( table == NULL ) {
		END_FUNCTION("CreateHashTable", FAILING );
		return NULL;
	}

	table->buckets = (LINKED_LIST**)CALLOC( bucket_size, sizeof(LINKED_LIST*) );
	if( table->buckets == NULL ) {
		END_FUNCTION("CreateHashTable", FAILING );
		return NULL;
	}

	for( i = 0; i < bucket_size; i++ ) {
		table->buckets[i] = CreateLinkedList();
		if( table->buckets[i] == NULL ) {
			DeleteHashTable( &table );
			END_FUNCTION("CreateHashTable", FAILING );
			return NULL;
		}
	}
	
	table->bucket_size = bucket_size;

	END_FUNCTION( "CreateHashTable", SUCCESS );
	return table;
}


RET_VAL PutInHashTable( CADDR_T key, UINT32 key_size, CADDR_T value, HASH_TABLE *table )
{
	RET_VAL ret = SUCCESS;
	UINT32 bucket_index = 0;
	LINKED_LIST *list = NULL;
	HASH_ENTRY *entry = NULL;
	HASH_ENTRY *target = NULL;


	START_FUNCTION("PutInHashTable");

	bucket_index = ComputeBucketIndex( key, key_size, table->bucket_size );
	list = table->buckets[bucket_index];

	ResetCurrentElement( list );
	while( (target = (HASH_ENTRY*)GetNextFromLinkedList(list)) != NULL ) {
		if( key_size == target->key_size ) {
			if( memcmp( key, target->key, key_size ) == 0 ) {
				/*same key is found*/
				target->value = value;
				END_FUNCTION( "PutInHashTable", ret );
				return ret;
			}
		}
	}

	/*same key not found*/
	entry = (HASH_ENTRY*)MALLOC( sizeof(HASH_ENTRY) );
	if( entry == NULL ) {
		return ErrorReport( FAILING, "PutInHashTable", "hash entry allocation error" );
	}
	entry->key = key;
	entry->key_size = key_size;
	entry->value = value;

	if( IS_FAILED( ( ret = AddElementInLinkedList( (CADDR_T)entry, list ) ) ) ) {
		END_FUNCTION( "PutInHashTable", ret );
		return ret;
	}	
	table->entry_count++;
	END_FUNCTION( "PutInHashTable", ret );
	return ret;
}


RET_VAL RemoveFromHashTable( CADDR_T key, UINT32 key_size, HASH_TABLE *table )
{
	RET_VAL ret = SUCCESS;
	UINT32 bucket_index = 0;
	LINKED_LIST *list = NULL;
	HASH_ENTRY *target = NULL;


	START_FUNCTION("RemoveFromHashTable");

	bucket_index = ComputeBucketIndex( key, key_size, table->bucket_size );
	list = table->buckets[bucket_index];

	ResetCurrentElement( list );
	while( (target = (HASH_ENTRY*)GetNextFromLinkedList(list)) != NULL ) {
		if( key_size == target->key_size ) {
			if( memcmp( key, target->key, key_size ) == 0 ) {
				/*same key is found*/
				RemoveCurrentFromLinkedList(list);
				table->entry_count--;
				END_FUNCTION( "RemoveFromHashTable", ret );
				return ret;
			}
		}
	}

	END_FUNCTION( "RemoveFromHashTable", ret );
	return ret;
}


CADDR_T GetValueFromHashTable( CADDR_T key, UINT32 key_size, HASH_TABLE *table )
{
	RET_VAL ret = SUCCESS;
	UINT32 bucket_index = 0;
	LINKED_LIST *list = NULL;
	HASH_ENTRY *target = NULL;


	START_FUNCTION("GetValueFromHashTable");

	bucket_index = ComputeBucketIndex( key, key_size, table->bucket_size );
	list = table->buckets[bucket_index];

	ResetCurrentElement( list );
	while( (target = (HASH_ENTRY*)GetNextFromLinkedList(list)) != NULL ) {
		if( key_size == target->key_size ) {
			if( memcmp( key, target->key, key_size ) == 0 ) {
				/*found*/
				END_FUNCTION( "GetValueFromHashTable", ret );
				return target->value;
			}
		}
	}

	END_FUNCTION( "GetValueFromHashTable", ret );
	return NULL;
}


LINKED_LIST *GenerateKeyList( HASH_TABLE *table )
{
	RET_VAL ret = SUCCESS;
	UINT32 i = 0;
	HASH_ENTRY *entry = NULL;
	LINKED_LIST *keys = NULL;
	LINKED_LIST *list = NULL;
	
	START_FUNCTION("GenerateKeyList");
	
	keys = CreateLinkedList();
	if( keys == NULL ) {
		return NULL;
	}

	for( i = 0; i < table->bucket_size; i++ ) {
		list = table->buckets[i];
		
		ResetCurrentElement(list);
		while( (entry = (HASH_ENTRY*)GetNextFromLinkedList(list)) != NULL ) {
			if( IS_FAILED( (ret = AddElementInLinkedList( entry->key, keys ) ) ) ) {
				DeleteLinkedList( &keys );
				END_FUNCTION( "GenerateKeyList", ret );
				return NULL;
			}
		}
	}
	
	END_FUNCTION( "GenerateKeyList", SUCCESS );
	return keys;
}

LINKED_LIST *GenerateValueList( HASH_TABLE *table )
{
	RET_VAL ret = SUCCESS;
	UINT32 i = 0;
	HASH_ENTRY *entry = NULL;
	LINKED_LIST *values = NULL;
	LINKED_LIST *list = NULL;

	START_FUNCTION("GenerateValueList");

	values = CreateLinkedList();
	if( values == NULL ) {
		return NULL;
	}

	for( i = 0; i < table->bucket_size; i++ ) {
		list = table->buckets[i];
		
		ResetCurrentElement(list);
		while( (entry = (HASH_ENTRY*)GetNextFromLinkedList(list)) != NULL ) {
			if( IS_FAILED( (ret = AddElementInLinkedList( entry->value, values ) ) ) ) {
				DeleteLinkedList( &values );
				END_FUNCTION( "GenerateValueList", ret );
				return NULL;
			}
		}
	}
	END_FUNCTION( "GenerateValueList", SUCCESS );
	return values;
}



UINT32 GetHashEntryCount( HASH_TABLE *table )
{
	return table->entry_count;
}


BOOL ExistInHashTable( CADDR_T key, UINT32 key_size, HASH_TABLE *table )
{
	RET_VAL ret = SUCCESS;
	UINT32 bucket_index = 0;
	LINKED_LIST *list = NULL;
	HASH_ENTRY *target = NULL;


	START_FUNCTION("ExistInHashTable");

	bucket_index = ComputeBucketIndex( key, key_size, table->bucket_size );
	list = table->buckets[bucket_index];

	ResetCurrentElement( list );
	while( (target = (HASH_ENTRY*)GetNextFromLinkedList(list)) != NULL ) {
		if( key_size == target->key_size ) {
			if( memcmp( key, target->key, key_size ) == 0 ) {
				/*found*/
				END_FUNCTION( "ExistInHashTable", ret );
				return TRUE;
			}
		}
	}

	END_FUNCTION( "ExistInHashTable", ret );
	return FALSE;
}


RET_VAL DeleteHashTable( HASH_TABLE **table )
{
	RET_VAL ret = SUCCESS;
	UINT32 i = 0;
	HASH_ENTRY *entry = NULL;
	LINKED_LIST *list = NULL;
	
	START_FUNCTION("GenerateKeyList");
	
	if( *table == NULL ) {
		END_FUNCTION( "GenerateKeyList", ret );
		return ret;
	}
	for( i = 0; i < (*table)->bucket_size; i++ ) {
		list = (*table)->buckets[i];
					
		ResetCurrentElement(list);
		while( (entry = (HASH_ENTRY*)GetNextFromLinkedList(list)) != NULL ) {
			FREE(entry);
		}
		DeleteLinkedList( &list );
	}
	FREE(*table);
	END_FUNCTION( "GenerateKeyList", ret );
	return ret;
}

UINT32 ComputeBucketIndex( CADDR_T key, UINT32 key_size, UINT32 bucket_size )
{
	UINT32 i = 0;
	UINT32 hash_code = 0;
	UINT32 index = 0;
	BYTE *bytes = NULL;

	START_FUNCTION("ComputeBucketIndex");
	bytes = (BYTE*)key;

	for( i = 0; i < key_size; i++ ) {
		hash_code += ((bytes[i])&0x000000FF);	
	}
	
	index = hash_code % bucket_size;

	END_FUNCTION( "ComputeBucketIndex", SUCCESS );
	return index;
}

