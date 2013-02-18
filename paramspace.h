/***************************************************************************
                          parameter.h  -  description
                             -------------------
    begin                : ven ott 4 2002
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

#ifndef PARAMSPACE_H
#define PARAMSPACE_H

#define ALPHA 1.0 //Used to decide whether a region has high, low or no innovation
#define DEBUG_LEVEL_DEBUG 1
#define SEVERE_DEBUG 1


using namespace std;


typedef struct 
{
	//These are indexes of the values vector of a parameter
    int a,b;
} Edges;




/*
class Region {
public: 
	double id;
	double innovation_score;
	bool valid;

//	Region()
//  ~Region();

	
	// The simulations of the current era that fall into this region
//	list<Simulation> simulations; //PENSO CHE SIA INUTILE


// the index is the one computed by Parameter::get_index()
	Edges edges[N_PARAMS];
private:

};
*/


class Region{
	public:
		double id;
		double innovation_score;
		bool valid;
	
	// the index is the one computed by Parameter::get_index()
		Edges edges[N_PARAMS];

		Region();
	
//  ~Region();
};



/********************************************************************************/
/***************DEBUG FUNCTIONS**************************************************/
/********************************************************************************/
//TODO: spostare in un file a parte

const string to_string(Region r);

const string to_string_region_concise(int era, int region_index);


// Exit the program if the region is not well-formed. Do nothing, otherwise
void well_formed(Region r, Explorer* expl);




#endif
