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

#include <map>

using namespace std;

enum region_category{HIGH_INNOVATION, NO_INNOVATION, LOW_INNOVATION};

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
		unsigned id;
		double innovation_score;
		bool valid;
	
	// the index is the one computed by Parameter::get_index()
		Edges edges[N_PARAMS];

		Region();
	
//  ~Region();
};

/**
 * It describes an era. In particular, it holds information about:
 * - which regions are present in that era
 * - the innovation score of each of such regions
 * - the classification in high, mid and low region
 */
class EraDescriptor{
	private:
		int era_id;
		std::map<unsigned, double> high_innovation_regions;
		std::map<unsigned, double> no_innovation_regions;
		std::map<unsigned, double> low_innovation_regions;

	public:
		EraDescriptor();
		void reinitialize(int era_id_);
		void add_region(unsigned region_id, double innovation_score, region_category category);
		const string tostring();
};



/********************************************************************************/
/***************DEBUG FUNCTIONS**************************************************/
/********************************************************************************/
//TODO: spostare in un file a parte

/**
 * category: can be "no_innovation", "low_innovation", "high_innovation"
 */
const string to_string(const Region r, int era, string category);

const string to_string_region_concise(int era, int region_index);


// Exit the program if the region is not well-formed. Do nothing, otherwise
void well_formed(Region r, Explorer* expl);




#endif
