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

typedef enum {HIGH_INNOVATION=1, NO_INNOVATION=2, LOW_INNOVATION=3} region_category;

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
		
		void set_edges(const Edges* src_edges);

		Region();
		const string tostring();
	
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
		std::map<unsigned, double> high_innovation_region_descriptors;
		std::map<unsigned, double> no_innovation_region_descriptors;
		std::map<unsigned, double> low_innovation_region_descriptors;

	public:
		EraDescriptor();
		void reinitialize(int era_id_);
		void add_region_descriptor(
			unsigned region_id, double innovation_score, region_category category);
		const string tostring();
};

class RegionHandler{
	private:
		// Event if having regions public would be faster, please leave regions
		// private. This reccomendation has the goal to ensure the consistency
		// of the algorithm
		vector<Region*> current_era_regions;
		vector<Region*> next_era_regions;
		vector<Region*> regions_to_be_deleted;
		unsigned last_region_id;
		
	public:
		RegionHandler();
		void add_region_to_next_era(Region* r);
		const Region* get_current_era_region(unsigned region_index);
		void new_era_initialization();
		const vector<Region*> get_current_era_regions();
		
		void annotate_for_deletion(const Region* r);
		
		// Calculate the innovation_score of each region. Return the sum of all
		// innovation scores
		double update_innovation_scores_of_current_era_regions(
			int era,
			vector<Simulation> old_pareto_set, vector<Simulation> new_pareto_set
		);
		
		#ifdef SEVERE_DEBUG
		//Check if all the current_era_regions are well formed
		void check_regions(Explorer* expl);
		#endif
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
