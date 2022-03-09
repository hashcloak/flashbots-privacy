/*
 * ProcessorBase.h
 *
 */

#ifndef PROCESSOR_PROCESSORBASE_H_
#define PROCESSOR_PROCESSORBASE_H_

#include <stack>
#include <string>
#include <fstream>
using namespace std;

#include "Tools/ExecutionStats.h"
#include "Tools/SwitchableOutput.h"
#include "OnlineOptions.h"

class ProcessorBase
{
  // Stack
  stack<long> stacki;

  ifstream input_file;
  string input_filename;
  size_t input_counter;

protected:
  // Optional argument to tape
  int arg;

  string get_parameterized_filename(int my_num, int thread_num,
      const string& prefix);

public:
  ExecutionStats stats;

  ofstream stdout_redirect_file;

  ProcessorBase();

  void pushi(long x) { stacki.push(x); }
  void popi(long& x) { x = stacki.top(); stacki.pop(); }

  int get_arg() const
    {
      return arg;
    }

  void set_arg(int new_arg)
    {
      arg=new_arg;
    }

  void open_input_file(const string& name);
  void open_input_file(int my_num, int thread_num, const string& prefix="");

  template<class T>
  T get_input(bool interactive, const int* params);
  template<class T>
  T get_input(istream& is, const string& input_filename, const int* params);

  void setup_redirection(int my_nu, int thread_num, OnlineOptions& opts,
      SwitchableOutput& out);
};

#endif /* PROCESSOR_PROCESSORBASE_H_ */
