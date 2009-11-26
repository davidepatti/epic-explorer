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
	int rank;
	string logfile;
#ifdef EPIC_MPI
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	char* base_path_cstr;
	int length;

	if(rank == 0) {
		base_path_cstr = getenv(BASE_DIR);
		length = strlen(base_path_cstr) + 1;
		//cerr << "DEBUG strlen : " << length << endl;
	}
	MPI_Bcast(&length,1,MPI_INT,0,MPI_COMM_WORLD);

	if(rank != 0){
		base_path_cstr = new char[length];
	}

	MPI_Bcast(base_path_cstr, length, MPI_CHAR, 0, MPI_COMM_WORLD);
	string base_path = string(base_path_cstr);
	setenv(BASE_DIR, base_path_cstr, 1);

	logfile = base_path + string(EE_LOG_PATH);
	write_to_log(rank,logfile,"Starting MPI epic process with rank "+to_string(rank)+" on base path "+base_path);

#else
	rank = 0;
	string base_path = string(getenv(BASE_DIR));
	logfile = base_path +string(EE_LOG_PATH);
	write_to_log(rank,logfile,"Starting epic process on base path "+base_path);
#endif

    User_interface *ui = new User_interface(base_path);
    ui->interact();

#ifdef EPIC_MPI
    MPI_Finalize();
#endif
    write_to_log(rank,logfile,"Terminating epic process");
    return EXIT_SUCCESS;
}
