/***************************************************************************
                          user_interface.h  -  description
                             -------------------
    begin                : Sat Aug 24 2002
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

#ifndef INTERFACE_USER_DEMO_H

#define INTERFACE_USER_DEMO_H

#include "explorer.h"
#include "estimator.h"
#include "trimaran_interface.h"
#include "processor.h"
#include "mem_hierarchy.h"
#include "environment.h"
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

/**
  *@author Davide Patti
  */



class User_interface {
public: 
	User_interface(const string& dir);
	~User_interface();
	void interact();
private:
	struct User_Settings user_settings;

	Trimaran_interface * trimaran_interface;
	Explorer * my_explorer;

	int show_menu();

	void edit_user_settings();

	void save_settings(string settings_file);
	void save_settings_wrapper();
	void load_settings(string settings_file);
	void load_settings_wrapper();

	void save_subspace_wrapper();
	void load_subspace_wrapper();

	void start_exploration_message();
	void edit_exploration_space();
	void reload_hmdes_file();
	void reload_memhierarchy_config();
	void compile_hmdes_file();
	void compile_benchmark();
	void execute_benchmark();
	void view_statistics();
	void compute_cost();
	void choose_benchmark();
	void info();
	void view_processor_config();
	void view_cache_config();
	void save_processor_config();
	void save_cache_config();
	void schedule_explorations();

	inline string status_string(bool b);
	string word;
	string base_path;

        int myrank;
	int mysize;
};

#endif
