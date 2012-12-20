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

#ifndef PARAMSPACE_H
#define PARAMSPACE_H

using namespace std;


typedef struct 
{
    int a,b;
} Edges;


class Region {
public: 
  Region();
  ~Region();

  double innovation_score;
  bool valid;

  vector<Region> split_region();



  // the index is the one computed by Parameter::get_index()
 Edges edges[N_PARAMS];
private:

};

#endif
