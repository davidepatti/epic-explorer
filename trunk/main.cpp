/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : sab ago 17 14:48:41 CEST 2002
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

#include "user_interface.h"
#include "explorer.h"
#include "common.h"
#include <signal.h>

string current_subspace; //TODO: find a better place

void test(void);

int main(int argc, char *argv[])
{
    User_interface *user_demo = new User_interface();
    user_demo->interact();

    return EXIT_SUCCESS;
}

void test()
{
    Explorer my_explorer;

    //suppose we want explore a space where only two particular
    //parameters can vary , for example , L1D size and  n. of integer_units .
    //
    
    // first create an appropriated space mask :
    
    // UNSET_ALL means : no parameter can be modified
    Space_mask mask1 = my_explorer.get_space_mask(UNSET_ALL); 

    // modify mask values associated to the parameters we want
    // explore:

    // build a space of all possible explorations associated to this
    // mask :

    Configuration base_conf = my_explorer.create_configuration();
    base_conf.L1I_size = 128;

    vector<Configuration> space1 = my_explorer.build_space(mask1,base_conf);

    // simultate space 
    vector<Simulation> sims1 = my_explorer.simulate_space(space1);

    // get non dominated simulations
    vector<Simulation> pareto_set1 = my_explorer.get_pareto(sims1);

    // save them
    my_explorer.save_simulations(pareto_set1,"TEMP_PROVA",SHOW_ALL);


}

