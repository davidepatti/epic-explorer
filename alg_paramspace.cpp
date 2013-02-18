
// ********************************************************************
//   GA-based exploration
// ********************************************************************

//******************************************************************************************//
#include "explorer.h"
#include "common.h"
#include "paramspace.h"
#include "parameter.h"
#include <values.h>
#include <list>
#include <vector>
#include <cstdlib> //for rand()

#include <iostream>



string logfile;
int myrank;
vector<Region*> regions, no_innovation_regions;

Region::Region(){
			id=0; innovation_score=0; valid=false;
			for (int i=0; i<N_PARAMS; i++){
				edges[i].a=0; edges[i].b=0;
			}
		}
  


// TODO: sistemare correttamente la questione del seed
unsigned long paramspace_seed = 1;

bool edges_equality(const Edges& e1, const Edges& e2)
{
	return (e1.a == e2.a && e1.b == e2.b );
}

// Return edges with edges.a=-1 and edges.b=-1 if it is not possible to merge
Edges merge_intervals(Edges e1, Edges e2)
{
	Edges first_edges, second_edges;
	if (e1.b < e2.a){
		first_edges = e1; second_edges = e2;
	} 
	else if (e2.b < e1.a){
		first_edges = e2; second_edges = e1;
	}
	else{
		string message = "alg_paramspace.cpp:"+ to_string(__LINE__) +
			": ERROR: e1.a=" + to_string(e1.a)+ "; e1.b="+ to_string(e1.b)+
			"; e2.a=" + to_string(e2.a)+ "; e2.b="+ to_string(e2.b)+
			". This edges are not valid";
		write_to_log(myrank,logfile,message);
		exit(-8712);
	}

	Edges return_edges;
	
	if (second_edges.a == (first_edges.b + 1) )
	{
		// The intervals are continguous
		return_edges.a = (first_edges.a);
		return_edges.b = (second_edges.b);
	}else{
		return_edges.a = -1;
		return_edges.b = -1;
	}		
}

EParameterType get_splitting_parameter()
{
	int val = round( (double)rand() * (N_PARAMS-1) / RAND_MAX );
	#ifdef SEVERE_DEBUG
	if (val<0 || val>=N_PARAMS){
		printf("alg_paramspace.cpp %d: Error, val=%d",__LINE__, val);
	}
	#endif
	return (EParameterType) val;
}

//TODO: si potrebbe evitare di passare expl se getParameterRanges fosse statico
// At the end of this method, r will be a region including all the parameter space
void transform_to_root_region(Region* r, Explorer* expl)
{
	vector<pair<int,int> > ranges = expl->getParameterRanges();
	
	if (ranges.size() != N_PARAMS)
	{
		string message = "alg_paramspace.cpp:"+ to_string(__LINE__) +": ERROR: N_PARAMS=" + 
			to_string(N_PARAMS) + " whereas getParameterRanges()"
			+" has returned "+ to_string(ranges.size() ) + "elements";
		write_to_log(myrank,logfile,message);
		exit(-716);
	}
	
	for(int i=0; i<N_PARAMS; i++)
	{
		Parameter par = expl->getParameter( (EParameterType)i );
		Edges e; e.a=0; e.b=par.get_size()-1;
		r->edges[i] = e;
	}
	return;
}

double roulette_wheel_arc_length(Region* r)
{
   	if (r->innovation_score >= 0)
   		return 1.0;
  	else
   		return pow(2, r->innovation_score );
}


// Implementa l'algoritmo di roulette wheel. L'ampiezza dell'arco associato a 
// ciascuna regione e' 1 oppure, se is e' negativo, 2^is (is e' l'innovation score 
// della regione
Region* select_region(int era,vector<Region*> regions)
{
	double roulette_wheel_length = 0;
	for (int i=0; i<regions.size(); i++)
	{
		Region* regionn = regions[i]; // da togliere
		double incremento = roulette_wheel_arc_length( regions[i] ); // da togliere
		printf("incremento=%e\n",incremento);
		//roulette_wheel_length += roulette_wheel_arc_length( regions[i] );
		roulette_wheel_length += incremento;
		printf("roulette wheel length=%e\n",roulette_wheel_length);
	}

	// TODO: meglio usare
	// http://www.johndcook.com/cpp_TR1_random.html
	// solo che mi genera numeri enormi e non capisco perche'
	double random_number =  (double)(rand()*roulette_wheel_length)/(RAND_MAX);

    printf("random_number=%e\n",random_number);
 
	// double r =  (double)rand()/(RAND_MAX);
    // double random_point = (double)(r*total_innovation_score);
    double partial_roulette = 0;
    
    int i; for (i=0; partial_roulette < random_number; i++)
    	partial_roulette += roulette_wheel_arc_length( regions[i] );
    	
    i--;
    
    printf("partial_roulette=%e\n",partial_roulette);
    string message = "The next simulation will be chosen inside region " 
    	+ to_string_region_concise(era,i);
    write_to_log(myrank,logfile,message);	
    
    return regions[i];
}


vector<Region*> split_region(int region_index)
{
    // we just want to split in 2, but leaving vector for general case
    Region* region = regions[region_index];
    
    Region *r1,*r2;
    vector<Region*> output_regions;
    EParameterType splitting_parameter = get_splitting_parameter();
    
    Edges* interval = &(region->edges[splitting_parameter]);
	int interval_size = (interval->b) - (interval->a) +1;
    
    if (interval_size == 1){
    	output_regions.push_back(region);
    }
    else if (interval_size > 1)
    {
		Edges interval1, interval2; 
    	interval1.a = interval->a; 
    	interval1.b= interval->a + (int)(interval_size/2)-1;
    	
    	interval2.b = interval->b;
    	interval2.a = interval1.b + 1;
    	
    	r1 = region;
    	r2 = new Region();
    	
    	r1->edges[splitting_parameter] = interval1;
    	r2->edges[splitting_parameter] = interval2;
    	
		output_regions.push_back(r1);
	    output_regions.push_back(r2);
    }else{
    	// This is a debug check
    	string message = "alg_paramspace.cpp:"+ to_string(__LINE__) +
			": ERROR: = splitting_parameter="+ to_string(splitting_parameter)+
			"; interval size="+to_string(interval_size);
		write_to_log(myrank,logfile,message);
		exit(-12721);
    }
    
    return output_regions;
}


// If r1 and r2 are contiguous, return the merged region and deallocate r1 and r2.
// Else, return NULL.
Region* merge_regions(Region* r1, Region* r2)
{
    // TBD: what returns when merging is not possible ?
    Region* r=NULL;
    Edges merged_interval;
    int merging_parameter=-1;
    int num_equal_edges = 0;
    
    for (int j=0; j<N_PARAMS; j++)
    {
    	Edges e1 = r1->edges[j];
    	Edges e2 = r2->edges[j];
    	
    	if ( edges_equality(e1, e2) )
    		num_equal_edges += 1;
    	
    	else{
    	
    		Edges new_interval = merge_intervals(e1, e2);
    		
    		if (new_interval.a==-1 || new_interval.b==-1)
    		{
	    		// This is a debug check:
	    		if (merging_parameter >= 0)
    			{
					string message = "alg_paramspace.cpp:"+ to_string(__LINE__) +
						": ERROR: = two merging parameters has been found. It is not possible";
					write_to_log(myrank,logfile,message);
					exit(-123221);
				}
				
				
				//else
				merging_parameter = j;
				merged_interval = new_interval;
    		}
    	}
    }
    
    if (num_equal_edges == N_PARAMS-1   ||    merging_parameter >= 0)
    {
    	// r1 is transformed in the new region
    	r1->edges[merging_parameter] = merged_interval;
    	
    	// the new region is given the max between the innovation scores of r1 and r2
    	r1->innovation_score = 
    		r1->innovation_score >= r2->innovation_score ?
    		r1->innovation_score : r2->innovation_score;
    		
    	delete r2;
    	return r1;
    }
    else
    	return NULL;
}


// Calculate s(s1 -> s2)
double separation(const Simulation& s1, const Simulation& s2)
{
    return 	fabs( (s1.energy-s2.energy) ) 		/ s1.energy 	+
    		fabs( (s1.area-s2.area) ) 			/ s1.area 		+
    		fabs( (s1.exec_time-s2.exec_time) ) 	/ s1.exec_time
    		;
}

double get_innovation_score(const Simulation& s, vector<Simulation> pareto)
{
	if (pareto.empty())
		return 0;

	double innovation_score = separation( pareto[0], s);
	for (int i=1; i<pareto.size(); i++)
	{
		// Calculate s(pareto[i] -> s)
		double temp=separation( pareto[i], s);
		if (temp<innovation_score)
			innovation_score = temp;
	}
	
    return innovation_score;
}

//TODO: mqybe the average innovation score must be returned
// Calculate the innovation_score of each region. Return the sum of all innovation
// scores
double update_region_innovation_scores(int era,vector<Region*> regions, 
	vector<Simulation> old_pareto_set, vector<Simulation> new_pareto_set)
{
	double total_innovation_score = 0;
	
	// Clear the innovaton score of all regions
	for (int i=0; i<regions.size(); i++)
	{
		if (regions[i]->innovation_score >= 0)
			regions[i]->innovation_score=0;
		//else
		//	do nothing because this region has negative innovation_score
	}

    for (int i=0; i<new_pareto_set.size(); i++)
    {
    	Simulation s = new_pareto_set[i];
   		Region* region = (Region*) (s.config.pointer);
   		
   		if (region->innovation_score<0 && get_innovation_score(s,old_pareto_set)>0){
	   		// We thought that this region was useless, whereas it has a configuration 
	   		// in the new pareto set. Before updating the innovation score of this region,
	   		// its penalty (i.e. the negative innovation_score) must be removed
	   		#ifdef DEBUG_LEVEL_DEBUG
			    string message = "Region ("+to_string_region_concise(era,i)+" had innovation score "
			    	+to_string(region->innovation_score)+" in previous era, but now a pareto config "
			    	+ "was found in it. Its innovation score is put to 0";
			    write_to_log(myrank,logfile,message);
	   		#endif
   			region->innovation_score=0;	
	   	}
		
   		
   		
   		// Notice that if a configuration of the new_pareto_set is already contained
   		// in the old_pareto_set, its innovation score is 0
   		double config_innovation_score = get_innovation_score(s,old_pareto_set);
   		region->innovation_score += config_innovation_score;
   		total_innovation_score += config_innovation_score;
    }	

	#ifdef DEBUG_LEVEL_DEBUG
		for (int i=0; i<regions.size(); i++){
			string message = "Innovation score of "+to_string_region_concise(era,i)
				+": "+to_string(regions[i]->innovation_score) ;
		    write_to_log(myrank,logfile,message);
   		}
	#endif
    
    return total_innovation_score;
}

void sort_by_innovation(vector<Region> * regions)
{
    // actually modifies the main set of regions...
}

// If a region has high innovation, split it. If it has low innovation, do nothing.
// If it has no_innovation try to merge. All the regions obtained as above are 
// returned in a vector.
vector<Region*> build_new_regions( 
	//vector<Region*> regions, 
	double old_average_innovation_score)
{
	vector<Region*> new_regions, regions_to_merge;
	for (int i=0; i<regions.size(); i++ )
	{
		Region* region = regions[i];
		
		if (region->innovation_score > (ALPHA * old_average_innovation_score) )
		{
			//high innovation region
			vector<Region*> out_regions = split_region(i);
			for (int j=0; j<out_regions.size(); j++)
				new_regions.push_back(out_regions[j]);
		}
		else if (region->innovation_score == 0)
		{
			//no innovation region
			regions_to_merge.push_back(region);
		}
		else
		{
			//low_innovation_region: leave it unchanged
			new_regions.push_back(region);
		}
	}

	// merging_mask[i] == true if the region has been merged. It is false otherwise
	bool merging_mask[regions.size()];

	// Now, try to merge the regions_to_merge
	for (int i=0; i<regions_to_merge.size(); i++)
	{
		for (int j=i+1; j<regions_to_merge.size() && !merging_mask[i]; j++)
		{
			if (!merging_mask[j])
			{
				Region* merged_region = 
					merge_regions(regions_to_merge[i], regions_to_merge[j] );

				if (merged_region != NULL)
				{
					new_regions.push_back(merged_region);
					merging_mask[i]=true;
					merging_mask[j]=true;
				}
			} 	
			//else do nothing because the j-th region has already been merged 
			//with another region
		}
		
		if (!merging_mask[i])
		{
			// regions_to_merge[i] can not be merged with any other region
			regions_to_merge[i]->innovation_score += -1;
			new_regions.push_back(regions_to_merge[i]);
		}
	}
	//TODO: find a more efficient way without swapping. It would be best if we are able to 
	//use only the vector regions, without using another vector and then swap .
	regions.swap(new_regions);
	return regions;
}


Configuration fix_random_configuration(Region* region, Explorer* expl)
{
			printf("Sono dentro fix_random_configuration\n");
			#ifdef SEVERE_DEBUG
				well_formed(*region,expl);
			#endif
				
			printf("Sono dopo il well-formed\n");

			
			Configuration conf;
			for (int j=0; j<N_PARAMS; j++)
			{
				Parameter par = expl->getParameter( (EParameterType)j );
				Edges edges = (*region).edges[j];
				
				par.set_random( edges.a, edges.b );
				conf.fix_parameter((EParameterType)j, par.get_val() );
			}
			conf.pointer = region;
			
			#ifdef DEBUG_LEVEL_DEBUG
			    string message = "Random configuration " + conf.configuration_to_string();
			    write_to_log(myrank,logfile,message);
			#endif
			
			return conf;
}

void Explorer::start_PARAMSPACE(double alpha, int k, int max_eras)
{
		std:vector<Region*> provaregioni;
		Region* provareg1,provareg2;
		provaregioni.push_back(provareg1);
	   	printf("!!!line %d: Primo pushback fatto\n",__LINE__); std::cin.ignore();



    // mpi parallel implementation currently unsupported on PARAMSPACE
    myrank = get_mpi_rank();
    assert(myrank == 0);
    

    // setup some not-algorithm-related stuff before starting
    current_algo="PARAMSPACE";
    logfile = get_base_dir()+string(EE_LOG_PATH);
    cout << "logfile: " << logfile << endl;

    Exploration_stats stats;
    stats.space_size = get_space_size();
    stats.start_time = time(NULL);
    reset_sim_counter();

	//To write stats
    string file_name = Options.benchmark+"_"+current_algo+"_"+current_space;

    string header = "["+Options.benchmark+"_"+current_algo+"] ";
    string message = header+"Building random space for " + to_string(k) + " simulations...";
    write_to_log(myrank,logfile,message);
    
	message = header+"Notation remark: the j-th region of era i will be indicated as" 
    		+ " (i,j)";
    write_to_log(myrank,logfile,message);
    
    
    //DA QUI COMINCIA
	// The number of feasible configurations
	int valid = 0 ;
    
    //The sum of the innovation_score of all regions
    double era_innovation_score=-1;
    // A negative number is used beacause we want all the regions in era 0 to be
    // "high innovation" regions, although in era 0 all regions have 
    // innovation score=0
    
//    vector<Region*> regions, no_innovation_regions;
    vector<Simulation> new_pareto_set, old_pareto_set;
    double old_average_innovation_score, new_average_innovation_score;

	Region* root_region = new Region();
//    Region* root_region = (Region*)malloc(sizeof(Region));
    transform_to_root_region(root_region, this);
    regions.push_back(root_region);
    
    #ifdef SEVERE_DEBUG
    printf("alg_paramspace.c %d: checking root region\n",__LINE__);
    for (int u=0; u<N_PARAMS; u++){
    	Edges edges = root_region->edges[u];
    	if (edges.a == edges.b){
    		printf("alg_paramspace.cpp %d: ERROR\n",__LINE__);
    		exit(65431);
    	}
    }	
    #endif
    
    // create a space of k randomly chosen configurations
    vector<Configuration> configs_to_simulate;

    for (int era=0; era<max_eras; era++)
    {
    	#ifdef DEBUG_LEVEL_DEBUG
    		string message = "Era " + to_string(era) + ". Regions are ";
		    write_to_log(myrank,logfile,message);
		    for (int j=0; j<regions.size(); j++)
			    write_to_log(myrank,logfile,
			    	to_string_region_concise(era,j)+" = "+to_string( *(regions[j]) ) );
			
			#ifdef SEVERE_DEBUG
			//Check if all regions are well formed
			for (int j=0; j<regions.size(); j++)
			    well_formed( *(regions[j]), this);
			#endif
    	#endif
    	
		// Use the budget of k simulations
		for(int i=0;i<k;i++)
		{
			Region* selected_region = select_region(era,regions);
			
			Configuration temp_conf = fix_random_configuration(selected_region,this);
			
			
			if (temp_conf.is_feasible())
			{
				valid++;
				configs_to_simulate.push_back(temp_conf);
			}
			
		}


		message = header+ "In era "+to_string(era)+", among "+to_string(k)+
			" valid configurations, "+ to_string(k) + " are valid";
		write_to_log(myrank,logfile,message);

		vector<Simulation> current_sims = simulate_space(configs_to_simulate);
		
		//TODO: Dobbiamo distruggere l'old_pareto_set prima?
		
		old_pareto_set = new_pareto_set;
		new_pareto_set = get_pareto(current_sims);
		
		double old_era_innovation_score = era_innovation_score;
		era_innovation_score = update_region_innovation_scores(
				era,regions, old_pareto_set, new_pareto_set);
				
		message = header+ "Era "+to_string(era)+": total innovation score= "+
				to_string(era_innovation_score);
		write_to_log(myrank,logfile,message);
				
		/*regions = */build_new_regions(/*regions, */old_era_innovation_score / regions.size());
		
		char era_string[30];
		sprintf(era_string, "_%d", era);

		save_simulations(current_sims,Options.benchmark+"_PARAMSPACE"+
			current_space+"_gen"+string(era_string)+".exp");
		save_simulations(new_pareto_set,Options.benchmark+"_PARAMSPACE"+
			current_space+"_gen"+string(era_string)+".pareto.exp");

	}


    // save statistics
    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats, file_name+".stat");
}




/********************************************************************************/
/***************DEBUG FUNCTIONS**************************************************/
/********************************************************************************/
//TODO: spostare in un file a parte

const string to_string(Region r)
{
	string s="[";
	for (int i=0; i<N_PARAMS; i++)
	{
		s = s+ to_string(r.edges[i].a)+ ":"+ to_string(r.edges[i].b)+";  ";
	}
	s = s+"]";
	return s;
}

const string to_string_region_concise(int era, int region_index)
{
	string s="("+to_string(era)+","+to_string(region_index)+")";
	return s;
}

void well_formed(Region r, Explorer* expl)
{	
	for (int j=0; j<N_PARAMS; j++)
	{
		Parameter par = expl->getParameter( (EParameterType)j );
		Edges edges = (r.edges)[j];

		if (edges.b >= par.get_values().size())
		{
			printf("alg_paramspace.cpp %d: Error checking region\n",__LINE__ );
			cout << to_string(r) << "\n";
			printf("parameter %d has %d values while edges.b=%d\n",
				j, par.get_values().size(), edges.b );
			exit(-761);
		}
	}
}








/*************************************************************************/

