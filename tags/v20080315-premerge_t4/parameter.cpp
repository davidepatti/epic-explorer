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

using namespace std;

Parameter::Parameter(){
    
}


Parameter::Parameter(vector<int> possible_values,int default_val)
{
    set_values(possible_values,default_val);
}


//DEPRECATED constructor ! ( only for compatibility...)
Parameter::Parameter(int * possible_values,int default_val) {

    int index=0;
    while (possible_values[index]!=-1) 
    {
	values.push_back(possible_values[index]);
	index++;
    }

    default_value = default_val;

    set_val(default_value);
}


Parameter::~Parameter(){

}

void Parameter::set_to_default()
{
    set_val(default_value);
}


int Parameter::get_val() const 
{
    return values[current];
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
    current = -1;

    for (int i =0;i<values.size();i++)
    {
	if (values[i]==new_value) current = i;
    }

    if (current==-1)
    {
	cout << "\n FATAL ERROR: Invalid parameter value set !!( new_value = " << new_value << ")";
	cout << "\n List of available values : " << endl;
	for (int i =0;i<values.size();i++) cout << values[i] << ",";
	cout << "\n Please check your subspace file in trimaran-workspace/epic-explorer/SUBSPACES";
	exit(-1);
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

	return (-1);
}
