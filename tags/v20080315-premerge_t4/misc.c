// common routines and functions
//


string get_base_dir() const 
{
    return string(getenv("HOME"));
}

void go_until(const string& dest,ifstream& ifs)
{
    while ( (ifs>>word) && (word!=dest)) ;
}

void wait_key()
{
    cout << "\n\n Press any key to continue..";
    getchar();
    getchar();
}
