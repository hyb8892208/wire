
#ifndef _ACFA_C_API_H_
#define _ACFA_C_API_H_

#ifdef __cplusplus
extern "C" {
#endif

// create a ini object
void* acfa_create(void);

// load content of a ini object from file
int acfa_load(void* handler, char* fName);

// open a ini file and return object;
void* acfa_open(char* fname);

char* acfa_get_comment_mark(void* handler);

void acfa_set_comment_mark(void* handler, char* m);

// close a ini file object
void acfa_close(void* handler);

// save the ini object
int acfa_save(void* handler);

// save the ini object to fName;
int acfa_save_to(void* handler, char* fName);

typedef char** c_array;

const char* acfa_get_value(void* handler, char* section, char* key);

int acfa_get_value_array(void* handler, char* section, char* key, c_array* array);

void acfa_free_c_array(c_array a);

c_array acfa_create_c_array(int num, int len);

int	acfa_set_value(void* handler, char* section, char* key, char* v, int autocreate);

int	acfa_set_value_array(void* handler, char* section, char* key, c_array array, int autocreate);

void acfa_append_value(void* handler, char* section, char*key, char*v, int delimiter);

int acfa_insert_before(void* handler, char* section, char* key, char* v, char* which);

int acfa_insert_after(void* handler, char* section, char* key, char* v, char* which);

int acfa_exist(void* handler, char* section, char* key, char* v);

int acfa_delete_key(void* handler, char* section, char* akey);

int acfa_delete_value(void* handler, char* section, char* akey, char* avalue, int  casesensitive);

int acfa_delete_section(void* handler, char* section);

int acfa_rename_section(void* handler, char* src, char* dst);

int acfa_count(void* handler, char* section, char* key);


#ifdef __cplusplus
}	//extern "C"
#endif


#endif //_ACFA_C_API_H_

