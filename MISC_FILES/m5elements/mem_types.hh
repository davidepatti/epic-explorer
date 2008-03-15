
#ifndef _M5E_MEM_TYPES_HH
#define _M5E_MEM_TYPES_HH

typedef unsigned long M5E_addr;
typedef unsigned long M5E_data;

enum M5E_mem_req_type {
	M5E_LOAD,
	M5E_STORE
};

typedef struct M5E_mem_req {
	enum M5E_mem_req_type type;
	M5E_addr addr;
	M5E_data data;
	int size;
	int is_valid;
	long id;          /* unique id assigned by dcache system */
	int register_id;  /* register# -- SIMU uses this as a non-unique ID */
} M5E_mem_req;


#endif /* _M5E_MEM_TYPES_HH */
