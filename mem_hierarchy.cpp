/***************************************************************************
                          memory_level.cpp  -  description
                             -------------------
    begin                : Sat Aug 17 2002
    copyright            : (C) 2002 by Davide Patti
    email                : davidepatti@email.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mem_hierarchy.h"

Mem_hierarchy::Mem_hierarchy(){

    string home = string(getenv("HOME"));

    cache_config_file = home+"/trimaran-workspace/cache.cfg";

    // L1 Data cache 
    L1D.label = "L1_data";
    L1D.type = L1D_CACHE;
    // L1 Instruction Cache
    L1I.label = "L1_inst";
    L1I.type = L1I_CACHE;
    // L2 Unified Cache 
    L2U.label = "L2_unified";
    L2U.type = L2U_CACHE;
}

void Mem_hierarchy::set_to_default()
{
    // default values are specified as a constructor Parameter

    L1D.size.set_to_default();
    L1D.block_size.set_to_default() ;
    L1D.associativity.set_to_default();

    L1I.size.set_to_default();
    L1I.block_size.set_to_default();
    L1I.associativity.set_to_default();

    L2U.size.set_to_default();
    L2U.block_size.set_to_default();
    L2U.associativity.set_to_default();

    // NOTE :
    // we are not interested in investigating subblock size, so we
    // alway consider its size equal to blocksize

    L1D.subblock_size = L1D.block_size.get_val();
    L1I.subblock_size = L1I.block_size.get_val();
    L2U.subblock_size = L2U.block_size.get_val();
}

void Mem_hierarchy::load_cache_config()
{
    std::ifstream input_file(cache_config_file.c_str());

    int value ;

    if (!input_file) 
    {
	cout << "\nError opening cache config file :" << cache_config_file;
	wait_key();
    }
    else
    {
	// Note that subblock size value in config file is ignored 
	// subblock size is assumed to be equal to block size

	go_until("L2U_blocksize",input_file);
	input_file >> value;
	L2U.block_size.set_val(value);
	L2U.subblock_size = L2U.block_size.get_val();

	go_until("L2U_size",input_file);
	input_file >> value;
	L2U.size.set_val(value);

	go_until("L2U_associativity",input_file);
	input_file >> value;
	L2U.associativity.set_val(value);


	go_until("L1I_blocksize",input_file);
	input_file >> value;
	L1I.block_size.set_val(value);
	L1I.subblock_size = L1I.block_size.get_val();

	go_until("L1I_size",input_file);
	input_file >> value;
	L1I.size.set_val(value);

	go_until("L1I_associativity",input_file);
	input_file >> value;
	L1I.associativity.set_val(value);


	go_until("L1D_blocksize",input_file);
	input_file >> value;
	L1D.block_size.set_val(value) ;
	L1D.subblock_size = L1D.block_size.get_val();

	go_until("L1D_size",input_file);
	input_file >> value;
	L1D.size.set_val(value);

	go_until("L1D_associativity",input_file);
	input_file >> value;
	L1D.associativity.set_val(value);

    }
}

void Mem_hierarchy::save_cache_config()
{
    std::ofstream output_file(cache_config_file.c_str());

    if (!output_file) 
    {
	cout << "\nerror opening hmdes file :" << cache_config_file;
	wait_key();
    }
    else
    {
	output_file <<"\n# EPIC explorer cache configuration ";
	output_file <<"\n";
	output_file <<"\n# level 2 unified cache";
	output_file <<"\nL2U_blocksize " << L2U.block_size.get_val();
	output_file <<"\nL2U_subblocksize " << L2U.block_size.get_val();
	output_file <<"\nL2U_size " << L2U.size.get_val() ;
	output_file <<"\nL2U_associativity " << L2U.associativity.get_val();
	output_file <<"\n";
	output_file <<"\n# level 1 instruction cache ";
	output_file <<"\n";
	output_file <<"\nL1I_blocksize " << L1I.block_size.get_val();
	output_file <<"\nL1I_subblocksize " << L1I.block_size.get_val();
	output_file <<"\nL1I_size " << L1I.size.get_val();
	output_file <<"\nL1I_associativity " << L1I.associativity.get_val();
	output_file <<"\n";
	output_file <<"\n# level 1 data cache";
	output_file <<"\n";
	output_file <<"\nL1D_blocksize " << L1D.block_size.get_val();
	output_file <<"\nL1D_subblocksize " <<L1D.block_size.get_val();
	output_file <<"\nL1D_size " << L1D.size.get_val();
	output_file <<"\nL1D_associativity " << L1D.associativity.get_val();
    }
}

void Mem_hierarchy::print_cache_config(const Cache& cache) const 
{

    //cout <<"\n---------------------------------\n";
    cout << cache.label;
    cout <<"\n---------------------------------\n";

    cout << "\n block Size : " << cache.block_size.get_val();
    cout << "\n size : " << cache.size.get_val();
    cout << "\n associativity: " << cache.associativity.get_val();
    
#ifdef VERBOSE
    cout << "\n subblock Size : " << cache.subblock_size;
    int sets = cache.size.get_val()/(cache.block_size.get_val()*cache.associativity.get_val());

    int tag = (int)(32 - logtwo_area(cache.block_size.get_val()) - logtwo_area(sets));

    cout << "\n( tag size : " << tag << " )";
    cout << "\n( sets : " << sets << " )";
#endif
    cout <<"\n---------------------------------\n";

}

// test configuration of whole mem hiearchy system
bool Mem_hierarchy::test_valid_config() const 
{
    // first check if each cache has an admissible configuration

    if ( !(test_valid_cache(L1D)) ||
	 !(test_valid_cache(L1I)) ||
	 !(test_valid_cache(L2U)) )
	return false;

    // then check if relative configurations of caches are acceptable.
    // here are discarded configurations in which L2 size is smaller
    // than total L1 cache sizes 

    if ( L2U.size.get_val() < L1D.size.get_val() + L1I.size.get_val() )
	return false;

    return true;
}

bool Mem_hierarchy::test_valid_cache(int C, int B, int A) const
{
    char fully_assoc;

    if (B<8) 
    {
	cout << "\n BLOCK SIZE must be > 8 bytes ! ";
	return false;
    }
    if (A==C/B) fully_assoc =1;
    else
	fully_assoc = 0;

    if (fully_assoc==0) 
    {
	if (C/(B*A)<=0) return false;

	//if ((8*B*A)<=0) return false;
    }
    else
    {
	//if (C/(2*B)<=0) return false;
	if (C/B<=0) return false;
    }

    return true;
}

bool Mem_hierarchy::test_valid_cache(const Cache& cache) const
{
    int A = cache.associativity.get_val();
    int B = cache.block_size.get_val();
    int C = cache.size.get_val();

    return test_valid_cache(C,B,A);
}

Mem_hierarchy::~Mem_hierarchy(){
}

