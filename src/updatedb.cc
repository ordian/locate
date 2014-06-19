#include "../include/suffix.h"
#include "../include/crawler.h"
#include "../include/utility.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/unordered_map.hpp>

#include <tbb/task_scheduler_init.h>


namespace po = boost::program_options;
namespace fs = boost::filesystem;


std::pair<std::string, std::string>  parse(int argc, char *argv[]) {
  std::string PATH, OUTPUT;
  po::options_description desc("Allowed options");
  desc.add_options()
      ("database-root,r",
       po::value<std::string>(&PATH)->required(),
       "Index catalog");
  desc.add_options()
      ("output,o",
       po::value<std::string>(&OUTPUT)->required()->default_value("index.db"),
       "Index name");
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
            .options(desc)
            .allow_unregistered()
            .run(),
            vm);
  po::notify(vm);
  return std::make_pair(PATH, OUTPUT);
}

/**
 * Format:
 *  paths
 *  unique_names
 *  refs (name -> list of path)
 *  suffixArray
 */
void writeIndex(std::vector<std::string> const& paths,
                std::ofstream & output) {

  /* build unique names */
  typedef boost::unordered_map<std::string, std::vector<int>> map;
  map namePaths;
  int n = paths.size();
  for (int i = 0; i != n; ++i) {
    size_t found = paths[i].find_last_of(utility::getSeparator());
    namePaths[paths[i].substr(found == std::string::npos ? 0 : found + 1)].push_back(i);
  }
  std::vector<std::pair<std::string, std::vector<int>>> nameRefs(namePaths.begin(), namePaths.end());
  std::vector<std::string> names;
  std::vector<std::vector<int>> refs;
  for (auto it = std::make_move_iterator(nameRefs.begin()),
           end = std::make_move_iterator(nameRefs.end());
       it != end;
       ++it) {
    names.push_back(std::move(it->first));
    refs.push_back(std::move(it->second));
  }
  /* build suffix array */
  suffix::Array array = suffix::buildArray(names);
  
  /* write to file stream */
  utility::write(output, paths);
  utility::write(output, names);
  utility::write(output, refs);
  utility::write(output, array);
}

int main(int argc, char *argv[]) try {
  std::string PATH, OUTPUT;
  std::tie(PATH, OUTPUT) = parse(argc, argv);
  
  fs::path root(PATH);
  if (!fs::exists(root) || !fs::is_directory(root)) {
    std::cerr << "Directory " << PATH << " doesn't exist!" << std::endl;
    return -2;
  }

  std::ofstream output(OUTPUT, std::ios::trunc | std::ios::binary);
  if (!output) {
    std::cerr << "Can't write to " << OUTPUT << std::endl;
    return -3;
  }

  int const threadCount = boost::thread::hardware_concurrency();
  tbb::task_scheduler_init init(threadCount);
  tbb::concurrent_vector<fs::path> paths;
  
  do {
    boost::asio::io_service service;
    boost::thread_group pool;
    {
      std::unique_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(service));
      for (int i = 0; i != threadCount; ++i)
        pool.create_thread(boost::bind(&boost::asio::io_service::run, &service));
      
      Crawler(root, service, paths).crawl();
    }
    pool.join_all();
  } while (false);

  std::vector<std::string> pathStrings(paths.size());
  std::transform(paths.begin(), paths.end(), pathStrings.begin(),
                 [&](fs::path const& path) { return path.string(); });
  
  writeIndex(pathStrings, output);
  
  return 0;

} catch (std::exception const& e) {
  std::cerr << e.what() << std::endl;
  return -1;
} 
















