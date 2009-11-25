// common routines and functions
//
#include "common.h"
#include <cstdlib>

#ifdef EPIC_MPI
#include "mpi.h"
#endif
bool Configuration::is_feasible()
{
  return ( (L1D_size + L1I_size <= L2U_size) &&
	   (L1D_size >= L1D_block * L1D_assoc) &&
	   (L1I_size >= L1I_block * L1I_assoc) &&
	   (L2U_size >= L2U_block * L2U_assoc) &&
	   (L1I_block <= L2U_block) &&
	   (L1D_block <= L2U_block) );
}

void Configuration::invalidate()
{
    num_clusters = -1;
    integer_units = -1;
    float_units = -1;
    branch_units = -1;
    memory_units = -1;
    gpr_static_size = -1;
    fpr_static_size = -1;
    pr_static_size = -1;
    cr_static_size = -1;
    btr_static_size = -1;

    L1D_size = -1;
    L1D_block = -1;
    L1D_assoc = -1;
    L1I_size = -1;
    L1I_block = -1;
    L1I_assoc = -1;
    L2U_size = -1;
    L2U_block = -1;
    L2U_assoc = -1;

    tcc_region = -1;	//db
    max_unroll_allowed = -1;	//db
    regroup_only = -1;	//db
    do_classic_opti = -1;	//db
    do_prepass_scalar_scheduling = -1;	//db
    do_postpass_scalar_scheduling = -1;	//db
    do_modulo_scheduling = -1;	//db
    memvr_profiled = -1;		//db	
}

bool Configuration::check_difference(const Configuration& conf, Space_mask mask)
{
    if (mask.integer_units && (integer_units!= conf.integer_units)) return true;
    if (mask.float_units && (float_units!= conf.float_units)) return true;
    if (mask.branch_units && (branch_units!=conf.branch_units)) return true;
    if (mask.memory_units && (memory_units!=conf.memory_units)) return true;
    if (mask.gpr_static_size && (gpr_static_size!=conf.gpr_static_size)) return true;
    if (mask.fpr_static_size && (fpr_static_size!=conf.fpr_static_size)) return true;
    if (mask.pr_static_size && (pr_static_size!= conf.pr_static_size)) return true;
    if (mask.cr_static_size && (cr_static_size!= conf.cr_static_size)) return true;
    if (mask.btr_static_size && (btr_static_size!=conf.btr_static_size)) return true;
    if (mask.num_clusters && (num_clusters!=conf.num_clusters)) return true;

    if (mask.L1D_size && (L1D_size!=conf.L1D_size)) return true;
    if (mask.L1D_block && (L1D_block!=conf.L1D_block)) return true;
    if (mask.L1D_assoc && (L1D_assoc!=conf.L1D_assoc)) return true;

    if (mask.L1I_size && (L1I_size!=conf.L1I_size)) return true;
    if (mask.L1I_block && (L1I_block!=conf.L1I_block)) return true;
    if (mask.L1I_assoc && (L1I_assoc!=conf.L1I_assoc)) return true;

    if (mask.L2U_size && (L2U_size!=conf.L2U_size)) return true;
    if (mask.L2U_block && (L2U_block!=conf.L2U_block)) return true;
    if (mask.L2U_assoc && (L2U_assoc!=conf.L2U_assoc)) return true;

    
    if (mask.tcc_region && (tcc_region!=conf.tcc_region)) return true;  //db
    if (mask.max_unroll_allowed && (max_unroll_allowed!=conf.max_unroll_allowed)) return true;  //db
    if (mask.regroup_only && (regroup_only|=conf.regroup_only)) return true;	//db
    if (mask.do_classic_opti && (do_classic_opti!=conf.do_classic_opti)) return true;	//db
    if (mask.do_prepass_scalar_scheduling && (do_prepass_scalar_scheduling!=conf.do_prepass_scalar_scheduling)) return true;	//db
    if (mask.do_postpass_scalar_scheduling && (do_postpass_scalar_scheduling!=conf.do_postpass_scalar_scheduling)) return true;	//db
    if (mask.do_modulo_scheduling && (do_modulo_scheduling!=conf.do_modulo_scheduling)) return true;	//db
    if (mask.memvr_profiled && (memvr_profiled!=conf.memvr_profiled)) return true; 	//db

    return false;
}

string Configuration::get_header() const
{
    char s[100];
    sprintf(s,"%u / %u %u %u %u / %u %u %u %u %u / %u %u %u / %u %u %u / %u %u %u / %u %u %u %u %u %u %u %u",
	    num_clusters, integer_units, float_units, branch_units,memory_units, 
	    gpr_static_size, fpr_static_size, pr_static_size, cr_static_size, btr_static_size, 
	    L1D_size, L1D_block, L1D_assoc, L1I_size, L1I_block, L1I_assoc, L2U_size, L2U_block, L2U_assoc, 
	    tcc_region, max_unroll_allowed, regroup_only, do_classic_opti, do_prepass_scalar_scheduling, do_postpass_scalar_scheduling, do_modulo_scheduling, memvr_profiled); //db

    return string(s);

}

string Configuration::get_executable_dir() const
{
    char s[100];
     sprintf(s,"%u_%u%u%u%u_%u%u%u%u%u_%u%u%u%u%u%u%u%u",
	    num_clusters, integer_units, float_units, branch_units, memory_units, 
	    gpr_static_size, fpr_static_size, pr_static_size, cr_static_size, btr_static_size,
	    tcc_region, max_unroll_allowed, regroup_only, do_classic_opti, do_prepass_scalar_scheduling, do_postpass_scalar_scheduling, do_modulo_scheduling, memvr_profiled); //db

    return string(s);
}

string Configuration::get_mem_dir() const
{
    char s[100];
    sprintf(s,"%u%u%u_%u%u%u_%u%u%u",L1D_size, L1D_block, L1D_assoc, L1I_size, L1I_block, L1I_assoc, L2U_size, L2U_block, L2U_assoc);

    return string(s);
}

//G
void Simulation::add_simulation(const Simulation& other)
{
  //assert(config == other.config); // same configuration
  if(energy_v.size() == 0 &&  area_v.size() == 0 && exec_time_v.size() == 0){ // single to multi-valued transform
    energy_v.push_back(energy);
    area_v.push_back(area);
    exec_time_v.push_back(exec_time);
  }
  if(other.energy_v.size()>0 &&  other.area_v.size()>0 && exec_time_v.size()>0) { // multiple valued simulation
    energy_v.insert(energy_v.end(), other.energy_v.begin(), other.energy_v.end());
    area_v.insert(area_v.end(), other.area_v.begin(), other.area_v.end());
    exec_time_v.insert(exec_time_v.end(), other.exec_time_v.begin(), other.exec_time_v.end());
  } else { // single valued simulation
    energy_v.push_back(other.energy);
    area_v.push_back(other.area);
    exec_time_v.push_back(other.exec_time);
  }
  // update mean values
  vector<double>::iterator it;
  double sum = 0;
  for(it = energy_v.begin(); it != energy_v.end(); it++)
	sum += *it;
  //cout<<"\n ENERGY: "<< energy <<" sum/size: "<< sum << "/" << energy_v.size() << endl;
  energy = sum / energy_v.size();
  sum = 0;
  for(it = area_v.begin(); it != area_v.end(); it++)
	sum += *it;
  area = sum / area_v.size();
  sum = 0;
  for(it = exec_time_v.begin(); it != exec_time_v.end(); it++)
	sum += *it;
  exec_time = sum / exec_time_v.size();
}


// File access utilities - mainly to improve readability
void go_until(const string& dest,ifstream& ifs)
{
    string word;
    while ( (ifs>>word) && (word.find(dest)==string::npos));
}

string skip_to(ifstream& ifs,int n)
{
    string word;
    for (int i=0;i<n;i++) ifs>>word;
    return word;
}

string skip_to(ifstream& ifs,const string& target)
{
    string word;
    while ( (ifs>>word) && (word.find(target)==string::npos));
    return word;
}

void wait_key()
{
    cout << "\n\n Press RETURN to continue..." << endl;
    getchar();
    getchar();
}
int count_word(const string& w, ifstream& ifs)
{
    string word;
    int n = 0;
    while ( ifs>>word ) if (word==w) n++;
    return n;
}

bool file_exists(const string& filename)
{
    FILE *fp;
    fp=fopen(filename.c_str(),"r");

    if (fp!=NULL)
    {
	fclose(fp);
	return true;
    }

    return false;
}

// redefinition of some commonly used C-style functions
extern "C" int atoi(const char*);
int atoi(const string& s)
{
    return atoi(s.c_str());
}

extern "C" long long atoll(const char*);

long long atoll(const string& s)
{
    return atoll(s.c_str());
}

extern "C" double atof(const char*);
double atof(const string& s)
{
    return atof(s.c_str());
}

double max(const double& a,const double& b)
{
    if (a>b) return a;
    return b;
}

string noyes(int x)
{
    if (x==1) return string("no");
    if (x==2) return string("yes");

    return string("NOT_VALID noyes");
}

void write_to_log(int pid,const string& logfile,const string& message)
{
    time_t now = time(NULL);
    string uora = string(asctime(localtime(&now)));
    uora[24]=' '; // to avoid newline after date
    string cmd = "echo \""+uora+" [P"+to_string(pid)+"]: "+message+"\" >> "+logfile;

    system(cmd.c_str());
}

int get_mpi_rank()
{
#ifdef EPIC_MPI
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
    return myrank;
#else
    return 0;
#endif
}

int get_mpi_size()
{
#ifdef EPIC_MPI
    int mysize;
    MPI_Comm_size(MPI_COMM_WORLD,&mysize);
    return mysize;
#else
    return 1;
#endif
}



