
#include "TSystem.h"
#include "superPlot.C"

//======================================================================

void loadMultiAliii(FILE *fp,
		    map<string, vector<string> >& m_table)
{
  string theline;

  // expect the first line to define the var names as column headers
  vector<string> v_vars;

  if (!getLine(fp,theline,"alias")) {
    cerr << "ERROR: include table file empty" << endl;
    return;
  } else {
    Tokenize(theline, v_vars,"\t");
    if (!v_vars.size()) {
      cerr << "ERROR: first line of include table ";
      cerr << "must contain var names as column headers" << endl;
      return;
    }
    vector<string> empty;
    cout << "Loading values for variables ";
    for (size_t i=0; i<v_vars.size(); i++) {
      pair<map<string,vector<string> >::iterator, bool> returnval;
      returnval = m_table.insert(pair<string,vector<string> >(v_vars[i],empty));
      if (!returnval.second) {
	cout << "\n\n!! Cannot multiply define two variables of the same name in alias table!";
	cout << endl;
	exit(-1);
      }
      cout << v_vars[i] << " ";
    }
    cout << endl;
  }
  while (getLine(fp,theline,"alias")) {
    if (!theline.size())   continue;
    if (theline[0] == '#') continue; // comments are welcome

    vector<string> v_tokens;
    Tokenize(theline, v_tokens,"\t");
    if (v_tokens.size() != v_vars.size()) {
      cerr << "Rejecting line in vartable " << theline << endl;
    } else {
      for (size_t i=0; i<v_vars.size(); i++)
	m_table[v_vars[i]].push_back(v_tokens[i]);
    }
  }  
}                                                      // loadMultiAliii

//======================================================================

void spDriver(const string& canvasLayout="canvas.txt",
	      const string& varTable="vartable.txt",
	      bool saveplots=false)
{
  using namespace std;
  map<string, vector<string> > m_aliastable;   // multiple values per alias

  FILE *fpin = fopen(varTable.c_str(),"r");
  if (!fpin) {
    cerr<<"Error, couldn't open alias include file '"<<varTable<<"'"<<endl;
    exit(-1);
  }
  cout << "Loading include table '" << varTable << "'" << endl;
  loadMultiAliii(fpin,m_aliastable);

  map<string, vector<string> >::const_iterator it = m_aliastable.begin();
  size_t nrows=it->second.size();
  for (size_t j=0; j<nrows; j++) {
    char tmp[40]; sprintf(tmp,"/tmp/%s_%d.txt",canvasLayout.c_str(),j);
    string tmpname(tmp);
    const string catstring = "cat "+canvasLayout+">>"+tmpname;
    FILE *fpout = fopen(tmpname.c_str(),"w");
    if (!fpout) {
      cerr << "FATAL ERROR: could not open "<<tmpname<<endl;
      exit(-1);
    }
    fprintf(fpout,"[ALIAS]\n");

    for (it  = m_aliastable.begin();
	 it != m_aliastable.end();
	 it++)
      fprintf(fpout,"%s=%s\n",it->first.c_str(), it->second[j].c_str());

    fclose(fpout);
    gSystem->Exec(catstring.c_str());
    superPlot(tmpname,saveplots);
    unlink(tmpname.c_str());
  }
  fclose(fpin);
}
