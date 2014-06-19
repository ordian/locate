#ifndef CRAWLER_H_
#define CRAWLER_H_

#include <boost/asio/io_service.hpp>
#include <boost/filesystem.hpp>

#include <tbb/concurrent_vector.h>

namespace fs = boost::filesystem;

struct Crawler {

  Crawler(fs::path const& root,
          boost::asio::io_service & service,
          tbb::concurrent_vector<fs::path> & names)
      : m_root(root)
      , m_service(service)
      , m_names(names)
  {}
  
  void crawl() {
    doCrawl(m_root);
  }
  
 private:
  Crawler(Crawler const&) = delete;
  Crawler & operator=(Crawler const&) = delete;

  void doCrawl(fs::path const& root) {
    m_service.post([=]() {
        try {
          for (fs::directory_iterator dir = fs::directory_iterator(root), end;
               dir != end;
               ++dir) {
            if (!fs::is_symlink(dir->status())) {
              if (fs::is_directory(dir->status()))
                doCrawl(dir->path());
              m_names.push_back(dir->path());
            }
          }
        } catch (fs::filesystem_error const &e) {
          std::cerr << "Error while crawling: " << e.what() << std::endl;
        }
      });
  }
  
  fs::path m_root;
  boost::asio::io_service & m_service;
  tbb::concurrent_vector<fs::path> & m_names;
};

#endif // CRAWLER_H_








