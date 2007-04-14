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
    sprintf(s,"%% %u %u %u %u / %u %u %u %u %u / %u %u %u / %u %u %u / %u %u %u ",
	    integer_units, float_units,branch_units,memory_units, gpr_static_size, fpr_static_size, pr_static_size, cr_static_size, btr_static_size, L1D_size, L1D_block, L1D_assoc, L1I_size, L1I_block, L1I_assoc, L2U_size, L2U_block, L2U_assoc);

    return string(s);

}


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
    cout << "\n\n Press any key to continue.." << endl;
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

template<typename T> string to_string(const T& t){
     stringstream s;
     s << t;
     return s.str();
}

extern "C" long long atoll(const char*);
extern "C" double atof(const char*);

long long atoll(const string& s)
{
    return atoll(s.c_str());
}

double atof(const string& s)
{
    return atof(s.c_str());
}

double max(const double& a,const double& b)
{
    if (a>b) return a;
    return b;
}
