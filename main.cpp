/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : sab ago 17 14:48:41 CEST 2002
    copyright            : (C) 2002 by Davide Patti
    email                : dpatti@diit.unict.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "user_interface.h"
#include "explorer.h"
#include "common.h"
#include <signal.h>

#ifdef EPIC_MPI
#include "mpi.h"
#endif

int main(int argc, char *argv[])
{
#ifdef EPIC_MPI
	MPI_Init(&argc,&argv);
	//MPI::Init(argc,argv);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	//rank = MPI::COMM_WORLD.Get_rank();
	char* base_path_cstr;
	int length;
	//cerr << "DEBUG " << rank << " - base_path is: " << base_path << endl; 
	if(rank == 0) {
		base_path_cstr = getenv(BASE_DIR);
		length = strlen(base_path_cstr) + 1;
		//cerr << "DEBUG strlen : " << length << endl;
	}
	MPI_Bcast(&length,1,MPI_INT,0,MPI_COMM_WORLD);
	//MPI::COMM_WORLD.Bcast(&length, 1, MPI_INT, 0);
	//cerr << "DEBUG " << rank << " - length is: " << length << endl;

	if(rank != 0){
		base_path_cstr = new char[length];
	}

	MPI_Bcast(base_path_cstr, length, MPI_CHAR, 0, MPI_COMM_WORLD);
	//MPI::COMM_WORLD.Bcast(base_path_cstr, length, MPI_CHAR, 0);
	//cerr << "DEBUG " << rank << " - base_path_cstr is: " << base_path_cstr << endl;
	string base_path = string(base_path_cstr);
	cerr << "DEBUG " << rank << " - NEW base_path is: " << base_path << endl;
	setenv(BASE_DIR, base_path_cstr, 1);
	//cerr << "DEBUG " << rank << " getenv(BASE_DIR) is: " << getenv(BASE_DIR) << endl;
	//cerr << "DEBUG " << rank << " get_base_dir is: " << get_base_dir() << endl;
	//MPI_Finalize();
	//MPI::Finalize();
#else
    string base_path = string(getenv(BASE_DIR));
#endif

    User_interface *ui = new User_interface(base_path);
    ui->interact();

#ifdef EPIC_MPI
    MPI_Finalize();
#endif
    return EXIT_SUCCESS;
}
