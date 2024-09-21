#include <iostream>
#include <fstream>

using namespace std;
int main(int argc, char **argv){

  ofstream input_file;
  input_file.open(argv[1],
                  std::ofstream::in);

   return 0;
}