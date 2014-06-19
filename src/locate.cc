#include "../include/suffix.h"
#include "../include/utility.h"

#include <boost/program_options.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;


std::pair<std::string, std::string>  parse(int argc, char const*argv[]) {
  std::string DATABASE, PATTERN;
  po::options_description desc("Allowed options");
  desc.add_options()
      ("database,d",
       po::value<std::string>(&DATABASE)->required()->default_value("index.db"),
       "Database name");
  desc.add_options()
      ("pattern,p",
       po::value<std::string>(&PATTERN)->required(),
       "Search pattern");
  po::positional_options_description pdesc;
  pdesc.add("pattern", 1);
  
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
            .options(desc)
            .positional(pdesc)
            .run(),
            vm);
  po::notify(vm);
  return std::make_pair(DATABASE, PATTERN);
}



int main(int argc, char const* argv[]) try {
  std::string DATABASE, PATTERN;
  std::tie(DATABASE, PATTERN) = parse(argc, argv);

  if (PATTERN.find(utility::getSeparator()) != std::string::npos)
    return 0;

  std::ifstream input(DATABASE, std::ios::binary);
  if (!input) {
    std::cerr << "Can't open database file " << DATABASE << std::endl;
    return 1;
  }

  std::vector<std::string> paths;
  std::vector<std::string> names;
  std::vector<std::vector<int> > refs;
  suffix::Array array;                                                         

  utility::read(input, paths);
  utility::read(input, names);
  utility::read(input, refs);
  utility::read(input, array);
  input.close();

  std::set<std::string> result;
  for (auto &p : suffix::search(array, names, PATTERN)) {
    for (int i : refs[p.first]) {
      result.insert(paths[i]);
    }
  }

  
  boost::copy(
      result | boost::adaptors::filtered([](std::string const& s)
                                         { return fs::exists(fs::path(s));} ),
      std::ostream_iterator<std::string>(std::cout, "\n"));
  
} catch (std::exception const& e) {
  std::cerr << e.what() << std::endl;
  return 1;
}













