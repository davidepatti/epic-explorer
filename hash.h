/***********************************************************************
 * hash.h
 * Simple hash table
 * Maurizio Palesi [mpalesi@diit.unict.it]
 ***********************************************************************
 */

#ifndef __HASH_H__
#define __HASH_H__

#include <cassert>
#include <vector>

using namespace std;

template <class T>
class Hash
{
public:

  vector<vector<T> > vhash;

  //----------------------------------------------------------------------

  Hash(int _size)
  {
    vhash.resize(_size);
  }

  //----------------------------------------------------------------------

  virtual bool equal(T &t1, T &t2) = 0;

  //----------------------------------------------------------------------

  virtual unsigned int T2Index(T &t) = 0;

  //----------------------------------------------------------------------

  void addT(T &t)
  {
    unsigned int index = T2Index(t);
    assert( index < vhash.size() );
    vhash[index].push_back(t);
  }

  //----------------------------------------------------------------------

  T * searchT(T &t)
  {
    unsigned int index = T2Index(t);
    assert( index < vhash.size() );
    
    bool found = false;
    unsigned int s = vhash[index].size();
    unsigned int i = 0;
    while (i<s && !found)
      {
	if (equal(vhash[index][i], t))
	  found = true;
	else
	  i++;
      }
    
    return found ? &(vhash[index][i]) : NULL;
  }

  //----------------------------------------------------------------------

  unsigned int avgLen()
  {
    unsigned int len = 0;

    for (int i=0; i<vhash.size(); i++)
      len += vhash[i].size();

    return (len/vhash.size());
  }
  
  //----------------------------------------------------------------------
  
  unsigned int maxLen()
  {
    unsigned int max = 0;

    for (int i=0; i<vhash.size(); i++)
      if (max < vhash[i].size())
	max = vhash[i].size();

    return max;
  }

  //----------------------------------------------------------------------
};

#endif
