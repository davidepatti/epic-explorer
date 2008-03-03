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

#ifndef PARAMETER_H
#define PARAMETER_H

#include <iostream>
#include <vector>

using namespace std;

class Parameter {
public: 
  Parameter();

  Parameter(int * possible_values, int default_val);
  Parameter(vector<int> possible_values,int default_val);

	
  ~Parameter();

  int get_val() const;
  int get_size() const;
  int get_default() const;
  void set_val(int new_value);

  void set_to_default();
  void set_to_first();
  void set_to_last();
  void set_random();

  int get_pos(int value);
  bool increase();
  bool decrease();

  vector<int> get_values(); // mau
  void set_values(vector<int> values,int default_val);
  void set_values(Parameter parameter);

  int get_first();
  int get_last();

private:

  vector<int> values; // vector of possible values 
  int current;  // index of current value in the array of values
  int default_value;

};

#endif
