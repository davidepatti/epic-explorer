
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
EraDescriptor current_era_descriptor;
RegionHandler region_handler;
FILE* region_logfile;
FILE* era_logfile;
int myrank;
string header; //Used to write logfiles
//vector<Region*> regions;

Region::Region(){
			id=0; innovation_score=0; valid=false;
			for (int i=0; i<N_PARAMS; i++){
				edges[i].a=0; edges[i].b=0;
			}
}

void Region::set_edges(const Edges* src_edges){
	memcpy( (void*)edges, (const void*)src_edges, sizeof(Edges)*N_PARAMS );
}

const string Region::tostring()
{
	string s= to_string(id);
	for (int i=0; i<N_PARAMS; i++)
		s = s+" "+ to_string(edges[i].a)+ ":"+ to_string( edges[i].b);

	return s;
}

EraDescriptor::EraDescriptor(){
	era_id = -1;
}

RegionHandler::RegionHandler(){
	last_region_id = 0;
}

void RegionHandler::add_region_to_next_era(Region *r)
{
	if(r->id == 0){
		// This region has never seen before and has no id assigned. Assign a new id
		// and print it
		r->id = ++last_region_id;
		fprintf(region_logfile, "%s\n", r->tostring().c_str() );
	}

	next_era_regions.push_back(r);
}

void RegionHandler::new_era_initialization()
{
	//TODO: find a more efficient way without swapping.
	current_era_regions.swap(next_era_regions);
	
	next_era_regions.clear();

	int ndel = regions_to_be_deleted.size();

	for(int i=0; i<ndel; i++)
	{
	    write_to_log(myrank,logfile,string("deleting region"));
	    delete regions_to_be_deleted[0];
	    regions_to_be_deleted.erase(regions_to_be_deleted.begin());
	}
	
}

void RegionHandler::new_run_initialization()
{
	last_region_id = 0;
	next_era_regions.clear();
	current_era_regions.clear();
	regions_to_be_deleted.clear();
}

const vector<Region*> RegionHandler::get_current_era_regions(){
	return (const vector<Region*> ) current_era_regions;
}

const Region* RegionHandler::get_current_era_region(unsigned region_index){
	return current_era_regions[region_index];
}

void RegionHandler::annotate_for_deletion(const Region* r)
{
		regions_to_be_deleted.push_back( (Region*)r);
}

#ifdef SEVERE_DEBUG
void RegionHandler::check_regions(Explorer* expl)
{
		//Check if all regions are well formed
		for (int j=0; j<current_era_regions.size(); j++)
			well_formed( *(current_era_regions[j]), expl);
}
#endif

void EraDescriptor::reinitialize(int era_id_)
{
	era_id = era_id_;
	
	high_innovation_region_descriptors.clear();
	no_innovation_region_descriptors.clear();
	low_innovation_region_descriptors.clear();
}

void EraDescriptor::add_region_descriptor(
	unsigned region_id, double innovation_score, region_category category)
{
	#ifdef SEVERE_DEBUG
	if (era_id < 0){
		printf("\nalg_paramspace.cpp %d: FATAL ERROR: maybe you forgot to call reinitialize(..) and the era id is not initialized\n", __LINE__);
		exit(EXIT_FAILURE);
	}
	#endif
	
	switch(category)
	{
		case HIGH_INNOVATION:	
					high_innovation_region_descriptors[region_id] = innovation_score;
					break;
							
		case NO_INNOVATION:		
					no_innovation_region_descriptors[region_id] = innovation_score;
					break;
							
		case LOW_INNOVATION:		
					low_innovation_region_descriptors[region_id] = innovation_score;
					break;

		default:			
					printf("\nalg_paramspace.cpp %d: FATAL ERROR: innovation category \"%d\"is invalid\n", __LINE__, category);
					exit(EXIT_FAILURE);
	}
}

const string EraDescriptor::tostring()
{
	string return_value = to_string(era_id)+": ";

	map<unsigned,double>::iterator it;
	for(	it=high_innovation_region_descriptors.begin(); 
			it!=high_innovation_region_descriptors.end(); ++it
	)
		return_value += to_string(it->first ) + "," + to_string(it->second ) + " ";
		
	return_value += "| ";
	for(	it=no_innovation_region_descriptors.begin(); 
			it!=no_innovation_region_descriptors.end(); ++it
	)
		return_value += to_string(it->first ) + "," + to_string(it->second ) + " ";

	return_value += "| ";
	for(	it=low_innovation_region_descriptors.begin(); 
			it!=low_innovation_region_descriptors.end(); ++it
	)
		return_value += to_string(it->first ) + "," + to_string(it->second ) + " ";
	
	return return_value;
}



// TODO: sistemare correttamente la questione del seed
unsigned long paramspace_seed = 1;

bool edges_equality(const Edges& e1, const Edges& e2)
{
	return (e1.a == e2.a && e1.b == e2.b );
}

bool edges_contiguity(const Edges& e1,const Edges& e2)
{
	if (e1.b <= e2.a){
	    return true;
	    // first_edges = e1; second_edges = e2;
	} 
	else if (e2.b <= e1.a){
	    return true;
	    // first_edges = e2; second_edges = e1;
	}

	return false;

}

// Return edges with edges.a=-1 and edges.b=-1 if it is not possible to merge.
// If the merge is possible, return the merged edges.
Edges merge_intervals(Edges e1, Edges e2)
{
	Edges first_edges, second_edges;
	if (e1.b <= e2.a){
		first_edges = e1; second_edges = e2;
	} 
	else if (e2.b <= e1.a){
		first_edges = e2; second_edges = e1;
	}
	else{
	    // DAV
	//	string message = "alg_paramspace.cpp:"+ to_string(__LINE__) +
	//		": ERROR: e1.a=" + to_string(e1.a)+ "; e1.b="+ to_string(e1.b)+
	//		"; e2.a=" + to_string(e2.a)+ "; e2.b="+ to_string(e2.b)+
	//		". This edges are not valid"+ "AGGIUNGI IL METODO verify_edges PER "+
	//		"DA INVOCARE OGNI VOLTA CHE UNA REGIONE VIENE SPLITTATA, PER VERIFICARE "+
	//		"SUBITO SE GLI EDGES CHE VENGONO PRODOTTI SONO VALIDI";
	//	write_to_log(myrank,logfile,message);
	//	exit(-8712);
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

	// DAV
	return return_edges;
}

// Return a parameter along which to split the region. If the region has interval size
// 1 along all the parameters, return 0-th parameter. The caller function can check if
// it is the case by verifying that the region has interval size == 1 along the 
// returned parameter
EParameterType get_splitting_parameter(const Region* region)
{
	// Find the parameters along which we can split the region
	vector<int> splitting_parameters;
	for (int i = 0; i < N_PARAMS; i++)
	{
		EParameterType par = (EParameterType) i;
		const Edges* interval = &(region->edges[par]);
		int interval_size = (interval->b) - (interval->a) +1;
		if(interval_size > 1)
			splitting_parameters.push_back(i);
	}
	
	if(splitting_parameters.size() == 0)
		return (EParameterType) 0;
	else{
		int extracted = round( (double)rand() * (splitting_parameters.size()-1) / 
						RAND_MAX );	
		return (EParameterType)splitting_parameters[extracted];
	}
}

//TODO: si potrebbe evitare di passare expl se getParameterRanges fosse statico
// At the end of this method, r will be a region including all the parameter space
void transform_to_root_region(Region* r, Explorer* expl)
{
	vector<pair<int,int> > ranges = expl->getParameterRanges();
	#ifdef SEVERE_DEBUG
	if (ranges.size() != N_PARAMS)
	{
		string message = "alg_paramspace.cpp:"+ to_string(__LINE__) +": ERROR: N_PARAMS=" + 
			to_string(N_PARAMS) + " whereas getParameterRanges()"
			+" has returned "+ to_string(ranges.size() ) + "elements";
		write_to_log(myrank,logfile,message);
		exit(-716);
	}
	#endif
	
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
Region* select_region(int era,const vector<Region*> regions)
{
	double roulette_wheel_length = 0;
	for (int i=0; i<regions.size(); i++)
	{
		Region* regionn = regions[i]; // da togliere
		double incremento = roulette_wheel_arc_length( regions[i] ); // da togliere
		//roulette_wheel_length += roulette_wheel_arc_length( regions[i] );
		roulette_wheel_length += incremento;
	}

	// TODO: meglio usare
	// http://www.johndcook.com/cpp_TR1_random.html
	// solo che mi genera numeri enormi e non capisco perche'
	double random_number =  (double)(rand()*roulette_wheel_length)/(RAND_MAX);

	// double r =  (double)rand()/(RAND_MAX);
    // double random_point = (double)(r*total_innovation_score);
    double partial_roulette = 0;
    
    int i; for (i=0; partial_roulette < random_number; i++)
    	partial_roulette += roulette_wheel_arc_length( regions[i] );
    	
    i--;
    
    string message = "Era "+to_string(era)+":The next simulation will be chosen \
    	inside region " + to_string( regions[i]->id );
    write_to_log(myrank,logfile,message);
    
    return regions[i];
}


void split_region(unsigned region_index)
{
    const Region* region = region_handler.get_current_era_region(region_index);
    
    Region *r1,*r2;
    EParameterType splitting_parameter = get_splitting_parameter(region);
    
    
    const Edges* interval = &(region->edges[splitting_parameter]);
	int interval_size = (interval->b) - (interval->a) +1;
    
    if (interval_size == 1){
    	#ifdef SEVERE_DEBUG
    	if(splitting_parameter != (EParameterType)0 ){
    		printf("\nalg_paramspace.cpp %d: FATAL ERROR: The only case when \
				get_splitting_parameter(..) can return a parameter along which the \
				region has interval size 1 is when the region has every interval of \
				 size 1. In this case, get_splitting_parameter(..) must return \
				 parameter 0. Here, the returned parameter is %d\n", 
				 __LINE__, (int)splitting_parameter);
    		 exit(654319);
    	}
    	#endif
    
    	// The selected interval cannot be split. Therefore, the region cannot 
    	// be split.
    	region_handler.add_region_to_next_era( (Region*) region);

   		string message = header+ "Region "+to_string(region->id)+
	   		" cannot be split because it is composed of one configuration only";
		write_to_log(myrank,logfile,message);

    }
    else if (interval_size > 1)
    {
    	// interval1 will be assigned to region r1. interval2 will be assigned to r2.
		Edges interval1, interval2; 
    	interval1.a = interval->a; 
    	interval1.b= interval->a + (int)(interval_size/2)-1;
    	
    	interval2.b = interval->b;
    	interval2.a = interval1.b + 1;
    	
    	r1 = new Region(); 
    	r1->set_edges( region->edges );

    	r2 = new Region();
    	r2->set_edges( region->edges );
    	
    	r1->edges[splitting_parameter] = interval1;
    	r2->edges[splitting_parameter] = interval2;
    	
    	region_handler.annotate_for_deletion(region);
	region_handler.add_region_to_next_era(r1);
	region_handler.add_region_to_next_era(r2);
	    
	string message = header+ "Region "+to_string(region->id)+
		" has been split in region "+to_string(r1->id)+" and region "+
		to_string(r2->id)+". The splitting parameter is "+
		to_string( (int)splitting_parameter );
	write_to_log(myrank,logfile,message);

	if (!edges_contiguity(interval1,interval2))
	{
	    string message = header + " FATAL: non-contiguos interval after splitting ";
	    write_to_log(myrank,logfile,message);
	}

    }
    else{
    	// This is a debug check
    	string message = "alg_paramspace.cpp:"+ to_string(__LINE__) +
			": ERROR: = splitting_parameter="+ to_string(splitting_parameter)+
			"; interval size="+to_string(interval_size);
		write_to_log(myrank,logfile,message);
		exit(-12721);
    }
}


// If r1 and r2 are contiguous, return the merged region and deallocate r1 and r2.
// Else, return NULL.
Region* merge_regions(const Region* r1, const Region* r2)
{
    // TBD: what returns when merging is not possible ?
    Region* r=NULL;
    Edges merged_interval;
    int merging_parameter=-1;
    int num_equal_edges = 0;

    bool stop_trying = false;
    
    for (int j=0; (j<N_PARAMS) && (!stop_trying); j++)
    {
    	Edges e1 = r1->edges[j];
    	Edges e2 = r2->edges[j];
    	
    	if ( edges_equality(e1, e2) )
    		num_equal_edges += 1;
    	
    	else
	{
    		Edges new_interval = merge_intervals(e1, e2);
    		
    		//#ifdef SEVERE_DEBUG
    		if (	(new_interval.a==-1 && new_interval.b!=-1 ) ||
			(new_interval.a!=-1 && new_interval.b==-1 ))
		{
		    printf("\nalg_paramspace.cpp %d: FATAL ERROR: Inconsistent return  value from merge_intervals(..) \n",__LINE__);
		    exit(EXIT_FAILURE);
		}
    		//#endif
    		
		// cannot merge intervals, so regions cannot be merged
    		if (new_interval.a==-1 && new_interval.b==-1)
    		{
		    stop_trying = true;
    		}
		else // mergeable parameter found
		{
		    // if more than one parameter is mergeable, regions are not contiguos and cannot
		    // be merged
		    if (merging_parameter >= 0) {	
			string message = "WARNING, cannot merge non contiguos regions " + to_string(r1->id)+"  "+to_string(r2->id);
			write_to_log(myrank,logfile,message);
			cout << message;
			stop_trying = true;
		    }
		    else
		    {
			merging_parameter = j;
			merged_interval = new_interval;
		    }
		}
    	}
    } // end of for loop
    
    if (num_equal_edges == N_PARAMS-1   &&    merging_parameter >= 0)
    {
    	Region* merged_region = new Region();
    	
    	merged_region->set_edges( r1->edges );
    	merged_region->edges[merging_parameter] = merged_interval;
    	
    	// the new region is given the max between the innovation scores of r1 and r2
    	merged_region->innovation_score = 
    		r1->innovation_score >= r2->innovation_score ?
    		r1->innovation_score : r2->innovation_score;
    		
    	return merged_region;
    }
    else
    { 
    
    	#ifdef SEVERE_DEBUG
    	if (num_equal_edges == N_PARAMS){
    		printf("\nalg_paramspace.cpp %d: FATAL ERROR: Two identical regions have been found\n",__LINE__);
    		exit(EXIT_FAILURE);
    	}
    	#endif
    }

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

double RegionHandler::update_innovation_scores_of_current_era_regions(
	int era,
	vector<Simulation> old_pareto_set, vector<Simulation> new_pareto_set)
{
	vector<Region*> regions = current_era_regions;
	double total_innovation_score = 0;
	
	// Clear the innovaton score of all regions
	for (int i=0; i<regions.size(); i++)
	{
		if (regions[i]->innovation_score >= 0)
			regions[i]->innovation_score=0;
		//else
		//	do nothing because this region has negative innovation_score
	}

	// We will take, one by one, all the configurations in the new pareto set.
	// For each of them, we will calculate the innovation score, we will retrieve
	// the region which contains that configuration and update the innovation score
	// of that region accordingly.
    for (int i=0; i<new_pareto_set.size(); i++)
    {
    	Simulation s = new_pareto_set[i];
   		Region* region = (Region*) (s.config.pointer);

   		// Notice that if a configuration of the new_pareto_set is already contained
   		// in the old_pareto_set, its innovation score is 0.
   		// This is the same as considering only the configurations added by the current era
   		double config_innovation_score = get_innovation_score(s,old_pareto_set);
   		
   		if (region->innovation_score<0 && config_innovation_score>0){
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
		
   		
   		
   		region->innovation_score += config_innovation_score;
   		total_innovation_score += config_innovation_score;
    }	

	#ifdef DEBUG_LEVEL_DEBUG
		for (int i=0; i<regions.size(); i++){
			string message = "Era "+to_string(era)+": Innovation score of region " +
					to_string( regions[i]->id ) +
					": "+to_string(regions[i]->innovation_score) ;
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
void build_next_era_regions( 
	//vector<Region*> regions, 
	double old_average_innovation_score, int era)
{
	vector<Region*> regions_to_merge;
	for (int i=0; i<region_handler.get_current_era_regions().size(); i++ )
	{
		const Region* region = region_handler.get_current_era_region(i);
		
		if (region->innovation_score > (ALPHA * old_average_innovation_score) )
		{
			//high innovation region
			current_era_descriptor.add_region_descriptor(
						region->id, region->innovation_score, HIGH_INNOVATION);
			split_region(i);
		}
		else if (region->innovation_score == 0)
		{
			//no innovation region
			current_era_descriptor.add_region_descriptor(
						region->id, region->innovation_score, NO_INNOVATION);
			regions_to_merge.push_back( (Region*) region);
			//We will try to merge this region with other ones, later
			
		}
		else
		{
			//low_innovation_region: leave it unchanged
			current_era_descriptor.add_region_descriptor(
						region->id,region->innovation_score, LOW_INNOVATION);
			region_handler.add_region_to_next_era( (Region*)region );
		}
	}
	
	

	// merging_mask[i] == true if the region has been merged. It is false otherwise
	bool merging_mask[ region_handler.get_current_era_regions().size() ];

	// Now, try to merge the regions_to_merge
	for (int i=0; i<regions_to_merge.size(); i++)
	{
		for (int j=i+1; j<regions_to_merge.size() && !merging_mask[i]; j++)
		{
			if (!merging_mask[j])
			{
				//The j-th region han not been merged yet
				Region* merged_region = 
					merge_regions(regions_to_merge[i], regions_to_merge[j] );

				if (merged_region != NULL)
				{
					region_handler.annotate_for_deletion( regions_to_merge[i] );
					region_handler.annotate_for_deletion( regions_to_merge[j] );
					region_handler.add_region_to_next_era(merged_region);
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
			region_handler.add_region_to_next_era(regions_to_merge[i]);
		}
	}
}

void update_era_logfile()
{
	fprintf(era_logfile, "%s\n", current_era_descriptor.tostring().c_str() );
}

Configuration fix_random_configuration(Region* region, Explorer* expl)
{
			#ifdef SEVERE_DEBUG
				well_formed(*region,expl);
			#endif
				
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
/*		std:vector<Region*> provaregioni;
		Region* provareg1,provareg2;
		provaregioni.push_back(provareg1);
	   	printf("!!!line %d: Primo pushback fatto\n",__LINE__); std::cin.ignore();
*/


    // mpi parallel implementation currently unsupported on PARAMSPACE
    myrank = get_mpi_rank();
    assert(myrank == 0);
    

    // setup some not-algorithm-related stuff before starting
    current_algo="PARAMSPACE";
    logfile = get_base_dir()+string(EE_LOG_PATH);
    string region_logfile_name = logfile + "-region";
    region_logfile = fopen(region_logfile_name.c_str(), "w+");
    string era_logfile_name = logfile + "-era";
    era_logfile = fopen(era_logfile_name.c_str(), "w+");
    cout << "logfile: " << logfile << endl;

    Exploration_stats stats;
    stats.space_size = get_space_size();
    stats.start_time = time(NULL);
    reset_sim_counter();

	//To write stats
    string file_name = Options.benchmark+"_"+current_algo+"_"+current_space;

    header = "["+Options.benchmark+"_"+current_algo+"] ";
    
    string message = header + "region_logfile: " + region_logfile_name;
	write_to_log(myrank,logfile,message);
    message = header + "era_logfile: " + era_logfile_name;
	write_to_log(myrank,logfile,message);
		
    
    message = header+"Building random space for " + to_string(k) + " simulations...";
    write_to_log(myrank,logfile,message);
    
	
    
    //DA QUI COMINCIA
	// The number of feasible configurations
	int valid = 0 ;
    
    //The sum of the innovation_score of all regions
    double era_innovation_score=-1;
    // A negative number is used beacause we want all the regions in era 0 to be
    // "high innovation" regions, although in era 0 all regions have 
    // innovation score=0
    
    region_handler.new_run_initialization();
    
//    vector<Region*> regions, no_innovation_regions;
    vector<Simulation> new_pareto_set, old_pareto_set;
    double old_average_innovation_score, new_average_innovation_score;

	Region* root_region = new Region();
    transform_to_root_region(root_region, this);
    region_handler.add_region_to_next_era(root_region);
    
    #ifdef SEVERE_DEBUG
    for (int u=0; u<N_PARAMS; u++){
    	Edges edges = root_region->edges[u];
    	if (edges.a == edges.b){
    		printf("alg_paramspace.cpp %d: ERROR: it's very strange that the root\
    		  region has an edge composed by only one value. If you think that this\
    		  is not an error, remove this check and recoompile\n",__LINE__);
    		exit(65431);
    	}
    }	
    #endif
    
    //In every era, new configurations to simulate will be added. Notice that the configurations
    //are simulated every era invoking simulate_space(configs_to_simulate) but if a configuration
    //was already simulated in a previous era, the simulation will not be run again.
    vector<Configuration> configs_to_simulate;

    for (int era=0; era<max_eras; era++)
    {
	    write_to_log(myrank,logfile,"\n");
		message = header+ "Era "+to_string(era)+" started";
		write_to_log(myrank,logfile,message);
    
		current_era_descriptor.reinitialize(era);
		region_handler.new_era_initialization();
		
		#ifdef SEVERE_DEBUG
		region_handler.check_regions(this);
		#endif
		
    	// #### FIRST PHASE OF THE ERA: choice of the configuration to simulate
    	
		// Use the budget of k simulations
		for(int i=0;i<k;i++)
		{
			Region* selected_region = 
				select_region(era,region_handler.get_current_era_regions() );
			Configuration temp_conf = fix_random_configuration(selected_region,this);
						
			if (temp_conf.is_feasible())
			{
				valid++;
				configs_to_simulate.push_back(temp_conf);
			}
		}

		message = header+ "In era "+to_string(era)+", "+to_string(k)+
			" configurations were added. The valid configurations (starting from the era 0) are "+ to_string(valid);
		write_to_log(myrank,logfile,message);

    	// #### SECOND PHASE OF THE ERA: simulations of choses configurations
    	
		vector<Simulation> current_sims = simulate_space(configs_to_simulate);
		
		
    	// #### THIRD PHASE OF THE ERA: pareto set and innovation_scores calculation
		
		//TODO: Dobbiamo distruggere l'old_pareto_set prima?
		
		old_pareto_set = new_pareto_set;
		new_pareto_set = get_pareto(current_sims);
		
		double old_era_innovation_score = era_innovation_score;
		era_innovation_score =
			region_handler.update_innovation_scores_of_current_era_regions(
				era,old_pareto_set, new_pareto_set
			);
				
		message = header+ "Era "+to_string(era)+": total innovation score= "+
				to_string(era_innovation_score);
		write_to_log(myrank,logfile,message);


    	// #### FOURTH PHASE OF THE ERA: calculation of the regions of the next era
		build_next_era_regions(
			old_era_innovation_score / 
			region_handler.get_current_era_regions().size(),
			era);
		
		// Even if the regions for the next era have been built, the current_era_descriptor
		// has been updated by build_new_regions(..) with the information about the regions 
		// of the current era. Therefore, we can update the region log file with the region
		// of the current era with the following function that is based on current_era_descriptor
		update_era_logfile();
		

		save_simulations(current_sims,file_name+"_"+to_string(era)+".exp");
		save_simulations(new_pareto_set,file_name+"_"+to_string(era)+".pareto.exp");

		stats.end_time = time(NULL);
		stats.n_sim = get_sim_counter();
		save_stats(stats, file_name+"_"+to_string(era)+".stat");
	}

	fclose(region_logfile);

    // save statistics
    stats.end_time = time(NULL);
    stats.n_sim = get_sim_counter();
    save_stats(stats, file_name+".stat");
}




/********************************************************************************/
/***************DEBUG FUNCTIONS**************************************************/
/********************************************************************************/
//TODO: spostare in un file a parte

/*
const string to_string(const Region r, int era, string category)
{
	string s = to_string_region_concise(era, r.id) + " " + category;
	for (int i=0; i<N_PARAMS; i++)
	{
		s = s+" "+ to_string(r.edges[i].a)+ ":"+ to_string(r.edges[i].b);
	}
	return s;
}
*/

const string to_string_region_concise(int era, int region_index)
{
	string s="("+to_string(era)+","+to_string(region_index)+")";
	return s;
}

#ifdef SEVERE_DEBUG
void well_formed(Region r, Explorer* expl)
{	
	for (int j=0; j<N_PARAMS; j++)
	{
		Parameter par = expl->getParameter( (EParameterType)j );
		Edges edges = (r.edges)[j];

		if ( r.id == 0)
		{
			printf("alg_paramspace.cpp %d: FATAL ERROR: region id not assigned\n",__LINE__ );
			exit(-762);
		}

		if (edges.b >= par.get_values().size())
		{
			printf("alg_paramspace.cpp %d: Error checking region\n",__LINE__ );
			printf("parameter %d has %d values while edges.b=%d\n",
				j, par.get_values().size(), edges.b );
			exit(-761);
		}
	}
}
#endif







/*************************************************************************/

