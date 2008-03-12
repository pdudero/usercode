#include "MyEDmodules/LjmetAnalyzer/src/myParticleRecord.h"
#include "MyEDmodules/LjmetAnalyzer/src/pdgid.h"

#include <iostream>
using namespace std;

myParticleRecord::myParticleRecord(vector<int> pdgids) {
  prec_t temp;
  temp.count = 0;

  // pre-initialize map with entries for given pdg IDs,
  // counts set to zero.
  //
  vector<int>::const_iterator i;
  for (i=pdgids.begin(); i!= pdgids.end(); i++) {
    m_particleRec_[abs(*i)] = temp;
  }
}

int myParticleRecord::found(int pdgid)
{
  return (m_particleRec_.find(abs(pdgid)) !=
	  m_particleRec_.end());
}

int myParticleRecord::count(int pdgid)
{
  if (found(pdgid))
    return (m_particleRec_[abs(pdgid)].count);
  else
    return 0;
}

int myParticleRecord::add(int pdgid)
{
  if (!found(pdgid))
    m_particleRec_[abs(pdgid)].count = 1;
  else
    m_particleRec_[abs(pdgid)].count++;

  return m_particleRec_[abs(pdgid)].count;
}

int myParticleRecord::quarkcount()
{
  int qcount = 0;
  for (int i=downq; i<=topq; i++)
    qcount += count(i);
  return qcount;
}

int myParticleRecord::leptoncount()
{
  int lcount = 0;
  for (int i=electron; i<=nu_tau; i++)
    lcount += count(i);
  return lcount;
}
  
void myParticleRecord::clearcounts() {

  // pre-initialize map with entries for given pdg IDs,
  // counts set to zero.
  //
  map<int,prec_t>::iterator i;
  for (i=m_particleRec_.begin(); i!= m_particleRec_.end(); i++) {
    i->second.count = 0;
  }
}
