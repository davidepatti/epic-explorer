// common routines and functions
//

#include "common.h"

bool Configuration::is_feasible()
{
  return ( (L1D_size + L1I_size <= L2U_size) &&
	   (L1D_size >= L1D_block * L1D_assoc) &&
	   (L1I_size >= L1I_block * L1I_assoc) &&
	   (L2U_size >= L2U_block * L2U_assoc) );
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

    return false;
}

string Configuration::to_string() const
{
    char s[100];
    sprintf(s,"%% %u / %u %u %u %u / %u %u %u %u %u / %u %u %u / %u %u %u / %u %u %u ",
	    num_clusters,integer_units, float_units,branch_units,memory_units, gpr_static_size, fpr_static_size, pr_static_size, cr_static_size, btr_static_size, L1D_size, L1D_block, L1D_assoc, L1I_size, L1I_block, L1I_assoc, L2U_size, L2U_block, L2U_assoc);

    return string(s);

}

string Configuration::get_processor_string() const
{
    char s[100];
    sprintf(s,"%u_%u%u%u%u_%u%u%u%u%u",
	    num_clusters,integer_units, float_units,branch_units,memory_units, gpr_static_size, fpr_static_size, pr_static_size, cr_static_size, btr_static_size);

    return string(s);
}

string Configuration::get_mem_hierarchy_string() const
{
    char s[100];
    sprintf(s,"%u%u%u_%u%u%u_%u%u%u",L1D_size, L1D_block, L1D_assoc, L1I_size, L1I_block, L1I_assoc, L2U_size, L2U_block, L2U_assoc);

    return string(s);
}

//G
void Simulation::add_simulation(const Simulation& other)
{
  assert(configuration == other.configuration); // same configuration
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
