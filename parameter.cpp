/***************************************************************************
                          parameter.cpp  -  description
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

#include "parameter.h"
#include "common.h"
#include <cstdlib>

using namespace std;

Parameter::Parameter(){
    set_label("NO_LABEL");
    current = NOT_VALID;
}


Parameter::Parameter(const string& l,vector<int> possible_values,int default_val)
{
    set_label(l);
    set_values(possible_values,default_val);
}


//DEPRECATED constructor ! ( only for compatibility...)
Parameter::Parameter(int * possible_values,int default_val) {

    int index=0;
    while (possible_values[index]!=NOT_VALID) 
    {
	values.push_back(possible_values[index]);
	index++;
    }
    default_value = default_val;
    set_val(default_value);
}

void Parameter::set_label(const string& l)
{
    label = l;
}

string Parameter::get_label() const
{
    return label;
}


Parameter::~Parameter(){

}

void Parameter::set_to_default()
{
    set_val(default_value);
}


int Parameter::get_val() const 
{
    if (current!=NOT_VALID)
	return values[current];
    else
	return NOT_VALID;
}

int Parameter::get_default() const
{
    return default_value;
}

int Parameter::get_size() const 
{
    return values.size();
}

void Parameter::set_val(int new_value)
{
    current = NOT_VALID; 

    for (int i =0;i<values.size();i++)
	if (values[i]==new_value) current = i;

    if (current==NOT_VALID)
    {
	string logfile = string(getenv(BASE_DIR))+string(EE_LOG_PATH);
	int myid = get_mpi_rank();
	write_to_log(myid,logfile," ERROR: not valid valuet for parameter '"+label+" (value=" + to_string(new_value) + "). Check subspace file.");
	exit(EXIT_FAILURE);
    }
}

void Parameter::set_random()
{
    float r =  (float)rand()/(RAND_MAX);

    int random_index = (int)(r*values.size());

    set_val(values[random_index]);
}

bool Parameter::increase()
{
    if (++current==values.size())
    {
	current--;
	return false;
    }
    return true;

}
bool Parameter::decrease()
{
    if (current>0) return --current;
    return false;
}

void Parameter::set_to_first()
{
    current = 0;
}

void Parameter::set_to_last()
{
    current = values.size()-1;
}

vector<int> Parameter::get_values() // mau
{
  return values;
}

void Parameter::set_values(vector<int> values,int default_val)
{
    this->values = values;
    default_value = default_val;
#ifdef DEBUG
    cout << "\n Parameter::set_values() ";
    for(int i=0;i<values.size();i++) cout << "\n values " << this->values[i];
#endif
    set_val(default_value);
}

void Parameter::set_values(Parameter parameter)
{
    set_values(parameter.get_values(),parameter.get_default());
}

int Parameter::get_first()
{
  return values[0];
}

int Parameter::get_last()
{
  return values[values.size()-1];
}

int Parameter::get_pos(int value)
{
	for(int i=0; i<values.size(); ++i)
		if (values[i] == value) return (i+1);

	return NOT_VALID;
}

// PARAMSPACE
// regardless a,b being allowed parameter values, this function returns values falling into interval
// notes: assuming a<=b , not the best implementation
// 
vector<int> Parameter::get_interval(int a, int b)
{
    vector<int> interval;
    for(int i=0; i<values.size(); ++i)
	if (values[i]>=a && values[i]<=b) interval.push_back(values[i]);

    return interval;
}
