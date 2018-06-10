#include "src/utility.cpp"

int main(int argc, char** argv)
{
  Argument argument = parse_arguments(argc, argv);
  run(argument);
}