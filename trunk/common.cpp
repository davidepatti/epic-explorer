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
    cout << "\n\n Press any key to continue..";
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
template<typename T> std::string to_string(const T& t){
     std::stringstream s;
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
